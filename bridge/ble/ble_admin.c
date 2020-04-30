#include <bridge/ble/ble_admin.h>
#include <bridge/gattlib/gattlib.h>
#include <bridge/bridge_main/task.h>
#include <time.h> 
#include <bridge/lock/connection/admin_connection.h>
#include <bridge/ble/ble_operation.h>

static char admin_str[] = "5c3a659f-897e-45e1-b016-007107c96df6";
enum {
  ADMIN_SM_TABLE_LEN = 4
};

typedef struct AdminConnection {
	gatt_connection_t* gatt_connection;
  igm_lock_t lock;
	enum BLE_ADMIN_STATE admin_step;
	size_t step_max_size;
	size_t  step_cur_size;
	size_t n_size_byte;
	uint8_t *step_data;
  uuid_t admin_uuid;
  int has_admin_result;
  ble_admin_result_t *admin_result;
}admin_connection_t;

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



int write_admin_step2(void *arg)
{
  serverLog(LL_NOTICE, "write_admin_step2 start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  ble_admin_param_t *param = ble_data->ble_param;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  size_t step2Bytes_len, payload_len;
  uint8_t *step2Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  int step2Len;
  int connectionID;

  for (int j = 0; j < param->lock->admin_key_len; j++)
  {
    printf("%x ", param->lock->admin_key[j]);
  }
  printf("\n");

  ret = igloohome_ble_lock_crypto_AdminConnection_beginConnection(
                            param->lock->admin_key, param->lock->admin_key_len);
  if (ret == ERROR_CONNECTION_ID)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_AdminConnection_beginConnection error");
		goto ADMIN_STEP2_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_AdminConnection_beginConnection success");
  connectionID = ret;
  setLockConnectionID(&(admin_connection->lock), connectionID);
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
  free(payloadBytes);
  free(step2Bytes);


  return 0;
ADMIN_STEP2_ERROR_EXIT:
  if (payloadBytes)
  {
    free(payloadBytes);
  }
  if (step2Bytes)
  {
    free(step2Bytes);
  }
  return 1;
}

int handle_step3_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_step3_message");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  admin_connection_t *admin_connection = 
                              (admin_connection_t *)ble_data->ble_connection;
  
  save_message_data(data, data_length, user_data);

  if (admin_connection->step_max_size == admin_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step2_message RECV step2 data finished");
    admin_connection->admin_step = BLE_ADMIN_ESTABLISHED;

    int rec_ret = igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(
			(admin_connection->lock).connectionID, 
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
      bleSetAdminResultGattConnection(admin_connection->admin_result, admin_connection->gatt_connection);
    }
     // 返回参数给调用进程
    serverLog(LL_NOTICE, "handle_step3_message bleSetBleResult to ble data");
    bleSetBleResult(
      ble_data, admin_connection->admin_result, sizeof(ble_admin_result_t));
    
    
    // ?>?释放内存?
    if (admin_connection->step_data)
    {
      admin_connection->step_max_size = 0;
      admin_connection->n_size_byte = 0;
      admin_connection->step_cur_size = 0;
      free(admin_connection->step_data);
      admin_connection->step_data = NULL;
    }
    serverLog(LL_NOTICE, "admin stop notification change");
    gattlib_notification_stop(
              admin_connection->gatt_connection, &admin_connection->admin_uuid);
    serverLog(LL_NOTICE, "g_main_loop_quit connection->properties_changes_loop");
    
    g_main_loop_quit(task_node->loop);
    // gattlib_disconnect(admin_connection->gatt_connection);
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
    serverLog(LL_NOTICE, "handle_step2_message RECV step2 data finished");
    admin_connection->admin_step = BLE_ADMIN_STEP1;
    g_main_loop_quit(task_node->loop);
  }
}

void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
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
  memset(admin_connection, 0, sizeof(admin_connection_t));

  admin_connection->has_admin_result = 1;
  admin_connection->admin_result = calloc(sizeof(ble_admin_result_t), 1);
  bleInitAdminResult(admin_connection->admin_result);
  bleSetAdminResultAddr(admin_connection->admin_result, 
                                      param->lock->addr, param->lock->addr_len);

  ble_data->adapter_name = NULL;
  ble_data->adapter = NULL;

  ret = gattlib_adapter_open(ble_data->adapter_name, &(ble_data->adapter));
  if (ret) {
		serverLog(LL_ERROR, 
      "ERROR: register_admin_notfication Failed to open adapter.");
		return 1;
	}
  serverLog(LL_NOTICE, "register_admin_notfication Success to open adapter.." );
  serverLog(LL_NOTICE, "register_admin_notfication ready to connection %s",
                                                            param->lock->addr);
  admin_connection->gatt_connection = gattlib_connect(
    NULL, param->lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
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
  serverLog(LL_NOTICE, "register_admin_notfication end --------");

  return 0;

ADMIN_ERROR_EXIT:
  serverLog(LL_ERROR, "register_admin_notfication ERROR EXIT.");
  if (admin_connection->gatt_connection)
  {
    gattlib_notification_stop(
      admin_connection->gatt_connection, &(admin_connection->admin_uuid));
    gattlib_disconnect(admin_connection->gatt_connection);
  }
  if (ble_data->ble_connection)
  {
    free(ble_data->ble_connection);
  }
  return 1;
}

static int waiting_admin_step3(void *arg)
{
  serverLog(LL_NOTICE, "waiting_pairing_step3");
  task_node_t *task_node = (task_node_t *)arg;

  // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // 当前 Linux 下, 这样用, works 
  serverLog(LL_NOTICE, "waiting_admin_step3 new loop waiting");
  // task_node->loop = g_main_loop_new(NULL, 0);
  g_main_loop_run(task_node->loop);
  serverLog(LL_NOTICE, "waiting_admin_step3 exit task_node->loop");
  return 0;
}


int waiting_admin_step1(void *arg)
{
  serverLog(LL_NOTICE, "waiting_pairing_step2");
  task_node_t *task_node = (task_node_t *)arg;

  // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // 当前 Linux 下, 这样用, works 
  serverLog(LL_NOTICE, "waiting_admin_step1 new loop waiting");
  task_node->loop = g_main_loop_new(NULL, 0);
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
        admin_connection->step_max_size = data[0] * (0xfe) + data[1] + 3;
        serverLog(LL_NOTICE, 
                      "2 bytes lenth %d", admin_connection->step_max_size);
      }
      else
      {
        admin_connection->n_size_byte = 1;
        admin_connection->step_max_size = data[0] + 1;
        serverLog(LL_NOTICE, 
                      "1 bytes lenth %d", admin_connection->step_max_size);
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
	}

  // 空间足够, 直接放下空间里面
	for (int j = 0; j < data_length; ) {
		// printf("%02x ", data[i]);
		admin_connection->step_data[admin_connection->step_cur_size++] = data[j++];
	}
}

// -------------------------------------

int bleInitAdminParam(ble_admin_param_t *admin_param)
{
  memset(admin_param, 0, sizeof(ble_admin_param_t));
  return 0;
}

int bleReleaseAdminParam(ble_admin_param_t *admin_param)
{
  if (admin_param->lock)
  {
    free(admin_param->lock);
    admin_param->lock = NULL;
  }
  return 0;
}


int bleSetAdminParam(ble_admin_param_t *admin_param, igm_lock_t *lock)
{
  bleReleaseAdminParam(admin_param);
  admin_param->lock = calloc(sizeof(igm_lock_t), 1);
  lockCopy(admin_param->lock, lock);
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
int bleSetAdminResultGattConnection(ble_admin_result_t *result, void* gatt_connection)
{
  result->gatt_connection =  gatt_connection;
  return 0;
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