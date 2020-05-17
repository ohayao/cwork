#include <bridge/ble/ble_pairing.h>
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
#include <bridge/ble/ble_operation.h>
#include <string.h>
#include <stdlib.h>

static char pairing_str[] = "5c3a659e-897e-45e1-b016-007107c96df6";
// 产生一个错误的返回.
int blePairingResultErr(ble_pairing_result_t *result);

//步骤分别为
// 1. BLE_PAIRING_BEGIN 监听变化
// 2. BLE_PAIRING_STEP1 写第一步
// 3. BLE_PAIRING_STEP2 等待锁返回第二步
// 4. BLE_PAIRING_STEP3 写第三步
// 5. BLE_PAIRING_STEP4 等待第四步
// 6. BLE_PAIRING_DONE 写commit, 也就done.
static int register_pairing_notfication(void *arg);
static int write_pairing_step1(void *arg);
static int waiting_pairing_step2(void *arg);
static int write_pairing_step3(void *arg);
static int waiting_pairing_step4(void *arg);
static int write_pairing_commit(void *arg);
static void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);
static int handle_step2_message(const uint8_t* data, int data_length,void* user_data);
static int handle_step4_message(const uint8_t* data, int data_length,void* user_data);
static int save_message_data(const uint8_t* data, int data_length, void* user_data);

enum {
  PAIRING_SM_TABLE_LEN =6
};
fsm_table_t pairing_fsm_table[PAIRING_SM_TABLE_LEN] = {
  {BLE_PAIRING_BEGIN,   register_pairing_notfication,  BLE_PAIRING_STEP1},
  {BLE_PAIRING_STEP1,   write_pairing_step1,           BLE_PAIRING_STEP2},
  {BLE_PAIRING_STEP2,   waiting_pairing_step2,         BLE_PAIRING_STEP3},
  {BLE_PAIRING_STEP3,   write_pairing_step3,           BLE_PAIRING_STEP4},
  {BLE_PAIRING_STEP4,   waiting_pairing_step4,         BLE_PAIRING_COMMIT},
  {BLE_PAIRING_COMMIT,  write_pairing_commit,          BLE_PAIRING_DONE},
};
static int end_pairing_gatt(void *arg);


fsm_table_t *getPairingFsmTable()
{
  return pairing_fsm_table;
}

int getPairingFsmTableLen()
{
  return PAIRING_SM_TABLE_LEN;
}

// 为什么这儿也有一个result, 原因是
// 我不想在Ble_data里面有各种操作,把不同部分的操作都归纳到ble_data里面
// 所以, 在不同部分的connection_t里面, 设置了一个result
// ble data 只复制二进制结果.这样可以让不同写和读的人自己知道协议
// 递信的人, 只要复制二进制结果就ok了.即使那个结果是一个指针, 只拿指针内存的地址就ok
// 你们两个写信和读信的人配合好释放内存.
// 写信的人,自己申请一块, 读信的人, 自己负责使用写信人提供的释放借口释放.
typedef struct ParingConnection {
	gatt_connection_t* gatt_connection;
  igm_lock_t lock;
	enum BLE_PAIRING_STATE pairing_step;
	size_t step_max_size;
	size_t  step_cur_size;
	size_t n_size_byte;
	uint8_t *step_data;
  uuid_t pairing_uuid;
  int has_pairing_result;
  ble_pairing_result_t *pairing_result;
}pairing_connection_t;

static int end_pairing_gatt(void *arg)
{
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  ret = gattlib_notification_stop(
        pairing_connection->gatt_connection, &pairing_connection->pairing_uuid);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_admin_gatt gattlib_notification_stop error");
    return ret;
  }
  ret = gattlib_disconnect(pairing_connection->gatt_connection);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_admin_gatt gattlib_disconnect error");
    return ret;
  }
  return ret;
}

int pairing_err_result_return(void *arg)
{
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
   pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  // 返回参数给调用进程
  serverLog(LL_NOTICE, 
    "write_pairing_commit bleSetBleResult to ble data");
  
  bleSetBleResult(
    ble_data, pairing_connection->pairing_result, sizeof(ble_pairing_result_t));
}

int pairing_success_result_return(void *arg)
{
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
   pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  // 返回参数给调用进程
  serverLog(LL_NOTICE, 
    "write_pairing_commit bleSetBleResult to ble data");
  bleSetBleResult(
    ble_data, pairing_connection->pairing_result, sizeof(ble_pairing_result_t));
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

int waiting_pairing_step4(void *arg)
{
  serverLog(LL_NOTICE, "waiting_pairing_step4");
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
  pairing_connection_t *pairing_connection = 
                            (pairing_connection_t *)ble_data->ble_connection;
  
  serverLog(LL_NOTICE, "waiting_pairing_step4 new loop waiting");
  task_node->loop = g_main_loop_new(NULL, 0);
  serverLog(LL_NOTICE, "waiting_pairing_step4 exit task_node->loop");
  g_main_loop_run(task_node->loop);  

  return 0;
}

int waiting_pairing_step2(void *arg)
{
  serverLog(LL_NOTICE, "waiting_pairing_step2");
  task_node_t *task_node = (task_node_t *)arg;

  // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // 当前 Linux 下, 这样用, works 
  serverLog(LL_NOTICE, "waiting_pairing_step2 new loop waiting");
  task_node->loop = g_main_loop_new(NULL, 0);
  g_main_loop_run(task_node->loop);
  serverLog(LL_NOTICE, "waiting_pairing_step2 exit task_node->loop");
  return 0;
}

int handle_step4_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_step4_message");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  save_message_data(data, data_length, user_data);
  if (pairing_connection->step_max_size == pairing_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step4_message RECV step2 data finished");
    pairing_connection->pairing_step = BLE_PAIRING_STEP4;

    g_main_loop_quit(task_node->loop);
  }
}

int handle_step2_message(const uint8_t* data, int data_length,void* user_data)
{
  serverLog(LL_NOTICE, "handle_step2_message -------------------");
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  serverLog(LL_NOTICE, "step_max_size %d, step_cur_size %d", pairing_connection->step_max_size, pairing_connection->step_cur_size);
  save_message_data(data, data_length, user_data);
  serverLog(LL_NOTICE, "step_max_size %d, step_cur_size %d", pairing_connection->step_max_size, pairing_connection->step_cur_size);
  if (pairing_connection->step_max_size == pairing_connection->step_cur_size)
  {
    int ret;
    serverLog(LL_NOTICE, "handle_step2_message RECV step2 data finished");
    pairing_connection->pairing_step = BLE_PAIRING_STEP2;
    g_main_loop_quit(task_node->loop);

  }
}

static void message_handler(
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
      serverLog(LL_NOTICE, 
      "pairing_connection->pairing_step BLE_PAIRING_STEP1 handle_step2_message");
      handle_step2_message(data, data_length, user_data);
      break;
    }
    case BLE_PAIRING_STEP3:
    {
      serverLog(LL_NOTICE, 
      "pairing_connection->pairing_step BLE_PAIRING_STEP3 handle_step4_message");
      handle_step4_message(data, data_length, user_data);
      break;
    }
    default:
    {
      serverLog(LL_ERROR, "pairing_connection->pairing_step error");
      break;
    }
  }
}

int write_pairing_commit(void *arg)
{
  serverLog(LL_NOTICE, "write_pairing_commit start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  void *step_data = pairing_connection->step_data;
  size_t step_max_size = pairing_connection->step_max_size;
  size_t n_size_byte = pairing_connection->n_size_byte;
  

  size_t commitBytes_len = 0;
	uint8_t *commitBytes = NULL;

  uint8_t *step4Bytes = NULL;
	size_t step4_size = 0;
  IgPairingStep4 *step4;

  size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;


  ret = igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
		(step_max_size-n_size_byte), (step_data+n_size_byte), &(step4Bytes)
	);
  if (!ret)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native error");
    goto COMMIT_ERROR_EXIT;
  }
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native success");
  step4_size = ret;
  step4 = (IgPairingStep4 *)step4Bytes;

  serverLog(LL_NOTICE, "debug show step4 password: ");

  if (step4->has_success && step4->success && 
                                        pairing_connection->has_pairing_result)
	{
    serverLog(LL_NOTICE, "set Pairing Result the Password");
    bleSetPairingResultPassword(
      pairing_connection->pairing_result, 1, step4->password, step4->password_size);
      serverLog(LL_NOTICE, "-------------------------step4.has_success ");
			// if (step4->has_password)
			// {
			// 		printf("step4.has_password: ");
			// 		for (int j = 0; j < step4->password_size;j++)
			// 		{
			// 				printf("%02x ", (step4->password)[j]);
			// 		}
			// 		printf("\n");
			// }
	}

  time_t current_time = time(NULL);

  commitBytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(
    current_time, &commitBytes);
  if (!commitBytes_len)
	{
    serverLog(LL_ERROR, "genPairingCommitNative failed!");
		goto COMMIT_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "genPairingCommitNative success");

  if (!build_msg_payload(
		&payloadBytes, &payload_len, commitBytes, commitBytes_len))
	{
    serverLog(LL_ERROR, "build_msg_payload failed!");
		goto COMMIT_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "build_msg_payload success!");

  ret = write_char_by_uuid_multi_atts(
		pairing_connection->gatt_connection, &pairing_connection->pairing_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed!");
		goto COMMIT_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success!");

  pairing_connection->pairing_step = BLE_PAIRING_DONE;

  serverLog(LL_NOTICE, "paired lock name %s addr %s", 
                pairing_connection->lock.name,  pairing_connection->lock.addr);

  uint8_t *admin_key;
  int key_len = igloohome_ble_lock_crypto_PairingConnection_getAdminKeyNative(
			time(NULL), &admin_key);
  if (key_len && admin_key && pairing_connection->has_pairing_result)
  {
    serverLog(LL_NOTICE, "set Pairing Result the  admin key:");
    bleSetPairingResultAdminKey(
      pairing_connection->pairing_result, 1, admin_key, key_len);
      serverLog(LL_NOTICE, "paired lock admin key:");
      // for (int j = 0; j < key_len; j++)
      // {
      //     printf("%02x ", admin_key[j]);
      // }
      // printf("\n");
  }

  // 返回参数给调用进程
  bleSetPairingSuccess(pairing_connection->pairing_result, 1);
  pairing_err_result_return(arg);
  if (admin_key)
  {
    free(admin_key);
  }
  free(commitBytes);
	free(payloadBytes);
  serverLog(LL_NOTICE, "release gatt_connection and notification");
  ret = end_pairing_gatt(arg);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_pairing_gatt error");
  }
  else
  {
    serverLog(LL_NOTICE, "end_pairing_gatt success ✓✓✓");
  }
  serverLog(LL_NOTICE, "write_pairing_commit end --------");
  
  return 0;

COMMIT_ERROR_EXIT:
  serverLog(LL_ERROR, "write_pairing_commit ERROR EXIT.");
  if (pairing_connection->has_pairing_result)
  {
    serverLog(LL_NOTICE, "blePairingResultErr.");
    blePairingResultErr(pairing_connection->pairing_result);
    pairing_err_result_return(arg);
  }
  if (commitBytes)
  {
    free(commitBytes);
  }
  if (payloadBytes)
  {
    free(payloadBytes);
  }
  ret = end_pairing_gatt(arg);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_pairing_gatt error");
  }
  else
  {
    serverLog(LL_NOTICE, "end_pairing_gatt success ✓✓✓");
  }
  return 1;
}

int write_pairing_step3(void *arg)
{
  serverLog(LL_NOTICE, "write_pairing_step3 start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = task_node->ble_data;
  pairing_connection_t *pairing_connection = 
                              (pairing_connection_t *)ble_data->ble_connection;
  void *step_data = pairing_connection->step_data;
  size_t step_max_size = pairing_connection->step_max_size;
  size_t n_size_byte = pairing_connection->n_size_byte;

  size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;
  uint8_t *step3Bytes;
  size_t step3Bytes_len = 0;

  step3Bytes_len = 
      igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
		        (step_max_size-n_size_byte), (step_data+n_size_byte), &step3Bytes);
  if (!step3Bytes_len)
  {
    serverLog(LL_NOTICE, "genPairingStep3Native error");
    goto STEP3_ERROR_EXIT;
  }
  serverLog(LL_NOTICE, "genPairingStep3Native Success");

  if (!build_msg_payload(
		&payloadBytes, &payload_len, step3Bytes, step3Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		goto STEP3_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "build_msg_payload Success");

  ret = write_char_by_uuid_multi_atts(
		pairing_connection->gatt_connection, &pairing_connection->pairing_uuid,
                                                     payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, 
                "write_char_by_uuid_multi_atts failed in writing th packags");
		goto STEP3_ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts Success");

  // 重设step_data等数据, 为了下次作准备
  if(pairing_connection->step_data)
  {
    free(pairing_connection->step_data);
    pairing_connection->step_data = NULL;
    pairing_connection->step_max_size = 0;
    pairing_connection->step_cur_size = 0;
    pairing_connection->n_size_byte = 0;
    serverLog(LL_NOTICE, "write_pairing_step3  reset step data success");
  }

  // 重设步骤
  pairing_connection->pairing_step = BLE_PAIRING_STEP3;

  free(step3Bytes);
  free(payloadBytes);
  return 0;

STEP3_ERROR_EXIT:
  // 释放产生的数据内存哦
  // 释放申请的内存
  serverLog(LL_ERROR, "write_pairing_step3 ERROR EXIT.");
  if (pairing_connection->has_pairing_result)
  {
    serverLog(LL_NOTICE, "blePairingResultErr.");
    blePairingResultErr(pairing_connection->pairing_result);
    pairing_err_result_return(arg);
  }
  if (step3Bytes)
  {
    free(step3Bytes);
  }

  if (payloadBytes)
  {
    free(payloadBytes);
  }
  ret = end_pairing_gatt(arg);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_pairing_gatt error");
  }
  else
  {
    serverLog(LL_NOTICE, "end_pairing_gatt success ✓✓✓");
  }
  return 1;
}

int write_pairing_step1(void *arg)
{
  serverLog(LL_NOTICE, "write_pairing_step1 start --------");
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

  if (!build_msg_payload(&payloadBytes, &payload_len, step1Bytes, step1Bytes_len))
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
	pairing_connection->pairing_step = BLE_PAIRING_STEP1;
  serverLog(LL_NOTICE, "write_pairing_step1 end --------");
  return 0;
STEP1_ERROR_EXIT:
  serverLog(LL_ERROR, "write_pairing_step1 ERROR EXIT.");
  if (pairing_connection->has_pairing_result)
  {
    serverLog(LL_NOTICE, "blePairingResultErr.");
    blePairingResultErr(pairing_connection->pairing_result);
    pairing_err_result_return(arg);
  }
  if (step1Bytes)
  {
    free(step1Bytes);
  }
  if (payloadBytes)
  {
    free(payloadBytes);
  }
  ret = end_pairing_gatt(arg);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_pairing_gatt error");
  }
  else
  {
    serverLog(LL_NOTICE, "end_pairing_gatt success ✓✓✓");
  }
}


int register_pairing_notfication(void *arg)
{
  serverLog(LL_NOTICE, "register_pairing_notfication start --------");
  int ret;
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
  ble_pairing_param_t *param = (ble_pairing_param_t *)(ble_data->ble_param);
  ble_data->ble_connection = calloc(sizeof(pairing_connection_t), 1);
  pairing_connection_t *pairing_connection = 
                            (pairing_connection_t *)ble_data->ble_connection;
  memset(pairing_connection, 0, sizeof(pairing_connection_t));

  // 返回的结果
  pairing_connection->has_pairing_result = 1;
  pairing_connection->pairing_result = calloc(sizeof(ble_pairing_result_t), 1);
  bleInitPairingResult(pairing_connection->pairing_result);
  bleSetPairingResultAddr(pairing_connection->pairing_result, 
                                      param->lock->addr, param->lock->addr_len);
  // char *adapter_name = NULL;
  // void *adapter = NULL;
  ble_data->adapter_name = NULL;
  ble_data->adapter = NULL;
  ret = gattlib_adapter_open(ble_data->adapter_name, &(ble_data->adapter));
  if (ret) {
		serverLog(LL_ERROR, 
      "ERROR: register_pairing_notfication Failed to open adapter.");
		return 1;
	}
  serverLog(LL_NOTICE, "register_pairing_notfication Success to open adapter.." );
  serverLog(LL_NOTICE, "register_pairing_notfication ready to connection %s",
                                                              param->lock->addr);
  pairing_connection->gatt_connection = gattlib_connect(
    NULL, param->lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (pairing_connection->gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		goto ERROR_EXIT;
	} 
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );
	

  if (
    gattlib_string_to_uuid(
      pairing_str, strlen(pairing_str), &(pairing_connection->pairing_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );
 

  gattlib_register_notification(
    pairing_connection->gatt_connection, message_handler, arg);
  ret = gattlib_notification_start(
      pairing_connection->gatt_connection, &pairing_connection->pairing_uuid);
  if (ret) {
    serverLog(LL_ERROR, "Fail to start notification.");
		goto ERROR_EXIT;
	}
  serverLog(LL_NOTICE, "success to start notification" );

  pairing_connection->pairing_step = BLE_PAIRING_STEP1;
  serverLog(LL_NOTICE, "register_pairing_notfication end --------");
  return 0;

ERROR_EXIT:
  serverLog(LL_ERROR, "register_pairing_notfication ERROR EXIT.");
  if (pairing_connection->has_pairing_result)
  {
    serverLog(LL_NOTICE, "blePairingResultErr.");
    blePairingResultErr(pairing_connection->pairing_result);
    pairing_err_result_return(arg);
  }
  if (pairing_connection->gatt_connection)
  {
    // gattlib_notification_stop(
    //   pairing_connection->gatt_connection, &(pairing_connection->pairing_uuid));
    // gattlib_disconnect(pairing_connection->gatt_connection);
    ret = end_pairing_gatt(arg);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "end_pairing_gatt error");
    }
    else
    {
      serverLog(LL_NOTICE, "end_pairing_gatt success ✓✓✓");
    }
  }
  // 
  // if (ble_data->ble_connection)
  // {
  //   free(ble_data->ble_connection);
  // }
  return 1;
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
  // lockCopy(pairing_param->lock, lock);
  serverLog(LL_NOTICE, "bleSetPairingParam sizeof %d", sizeof(igm_lock_t));
  memcpy(pairing_param->lock, lock, sizeof(igm_lock_t));
  return 0;
}

// --------------------------------------------------------------------

int bleInitPairingResult(ble_pairing_result_t *result)
{
  memset(result, 0, sizeof(ble_pairing_result_t));
  return 0;
}

int bleSetPairingSuccess(ble_pairing_result_t *pairing_result, int s)
{
  pairing_result->pairing_successed = s;
  return 0;
}

int bleReleaseResultAdminKey(ble_pairing_result_t *result)
{

  if (result && result->has_admin_key && result->admin_key)
  {
    result->has_admin_key = 0;
    result->admin_key_len = 0;
    free(result->admin_key);
    result->admin_key = NULL;
  }
  return 0;
}

int bleReleaseResultPassword(ble_pairing_result_t *result)
{
  if (result && result->has_password && result->password)
  {
    result->has_password = 0;
    result->password_size = 0;
    free(result->password);
    result->password = NULL;
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

int bleSetPairingResultAdminKey(ble_pairing_result_t *result, int has_admin_key,
  uint8_t *admin_key, int admin_key_len)
{
  bleReleaseResultAdminKey(result);
  result->has_admin_key = has_admin_key;
  if (result->has_admin_key)
  {
    result->admin_key_len = admin_key_len;
    result->admin_key = calloc(admin_key_len, 1);
    memcpy(result->admin_key, admin_key, admin_key_len);
  }
  else
  {
    result->admin_key_len = 0;
    result->admin_key = NULL;
  }
  return 0;
}

int bleSetPairingResultPassword(ble_pairing_result_t *result, int has_password,
  uint8_t *password, int password_size)
{
  bleReleaseResultPassword(result);
  result->has_password = has_password;
  if (result->has_password)
  {
    result->password_size = password_size;
    result->password = calloc(password_size, 1);
    memcpy(result->password, password, password_size);
  }
  else
  {
    result->password_size = 0;
    result->password = NULL;
  }
  return 0;
}

int bleGetPairingResultAdminKey(ble_pairing_result_t *result, 
  uint8_t *admin_key, int *p_admin_key_len)
{
  if (result->has_admin_key)
  {
    *p_admin_key_len = result->admin_key_len;
    memcpy(admin_key, result->admin_key, result->admin_key_len);
  }
  return 0;
}

int bleGetPairingResultPassword(ble_pairing_result_t *result, 
  uint8_t *password, int *p_password_size)
{
  if (result->has_password)
  {
    *p_password_size = result->password_size;
    memcpy(password, result->password, result->password_size);
  }
  return 0;
}

int bleSetPairingResultAddr(ble_pairing_result_t *result, 
  char *addr, size_t addr_len)
{
  memset(result->addr,0, MAX_DEVICE_ADDR);
  memcpy(result->addr, addr, addr_len>MAX_DEVICE_ADDR?MAX_DEVICE_ADDR:addr_len);
  return 0;
}

// 设置返回出错结果
int blePairingResultErr(ble_pairing_result_t *result)
{
  result->pairing_successed = 0;
  bleSetPairingSuccess(result, 0);
  bleSetPairingResultPassword(result, 0, NULL, 0);
  bleSetPairingResultAdminKey(result, 0, NULL, 0);
  return 0;
}

// --------------------------------------------------------------------
