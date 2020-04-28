#include <bridge/ble/ble_pairing.h>
#include <string.h>
#include <stdlib.h>
#include <bridge/bridge_main/task.h>
#include <bridge/bridge_main/log.h>
#include <pthread.h>
#include <unistd.h>
#include <bridge/lock/messages/PairingStep1.h>
#include <bridge/lock/messages/PairingStep2.h>
#include <bridge/lock/messages/PairingStep3.h>
#include <bridge/lock/messages/PairingStep4.h>
#include <bridge/lock/messages/PairingCommit.h>
#include <bridge/lock/connection/pairing_connection.h>

char* adapter_name;
void* adapter;
char pairing_str[] = "5c3a659e-897e-45e1-b016-007107c96df6";


int discoverLock(void *arg);

//步骤分别为
// 1. BLE_PAIRING_BEGIN 监听变化
// 2. BLE_PAIRING_STEP1 写第一步
// 3. BLE_PAIRING_STEP2 等待锁返回第二步
// 4. BLE_PAIRING_STEP3 写第三步
// 5. BLE_PAIRING_STEP4 等待第四步
// 6. BLE_PAIRING_DONE 写commit, 也就done.
int register_pairing_notfication(void *arg);
int write_pairing_step1(void *arg);
int waiting_pairing_step2(void *arg);
int write_pairing_step3(void *arg);
int waiting_pairing_step4(void *arg);
int write_pairing_commit(void *arg);
void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);
int handle_step2_message(const uint8_t* data, int data_length,void* user_data);
int save_message_data(const uint8_t* data, int data_length, void* user_data);
int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len);
fsm_table_t pairing_fsm_table[6] = {
  {BLE_PAIRING_BEGIN,   register_pairing_notfication,  BLE_PAIRING_STEP1},
  {BLE_PAIRING_STEP1,   write_pairing_step1,           BLE_PAIRING_STEP2},
  {BLE_PAIRING_STEP2,   waiting_pairing_step2,         BLE_PAIRING_STEP3},
  // {BLE_PAIRING_STEP3,   write_pairing_step3,           BLE_PAIRING_STEP4},
  // {BLE_PAIRING_STEP4,   waiting_pairing_step4,         BLE_PAIRING_COMMIT},
  // {BLE_PAIRING_COMMIT,  write_pairing_commit,          BLE_PAIRING_DONE},
};


typedef struct ParingConnection {
	gatt_connection_t* gatt_connection;
  pthread_mutex_t wait_mutex;
  igm_lock_t lock;
	enum BLE_PAIRING_STATE pairing_step;
	size_t step_max_size;
	size_t  step_cur_size;
	size_t n_size_byte;
	uint8_t *step_data;
  uuid_t pairing_uuid;
}pairing_connection_t;


int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len)
{
  int BLE_ATT_MAX_BYTES = 20;
	int ret = GATTLIB_SUCCESS;
	uint8_t *tmp_bytes[BLE_ATT_MAX_BYTES];
	size_t size_left;
	int i;
	for (i = 0; i < buffer_len; i += BLE_ATT_MAX_BYTES)
	{
		size_left = buffer_len-i>=BLE_ATT_MAX_BYTES?BLE_ATT_MAX_BYTES:buffer_len-i;
		memset(tmp_bytes, 0, BLE_ATT_MAX_BYTES);
		memcpy(tmp_bytes, buffer+i, size_left);
		ret = gattlib_write_char_by_uuid(
			gatt_connection, uuid, tmp_bytes, size_left);
		if (ret != GATTLIB_SUCCESS) {
      serverLog(LL_ERROR, "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid failed in writint ");
			return ret;
		}
    serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid success in writing");
	}
	return ret;
}

int save_message_data(const uint8_t* data, int data_length, void* user_data)
{
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  if (pairing_connection->step_max_size == 0)
  {
    if (data_length<3)
    {
      serverLog(LL_ERROR, "data_length < 3, can't get len");
      return 1;
    }
    else
    {
      if (data[2] == 0xff)
      {
        pairing_connection->n_size_byte = 3;
        pairing_connection->step_max_size = data[0] * (0xfe) + data[1] + 3;
        serverLog(LL_NOTICE, 
                      "2 bytes lenth %d", pairing_connection->step_max_size);
      }
      else
      {
        pairing_connection->n_size_byte = 1;
        pairing_connection->step_max_size = data[0] + 1;
        serverLog(LL_NOTICE, 
                      "1 bytes lenth %d", pairing_connection->step_max_size);
      }
      pairing_connection->step_cur_size = 0;
      pairing_connection->step_data = (uint8_t *)malloc(
                                            pairing_connection->step_max_size);
      if (!(pairing_connection->step_data))
      {
        serverLog(LL_ERROR, "save_message_data malloc err");
        return 1;
      }
    }
  }

  int size_left = 
        pairing_connection->step_max_size - pairing_connection->step_cur_size;

  if (size_left < data_length)
	{
    serverLog(LL_NOTICE, "size_left < data_length");
    pairing_connection->step_max_size += 85;
		uint8_t *old_data = pairing_connection->step_data;
		pairing_connection->step_data = (uint8_t *)malloc(
                                              pairing_connection->step_max_size);
		if (!pairing_connection->step_data)
		{
			serverLog(LL_ERROR, "save_message_data malloc err");
			return 4;
		}
		memcpy(
      pairing_connection->step_data, old_data, pairing_connection->step_cur_size);
		free(old_data);
	}

  // 空间足够, 直接放下空间里面
	for (int j = 0; j < data_length; ) {
		// printf("%02x ", data[i]);
		pairing_connection->step_data[pairing_connection->step_cur_size++] = data[j++];
	}
}

int waiting_pairing_step2(void *arg)
{
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  while (1)
  {
    serverLog(LL_NOTICE, "waiting_pairing_step2 waiting");
    pthread_mutex_lock(&pairing_connection->wait_mutex);
    if (pairing_connection->pairing_step == BLE_PAIRING_STEP2) break;
    pthread_mutex_unlock(&pairing_connection->wait_mutex);
    usleep(5000000);
  }
  return 0;
}

int handle_step2_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_step2_message");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  save_message_data(data, data_length, user_data);
  if (pairing_connection->step_max_size == pairing_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step2_message RECV step2 data finished");
    pthread_mutex_lock(&pairing_connection->wait_mutex);
    pairing_connection->pairing_step = BLE_PAIRING_STEP2;
    pthread_mutex_unlock(&pairing_connection->wait_mutex);

  }
}

void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
    serverLog(LL_NOTICE, "message_handler");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  switch(pairing_connection->pairing_step)
  {
    case BLE_PAIRING_STEP1:
    {
      handle_step2_message(data, data_length, user_data);
      break;
    }
  }
}

int write_pairing_step1(void *arg)
{
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  size_t step1Bytes_len, payload_len;
  uint8_t *step1Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  ret = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (!ret)
  {
    serverLog(LL_ERROR, "beginConnection fail");
    return 1;
  }
  serverLog(LL_NOTICE, "beginConnection success");

  step1Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			                                                            &step1Bytes);
  if (step1Bytes == NULL)
  {
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native fail");
    return 1;
  }
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native success");

  if (!build_msg_payload(&payloadBytes, payload_len, step1Bytes, step1Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto STEP1_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "success in build_msg_payload");

  ret = write_char_by_uuid_multi_atts(
    pairing_connection->gatt_connection, &pairing_connection->pairing_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto STEP1_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success in writing packages");
	// std::lock_guard<std::mutex> lock(pairing_step_mutex);
  pthread_mutex_lock(&pairing_connection->wait_mutex);
	pairing_connection->pairing_step = BLE_PAIRING_STEP1;
  pthread_mutex_unlock(&pairing_connection->wait_mutex);
  return 0;
STEP1_ERROR_EXIT:
  if (step1Bytes)
  {
    free(step1Bytes);
  }
  if (payloadBytes)
  {
    free(payloadBytes);
  }
}

int register_pairing_notfication(void *arg)
{
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  ble_pairing_param_t *param = (ble_pairing_param_t *)ble_data->ble_param;
  ble_data->ble_connection = calloc(sizeof(pairing_connection_t), 1);
  pairing_connection_t *pairing_connection = 
                            (pairing_connection_t *)ble_data->ble_connection;
  memset(pairing_connection, 0, sizeof(pairing_connection_t));
  pthread_mutex_init(&pairing_connection->wait_mutex, PTHREAD_MUTEX_TIMED_NP);

  ret = gattlib_adapter_open(ble_data->adapter_name, &(ble_data->adapter));
  if (ret) {
		serverLog(LL_ERROR, 
      "ERROR: register_pairing_notfication Failed to open adapter.");
		return 1;
	}
  pairing_connection->gatt_connection = gattlib_connect(
    NULL, param->lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (pairing_connection->gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		goto ERROR_EXIT;
	} 
  else {
    serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );
	}

  if (
    gattlib_string_to_uuid(
      pairing_str, strlen(pairing_str), &(pairing_connection->pairing_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  else
  {
    serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );
  }

  gattlib_register_notification(
    pairing_connection->gatt_connection, message_handler, arg);
  ret = gattlib_notification_start(
      pairing_connection->gatt_connection, &(pairing_connection->pairing_uuid));
  if (ret) {
    serverLog(LL_ERROR, "Fail to start notification.");
		goto ERROR_EXIT;
	}
	else
	{
    serverLog(LL_NOTICE, "success to start notification" );
	}

  pairing_connection->pairing_step = BLE_PAIRING_STEP1;
  return 0;

ERROR_EXIT:
  serverLog(LL_ERROR, "register_pairing_notfication ERROR EXIT.");
  if (pairing_connection->gatt_connection)
  {
    gattlib_notification_stop(
      pairing_connection->gatt_connection, &(pairing_connection->pairing_uuid));
    gattlib_disconnect(pairing_connection->gatt_connection);
  }
  if (ble_data->ble_connection)
  {
    free(ble_data->ble_connection);
  }
}

// -------------------------------------

int bleInitPairingParam(ble_pairing_param_t *pairing_param)
{
  memset(pairing_param, 0, sizeof(ble_pairing_param_t));
  return 0;
}

int bleReleaseParingParam(ble_pairing_param_t *pairing_param)
{
  if (pairing_param->lock)
  {
    free(pairing_param->lock);
    pairing_param->lock = NULL;
  }
  return 0;

}
int bleSetPairingParam(ble_pairing_param_t *pairing_param, igm_lock_t *lock)
{
  bleReleaseParingParam(pairing_param);
  pairing_param->lock = calloc(sizeof(igm_lock_t), 1);
  memset(pairing_param->lock, 0, sizeof(igm_lock_t));
  return 0;
}

// --------------------------------------------------------------------

int bleInitPairingResult(ble_pairing_result_t *result)
{
  memset(result, 0, sizeof(ble_pairing_result_t));
  return 0;
}

int bleReleaseResultAdminKey(ble_pairing_result_t *result)
{
  if (result->admin_key)
  {
    free(result->admin_key);
    result->admin_key = NULL;
    result->admin_key_len = 0;
  }
  return 0;
}

int bleReleaseResultPassword(ble_pairing_result_t *result)
{
  if (result->password)
  {
    free(result->password);
    result->password = NULL;
    result->password_size = 0;
  }
  return 0;
}

int bleReleasePairingResult(ble_pairing_result_t **pp_result)
{
  ble_pairing_result_t *result = *pp_result;
  bleReleaseResultPassword(result);
  bleReleaseResultAdminKey(result);
  free(result);
  *pp_result = NULL;
  return 0;
}

int bleSetPairingResultAdminKey(ble_pairing_result_t *result, 
  uint8_t *admin_key, int admin_key_len)
{
  bleReleaseResultAdminKey(result);
  result->admin_key_len = admin_key_len;
  result->admin_key = calloc(admin_key_len,1);
  memcpy(result->admin_key, admin_key, admin_key_len);
  return 0;
}

int bleSetPairingResultPassword(ble_pairing_result_t *result, 
  uint8_t *password, int password_size)
{
  bleReleaseResultPassword(result);
  result->password_size = password_size;
  result->password = calloc(password_size,1);
  memcpy(result->password, password, password_size);
  return 0;
}

void bleGetPairingResultAdminKey(ble_pairing_result_t *result, 
  uint8_t *admin_key, int *p_admin_key_len)
{
  *p_admin_key_len = result->admin_key_len;
  memcpy(admin_key, result->admin_key, result->admin_key_len);
}

void bleGetPairingResultPassword(ble_pairing_result_t *result, 
  uint8_t *password, int *p_password_size)
{
  *p_password_size = result->password_size;
  memcpy(password, result->password, result->password_size);
}

// --------------------------------------------------------------------

