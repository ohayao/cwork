#include <bridge/ble/ble_admin.h>
#include <bridge/gattlib/gattlib.h>
#include <bridge/bridge_main/task.h>
#include <time.h> 
#include <bridge/lock/connection/admin_connection.h>
#include <bridge/ble/ble_operation.h>
#include <bridge/lock/messages/AdminUnlockRequest.h>
#include <bridge/lock/messages/AdminUnlockResponse.h>

#include <bridge/lock/messages/UnpairRequest.h>
#include <bridge/lock/messages/UnpairResponse.h>
#include <bridge/lock/messages/AdminLockRequest.h>
#include <bridge/lock/messages/AdminLockResponse.h>

static char admin_str[] = "5c3a659f-897e-45e1-b016-007107c96df6";
enum {
  ADMIN_SM_TABLE_LEN = 4
};

enum {
  ADMIN_UNPAIR_SM_TABLE_LEN = 6
};

enum {
  ADMIN_UNLOCK_SM_TABLE_LEN = 6
};

enum {
  ADMIN_LOCK_SM_TABLE_LEN = 6
};

typedef struct AdminConnection {
	gatt_connection_t* gatt_connection;
  igm_lock_t *lock;
	enum BLE_ADMIN_STATE admin_step;
	size_t step_max_size;
	size_t  step_cur_size;
	size_t n_size_byte;
	uint8_t *step_data;
  uuid_t admin_uuid;
  int has_admin_result;
  ble_admin_result_t *admin_result;
}admin_connection_t;

void initAdminConnection(admin_connection_t *admin_connection)
{
  memset(admin_connection, 0, sizeof(admin_connection_t));
}

int releaseAdminConnection(admin_connection_t **pp_admin_connection)
{
  admin_connection_t *admin_connection = *pp_admin_connection;
  int ret = gattlib_notification_stop(
        admin_connection->gatt_connection, &admin_connection->admin_uuid);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "clearAdminConnectionGattConenction gattlib_notification_stop error");
    return ret;
  }
  ret = gattlib_disconnect(admin_connection->gatt_connection);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, " gattlib_disconnect error");
    return ret;
  }
  releaseLock(&admin_connection->lock);
  free(admin_connection->admin_result);
  free(*pp_admin_connection);
  *pp_admin_connection = NULL;
  return 0;
}

static int clearAdminConnection(admin_connection_t **pp_admin_connection);
static int clearAdminConnectionStepData(admin_connection_t *admin_connection);

static int clearAdminConnectionStepData(admin_connection_t *admin_connection)
{
  if (admin_connection->step_data && admin_connection->step_max_size)
  {
    admin_connection->step_max_size = 0;
    admin_connection->step_cur_size = 0;
    free(admin_connection->step_data);
    admin_connection->step_data = NULL;
  }
  return 0;
}

// admin
static int register_admin_notfication(void *arg);
static void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);
static int waiting_admin_step1(void *arg);
static int handle_step1_message(const uint8_t* data, int data_length,void* user_data);
static int save_message_data(const uint8_t* data, int data_length, void* user_data);
static int write_admin_step2(void *arg);
static int waiting_admin_step3(void *arg);
static int handle_step3_message(const uint8_t* data, int data_length,void* user_data);


fsm_table_t admin_fsm_table[ADMIN_SM_TABLE_LEN] = {
  {BLE_ADMIN_BEGIN,       register_admin_notfication,  BLE_ADMIN_STEP1},
  {BLE_ADMIN_STEP1,       waiting_admin_step1,         BLE_ADMIN_STEP2},
  {BLE_ADMIN_STEP2,       write_admin_step2,         BLE_ADMIN_ESTABLISHED},
  {BLE_ADMIN_ESTABLISHED, waiting_admin_step3,     BLE_ADMIN_DONE},
};

// unpair
static int write_unpair_request(void *arg);
static int handle_unpair_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_unpair_result(void *arg);

fsm_table_t admin_unpair_fsm_table[ADMIN_UNPAIR_SM_TABLE_LEN] = {
  {BLE_ADMIN_BEGIN,         register_admin_notfication,   BLE_ADMIN_STEP1},
  {BLE_ADMIN_STEP1,         waiting_admin_step1,          BLE_ADMIN_STEP2},
  {BLE_ADMIN_STEP2,         write_admin_step2,            BLE_ADMIN_ESTABLISHED},
  {BLE_ADMIN_ESTABLISHED,   waiting_admin_step3,          BLE_ADMIN_UNPAIR_REQUEST},
  {BLE_ADMIN_UNPAIR_REQUEST,   write_unpair_request,         BLE_ADMIN_UNPAIR_RESULT},
  {BLE_ADMIN_UNPAIR_RESULT, waiting_unpair_result,        BLE_ADMIN_UNPAIR_DONE},
};

// unlock --------------------------------
static int write_unlock_request(void *arg);
static int handle_unlock_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_unlock_result(void *arg);

fsm_table_t admin_unlock_fsm_table[ADMIN_UNLOCK_SM_TABLE_LEN] = {
  {BLE_ADMIN_BEGIN,         register_admin_notfication,   BLE_ADMIN_STEP1},
  {BLE_ADMIN_STEP1,         waiting_admin_step1,          BLE_ADMIN_STEP2},
  {BLE_ADMIN_STEP2,         write_admin_step2,            BLE_ADMIN_ESTABLISHED},
  {BLE_ADMIN_ESTABLISHED,   waiting_admin_step3,          BLE_ADMIN_UNLOCK_REQUEST},
  {BLE_ADMIN_UNLOCK_REQUEST,   write_unlock_request,         BLE_ADMIN_UNLOCK_RESULT},
  {BLE_ADMIN_UNLOCK_RESULT,  waiting_unlock_result,        BLE_ADMIN_UNLOCK_DONE},
};

// lock --------------------------------
static int write_lock_request(void *arg);
static int handle_lock_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_lock_result(void *arg);

fsm_table_t admin_lock_fsm_table[ADMIN_LOCK_SM_TABLE_LEN] = {
  {BLE_ADMIN_BEGIN,         register_admin_notfication,   BLE_ADMIN_STEP1},
  {BLE_ADMIN_STEP1,         waiting_admin_step1,          BLE_ADMIN_STEP2},
  {BLE_ADMIN_STEP2,         write_admin_step2,            BLE_ADMIN_ESTABLISHED},
  {BLE_ADMIN_ESTABLISHED,   waiting_admin_step3,         BLE_ADMIN_LOCK_REQUEST},
  {BLE_ADMIN_LOCK_REQUEST,   write_lock_request,         BLE_ADMIN_LOCK_RESULT},
  {BLE_ADMIN_LOCK_RESULT,   waiting_lock_result,        BLE_ADMIN_LOCK_DONE},
};

int write_admin_step2(void *arg)
{
  serverLog(LL_NOTICE, "write_admin_step2 start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  size_t step2Bytes_len, payload_len;
  uint8_t *step2Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  int step2Len;
  int connectionID;

  ret = igloohome_ble_lock_crypto_AdminConnection_beginConnection(
                            admin_connection->lock->admin_key, admin_connection->lock->admin_key_len);
  if (ret == ERROR_CONNECTION_ID)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_AdminConnection_beginConnection error");
		goto ADMIN_STEP2_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_AdminConnection_beginConnection success");
  connectionID = ret;
  setLockConnectionID(admin_connection->lock, connectionID);
  serverLog(LL_NOTICE, "set connectionID to Lock");

  step2Len = igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native(
		connectionID, 
    admin_connection->step_data + admin_connection->n_size_byte, 
    admin_connection->step_max_size - admin_connection->n_size_byte, 
    &step2Bytes);
	if (!step2Len)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native err");
		goto ADMIN_STEP2_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native success");

  if (!build_msg_payload(
		&payloadBytes, &payload_len, step2Bytes, step2Len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto ADMIN_STEP2_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "build_msg_payload success");

  ret = write_char_by_uuid_multi_atts(
		admin_connection->gatt_connection, &admin_connection->admin_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing packags");
		goto ADMIN_STEP2_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "admin write step2 write_char_by_uuid_multi_atts success in writing packages");

  admin_connection->admin_step = BLE_ADMIN_STEP2;

  admin_connection->step_max_size = 0;
  admin_connection->n_size_byte = 0;
  admin_connection->step_cur_size = 0;
  free(admin_connection->step_data);
  admin_connection->step_data = NULL;

  free(step2Bytes);
  step2Bytes = NULL;
  free(payloadBytes);
  payloadBytes = NULL;

  return 0;
ADMIN_STEP2_ERROR_EXIT:
  
  if (step2Bytes)
  {
    free(step2Bytes);
    step2Bytes = NULL;
  }
  if (payloadBytes)
  {
    free(payloadBytes);
    payloadBytes = NULL;
  }
  return 1;
}

static int write_lock_request(void *arg)
{
  serverLog(LL_NOTICE, "write_lock_request start --------");
  int ret = 0;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  srand(time(0));
  int requestID = rand() % 2147483647;
  size_t buf_size = 32;
  size_t encode_size = 0;
  uint8_t buf[buf_size];
  uint8_t *encryptPayloadBytes;
	size_t encryptPayloadBytes_len;

  IgAdminLockRequest lock_request;
  ig_AdminLockRequest_init(&lock_request);
  ig_AdminLockRequest_set_operation_id(&lock_request, requestID);
  ig_AdminLockRequest_set_password(
    &lock_request, admin_connection->lock->password, admin_connection->lock->password_size);
  IgSerializerError IgErr = ig_AdminLockRequest_encode(
		&lock_request, buf, buf_size, &encode_size);
  if (IgErr)
	{
    serverLog(LL_ERROR, "ig_UnpairRequest_encode err");
    goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "ig_UnpairRequest_encode success size:" );

  uint32_t retvalMaxLen = encryptDataSize(encode_size);
  uint8_t *retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);


  int32_t retvalLen = AdminConnection_encryptNative(
    admin_connection->lock->connectionID, buf, encode_size, &retvalBytes);
  if (!retvalLen) 
  {
    serverLog(LL_ERROR, "failed in AdminConnection_encryptNative");
    goto UNPAIR_REQUEST_ERROR;
  }

  serverLog(LL_NOTICE, "ble_unpair_write_unpairreques retvalLen: %d", retvalLen);

  if (!build_msg_payload(
		&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "build_msg_payload success");


  ret = write_char_by_uuid_multi_atts(
		admin_connection->gatt_connection, &admin_connection->admin_uuid, 
    encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

  free(encryptPayloadBytes);
  retvalBytes = encryptPayloadBytes;
  free(retvalBytes);
  retvalBytes = NULL;
  admin_connection->admin_step = BLE_ADMIN_LOCK_REQUEST;
  
  return 0;

UNPAIR_REQUEST_ERROR:

  if (encryptPayloadBytes)
  {
    free(encryptPayloadBytes);
  }
  return 1;
}

static int write_unlock_request(void *arg)
{
  serverLog(LL_NOTICE, "write_unlock_request start --------");
  int ret = 0;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  srand(time(0));
  int requestID = rand() % 2147483647;
  size_t buf_size = 32;
  size_t encode_size = 0;
  uint8_t buf[buf_size];
  uint32_t retvalMaxLen;
  uint8_t *retvalBytes;
  uint8_t *encryptPayloadBytes;
	size_t encryptPayloadBytes_len;

  IgAdminUnlockRequest unlock_request;
  ig_AdminUnlockRequest_init(&unlock_request);
  ig_AdminUnlockRequest_set_operation_id(&unlock_request, requestID);
  ig_AdminUnlockRequest_set_password(
    &unlock_request, admin_connection->lock->password, admin_connection->lock->password_size);
  IgSerializerError IgErr = ig_AdminUnlockRequest_encode(
		&unlock_request, buf, buf_size, &encode_size);
  if (IgErr)
	{
    serverLog(LL_ERROR, "ig_UnpairRequest_encode err");
    goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "ig_UnpairRequest_encode success size:" );

  Connection *encry_connection = getConnection(admin_connection->lock->connectionID);
	if (!encry_connection)
	{
    serverLog(LL_ERROR, "ble_unpair_write_unpairreques can't get encry_connection");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "ble_unpair_write_unpairreques success getting encry_connection" );

  retvalMaxLen = encryptDataSize(encode_size);
  serverLog(LL_NOTICE, "ble_unpair_write_unpairreques PayloadBytes_len: %d", retvalMaxLen);
  retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);

  int32_t retvalLen = encryptData(
    buf, encode_size,
    retvalBytes, retvalMaxLen,
    encry_connection->key, kConnectionKeyLength,
    encry_connection->txNonce, kNonceLength
  );
  serverLog(LL_NOTICE, "------------- encry_connection->key: ");
  // for (int j = 0; j < kConnectionKeyLength; j++)
  // {
  //   printf("%02x ", encry_connection->key[j]);
  // }
  // printf("\n");

  serverLog(LL_NOTICE, "------------- encry_connection->txNonce: ");
  // for (int j = 0; j < kNonceLength; j++)
  // {
  //   printf("%02x ", encry_connection->txNonce[j]);
  // }
  // printf("\n");

  incrementNonce(encry_connection->txNonce);
  serverLog(LL_NOTICE, "ble_unpair_write_unpairreques retvalLen: %d", retvalLen);

  if (!build_msg_payload(
		&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "build_msg_payload success");


  ret = write_char_by_uuid_multi_atts(
		admin_connection->gatt_connection, &admin_connection->admin_uuid, 
    encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");
  
  free(encryptPayloadBytes);
  encryptPayloadBytes = NULL;
  free(retvalBytes);
  retvalBytes = NULL;
  
  admin_connection->admin_step = BLE_ADMIN_UNLOCK_REQUEST;

  return 0;


UNPAIR_REQUEST_ERROR:
  if (encryptPayloadBytes)
  {
    free(encryptPayloadBytes);
    encryptPayloadBytes = NULL;
  }
  if (retvalBytes)
  {
    free(retvalBytes);
    retvalBytes = NULL;
  }
  return 1;
}

static int write_unpair_request(void *arg)
{
  serverLog(LL_NOTICE, "write_unpair_request start --------");
  int ret = 0;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  srand(time(0));
  int requestID = rand() % 2147483647;
  size_t buf_size = 32;
  size_t encode_size = 0;
  uint8_t buf[buf_size];
  uint8_t *encryptPayloadBytes;
	size_t encryptPayloadBytes_len;
  uint32_t retvalMaxLen;
  uint8_t *retvalBytes;
  int32_t retvalLen;

  IgUnpairRequest unpair_request;
  ig_UnpairRequest_init(&unpair_request);
  ig_UnpairRequest_set_operation_id(&unpair_request, requestID);
  ig_UnpairRequest_set_password(
    &unpair_request, admin_connection->lock->password, admin_connection->lock->password_size);
  IgSerializerError IgErr = ig_UnpairRequest_encode(
		&unpair_request, buf, buf_size, &encode_size);
  if (IgErr)
	{
    serverLog(LL_ERROR, "ig_UnpairRequest_encode err");
    goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "ig_UnpairRequest_encode success size:" );

  retvalMaxLen = encryptDataSize(encode_size);
  retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);

  retvalLen = AdminConnection_encryptNative(
    admin_connection->lock->connectionID, buf, encode_size, &retvalBytes);
  if (!retvalLen) 
  {
    serverLog(LL_ERROR, "failed in AdminConnection_encryptNative");
    goto UNPAIR_REQUEST_ERROR;
  }

  if (!build_msg_payload(
		&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "build_msg_payload success");


  ret = write_char_by_uuid_multi_atts(
		admin_connection->gatt_connection, &admin_connection->admin_uuid, 
    encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto UNPAIR_REQUEST_ERROR;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

  free(retvalBytes);
  retvalBytes = NULL;

  free(encryptPayloadBytes);
  encryptPayloadBytes = NULL;
  
  admin_connection->admin_step = BLE_ADMIN_UNPAIR_REQUEST;
  return 0;


UNPAIR_REQUEST_ERROR:
  if (retvalBytes)
  {
    free(retvalBytes);
    retvalBytes = NULL;
  }
  
  if (encryptPayloadBytes)
  {
    free(encryptPayloadBytes);
    encryptPayloadBytes = NULL;
  }
  
  return 1;
}

int handle_step3_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_admin_step3_message");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  save_message_data(data, data_length, user_data);

  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step3_message RECV step3 data finished");
    admin_connection->admin_step = BLE_ADMIN_ESTABLISHED;

    int rec_ret = igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(
			(admin_connection->lock)->connectionID, 
      admin_connection->step_data + admin_connection->n_size_byte, 
      admin_connection->step_max_size-admin_connection->n_size_byte
		);

    if (!rec_ret){
      serverLog(LL_ERROR, "igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native err");
		}
    serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native success");
    if (admin_connection->has_admin_result)
    {
      serverLog(LL_NOTICE, "set admin result to success");
      admin_connection->admin_result->admin_successed = 1;
    }
     // 返回参数给调用进程
    serverLog(LL_NOTICE, "handle_step3_message bleSetBleResult to ble data");
    // bleSetBleResult(
    //   ble_data, admin_connection->admin_result, sizeof(ble_admin_result_t));
    
    
    // ?>?释放内存?
    admin_connection->step_max_size = 0;
    admin_connection->n_size_byte = 0;
    admin_connection->step_cur_size = 0;
    free(admin_connection->step_data);
    admin_connection->step_data = NULL;
    serverLog(LL_NOTICE, "g_main_loop_quit connection->properties_changes_loop");
    g_main_loop_quit(task_node->loop);
  }
}

static int handle_lock_responce(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_lock_responce");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  save_message_data(data, data_length, user_data);
  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_lock_responce RECV step2 data finished");
    admin_connection->admin_step = BLE_ADMIN_UNPAIR_RESULT;
   
    g_main_loop_quit(task_node->loop);
  }
}


static int handle_unlock_responce(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_unlock_responce--------------------------------");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  save_message_data(data, data_length, user_data);

  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_unlock_responce RECV step2 data finished");
    admin_connection->admin_step = BLE_ADMIN_UNLOCK_RESULT;

    size_t messageLen = 
      admin_connection->step_max_size - admin_connection->n_size_byte;
    uint8_t *data_start = admin_connection->step_data + admin_connection->n_size_byte;
    uint8_t messageBytes[messageLen];
    memcpy(messageBytes, data_start, messageLen);

    uint32_t responceMaxLen = decryptDataSize(messageLen);
    uint8_t responceBytes[responceMaxLen];
    serverLog(LL_NOTICE, "messageLen %d", messageLen);
    int32_t responceLen = AdminConnection_decryptNative(
      (admin_connection->lock)->connectionID, messageBytes, messageLen, (uint8_t **)(&responceBytes));
    if (!responceLen)
    { 
      serverLog(LL_ERROR, "ble_unpair_write_unpairreques responceLen %d", responceLen);
    }
    serverLog(LL_NOTICE, "ble_unpair_write_unpairreques responceLen %d", responceLen);
    
    IgAdminUnlockResponse admin_unlock_resppnce;
    ig_AdminUnlockResponse_init(&admin_unlock_resppnce);
    IgSerializerError err = ig_AdminUnlockResponse_decode(
      responceBytes, responceLen, &admin_unlock_resppnce, 0
    );
    if (err)
    {
      serverLog(LL_NOTICE, "ig_AdminUnlockResponse_decode err %d", err);
    }
    serverLog(LL_NOTICE, "has unlock response %d %d",admin_unlock_resppnce.has_result, admin_unlock_resppnce.result);
    if (admin_connection->has_admin_result && admin_unlock_resppnce.has_result)
    {
      serverLog(LL_NOTICE, "set admin result to success");
      admin_connection->admin_result->unlock_result= admin_unlock_resppnce.result;
    }
     // 返回参数给调用进程
    serverLog(LL_NOTICE, "handle_step3_message bleSetBleResult to ble data");
    bleSetBleResult(
      ble_data, admin_connection->admin_result, sizeof(ble_admin_result_t));
    
    AdminConnection_endConnection((admin_connection->lock)->connectionID);

    serverLog(LL_NOTICE, "AdminConnection_endConnection success");
    
UNLOCK_RESULT_EXIT:
    serverLog(LL_NOTICE, "UNLOCK_RESULT_EXIT--------------------------------");
    g_main_loop_quit(task_node->loop);
  }
}

static int handle_unpair_responce(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_unpair_responce");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  save_message_data(data, data_length, user_data);
  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_unpair_responce RECV data finished");
    serverLog(LL_NOTICE, "handle_unpair_responce RECV data: ");
    admin_connection->admin_step = BLE_ADMIN_UNPAIR_RESULT;

    size_t messageLen = 
      admin_connection->step_max_size - admin_connection->n_size_byte;
    uint8_t *data_start = admin_connection->step_data + admin_connection->n_size_byte;
    uint8_t messageBytes[messageLen];
    memcpy(messageBytes, data_start, messageLen);
    // for (int j = 0; j < messageLen; j++)
    // {
    //   printf("%02x ", messageBytes[j]);
    // }
    // printf("\n");

    uint32_t responceMaxLen = decryptDataSize(messageLen);
    uint8_t responceBytes[responceMaxLen];
    // uint8_t *responceBytes = (uint8_t *)calloc(responceMaxLen,1);
    serverLog(LL_NOTICE, "messageLen %d", messageLen);
    int32_t responceLen = AdminConnection_decryptNative(
      (admin_connection->lock)->connectionID, messageBytes, messageLen, (uint8_t **)(&responceBytes));
    if (!responceLen)
    { 
      serverLog(LL_ERROR, "ble_unpair_write_unpairreques responceLen %d", responceLen);
    }
    serverLog(LL_NOTICE, "ble_unpair_write_unpairreques responceLen %d", responceLen);
    // serverLog(LL_NOTICE, "unpair responce :");
    // for (int j = 0; j < responceLen; j++)
    // {
    //   printf("%02x ", responceBytes[j]);
    // }
    // printf("\n");
    IgUnpairResponse unpair_resppnce;
    ig_UnpairResponse_init(&unpair_resppnce);
    IgSerializerError err = ig_UnpairResponse_decode(
      responceBytes, 
      responceLen, 
      &unpair_resppnce, 0 );

    if (err)
    {
      serverLog(LL_NOTICE, "ig_UnpairResponse_decode err %d", err);
    }
    serverLog(LL_NOTICE, "has unpair response %d %d",unpair_resppnce.has_result, unpair_resppnce.result);
    
    if (admin_connection->has_admin_result && unpair_resppnce.has_result)
    {
      serverLog(LL_NOTICE, "set admin result to success");
      admin_connection->admin_result->unpair_result= unpair_resppnce.result;
    }
     // 返回参数给调用进程
    serverLog(LL_NOTICE, "handle_unpair_response bleSetBleResult to ble data");
    bleSetBleResult(
      ble_data, admin_connection->admin_result, sizeof(ble_admin_result_t));
    
  UNPAIR_RESULT_EXIT:
    g_main_loop_quit(task_node->loop);

  }
}

int handle_step1_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_step1_message");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  save_message_data(data, data_length, user_data);
  
  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step1_message RECV step2 data finished");
    admin_connection->admin_step = BLE_ADMIN_STEP1;
    g_main_loop_quit(task_node->loop);
  }
}

void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
  serverLog(LL_NOTICE, "message_handler---------------");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  switch(admin_connection->admin_step)
  {
    case BLE_ADMIN_BEGIN:
    {
      serverLog(LL_NOTICE, 
      "admin_connection->admin_step BLE_ADMIN_BEGIN handle_step1_message");
      handle_step1_message(data, data_length,user_data);
      break;
    }
    case BLE_ADMIN_STEP2:
    {
      serverLog(LL_NOTICE, 
      "admin_connection->admin_step BLE_PAIRING_STEP3 handle_step3_message");
      handle_step3_message(data, data_length, user_data);
      break;
    }
    case BLE_ADMIN_UNPAIR_REQUEST:
    {
      serverLog(LL_NOTICE, 
      "BLE_ADMIN_UNPAIR_REQUEST handle_unpair_responce");
      handle_unpair_responce(data, data_length,user_data);
      break;
    }
    case BLE_ADMIN_UNLOCK_REQUEST:
    {
      // for (int j = 0; j < data_length; j++)
      // {
      //   printf("%02x ", data[j]);
      // }
      // printf("\n");
      serverLog(LL_NOTICE, 
      "BLE_ADMIN_UNLOCK_REQUEST handle_unlock_responce");
      handle_unlock_responce(data, data_length,user_data);
      break;
    }
    case BLE_ADMIN_LOCK_REQUEST:
    {
      serverLog(LL_NOTICE, 
      "BLE_ADMIN_LOCK_REQUEST handle_lock_responce");
      handle_lock_responce(data, data_length,user_data);
      break;
    }
    
    default:
    {
      serverLog(LL_ERROR, "admin_connection->admin_step error");
      break;
    }
  }
}

int register_admin_notfication(void *arg)
{
  serverLog(LL_NOTICE, "register_admin_notfication start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
  
  ble_admin_param_t *param = (ble_admin_param_t *)(ble_data->ble_param);
  // 分配 connection, 传递到其他函数的数据,
  ble_data->ble_connection = calloc(sizeof(admin_connection_t), 1);
  admin_connection_t *admin_connection = 
                            (admin_connection_t *)ble_data->ble_connection;
  initAdminConnection(admin_connection);
  
  getLock(&admin_connection->lock);
  copyLock(admin_connection->lock, param->lock);
  serverLog(LL_NOTICE, "register_admin_notfication release admin param");
  bleReleaseAdminParam(&param);
  
  serverLog(LL_NOTICE, "register_admin_notfication apply for a ble_admin_result_t, need user to release");
  admin_connection->has_admin_result = 1;
  admin_connection->admin_result = calloc(sizeof(ble_admin_result_t), 1);
  bleInitAdminResult(admin_connection->admin_result);
  bleSetAdminResultAddr(admin_connection->admin_result, 
                                      admin_connection->lock->addr, admin_connection->lock->addr_len);

  ble_data->adapter_name = NULL;
  ble_data->adapter = NULL;
  serverLog(LL_NOTICE, "register_admin_notfication ready to connection %s",
                                                            admin_connection->lock->addr);
  admin_connection->gatt_connection = gattlib_connect(
    NULL, admin_connection->lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (admin_connection->gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		goto ADMIN_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );
  if (
    gattlib_string_to_uuid(
      admin_str, strlen(admin_str), &(admin_connection->admin_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to admin_uuid fail");
    goto ADMIN_ERROR_EXIT;
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to admin_uuid success." );


  gattlib_register_notification(
    admin_connection->gatt_connection, message_handler, arg);
  ret = gattlib_notification_start(
      admin_connection->gatt_connection, &admin_connection->admin_uuid);
  if (ret) {
    serverLog(LL_ERROR, "Fail to start notification.");
		goto ADMIN_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "success to start notification" );

  admin_connection->admin_step = BLE_ADMIN_BEGIN;
  task_node->loop = g_main_loop_new(NULL, 0);
  return 0;

ADMIN_ERROR_EXIT:
  serverLog(LL_ERROR, "register_admin_notfication ERROR EXIT.");
  
  if (ble_data->ble_connection)
  {
    free(ble_data->ble_connection);
  }
  return 1;
}



static int waiting_unpair_result(void *arg)
{
  serverLog(LL_NOTICE, "waiting_unpair_result");
  task_node_t *task_node = (task_node_t *)arg;

  // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // 当前 Linux 下, 这样用, works 
  serverLog(LL_NOTICE, "waiting_unpair_result new loop waiting");
  task_node->loop = g_main_loop_new(NULL, 0);
  if (!task_node->loop)
  {
    serverLog(LL_ERROR, "task_node->loop error");
  }
  g_main_loop_run(task_node->loop);
  serverLog(LL_NOTICE, "waiting_unpair_result exit task_node->loop");
  return 0;
}

static int waiting_unlock_result(void *arg)
{
  serverLog(LL_NOTICE, "waiting_unlock_result");
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
  admin_connection_t *admin_connection = 
                            (admin_connection_t *)ble_data->ble_connection;

  serverLog(LL_NOTICE, "waiting_unlock_result new loop waiting");
  g_main_loop_run(task_node->loop);
  g_main_loop_unref(task_node->loop);
  task_node->loop = NULL;
  int ret = releaseAdminConnection(&admin_connection);
  if (ret)
  {
    serverLog(LL_ERROR, "waiting_unlock_result releaseAdminConnection");
    return ret;
  }
  return 0;
}


static int waiting_lock_result(void *arg)
{
  serverLog(LL_NOTICE, "waiting_unlock_result");
  task_node_t *task_node = (task_node_t *)arg;

  // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // 当前 Linux 下, 这样用, works 
  serverLog(LL_NOTICE, "waiting_unlock_result new loop waiting");
  task_node->loop = g_main_loop_new(NULL, 0);
  if (!task_node->loop)
  {
    serverLog(LL_ERROR, "task_node->loop error");
  }
  g_main_loop_run(task_node->loop);
  serverLog(LL_NOTICE, "waiting_unlock_result exit task_node->loop");
  return 0;
}

static int waiting_admin_step3(void *arg)
{
  serverLog(LL_NOTICE, "waiting_admin_step3");
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
  admin_connection_t *admin_connection = 
                            (admin_connection_t *)ble_data->ble_connection;

  serverLog(LL_NOTICE, "waiting_admin_step3 new loop waiting");
  g_main_loop_run(task_node->loop);
  admin_connection->admin_step = BLE_ADMIN_DONE;
  serverLog(LL_NOTICE, "waiting_admin_step3 exit task_node->loop");
  return 0;
}

int waiting_admin_step1(void *arg)
{
  serverLog(LL_NOTICE, "waiting_pairing_step2");
  task_node_t *task_node = (task_node_t *)arg;

  serverLog(LL_NOTICE, "waiting_admin_step1 new loop waiting");
  g_main_loop_run(task_node->loop);
  serverLog(LL_NOTICE, "waiting_admin_step2 exit task_node->loop");
  return 0;
}

int save_message_data(const uint8_t* data, int data_length, void* user_data)
{
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;

  // 如果没有分配内存                            
  if (admin_connection->step_max_size == 0)
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
        admin_connection->n_size_byte = 3;
        admin_connection->step_max_size = 
                    data[0] * (0xfe) + data[1] + admin_connection->n_size_byte;
        serverLog(LL_NOTICE, 
                        "2 bytes lenth %d", admin_connection->step_max_size);
      }
      else
      {
        serverLog(LL_NOTICE, 
                      "data[0] %d %02x", data[0], data[0]);
        admin_connection->n_size_byte = 1;
        admin_connection->step_max_size = data[0] + admin_connection->n_size_byte;
      }
      admin_connection->step_cur_size = 0;
      admin_connection->step_data = (uint8_t *)malloc(
                                            admin_connection->step_max_size);
      if (!(admin_connection->step_data))
      {
        serverLog(LL_ERROR, "save_message_data malloc err");
        return 1;
      }
    }
  }

  int size_left = 
        admin_connection->step_max_size - admin_connection->step_cur_size;
  // 帕不够
  if (size_left < data_length)
	{
    serverLog(LL_NOTICE, "size_left < data_length");
    admin_connection->step_max_size += 85;
		uint8_t *old_data = admin_connection->step_data;
		admin_connection->step_data = (uint8_t *)malloc(
                                              admin_connection->step_max_size);
		if (!admin_connection->step_data)
		{
			serverLog(LL_ERROR, "save_message_data malloc err");
			return 4;
		}
		memcpy(
      admin_connection->step_data, old_data, admin_connection->step_cur_size);
		free(old_data);
    old_data = NULL;
	}

  // 空间肯定足够, 直接放下空间里面
	for (int j = 0; j < data_length; ) {
		admin_connection->step_data[admin_connection->step_cur_size++] = data[j++];
	}
}

// -------------------------------------

int bleInitAdminParam(ble_admin_param_t *admin_param)
{
  memset(admin_param, 0, sizeof(ble_admin_param_t));
  return 0;
}

int bleReleaseAdminParam(ble_admin_param_t **pp_admin_param)
{
  ble_admin_param_t *admin_param = *pp_admin_param;
  if (admin_param->lock)
  {
    serverLog(LL_NOTICE, "bleReleaseAdminParam");
    releaseLock(&admin_param->lock);
    admin_param->lock = NULL;
  }
  free(*pp_admin_param );
  *pp_admin_param = NULL;
  return 0;
}


int bleSetAdminParam(ble_admin_param_t *admin_param, igm_lock_t *lock)
{
  releaseLock(&admin_param->lock);
  admin_param->lock = calloc(sizeof(igm_lock_t), 1);
  copyLock(admin_param->lock, lock);
  return 0;
}

// -------------------------------------

int bleInitAdminResult(ble_admin_result_t *result)
{
  memset(result, 0, sizeof(ble_admin_result_t));
  return 0;
}

int bleSetAdminResultAddr(ble_admin_result_t *result, 
  char *addr, size_t addr_len)
{
  memset(result->addr,0, MAX_DEVICE_ADDR);
  memcpy(result->addr, addr, addr_len>MAX_DEVICE_ADDR?MAX_DEVICE_ADDR:addr_len);
  return 0;
}

int bleIsAdminSuccess(ble_admin_result_t *result)
{
  return result->admin_successed;
}
// -----------------------------------------------

fsm_table_t *getAdminFsmTable()
{
  return admin_fsm_table;
}

int getAdminFsmTableLen()
{
  return ADMIN_SM_TABLE_LEN;
}

// unpair

fsm_table_t *getAdminUnpairFsmTable()
{
  return admin_unpair_fsm_table;
}

int getAdminUnpairFsmTableLen()
{
  return ADMIN_UNPAIR_SM_TABLE_LEN;
}

// unlock

fsm_table_t *getAdminUnlockFsmTable()
{
  return admin_unlock_fsm_table;
}

int getAdminUnlockFsmTableLen()
{
  return ADMIN_UNLOCK_SM_TABLE_LEN;
}

// lock
fsm_table_t *getAdminLockFsmTable()
{
  return admin_lock_fsm_table;
}

int getAdminLockFsmTableLen()
{
  return ADMIN_LOCK_SM_TABLE_LEN;
}

void bleReleaseAdminResult(ble_admin_result_t **pp_result)
{
  if (!pp_result) return;
  if (!*pp_result) return;
  free(*pp_result);
  *pp_result = NULL;
}