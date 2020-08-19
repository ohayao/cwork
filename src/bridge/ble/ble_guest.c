#include <time.h> 
#include "bridge/ble/ble_guest.h"
#include "bridge/gattlib/gattlib.h"
#include "bridge/bridge_main/task.h"
#include "bridge/lock/connection/guest_connection.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/lock/messages/UnlockRequest.h"
#include "bridge/lock/messages/UnlockResponse.h"
#include "bridge/lock/messages/UnpairRequest.h"
#include "bridge/lock/messages/UnpairResponse.h"
#include "bridge/lock/messages/LockRequest.h"
#include "bridge/lock/messages/LockResponse.h"
#include "bridge/lock/messages/GetLogsRequest.h"
#include "bridge/lock/messages/GetLogsResponse.h"
#include "bridge/lock/messages/GetLockStatusRequest.h"
#include "bridge/lock/messages/GetLockStatusResponse.h"
#include "bridge/lock/messages/CreatePinRequest.h"
#include "bridge/lock/messages/CreatePinResponse.h"
#include "bridge/lock/messages/DeletePinRequest.h"
#include "bridge/lock/messages/DeletePinResponse.h"
#include "bridge/lock/messages/SetTimeRequest.h"
#include "bridge/lock/messages/SetTimeResponse.h"
#include "bridge/lock/messages/GetBatteryLevelRequest.h"
#include "bridge/lock/messages/GetBatteryLevelResponse.h"


static char guest_str[] = "5c3a65a0-897e-45e1-b016-007107c96df6";

// --------------------------- function declearation --------------------------

int guest_connection_and_do_cmd(void *arg);
static void message_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);
static int handle_step2_message(const uint8_t* data, int data_length,void* user_data);
static int save_message_data(const uint8_t* data, int data_length, void* user_data);
static int write_guest_step3(void *arg);
static int waiting_guest_step4(void *arg);
static int handle_step4_message(const uint8_t* data, int data_length,void* user_data);

// lock, , no memory leak
static int write_cmd_request(void *arg);
static int handle_lock_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_lock_result(void *arg);

// unlock, , no memory leak
static int write_unlock_request(void *arg);
static int handle_unlock_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_unlock_result(void *arg);

// Get Logs Request, OK, no memory leak
static int writeGetLogsRequest(void *arg);
static int waitingGetLogsResult(void *arg);
static int handleGetLogsResponce(const uint8_t* data, int data_length,void* user_data);

// Create Pin Request, no memory leak
static int writeCreatePinRequest(void *arg);
static int waitingCreatePinRequestResult(void *arg);
static int handleCreatePinResponce(const uint8_t* data, int data_length,void* user_data);

// Get Locks Status
static int writeGetLockStatusRequest(void *arg);
static int waitingGetLockStatusResult(void *arg);
static int handleGetLockStatusResponce(const uint8_t* data, int data_length,void* user_data);

// Delete pin Request, no memory leak
static int writeDeletePinRequest(void *arg);
static int waitingDeletePinRequestResult(void *arg);
static int handleDeletePinResponce(const uint8_t* data, int data_length,void* user_data);

// get battery level
static int writeGetBatteryLevelRequest(void *arg);
static int waitingGetBatteryLevelResult(void *arg);
static int handleGetBatteryLevelResponce(const uint8_t* data, int data_length,void* user_data);

// set time request
static int writeSetTimeRequest(void *arg);
static int waitingSetTimeResult(void *arg);
static int handleSetTimeResponce(const uint8_t* data, int data_length,void* user_data);

static int write_unpair_request(void *arg);
static int handle_unpair_responce(const uint8_t* data, int data_length,void* user_data);
static int waiting_unpair_result(void *arg);

enum {
  GUEST_SM_TABLE_LEN = 4
};

enum {
  GUEST_UNPAIR_SM_TABLE_LEN = 6
};

enum {
  GUEST_UNLOCK_SM_TABLE_LEN = 6
};

enum {
  GUEST_GET_LOGS_SM_TABLE_LEN = 6
};

enum {
  GUEST_GET_LOCK_STATUS_SM_TABLE_LEN = 6
};

enum {
  GUEST_CREATE_PIN_REQUEST_SM_TABLE_LEN = 6
};

enum {
  GUEST_DELETE_PIN_REQUEST_SM_TABLE_LEN = 6
};

enum {
  GUEST_GET_BATTERY_LEVEL_SM_TABLE_LEN = 6
};

enum {
  GUEST_SET_TIME_SM_TABLE_LEN = 6
};

enum {
  GUEST_LOCK_SM_TABLE_LEN = 3
};

// ------------------------- GuestConnection ---------------------------------

typedef struct GuestConnection {
	gatt_connection_t* gatt_connection;
	igm_lock_t *lock;
	size_t cmd_request_size;
	void *cmd_request;
	enum BLE_GUEST_STATE guest_step;
	size_t step_max_size;
	size_t  step_cur_size;
	size_t n_size_byte;
	uint8_t *step_data;
	uuid_t guest_uuid;
	//int has_ble_result;
	//ble_guest_result_t *ble_result;
	int waiting_err;
	int receive_err;
}guest_connection_t;

void initGuestConnection(guest_connection_t *guest_connection) {
	memset(guest_connection, 0, sizeof(guest_connection_t));
}

void copyGuestConnectionRequest(guest_connection_t *guest_connection, void *cmd_request, size_t cmd_request_size) {
	guest_connection->cmd_request = malloc(cmd_request_size);
	memset(guest_connection->cmd_request, 0, cmd_request_size);
	memcpy(guest_connection->cmd_request, cmd_request, cmd_request_size);
}

void releaseGuestConnectionData(guest_connection_t *guest_connection) {
	if (!guest_connection)
		return;
	if (guest_connection->step_max_size && guest_connection->step_data) {
		guest_connection->step_max_size = 0;
		guest_connection->n_size_byte = 0;
		guest_connection->step_cur_size = 0;
		free(guest_connection->step_data);
		guest_connection->step_data = NULL;
	}
}

int releaseGuestConnection(guest_connection_t **pp_guest_connection) {
	guest_connection_t *guest_connection = *pp_guest_connection;
	if (NULL == guest_connection) {
		return 0;
	}
	serverLog(LL_NOTICE, "in releaseGuestConnection.");
	int ret;
	if (guest_connection->cmd_request) {
		guest_connection->cmd_request_size = 0;
		free(guest_connection->cmd_request);
		guest_connection->cmd_request = NULL;
	}
	serverLog(LL_NOTICE, "in releaseGuestConnection, releaseLock.");
	releaseLock(&guest_connection->lock);
	serverLog(LL_NOTICE, "in releaseGuestConnection, bleReleaseGuestResult.");
	//bleReleaseGuestResult(&ble_data->ble_result);
	if(guest_connection->gatt_connection) {
		ret = gattlib_notification_stop(guest_connection->gatt_connection, &guest_connection->guest_uuid);
		if (ret != GATTLIB_SUCCESS) {
			serverLog(LL_ERROR, "clearGuestConnectionGattConenction gattlib_notification_stop error");
			return ret;
		}
		serverLog(LL_NOTICE, "before gattlib_disconnect, gatt_connection[%d].", guest_connection->gatt_connection);
		ret = gattlib_disconnect(guest_connection->gatt_connection);
		if (ret != GATTLIB_SUCCESS) {
			serverLog(LL_ERROR, " gattlib_disconnect error");
			return ret;
		}
		serverLog(LL_NOTICE, "after gattlib_disconnect, gatt_connection[%d]. will be NULL.", guest_connection->gatt_connection);
		guest_connection->gatt_connection = NULL;
	}
	serverLog(LL_NOTICE, "in releaseGuestConnection, free(*pp_guest_connection).");
	if(*pp_guest_connection) {
		free(*pp_guest_connection);
		*pp_guest_connection = NULL;
	}
	serverLog(LL_NOTICE, "in releaseGuestConnection end");
	return 0;
}

static int clearGuestConnectionStepData(guest_connection_t *guest_connection) {
	if (guest_connection->step_data && guest_connection->step_max_size) {
		guest_connection->step_max_size = 0;
		guest_connection->step_cur_size = 0;
		free(guest_connection->step_data);
		guest_connection->step_data = NULL;
	}
	return 0;
}

// --------------------------- waiting function -----------------------------

// memcheck 会超时, 所以设置大点调试
static size_t main_loop_timeout = 5;
static gboolean stop_main_loop_func(gpointer data) {
	serverLog(LL_ERROR, "stop_main_loop_func timeout");
	task_node_t *task_node = (task_node_t *)data;
	g_source_remove(task_node->timeout_id);
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	guest_connection->waiting_err = 1;
/*
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
*/
	g_main_loop_quit(task_node->loop);
	return FALSE;
}


void message_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
	serverLog(LL_NOTICE, "-------------new msg in message_handler---------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	char xbuf[1024] = {0};
	ByteToHexStr(data, xbuf, data_length);
	serverLog(LL_NOTICE,"get gatt notify, step[%d], data_len[%d], data[%s].", guest_connection->guest_step, data_length, xbuf);
	switch(guest_connection->guest_step) {
		case BLE_GUEST_BEGIN:
			{
				serverLog(LL_NOTICE, "in message_handler, BLE_GUEST_BEGIN do handle_step2_message");
				handle_step2_message(data, data_length, user_data);
				break;
			}
		case BLE_GUEST_STEP4:
			{
				serverLog(LL_NOTICE, "in message_handler, BLE_PAIRING_STEP4 do handle_step4_message");
				handle_step4_message(data, data_length, user_data);
				break;
			}
		case BLE_GUEST_CMD_RESULT:
			{
				serverLog(LL_NOTICE,"in message_handler, BLE_GUEST_CMD_REQUEST do handle_unlock_responce.");
				handle_cmd_responce(data, data_length, user_data);
				break;
			}
		default:
			{
				serverLog(LL_ERROR, "in message_handler, guest_connection->guest_step[%d] error.", guest_connection->guest_step);
				break;
			}
	}
}


#define ERR_EXIT(ERR, TARGET) \
	err = ERR;\
	goto TARGET;

void Close_Connection(guest_connection_t *guest_connection) {
	igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);
	// 释放 guest_connection 里面的数据
	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
	}
}


// 传入已经分配的资源:
// task_node->ble_data, 需要利用它返回结果给调用者
// task_node->ble_data->ble_param, 当前拷贝后立刻释放
// 自行分配的资源:
// task_node->ble_data->ble_connection 共享给所有步骤
// task_node->ble_data->ble_connection->ble_result 需要共享
// ble_data->adapter, 保存了当前的蓝牙
// task_node->ble_data->ble_connection->gatt_connection, 保存当前的gatt连接,共享
// task_node->loop: 共享
// gattlib_register_notification, 共享
// 出错释放:
// ble_data 返回出错结果
// ble_connection 释放所有里面的内容
// gattlib_notification_stop
// ble_data->adapter close
// task_node->loop
int guest_connection_and_do_cmd(void *arg) {
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd start --------");
	int ret = 0, err = 0, finish = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	sysinfo_t* si = task_node->sysif;
	task_node->loop = NULL;

	ble_guest_param_t *param = (ble_guest_param_t *)(ble_data->ble_param);
	// 分配 connection, 传递到其他函数的数据,
	ble_data->ble_connection = malloc(sizeof(guest_connection_t));
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	initGuestConnection(guest_connection);

	guest_connection->lock = param->lock;
	//getLock(&guest_connection->lock);
	//copyLock(guest_connection->lock, param->lock);

	copyGuestConnectionRequest(guest_connection, param->cmd_request, param->cmd_request_size);
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd release guest param");
	//bleReleaseGuestParam(&param);

	serverLog(LL_NOTICE, "guest_connection_and_do_cmd apply for a ble_guest_result_t, need user to release");
	/*
	guest_connection->has_ble_result = 1;
	ble_data->ble_result = malloc(sizeof(ble_guest_result_t));
	initGuestResult(ble_data->ble_result);
	setGuestResultAddr(ble_data->ble_result, guest_connection->lock->addr, guest_connection->lock->addr_len);
	*/
	ble_data->ble_result_len = sizeof(ble_guest_result_t);
	ble_data->ble_result = malloc(ble_data->ble_result_len);
	initGuestResult(ble_data->ble_result);

	//ble_data->adapter_name = NULL;
	//ble_data->adapter = NULL;

	if(NULL == si->ble_adapter) {
		ret = gattlib_adapter_open(NULL, &(si->ble_adapter));
		if (ret) {
			serverLog(LL_ERROR, "ERROR: guest_connection_and_do_cmd Failed to open adapter.");
			//goto GUEST_ERROR_EXIT;
		}
	}

	serverLog(LL_NOTICE,"in guest_connection_and_do_cmd ready to connection adapter[%d], lockaddr[%s].", si->ble_adapter, guest_connection->lock->addr);
	//optimise this short connection to long!
	if(NULL == guest_connection->lock->addr){
		serverLog(LL_ERROR, "lock addr is NULL err." );
		ERR_EXIT(ERR_GATT, GUEST_ERROR_EXIT)
	}
	guest_connection->gatt_connection = gattlib_connect(si->ble_adapter, guest_connection->lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (NULL == guest_connection->gatt_connection) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		ERR_EXIT(ERR_GATT, GUEST_ERROR_EXIT)
	}
	serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device, gatt_connection[%d].", guest_connection->gatt_connection );
	if (gattlib_string_to_uuid( guest_str, strlen(guest_str), &(guest_connection->guest_uuid))<0) {
		serverLog(LL_ERROR, "gattlib_string_to_uuid to guest_uuid fail");
		ERR_EXIT(ERR_GATT, GUEST_ERROR_EXIT)
	}
	//serverLog(LL_NOTICE, "gattlib_string_to_uuid to guest_uuid[%02x] success.", guest_connection->guest_uuid);
	serverLog(LL_NOTICE,"gattlib_string_to_uuid to guest_uuid[%02x] success.", guest_connection->guest_uuid);
	//*/

	gattlib_register_notification(guest_connection->gatt_connection, message_handler, arg);
	ret = gattlib_notification_start(guest_connection->gatt_connection, &guest_connection->guest_uuid);
	if (ret) {
		serverLog(LL_ERROR, "gattlib_notification_start err[%d].", ret);
		ERR_EXIT(ERR_GATT, GUEST_ERROR_EXIT)
	}
	serverLog(LL_NOTICE, "success to start notification" );

	guest_connection->guest_step = BLE_GUEST_BEGIN;
	task_node->loop = g_main_loop_new(NULL, 0);
	if(NULL == task_node->loop) {
		serverLog(LL_ERROR, "g_main_loop_new return NULL.");
		ERR_EXIT(ERR_GLIB_LOOP, GUEST_ERROR_EXIT)
	}
	if (main_loop_timeout>0) {
		task_node->timeout_id = g_timeout_add_seconds(main_loop_timeout, stop_main_loop_func, arg);
	}

	serverLog(LL_NOTICE, "write_guest_step1 begin");
	ret = write_guest_step1(task_node);
	if(ret) {
		serverLog(LL_ERROR, "write_guest_step1 err[%d].", ret);
		ERR_EXIT(ERR_STEP1, GUEST_ERROR_EXIT)
	}

	serverLog(LL_NOTICE, "waite_guest_step2 begin");
	//step 2
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err) {
		serverLog(LL_ERROR, "write_guest_step2 err, waiting_err[%d]. receive_err[%d].", guest_connection->waiting_err, guest_connection->receive_err);
		ERR_EXIT(ERR_WAITING, GUEST_ERROR_EXIT)
	}
	serverLog(LL_NOTICE, "after g_main_loop_run.");


GUEST_ERROR_EXIT:
	setGuestResultErr(ble_data->ble_result, err);
		// 需要设置错误结果给 guest_connection, 拷贝这个结果到ble_data
		//bleSetBleResult(ble_data, ble_data->ble_result);
	Close_Connection(guest_connection);
	if (task_node->loop) {
		g_main_loop_unref(task_node->loop);
		task_node->loop = NULL;
	}
	g_source_remove(task_node->timeout_id);
	return err;
}

int write_guest_step1(void *arg) {
	serverLog(LL_NOTICE, "write_guest_step1 start --------");
	int ret = 0, err = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	size_t step1Bytes_len = 0, payload_len = 0;
	uint8_t *step1Bytes = NULL;
	uint8_t *payloadBytes = NULL;
	int step1Len = 0;
	int connectionID = 0;

	serverLog(LL_NOTICE,"in write_guest_step1, do igloohome_ble_lock_crypto_GuestConnection_beginConnection, key[%02x],key_len[%d].", guest_connection->lock->key, guest_connection->lock->key_len);
	connectionID = igloohome_ble_lock_crypto_GuestConnection_beginConnection(guest_connection->lock->key, guest_connection->lock->key_len);
	if (ERROR_CONNECTION_ID == connectionID) {
		serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_beginConnection error[%d], key[%s], ken_len[%d]", ret, guest_connection->lock->key, guest_connection->lock->key_len);
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP1_EXIT)
	}
	serverLog(LL_NOTICE,"igloohome_ble_lock_crypto_GuestConnection_beginConnection success. connectionID[%d]", connectionID);
	setLockConnectionID(guest_connection->lock, connectionID);
	
	step1Len = igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native(
			connectionID,
			guest_connection->lock->token,
			guest_connection->lock->token_len,
			&step1Bytes);
	if (!step1Len) {
		serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native err");
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP1_EXIT)
	}
	serverLog(LL_NOTICE,"igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native success, connectionID[%d], ret step1Len[%d], token[%02x], token_len[%d].", connectionID, step1Len, guest_connection->lock->token, guest_connection->lock->token_len);

	if (!build_msg_payload(&payloadBytes, &payload_len, step1Bytes, step1Len)) {
		serverLog(LL_ERROR, "failed in build_msg_payload.");
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP1_EXIT)
	}
	serverLog(LL_NOTICE,"build_msg_payload success.");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection,
			&guest_connection->guest_uuid,
			payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts err[%d]", ret);
		ERR_EXIT(ERR_SEND, GUEST_STEP1_EXIT)
	}
	serverLog(LL_NOTICE,"write_char_by_uuid_multi_atts ret[%d] succ, payload_len[%d].", ret, payload_len);

	//guest_connection->guest_step = BLE_GUEST_STEP3;

GUEST_STEP1_EXIT:
	if(step1Bytes){
		free(step1Bytes);
		step1Bytes = NULL;
	}
	if(payloadBytes) {
		free(payloadBytes);
		payloadBytes = NULL;
	}
	if(err) {
		//setGuestResultErr(ble_data->ble_result, err);
		//Close_Connection(guest_connection);
		return err;
	}

	return 0;
}


int handle_step2_message(const uint8_t* data, int data_length,void* user_data) {
	serverLog(LL_NOTICE, "handle_step2_message");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	//******
	//g_main_loop_quit(task_node->loop);

	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	save_message_data(data, data_length, user_data);

	serverLog(LL_NOTICE,"in handle_step2_message step_max_size[%d], step_cur_size[%d]", guest_connection->step_max_size, guest_connection->step_cur_size);
	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		serverLog(LL_NOTICE, "handle_step2_message RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_STEP3;
		//*****
		int ret = write_guest_step3(task_node);
		if(ret){
			guest_connection->receive_err = 1;
			g_main_loop_quit(task_node->loop);
			return ret;
		}
	}else{
		serverLog(LL_NOTICE,"handle_step2_message recv Err, step_max_size[%d], step_cur_size[%d].", guest_connection->step_max_size, guest_connection->step_cur_size);
	}
	return 0;
}

int save_message_data(const uint8_t* data, int data_length, void* user_data) {
    task_node_t *task_node = (task_node_t *)user_data;
    ble_data_t *ble_data = task_node->ble_data;
    guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	serverLog(LL_NOTICE,"in save_message_data head, data_length[%d], step_max_size[%d], step_cur_size[%d].", data_length, guest_connection->step_max_size, guest_connection->step_cur_size);

    // 如果没有分配内存                            
    if (0 == guest_connection->step_max_size) {
        if (data_length < 3) {
            serverLog(LL_ERROR, "data_length < 3, can't get len");
            return 1;
        } else {
            if (data[2] == 0xff) {
                guest_connection->n_size_byte = 3;
                guest_connection->step_max_size = data[0] * (0xfe) + data[1] + guest_connection->n_size_byte;
                serverLog(LL_NOTICE, "2 bytes lenth %d", guest_connection->step_max_size);
            } else {
                serverLog(LL_NOTICE, "data[0] %d, %02x", data[0], data[0]);
                guest_connection->n_size_byte = 1;
                guest_connection->step_max_size = data[0] + guest_connection->n_size_byte;
            }
            guest_connection->step_cur_size = 0;
            guest_connection->step_data = (uint8_t *)malloc(guest_connection->step_max_size);
            if (!(guest_connection->step_data)) {
                serverLog(LL_ERROR, "save_message_data malloc err");
                return 1;
            }
        }
    }

    // 帕不够
    //int size_left = guest_connection->step_max_size - guest_connection->step_cur_size;
    //if (size_left < data_length) {
    //    serverLog(LL_NOTICE, "in save_message_data size_left[%d] < data_length[%d]..", size_left , data_length);
    //    guest_connection->step_max_size += 85;
	//	guest_connection->step_data = (uint8_t *)realloc(guest_connection->step_data, guest_connection->step_max_size);
	/*
        uint8_t *old_data = guest_connection->step_data;
        guest_connection->step_data = (uint8_t *)malloc(guest_connection->step_max_size);
        if (!guest_connection->step_data) {
            serverLog(LL_ERROR, "save_message_data malloc err");
            return 4;
        }
        memcpy( guest_connection->step_data, old_data, guest_connection->step_cur_size);
        free(old_data);
        old_data = NULL;
	*/
    //}

    // 空间肯定足够, 直接放下空间里面
    for (int j = 0; j < data_length; ) {
        guest_connection->step_data[guest_connection->step_cur_size++] = data[j++];
    }
	serverLog(LL_NOTICE,"in save_message_data tail, data_length[%d], step_max_size[%d], step_cur_size[%d].", data_length, guest_connection->step_max_size, guest_connection->step_cur_size);
	return 0;
}


int write_guest_step3(void *arg) {
    serverLog(LL_NOTICE, "write_guest_step3 start --------");
    int ret=0, err=0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	size_t step2Bytes_len = 0, payload_len = 0;
	uint8_t *step2Bytes = NULL;
	uint8_t *payloadBytes = NULL;
	int step2Len = 0;
	int connectionID = 0;

	step2Len = igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native(
			connectionID, 
			guest_connection->step_data + guest_connection->n_size_byte, 
			guest_connection->step_max_size - guest_connection->n_size_byte, 
			&step2Bytes);
	if (!step2Len) {
		serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native err");
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP3_EXIT)
	}
	serverLog(LL_NOTICE,"igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native success.");

	if (!build_msg_payload(&payloadBytes, &payload_len, step2Bytes, step2Len)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP3_EXIT)
	}
	serverLog(LL_NOTICE, "build_msg_payload success");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts err[%d]", ret);
		ERR_EXIT(ERR_CONNECTION, GUEST_STEP3_EXIT)
	}
	serverLog(LL_NOTICE,"in write_guest_step3 write_char_by_uuid_multi_atts ret[%d], payload_len[%d].", ret, payload_len);

	guest_connection->guest_step = BLE_GUEST_STEP4;
	releaseGuestConnectionData(guest_connection);

GUEST_STEP3_EXIT:
	if(step2Bytes) {
		free(step2Bytes);
		step2Bytes = NULL;
	}
	if(payloadBytes) {
		free(payloadBytes);
		payloadBytes = NULL;
	}
	//releaseGuestConnectionData(guest_connection);

	return err;
}

static int waiting_guest_step4(void *arg) {
	serverLog(LL_NOTICE, "waiting_guest_step4");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	serverLog(LL_NOTICE, "waiting_guest_step4 new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_STEP4_ERROR;
	serverLog(LL_NOTICE, "waiting_guest_step4 end loop waiting, BLE_GUEST_DONE");
	guest_connection->guest_step = BLE_GUEST_DONE;
	return 0;

WAITING_STEP4_ERROR:
	serverLog(LL_ERROR, "WAITING_STEP4_ERROR");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	//releaseGuestConnectionData(guest_connection);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_guest_step4 releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "waiting_guest_step4 releaseGuestConnection success");
/*
	ret = gattlib_adapter_close(ble_data->adapter);
	if (ret) {
		serverLog(LL_ERROR, "gattlib_adapter_close error ");
		return ret;
	}*/
	return 1;
}

int handle_step4_message(const uint8_t* data, int data_length,void* user_data) {
	serverLog(LL_NOTICE, "handle_guest_step4_message");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	int ret = 0, err = 0;
	//******

	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		serverLog(LL_NOTICE, "handle_step4_message RECV step3 data finished[%d]", guest_connection->step_max_size);
		guest_connection->guest_step = BLE_GUEST_STEP4;

		int rec_ret = igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native(
				(guest_connection->lock)->connectionID, 
				guest_connection->step_data + guest_connection->n_size_byte, 
				guest_connection->step_max_size-guest_connection->n_size_byte
				);

		if (!rec_ret){
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native err");
			guest_connection->receive_err = 1;
			ERR_EXIT(ERR_CONNECTION, STEP4_EXIT)
		}
		serverLog(LL_NOTICE,"igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native success");
		setGuestResultErr(ble_data->ble_result, 0);
		// 返回参数给调用进程
		serverLog(LL_NOTICE, "ble connection finish.");

		//do CMD
			ret = write_cmd_request(task_node);
			releaseGuestConnectionData(guest_connection);
	
	}else{
		serverLog(LL_ERROR, "step_max_size[%d] != step_cur_size[%d], continue recv!", guest_connection->step_max_size, guest_connection->step_cur_size);
	}

STEP4_EXIT:
	setGuestResultErr(ble_data->ble_result, err);
	//g_main_loop_quit(task_node->loop);
	return err;
}

static int write_cmd_request(void *arg) {
	serverLog(LL_NOTICE, "write_lock_request start --------");
	int ret = 0, err = 0;
	task_node_t* task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	srand(time(0));
	int requestID = rand() % 2147483647;
	size_t buf_size = 128;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	if(TASK_BLE_GUEST_LOCK == task_node->task_type){
		//encode without encryption
		IgLockRequest lock_request;
		ig_LockRequest_init(&lock_request);
		ig_LockRequest_set_operation_id(&lock_request, requestID);
		ig_LockRequest_set_password(&lock_request, guest_connection->lock->password, guest_connection->lock->password_size);
		IgSerializerError IgErr = ig_LockRequest_encode(&lock_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_LockRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
	} else if(TASK_BLE_GUEST_UNLOCK == task_node->task_type){
		IgUnlockRequest unlock_request;
		ig_UnlockRequest_init(&unlock_request);
		ig_UnlockRequest_set_operation_id(&unlock_request, requestID);
		ig_UnlockRequest_set_password(&unlock_request, guest_connection->lock->password, guest_connection->lock->password_size);
		//encode without encryption
		IgSerializerError IgErr = ig_UnlockRequest_encode(&unlock_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_UnpairRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_GuestUnlockRequest_encode success." );
	} else if (TASK_BLE_GUEST_GETLOCKSTATUS == task_node->task_type) {
		IgGetLockStatusRequest getlockstatus_request;
		ig_GetLockStatusRequest_init(&getlockstatus_request);
		ig_GetLockStatusRequest_set_operation_id(&getlockstatus_request, requestID);
		ig_GetLockStatusRequest_set_password(&getlockstatus_request, guest_connection->lock->password, guest_connection->lock->password_size);

		IgSerializerError IgErr = ig_GetLockStatusRequest_encode(&getlockstatus_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_GetLockStatusRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_GetLockStatusRequest_encode success size:[%d].", encode_size);
	} else if (TASK_BLE_GUEST_GET_BATTERY_LEVEL == task_node->task_type) {
		IgGetBatteryLevelRequest bl_request;
		ig_GetBatteryLevelRequest_init(&bl_request);
		serverLog(LL_NOTICE, "requestID: %d", requestID);
		ig_GetBatteryLevelRequest_set_operation_id( &bl_request, requestID);
		ig_GetBatteryLevelRequest_set_password( &bl_request, guest_connection->lock->password, guest_connection->lock->password_size);
		if (!ig_GetBatteryLevelRequest_is_valid(&bl_request)) {
			serverLog(LL_ERROR, "bl_request invalid");
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}

		IgSerializerError IgErr = ig_GetBatteryLevelRequest_encode( &bl_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_GetBatteryLevelRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_GetBatteryLevelRequest_encode success size[%d].", encode_size);

	} else if (TASK_BLE_GUEST_GETLOGS == task_node->task_type) {
		time_t cur_timestamp = time(NULL);
		IgGetLogsRequest getlogs_request;
		ig_GetLogsRequest_init(&getlogs_request);
		ig_GetLogsRequest_set_operation_id(&getlogs_request, requestID);
		ig_GetLogsRequest_set_password(&getlogs_request, guest_connection->lock->password, guest_connection->lock->password_size);
		ig_GetLogsRequest_set_timestamp(&getlogs_request, cur_timestamp);
		IgSerializerError IgErr = ig_GetLogsRequest_encode(&getlogs_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_GetLogsRequest_encode err.");
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_GetLogsRequest_encode success size[%d].", encode_size);

	} else if (TASK_BLE_GUEST_CREATE_PIN == task_node->task_type) {
		IgCreatePinRequest *cmd_req = guest_connection->cmd_request;
		IgCreatePinRequest create_pin_request;
		ig_CreatePinRequest_init(&create_pin_request);
		serverLog(LL_NOTICE, "requestID[%d], cmd_req->operation_id[%d]; guest_connection->lock->password[%s], cmd_req->password[%s].",
			requestID, cmd_req->operation_id, guest_connection->lock->password, cmd_req->password);

		ig_CreatePinRequest_set_operation_id( &create_pin_request, requestID);
		ig_CreatePinRequest_set_password(&create_pin_request, guest_connection->lock->password, guest_connection->lock->password_size);
		//ig_CreatePinRequest_set_password(&create_pin_request, cmd_req->password, cmd_req->password_size);
		if (!cmd_req ||  !ig_CreatePinRequest_is_valid(cmd_req)) {
			serverLog(LL_ERROR, "cmd_req NULL or cmd_req don't have pin");
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}

		ig_CreatePinRequest_set_new_pin(&create_pin_request, cmd_req->new_pin, cmd_req->new_pin_size);
		//@@@test
		char xbuf[1024] = {0};
		ByteToHexStr(create_pin_request.new_pin, xbuf, create_pin_request.new_pin_size);
		serverLog(LL_NOTICE,"ig_CreatePinRequest_set_new_pin:[%s].", xbuf);

		//@@@test
		memset(xbuf, 0x0, sizeof(xbuf));
		ByteToHexStr(create_pin_request.password, xbuf, create_pin_request.password_size);
		serverLog(LL_NOTICE,"ig_CreatePinRequest_set_password:[%s]", xbuf);

		// optionnal
		if (cmd_req->has_start_date) {
			ig_CreatePinRequest_set_start_date( &create_pin_request, cmd_req->start_date);
			serverLog(LL_NOTICE, "ig_CreatePinRequest_set_start_date [%d].", create_pin_request.start_date);
		}

		if (cmd_req->has_end_date) {
			ig_CreatePinRequest_set_end_date( &create_pin_request, cmd_req->end_date);
			serverLog(LL_NOTICE, "ig_CreatePinRequest_set_end_date [%d].", create_pin_request.end_date);
		}

		if (cmd_req->has_pin_type) {
			ig_CreatePinRequest_set_pin_type( &create_pin_request, cmd_req->pin_type);
			serverLog(LL_NOTICE, "ig_CreatePinRequest_set_pin_type [%d].", create_pin_request.pin_type);
		}

		if (cmd_req->has_operation_id) {
			ig_CreatePinRequest_set_operation_id(&create_pin_request, cmd_req->operation_id);
			serverLog(LL_NOTICE, "ig_CreatePinRequest_set_operation_id [%d].", create_pin_request.operation_id);
		}
		//@@test
		serverLog(LL_NOTICE,"create pin option: start_date[%u], end_date[%u], pin_type[%d], operation_id[%d].", 
				create_pin_request.start_date,
				create_pin_request.end_date,
				create_pin_request.pin_type,
				create_pin_request.operation_id);

		IgSerializerError IgErr = ig_CreatePinRequest_encode(&create_pin_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_CreatePinRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_CreatePinRequest_encode success size[%d].", encode_size);
	} else if (TASK_BLE_GUEST_DEL_PIN == task_node->task_type) {
		IgDeletePinRequest *cmd_req = guest_connection->cmd_request;
		IgDeletePinRequest delete_pin_request;
		ig_DeletePinRequest_init(&delete_pin_request);
		ig_DeletePinRequest_set_operation_id( &delete_pin_request, requestID);
		ig_DeletePinRequest_set_password( &delete_pin_request, guest_connection->lock->password, guest_connection->lock->password_size);
		if (!cmd_req||  !ig_DeletePinRequest_is_valid(cmd_req)) {
			serverLog(LL_ERROR, "cmd_req NULL or cmd_req don't have pin.");
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}

		ig_DeletePinRequest_set_old_pin( &delete_pin_request, cmd_req->old_pin, cmd_req->old_pin_size);

		if (cmd_req->has_operation_id) {
			ig_DeletePinRequest_set_operation_id(&delete_pin_request, cmd_req->operation_id);
		}

		IgSerializerError IgErr = ig_DeletePinRequest_encode( &delete_pin_request, buf, buf_size, &encode_size);
		if (IgErr) {
			serverLog(LL_ERROR, "ig_DeletePinRequest_encode err[%d].", IgErr);
			ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
		}
		serverLog(LL_NOTICE, "ig_DeletePinRequest_encode success size[%d].", encode_size);
	} else {
		serverLog(LL_ERROR, "cmd task_type[%d] err.", task_node->task_type);
		ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
	}
	guest_connection->guest_step = BLE_GUEST_CMD_RESULT;
	serverLog(LL_NOTICE, "ig cmd[%d] Request_encode success.", task_node->task_type);

	//@@@test
	char xbuf[1024] = {0};
	ByteToHexStr(buf, xbuf, encode_size);
	serverLog(LL_NOTICE,"ig_LockRequest_encode success, buf:[%s].", xbuf);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(
			guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
	}
	serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_encryptNative success");

	if (!build_msg_payload(&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
	}
	//@@@test
	memset(xbuf,0x0,sizeof(xbuf));
	ByteToHexStr(encryptPayloadBytes, xbuf, encryptPayloadBytes_len);
	serverLog(LL_NOTICE, "build_msg_payload success, encryptPayloadBytes:[%s].", xbuf);

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		ERR_EXIT(ERR_CMD, LOCK_REQUEST_ERROR)
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success.");

LOCK_REQUEST_ERROR:
	setGuestResultErr(ble_data->ble_result, err);
//	ig_LockRequest_deinit(&lock_request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}

	return err;
}

int handle_cmd_responce(const uint8_t* data, int data_length, void* user_data) {
	serverLog(LL_NOTICE,"in handle_lock_responce -------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	int err = 0;
	int responceLen;
	uint8_t *responceBytes = NULL;

	//guest_connection->guest_step = BLE_GUEST_LOCK_RESULT;
	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		serverLog(LL_NOTICE, "handle_cme_responce RECV step2 data finished");

		size_t messageLen = guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(
				guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen) { 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen %d", responceLen);

		if(TASK_BLE_GUEST_LOCK == task_node->task_type){
			IgLockResponse guest_lock_responce;
			ig_LockResponse_init(&guest_lock_responce);
			IgSerializerError err = ig_LockResponse_decode( responceBytes, responceLen, &guest_lock_responce, 0);
			if (err) {
				serverLog(LL_ERROR, "ig_GuestLockResponse_decode err [%d].", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}
			serverLog(LL_NOTICE, "has lock response [%d] result [%d]", guest_lock_responce.has_result, guest_lock_responce.result);

			if (guest_lock_responce.has_result) {
				serverLog(LL_NOTICE, "set guest result to success");
				ble_data->ble_result->result= guest_lock_responce.result;
			}
			setGuestResultCMDResponse( ble_data->ble_result, &guest_lock_responce, sizeof(IgLockResponse));
		} else if (TASK_BLE_GUEST_UNLOCK == task_node->task_type) {
			IgUnlockResponse guest_unlock_responce;
			ig_UnlockResponse_init(&guest_unlock_responce);
			IgSerializerError err = ig_UnlockResponse_decode( responceBytes, responceLen, &guest_unlock_responce, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_GuestUnlockResponse_decode err %d", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}
			serverLog(LL_NOTICE, "has unlock response [%d] error [%d].", guest_unlock_responce.has_result, guest_unlock_responce.result);

			if (guest_unlock_responce.has_result) {
				serverLog(LL_NOTICE, "set guest result to success.");
				ble_data->ble_result->result = guest_unlock_responce.result;
			}
			setGuestResultCMDResponse( ble_data->ble_result, &guest_unlock_responce, sizeof(IgUnlockResponse));
		} else if (TASK_BLE_GUEST_GETLOCKSTATUS == task_node->task_type) {
			IgGetLockStatusResponse status_resp;
			ig_GetLockStatusResponse_init(&status_resp);
			IgSerializerError err = ig_GetLockStatusResponse_decode(responceBytes, responceLen, &status_resp, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err[%d]", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}

			serverLog(LL_NOTICE, "get status resp: has_result[%d],error[%d],has_lock_open[%d],lock_open[%d],has_cell_network_status[%d],cell_network_status[%d], has_door_open[%d], door_open[%d], has_operation_id[%d], operation_id[%d].",
					status_resp.has_result, 
					status_resp.result,
					status_resp.has_lock_open,
					status_resp.lock_open,
					status_resp.has_cell_network_status,
					status_resp.cell_network_status,
					status_resp.has_door_open,
					status_resp.door_open,
					status_resp.has_operation_id,
					status_resp.operation_id);
			if (status_resp.has_result) {
				serverLog(LL_NOTICE, "set guest result to success");
				ble_data->ble_result->result = status_resp.result;
			}

			if (status_resp.has_lock_open) {
				BleSetStatusRes(ble_data, status_resp.lock_open); 
			}
		} else if (TASK_BLE_GUEST_GET_BATTERY_LEVEL == task_node->task_type) {
			IgGetBatteryLevelResponse responce;
			ig_GetBatteryLevelResponse_init(&responce);
			IgSerializerError err = ig_GetBatteryLevelResponse_decode( responceBytes, responceLen, &responce, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err %d", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}

			serverLog(LL_NOTICE, "has get battery level response %d error %d", responce.has_result, responce.result);
			if (responce.has_result) {
				serverLog(LL_NOTICE, "get battery result[%d]", responce.result);
				ble_data->ble_result->result = responce.result;
			}
			if (responce.has_battery_level){
				serverLog(LL_NOTICE, "get battery level[%d]", responce.battery_level);
				BleSetBatteryLRes(ble_data, responce.battery_level); 
			}
			setGuestResultCMDResponse( ble_data->ble_result, &responce, sizeof(IgDeletePinResponse));
		} else if (TASK_BLE_GUEST_GETLOGS == task_node->task_type) {
			IgGetLogsResponse guest_getlogs_responce;
			ig_GetLogsResponse_init(&guest_getlogs_responce);
			IgSerializerError err = ig_GetLogsResponse_decode(responceBytes, responceLen, &guest_getlogs_responce, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_GetLogsResponse_decode err [%d].", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}
			serverLog(LL_NOTICE, "get log info has_result[%d], result[%d], has_data[%d], data_size[%d].", guest_getlogs_responce.has_result, guest_getlogs_responce.result, guest_getlogs_responce.has_data, guest_getlogs_responce.data_size);

			if (guest_getlogs_responce.has_result) {
				serverLog(LL_NOTICE, "set guest result to success.");
				ble_data->ble_result->result= guest_getlogs_responce.result;
			}
			setGuestResultCMDResponse( ble_data->ble_result, &guest_getlogs_responce, sizeof(IgGetLogsResponse));

		} else if (TASK_BLE_GUEST_CREATE_PIN == task_node->task_type) {
			IgCreatePinResponse guest_create_pin_responce;
			ig_CreatePinResponse_init(&guest_create_pin_responce);
			IgSerializerError err = ig_CreatePinResponse_decode( responceBytes, responceLen, &guest_create_pin_responce, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err[%d].", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}

			serverLog(LL_NOTICE, "has create pin result[%d], result[%d].", guest_create_pin_responce.has_result, guest_create_pin_responce.result);
			if (guest_create_pin_responce.has_result) {
				serverLog(LL_NOTICE, "set guest result to success");
				ble_data->ble_result->result = guest_create_pin_responce.result;
			}
			setGuestResultCMDResponse( ble_data->ble_result, &guest_create_pin_responce, sizeof(IgCreatePinResponse));
		} else if (TASK_BLE_GUEST_DEL_PIN == task_node->task_type) {
			IgDeletePinResponse responce;
			ig_DeletePinResponse_init(&responce);
			IgSerializerError err = ig_DeletePinResponse_decode( responceBytes, responceLen, &responce, 0);
			if (err) {
				serverLog(LL_NOTICE, "ig_DeletePinResponse_decode err[%d].", err);
				guest_connection->receive_err = 1;
				ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
			}

			serverLog(LL_NOTICE, "has delete pin has result[%d], result[%d].", responce.has_result, responce.result);
			if (responce.has_result) {
				ble_data->ble_result->result = responce.result;
			}
			setGuestResultCMDResponse( ble_data->ble_result, &responce, sizeof(IgDeletePinResponse));
		} else {
			serverLog(LL_ERROR, "cmd task_type[%d] err.", task_node->task_type);
			ERR_EXIT(ERR_CMD, LOCK_RESPONCE_EXIT)
		}

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");
	} else {
		serverLog(LL_ERROR, "step_max_size[%d], step_cur_size[%d], do recv again.", guest_connection->step_max_size, guest_connection->step_cur_size);
		return 0;
	}

LOCK_RESPONCE_EXIT:
	setGuestResultErr(ble_data->ble_result, err);
	if (responceBytes) {
		free(responceBytes);
		responceBytes = NULL;
	}
	serverLog(LL_NOTICE,"@@@ in handle_lock_responce EXIT do g_main_loop_quit, should no time out!");
	g_main_loop_quit(task_node->loop);

	return err;
}

/*
//  ------------------------ Guest step start ------------------------


int waiting_guest_step2(void *arg) {
	serverLog(LL_NOTICE, "waiting_pairing_step2.");
	task_node_t *task_node = (task_node_t *) arg;
	ble_data_t *ble_data = (ble_data_t *) (task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *) ble_data->ble_connection;

	serverLog(LL_NOTICE, "waiting_guest_step2 new loop waiting.");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_STEP2_ERROR;
	serverLog(LL_NOTICE, "waiting_guest_step2 exit task_node->loop");
	return 0;

WAITING_STEP2_ERROR:
	// 这儿, 需要释放订阅,释放数据
	serverLog(LL_ERROR, "WAITING_STEP2_ERROR ");
	g_main_loop_unref(task_node->loop);
	releaseGuestConnectionData(guest_connection);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_guest_step2 releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "waiting_guest_step2 releaseGuestConnection success");

	return 1;
}
fsm_table_t guest_fsm_table[GUEST_SM_TABLE_LEN] = {
  {BLE_GUEST_BEGIN,     guest_connection_and_do_cmd,		BLE_GUEST_STEP2},
  {BLE_GUEST_STEP2,     waiting_guest_step2,			BLE_GUEST_STEP3},
  {BLE_GUEST_STEP3,     write_guest_step3,				BLE_GUEST_STEP4},
  {BLE_GUEST_STEP4,		waiting_guest_step4,		BLE_GUEST_DONE},
};

// ------------------------ lock --------------------------------

fsm_table_t guest_lock_fsm_table[GUEST_LOCK_SM_TABLE_LEN] = {
  {BLE_GUEST_BEGIN,         guest_connection_and_do_cmd,	BLE_GUEST_STEP2},
  {BLE_GUEST_STEP2,         waiting_guest_step2,        BLE_GUEST_STEP3},
  {BLE_GUEST_STEP3,         write_guest_step3,          BLE_GUEST_STEP4},
  {BLE_GUEST_STEP4,			waiting_guest_step4,        BLE_GUEST_LOCK_REQUEST},
  {BLE_GUEST_LOCK_REQUEST,	write_lock_request,         BLE_GUEST_LOCK_RESULT},
  {BLE_GUEST_LOCK_RESULT,   waiting_lock_result,        BLE_GUEST_LOCK_DONE},
};

fsm_table_t *getGuestLockFsmTable() {
  return guest_lock_fsm_table;
}

int getGuestLockFsmTableLen() {
  return GUEST_LOCK_SM_TABLE_LEN;
}

static int waiting_lock_result(void *arg) {
	serverLog(LL_NOTICE, "begin waiting_lock_result");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	// 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
	// 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
	// 当前 Linux 下, 这样用, works 
	serverLog(LL_NOTICE,"waiting_lock_result new g_main_loop_run.");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_LOCK_ERROR;
	serverLog(LL_NOTICE, "waiting_lock_result exit task_node->loop");
	g_source_remove(task_node->timeout_id);
	serverLog(LL_NOTICE,"waiting_lock_result will g_main_loop_unref.");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_unlock_result releaseGuestConnection error");
		return ret;
	}
	return 0;

WAITING_LOCK_ERROR:
	serverLog(LL_ERROR, "WAITING_LOCK_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	setGuestResultErr(ble_data->ble_result, 1);
	//???
	serverLog(LL_NOTICE, "in waiting_lock_result, will //bleSetBleResult.");
	//bleSetBleResult(ble_data, ble_data->ble_result);
	serverLog(LL_NOTICE, "in waiting_lock_result, will releaseGuestConnection.");
	releaseGuestConnectionData(guest_connection);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	return 1;
}


// -------------------------------- unlock --------------------------------
fsm_table_t guest_unlock_fsm_table[GUEST_UNLOCK_SM_TABLE_LEN] = {
  {BLE_GUEST_BEGIN,				guest_connection_and_do_cmd,   BLE_GUEST_STEP2},
  {BLE_GUEST_STEP2,				waiting_guest_step2,          BLE_GUEST_STEP3},
  {BLE_GUEST_STEP3,				write_guest_step3,            BLE_GUEST_STEP4},
  {BLE_GUEST_STEP4,				waiting_guest_step4,          BLE_GUEST_UNLOCK_REQUEST},
  {BLE_GUEST_UNLOCK_REQUEST,	write_unlock_request,         BLE_GUEST_UNLOCK_RESULT},
  {BLE_GUEST_UNLOCK_RESULT,		waiting_unlock_result,        BLE_GUEST_UNLOCK_DONE},
};

fsm_table_t *getGuestUnlockFsmTable() {
  return guest_unlock_fsm_table;
}

int getGuestUnlockFsmTableLen() {
  return GUEST_UNLOCK_SM_TABLE_LEN;
}

static int write_unlock_request(void *arg) {
	serverLog(LL_NOTICE, "write_unlock_request start --------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	srand(time(0));
	int requestID = rand() % 2147483647;
	const size_t buf_size = 32;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	//op_cmd & pin
	//unlock
	IgUnlockRequest unlock_request;
	ig_UnlockRequest_init(&unlock_request);
	ig_UnlockRequest_set_operation_id(&unlock_request, requestID);
	ig_UnlockRequest_set_password(&unlock_request, guest_connection->lock->password, guest_connection->lock->password_size);
	//encode without encryption
	IgSerializerError IgErr = ig_UnlockRequest_encode(&unlock_request, buf, buf_size, &encode_size);
	if (IgErr) {
		serverLog(LL_ERROR, "ig_UnpairRequest_encode err[%d].", IgErr);
		goto UNLOCK_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "ig_GuestUnlockRequest_encode success." );

	//add
	char* plockcmd = buf;//task_node->lock_cmd;
	unsigned int lockcmd_size = encode_size;//task_node->lock_cmd_size; 
	//@@@test
	char xbuf[1024] = {0};
	ByteToHexStr(plockcmd, xbuf, lockcmd_size);
	serverLog(LL_NOTICE,"@@@ ig_UnlockRequest_encode success, lock_cmd size[%u], lock_cmd[%s]", lockcmd_size, xbuf);//task_node->lock_cmd_size);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(
			guest_connection->lock->connectionID, plockcmd, lockcmd_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		goto UNLOCK_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "success in igloohome_ble_lock_crypto_GuestConnection_encryptNative");

	if (!build_msg_payload(&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto UNLOCK_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success.");
	//@@@test
	memset(xbuf,0x0,sizeof(xbuf));
	ByteToHexStr(encryptPayloadBytes, xbuf, encryptPayloadBytes_len);
	serverLog(LL_NOTICE, "build_msg_payload success, encryptPayloadBytes:[%s].", xbuf);

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto UNLOCK_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	//ig_GuestUnlockRequest_deinit(&unlock_request);
	guest_connection->guest_step = BLE_GUEST_UNLOCK_RESULT; //BLE_GUEST_UNLOCK_REQUEST;
	return 0;

UNLOCK_REQUEST_ERROR:
	serverLog(LL_ERROR, "UNLOCK_REQUEST_ERROR");
	//ig_GuestUnlockRequest_deinit(&unlock_request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");

	//ret = gattlib_adapter_close(ble_data->adapter);
	//if (ret) {
//		serverLog(LL_ERROR, "gattlib_adapter_close error ");
//		return ret;
//	}
	return 1;
}

static int waiting_unlock_result(void *arg) {
	serverLog(LL_NOTICE, "waiting_unlock_result");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	serverLog(LL_NOTICE,"in waiting_unlock_result new g_main_loop_run.");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err){
		serverLog(LL_NOTICE,"waiting_err[%d], receive_err[%d]", guest_connection->waiting_err, guest_connection->receive_err);
		goto WAITING_UNLOCK_ERROR;
	}
	g_source_remove(task_node->timeout_id);
	serverLog(LL_NOTICE,"in waiting_unlock_result will g_main_loop_unref.");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_unlock_result releaseGuestConnection error");
		return ret;
	}

	return 0;

WAITING_UNLOCK_ERROR:
	serverLog(LL_ERROR, "WAITING_UNLOCK_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	releaseGuestConnectionData(guest_connection);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	return 1;
}

static int handle_unlock_responce(const uint8_t* data, int data_length,void* user_data) {
	serverLog(LL_NOTICE, "handle_unlock_responce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	guest_connection->guest_step = BLE_GUEST_UNLOCK_RESULT;
	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		serverLog(LL_NOTICE,"@@@ in handle_lock_responce do g_main_loop_quit, should no time out!");
		g_main_loop_quit(task_node->loop);
		serverLog(LL_NOTICE, "handle_unlock_responce RECV step2 data finished");

		size_t messageLen = guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(
				guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen) { 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			goto UNLOCK_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen %d", responceLen);

		IgUnlockResponse guest_unlock_responce;
		ig_UnlockResponse_init(&guest_unlock_responce);
		IgSerializerError err = ig_UnlockResponse_decode( responceBytes, responceLen, &guest_unlock_responce, 0);
		if (err) {
			serverLog(LL_NOTICE, "ig_GuestUnlockResponse_decode err %d", err);
			guest_connection->receive_err = 1;
			goto UNLOCK_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "has unlock response [%d] error [%d].", guest_unlock_responce.has_result, guest_unlock_responce.result);

		if (guest_unlock_responce.has_result) {
			serverLog(LL_NOTICE, "set guest result to success.");
			ble_data->ble_result->result = guest_unlock_responce.result;
		}
		// 复制所返回的结果 这儿会有内存申请
		setGuestResultCMDResponse( ble_data->ble_result, &guest_unlock_responce, sizeof(IgUnlockResponse));

		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");

		//bleSetBleResult(ble_data, ble_data->ble_result);

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");

		serverLog(LL_NOTICE, "UNLOCK_RESULT_EXIT--------------------------------");

	} else {
		serverLog(LL_ERROR, "Recv ERR, step_max_size[%d], step_cur_size[%d].", guest_connection->step_max_size, guest_connection->step_cur_size);
		return 0;
	}

UNLOCK_RESPONCE_EXIT:
		if (responceBytes) {
			free(responceBytes);
			responceBytes = NULL;
		}
		serverLog(LL_NOTICE,"in handle_unlock_responce will g_main_loop_quit.");

	return 0;
}


// -------------------------------- GETLOGS --------------------------------

fsm_table_t guest_getlogs_fsm_table[GUEST_GET_LOGS_SM_TABLE_LEN] = {
	{BLE_GUEST_BEGIN,         guest_connection_and_do_cmd,   BLE_GUEST_STEP2},
  {BLE_GUEST_STEP2,         waiting_guest_step2,          BLE_GUEST_STEP3},
  {BLE_GUEST_STEP3,         write_guest_step3,            BLE_GUEST_STEP4},
  {BLE_GUEST_STEP4,			waiting_guest_step4,          BLE_GUEST_GETLOGS_REQUEST},
  {BLE_GUEST_GETLOGS_REQUEST,   writeGetLogsRequest,      BLE_GUEST_GETLOGS_RESULT},
  {BLE_GUEST_GETLOGS_RESULT,  waitingGetLogsResult,        BLE_GUEST_GETLOGS_DONE},
};

fsm_table_t *getGuestGetLogsFsmTable() {
  return guest_getlogs_fsm_table;
}

int getGuestGetLogsFsmTableLen() {
  return GUEST_GET_LOGS_SM_TABLE_LEN;
}

static int writeGetLogsRequest(void *arg) {
	serverLog(LL_NOTICE, "writeGetLogsRequest start --------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	srand(time(0));
	int requestID = rand() % 2147483647;
	time_t cur_timestamp = time(NULL);
	size_t buf_size = 32;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	IgGetLogsRequest getlogs_request;
	ig_GetLogsRequest_init(&getlogs_request);
	ig_GetLogsRequest_set_operation_id(&getlogs_request, requestID);
	ig_GetLogsRequest_set_password(
			&getlogs_request, guest_connection->lock->password, guest_connection->lock->password_size);
	ig_GetLogsRequest_set_timestamp(&getlogs_request, cur_timestamp);
	IgSerializerError IgErr = ig_GetLogsRequest_encode(&getlogs_request, buf, buf_size, &encode_size);
	if (IgErr) {
		serverLog(LL_ERROR, "ig_GetLogsRequest_encode err");
		goto GETLOGS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "ig_UnpairRequest_encode success size:" );

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		goto GETLOGS_REQUEST_ERROR;
	}

	if (!build_msg_payload(&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto GETLOGS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto GETLOGS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}

	ig_GetLogsRequest_deinit(&getlogs_request);
	guest_connection->guest_step = BLE_GUEST_GETLOGS_REQUEST;
	return 0;

GETLOGS_REQUEST_ERROR:
	serverLog(LL_ERROR, "GETLOGS_REQUEST_ERROR");
	ig_GetLogsRequest_deinit(&getlogs_request);

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, 
			"guest_connection_and_do_cmd releaseGuestConnection success");
	return 1;
}

static int waitingGetLogsResult(void *arg) {
	serverLog(LL_NOTICE, "waitingGetLogsResult -------------------------------");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	serverLog(LL_NOTICE, "waitingGetLogsResult new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_GETLOGS_ERROR;
	g_source_remove(task_node->timeout_id);
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	serverLog(LL_NOTICE, "waitingGetLogsResult exit task_node->loop");

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_unlock_result releaseGuestConnection error");
		return ret;
	}
	return 0;

WAITING_GETLOGS_ERROR:
	serverLog(LL_ERROR, "WAITING_GETLOGS_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	releaseGuestConnectionData(guest_connection);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	return 0;
}

static int handleGetLogsResponce(const uint8_t* data, int data_length,void* user_data) {
	serverLog(LL_NOTICE, "handleGetLogsResponce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) { 
        int ret;
		serverLog(LL_NOTICE, "handle_unlock_responce RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_GETLOGS_RESULT;

		size_t messageLen = guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen) { 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			goto GETLOGS_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative success");

		IgGetLogsResponse guest_getlogs_responce;
		ig_GetLogsResponse_init(&guest_getlogs_responce);
		IgSerializerError err = ig_GetLogsResponse_decode(responceBytes, responceLen, &guest_getlogs_responce, 0);
		if (err) {
			serverLog(LL_NOTICE, "ig_GetLogsResponse_decode err %d", err);
			guest_connection->receive_err = 1;
			goto GETLOGS_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "get log info has_result[%d], result[%d], has_data[%d], data_size[%d]", guest_getlogs_responce.has_result, guest_getlogs_responce.result, guest_getlogs_responce.has_data, guest_getlogs_responce.data_size);

		if (guest_getlogs_responce.has_result) {
			serverLog(LL_NOTICE, "set guest result to success");
			ble_data->ble_result->result= guest_getlogs_responce.result;
		}
		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");
		setGuestResultCMDResponse( ble_data->ble_result, &guest_getlogs_responce, sizeof(IgGetLogsResponse));

		//bleSetBleResult( ble_data, ble_data->ble_result);
		// ig_GetLogsResponse_deinit(&guest_getlogs_responce);
		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");
		serverLog(LL_NOTICE, "GetLogs_RESULT_EXIT--------------------------------");

GETLOGS_RESPONCE_EXIT:
		if (responceBytes) {
			free(responceBytes);
			responceBytes = NULL;
		}
		g_main_loop_quit(task_node->loop);
	}
}


// -------------------------------- GETLOCKSTATUS --------------------------------
// GUEST_GET_LOCK_STATUS_SM_TABLE_LEN
fsm_table_t guest_getlockstatus_fsm_table[GUEST_GET_LOCK_STATUS_SM_TABLE_LEN] = {
  {BLE_GUEST_BEGIN,         guest_connection_and_do_cmd,   BLE_GUEST_STEP2},
  {BLE_GUEST_STEP2,         waiting_guest_step2,          BLE_GUEST_STEP3},
  {BLE_GUEST_STEP3,         write_guest_step3,            BLE_GUEST_STEP4},
  {BLE_GUEST_STEP4,   waiting_guest_step4,          BLE_GUEST_GETLOCKSTATUS_REQUEST},
  {BLE_GUEST_GETLOCKSTATUS_REQUEST,		writeGetLockStatusRequest,         BLE_GUEST_GETLOCKSTATUS_RESULT},
  {BLE_GUEST_GETLOCKSTATUS_RESULT,		waitingGetLockStatusResult,        BLE_GUEST_UNLOCK_DONE},
};

fsm_table_t *getGuestGetLockStatusFsmTable() {
	return guest_getlockstatus_fsm_table;
}

int getGuestGetLockStatusFsmTableLen() {
	return GUEST_GET_LOCK_STATUS_SM_TABLE_LEN;
}

static int writeGetLockStatusRequest(void *arg) {
	serverLog(LL_NOTICE, "writeGetLockStatusRequest start --------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	srand(time(0));
	int requestID = rand() % 2147483647;
	time_t cur_timestamp = time(NULL);
	size_t buf_size = 32;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	IgGetLockStatusRequest getlockstatus_request;
	ig_GetLockStatusRequest_init(&getlockstatus_request);
	ig_GetLockStatusRequest_set_operation_id(&getlockstatus_request, requestID);
	ig_GetLockStatusRequest_set_password(&getlockstatus_request, guest_connection->lock->password, guest_connection->lock->password_size);

	IgSerializerError IgErr = ig_GetLockStatusRequest_encode(&getlockstatus_request, buf, buf_size, &encode_size);
	if (IgErr) {
		serverLog(LL_ERROR, "ig_GetLockStatusRequest_encode err[%d].", IgErr);
		goto GETLOCKSTATUS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "ig_GetLockStatusRequest_encode success size:[%d].", encode_size);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative, err[%d].", retvalLen);
		goto GETLOCKSTATUS_REQUEST_ERROR;
	}

	if (!build_msg_payload(&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto GETLOCKSTATUS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (GATTLIB_SUCCESS != ret) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts err[%d].", ret);
		goto GETLOCKSTATUS_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	ig_GetLockStatusRequest_deinit(&getlockstatus_request);
	guest_connection->guest_step = BLE_GUEST_GETLOCKSTATUS_REQUEST;
	return 0;

GETLOCKSTATUS_REQUEST_ERROR:
	serverLog(LL_ERROR, "GETLOCKSTATUS_REQUEST_ERROR");
	ig_GetLockStatusRequest_deinit(&getlockstatus_request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	//setGuestResultErr(ble_data->ble_result, 1);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	return 1;
}

static int waitingGetLockStatusResult(void *arg) {
	serverLog(LL_NOTICE, "waitingGetLockStatusResult -------------------------------");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	serverLog(LL_NOTICE, "waitingGetLockStatusResult new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err) {
		goto WAITING_GETLOCKSTATUS_ERROR;
	}
	g_source_remove(task_node->timeout_id);
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	serverLog(LL_NOTICE, "waitingGetLockStatusResult exit task_node->loop");

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waiting_unlock_result releaseGuestConnection error[%d]", ret);
		return ret;
	}
	return 0;

WAITING_GETLOCKSTATUS_ERROR:
	serverLog(LL_ERROR, "WAITING_GETLOCKSTATUS_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	releaseGuestConnectionData(guest_connection);
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	return 0;
}

static int handleGetLockStatusResponce(const uint8_t* data, int data_length,void* user_data)
{
	serverLog(LL_NOTICE, "handleGetLockStatusResponce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		int ret;
		serverLog(LL_NOTICE, "handle_unlock_responce RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_GETLOCKSTATUS_RESULT;

		size_t messageLen = guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen) { 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative return responceLen=0 err.");
			guest_connection->receive_err = 1;
			goto GETLOCKSTATUS_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen[%d]", responceLen);

		IgGetLockStatusResponse status_resp;
		ig_GetLockStatusResponse_init(&status_resp);
		IgSerializerError err = ig_GetLockStatusResponse_decode(responceBytes, responceLen, &status_resp, 0);
		if (err) {
			serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err[%d]", err);
			guest_connection->receive_err = 1;
			goto GETLOCKSTATUS_RESPONCE_EXIT;
		}

		serverLog(LL_NOTICE, "get status resp: has_result[%d],error[%d],has_lock_open[%d],lock_open[%d],has_cell_network_status[%d],cell_network_status[%d], has_door_open[%d], door_open[%d], has_operation_id[%d], operation_id[%d].",
				status_resp.has_result, 
				status_resp.result,
				status_resp.has_lock_open,
				status_resp.lock_open,
				status_resp.has_cell_network_status,
				status_resp.cell_network_status,
				status_resp.has_door_open,
				status_resp.door_open,
				status_resp.has_operation_id,
				status_resp.operation_id);
		if (status_resp.has_result) {
			serverLog(LL_NOTICE, "set guest result to success");
			ble_data->ble_result->result = status_resp.result;
		}

		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");
		//bleSetBleResult(ble_data, ble_data->ble_result);

		if (status_resp.has_lock_open) {
			BleSetStatusRes(ble_data, status_resp.lock_open); 
		}

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");
		serverLog(LL_NOTICE, "GetLogs_RESULT_EXIT--------------------------------");

GETLOCKSTATUS_RESPONCE_EXIT:
		if (responceBytes) free(responceBytes);
		g_main_loop_quit(task_node->loop);
	}
}

// ------------------------ create pin request -----------------------------

fsm_table_t guest_createpinrequest_fsm_table[GUEST_CREATE_PIN_REQUEST_SM_TABLE_LEN] = {
	{BLE_GUEST_BEGIN,				guest_connection_and_do_cmd,		BLE_GUEST_STEP2},
	{BLE_GUEST_STEP2,				waiting_guest_step2,			BLE_GUEST_STEP3},
	{BLE_GUEST_STEP3,				write_guest_step3,				BLE_GUEST_STEP4},
	{BLE_GUEST_STEP4,				waiting_guest_step4,			BLE_GUEST_CREATEPINREQUEST_REQUEST},
	{BLE_GUEST_CREATEPINREQUEST_REQUEST,		writeCreatePinRequest,			BLE_GUEST_CREATEPINREQUEST_RESULT},
	{BLE_GUEST_CREATEPINREQUEST_RESULT,			waitingCreatePinRequestResult,	BLE_GUEST_UNLOCK_DONE},
};

fsm_table_t *getGuestCreatePinRequestFsmTable() {
	return guest_createpinrequest_fsm_table;
}

int getGuestCreatePinRequestFsmTableLen() {
	return GUEST_CREATE_PIN_REQUEST_SM_TABLE_LEN;
}

static int writeCreatePinRequest(void *arg) {
	serverLog(LL_NOTICE, "writeCreatePinRequest start -------------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	IgCreatePinRequest *request = guest_connection->cmd_request;

	srand(time(0));
	int requestID = rand() % 2147483647;
	time_t cur_timestamp = time(NULL);
	size_t buf_size = 64;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	IgCreatePinRequest create_pin_request;
	ig_CreatePinRequest_init(&create_pin_request);
	serverLog(LL_NOTICE, "requestID: %d", requestID);
	ig_CreatePinRequest_set_operation_id( &create_pin_request, requestID);
	ig_CreatePinRequest_set_password(&create_pin_request, guest_connection->lock->password, guest_connection->lock->password_size);
	if (!request ||  !ig_CreatePinRequest_is_valid(request)) {
		serverLog(LL_ERROR, "request NULL or request don't have pin");
		goto PIN_REQUEST_ERROR;
	}

	ig_CreatePinRequest_set_new_pin( &create_pin_request, request->new_pin, request->new_pin_size);
	//@@@test
	char xbuf[1024] = {0};
	ByteToHexStr(create_pin_request.new_pin, xbuf, create_pin_request.new_pin_size);
	serverLog(LL_NOTICE,"ig_CreatePinRequest_set_new_pin:[%s].", xbuf);

	ig_CreatePinRequest_set_password(&create_pin_request, request->password, request->password_size);
	//@@@test
	memset(xbuf, 0x0, sizeof(xbuf));
	ByteToHexStr(create_pin_request.password, xbuf, create_pin_request.password_size);
	serverLog(LL_NOTICE,"ig_CreatePinRequest_set_password:[%s]", xbuf);

	// optionnal
	if (request->has_start_date) {
		ig_CreatePinRequest_set_start_date(
				&create_pin_request, request->start_date);
		serverLog(LL_NOTICE, "ig_CreatePinRequest_set_start_date %d", create_pin_request.start_date);
	}

	if (request->has_end_date) {
		ig_CreatePinRequest_set_end_date(
				&create_pin_request, request->end_date);
		serverLog(LL_NOTICE, "ig_CreatePinRequest_set_end_date %d", create_pin_request.end_date);
	}

	if (request->has_pin_type) {
		ig_CreatePinRequest_set_pin_type(
				&create_pin_request, request->pin_type);
		serverLog(LL_NOTICE, "ig_CreatePinRequest_set_pin_type %d", create_pin_request.pin_type);
	}

	if (request->has_operation_id) {
		ig_CreatePinRequest_set_operation_id(&create_pin_request, request->operation_id);
		serverLog(LL_NOTICE, "ig_CreatePinRequest_set_operation_id %d", create_pin_request.operation_id);
	}
	//@@test
	serverLog(LL_NOTICE,"create pin option: start_date[%u], end_date[%u], pin_type[%d], operation_id[%d].", 
		create_pin_request.start_date,
		create_pin_request.end_date,
		create_pin_request.pin_type,
		create_pin_request.operation_id);

	IgSerializerError IgErr = ig_CreatePinRequest_encode(
			&create_pin_request, buf, buf_size, &encode_size);
	if (IgErr)
	{
		serverLog(LL_ERROR, "ig_CreatePinRequest_encode err %d", IgErr);
		goto PIN_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "ig_CreatePinRequest_encode success size: %d", encode_size);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(
			guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		goto PIN_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_encryptNative success" );

	if (!build_msg_payload(
				&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto PIN_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success.");
	//@@@test
	memset(xbuf,0x0,sizeof(xbuf));
	ByteToHexStr(encryptPayloadBytes, xbuf, encryptPayloadBytes_len);
	serverLog(LL_NOTICE, "build_msg_payload success, encryptPayloadBytes:[%s].", xbuf);

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (GATTLIB_SUCCESS != ret) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags.");
		goto PIN_REQUEST_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success.");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}

	ig_CreatePinRequest_deinit(&create_pin_request);
	ig_CreatePinRequest_deinit(request);
	guest_connection->guest_step = BLE_GUEST_CREATEPINREQUEST_REQUEST;
	return 0;

	// 出错处理
PIN_REQUEST_ERROR:
	serverLog(LL_ERROR, "PIN_REQUEST_ERROR");
	ig_CreatePinRequest_deinit(&create_pin_request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ig_CreatePinRequest_deinit(request);
	ret = releaseGuestConnection(&guest_connection);
	if (ret)
	{
		serverLog(LL_ERROR, 
				"releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, 
			"releaseGuestConnection success");
	serverLog(LL_NOTICE, "gattlib_adapter_close success");
	return 1;
}

static int waitingCreatePinRequestResult(void *arg)
{
	serverLog(LL_NOTICE, "waitingCreatePinRequestResult -------------------------------");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	IgCreatePinRequest *request = guest_connection->cmd_request;

	serverLog(LL_NOTICE, "waitingCreatePinRequestResult new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_CREATE_PIN_ERROR;
	g_source_remove(task_node->timeout_id);
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	serverLog(LL_NOTICE, "waitingGetLockStatusResult exit task_node->loop");

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret)
	{
		serverLog(LL_ERROR, "waiting_unlock_result releaseGuestConnection error");
		return ret;
	}
	return 0;

WAITING_CREATE_PIN_ERROR:
	serverLog(LL_ERROR, "WAITING_CREATE_PIN_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	releaseGuestConnectionData(guest_connection);

	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ig_CreatePinRequest_deinit(request);
	ret = releaseGuestConnection(&guest_connection);
	if (ret)
	{
		serverLog(LL_ERROR, 
				"guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, 
			"guest_connection_and_do_cmd releaseGuestConnection success");

}

static int handleCreatePinResponce(const uint8_t* data, int data_length,void* user_data)
{
	serverLog(LL_NOTICE, "handleCreatePinResponce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size)
	{
		int ret;
		serverLog(LL_NOTICE, "handleCreatePinResponce RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_CREATEPINREQUEST_RESULT;

		size_t messageLen = 
			guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(
				guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen)
		{ 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			goto PINREQUEST_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen %d", responceLen);

		IgCreatePinResponse guest_create_pin_responce;
		ig_CreatePinResponse_init(&guest_create_pin_responce);
		IgSerializerError err = ig_CreatePinResponse_decode(
				responceBytes, responceLen, &guest_create_pin_responce, 0
				);
		if (err)
		{
			serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err %d", err);
			guest_connection->receive_err = 1;
			goto PINREQUEST_RESPONCE_EXIT;
		}

		serverLog(LL_NOTICE, "has create pin result %d result %d",
				guest_create_pin_responce.has_result, guest_create_pin_responce.result);
		if (guest_create_pin_responce.has_result) {
			serverLog(LL_NOTICE, "set guest result to success");
			ble_data->ble_result->result = guest_create_pin_responce.result;
		}
		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");
		setGuestResultCMDResponse(
				ble_data->ble_result, &guest_create_pin_responce, sizeof(IgCreatePinResponse));
		//bleSetBleResult( ble_data, ble_data->ble_result);

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");

		serverLog(LL_NOTICE, "GetLogs_RESULT_EXIT--------------------------------");

PINREQUEST_RESPONCE_EXIT:
		if (responceBytes) {
			free(responceBytes);
			responceBytes = NULL;
		}
		g_main_loop_quit(task_node->loop);

	}
}

// ------------------------ delete pin request -----------------------------

fsm_table_t guest_delete_pin_request_fsm_table[GUEST_DELETE_PIN_REQUEST_SM_TABLE_LEN] = {
	{BLE_GUEST_BEGIN,         guest_connection_and_do_cmd,   BLE_GUEST_STEP2},
	{BLE_GUEST_STEP2,         waiting_guest_step2,          BLE_GUEST_STEP3},
	{BLE_GUEST_STEP3,         write_guest_step3,            BLE_GUEST_STEP4},
	{BLE_GUEST_STEP4,   waiting_guest_step4,          BLE_GUEST_UNLOCK_REQUEST},
	{BLE_GUEST_UNLOCK_REQUEST,   writeDeletePinRequest,         BLE_GUEST_UNLOCK_RESULT},
	{BLE_GUEST_UNLOCK_RESULT,  waitingDeletePinRequestResult,        BLE_GUEST_UNLOCK_DONE},
};

fsm_table_t *getGuestDeletePinRequestFsmTable()
{
	return guest_delete_pin_request_fsm_table;
}

int getGuestDeletePinRequestFsmTableLen()
{
	return GUEST_CREATE_PIN_REQUEST_SM_TABLE_LEN;
}

static int writeDeletePinRequest(void *arg)
{
	serverLog(LL_NOTICE, "writeDeletePinRequest start --------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	IgDeletePinRequest *request = guest_connection->cmd_request;

	srand(time(0));
	int requestID = rand() % 2147483647;
	time_t cur_timestamp = time(NULL);
	size_t buf_size = 64;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	IgDeletePinRequest delete_pin_request;
	ig_DeletePinRequest_init(&delete_pin_request);
	serverLog(LL_NOTICE, "requestID: %d", requestID);
	ig_DeletePinRequest_set_operation_id(
			&delete_pin_request, requestID);
	ig_DeletePinRequest_set_password(
			&delete_pin_request, guest_connection->lock->password, guest_connection->lock->password_size);
	if (!request ||  !ig_DeletePinRequest_is_valid(request))
	{
		serverLog(LL_ERROR, "request NULL or request don't have pin");
		goto DELETE_PIN_ERROR;
	}

	ig_DeletePinRequest_set_old_pin(
			&delete_pin_request, request->old_pin, request->old_pin_size);

	if (request->has_operation_id)
	{
		ig_DeletePinRequest_set_operation_id(&delete_pin_request, request->operation_id);
	}

	IgSerializerError IgErr = ig_DeletePinRequest_encode(
			&delete_pin_request, buf, buf_size, &encode_size);
	if (IgErr)
	{
		serverLog(LL_ERROR, "ig_DeletePinRequest_encode err %d", IgErr);
		goto DELETE_PIN_ERROR;
	}
	serverLog(LL_NOTICE, "ig_DeletePinRequest_encode success size: %d", encode_size);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative(
			guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) 
	{
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		goto DELETE_PIN_ERROR;
	}
	serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_encryptNative success" );

	if (!build_msg_payload(
				&encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen))
	{
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto DELETE_PIN_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto DELETE_PIN_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}

	ig_DeletePinRequest_deinit(&delete_pin_request);
	ig_DeletePinRequest_deinit(request);
	guest_connection->guest_step = BLE_GUEST_DELETEPINREQUEST_REQUEST;
	return 0;

	// 出错处理
DELETE_PIN_ERROR:
	serverLog(LL_ERROR, "DELETE_PIN_ERROR");
	ig_DeletePinRequest_deinit(&delete_pin_request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ig_DeletePinRequest_deinit(request);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "releaseGuestConnection success");
	serverLog(LL_NOTICE, "gattlib_adapter_close success");
	return 1;
}

static int waitingDeletePinRequestResult(void *arg)
{
	serverLog(LL_NOTICE, "waitingDeletePinRequestResult -------------------------------");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	IgDeletePinRequest *request = guest_connection->cmd_request;

	serverLog(LL_NOTICE, "waitingDeletePinRequestResult new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_DELETE_PIN_ERROR;
	g_source_remove(task_node->timeout_id);
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	serverLog(LL_NOTICE, "waitingGetLockStatusResult exit task_node->loop");

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret)
	{
		serverLog(LL_ERROR, "waitingGetLockStatusResult releaseGuestConnection error");
		return ret;
	}
	return 0;

WAITING_DELETE_PIN_ERROR:
	// Bug? corrupted double-linked list
	// Aborted
	serverLog(LL_ERROR, "WAITING_DELETE_PIN_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	serverLog(LL_ERROR, "releaseGuestConnectionData ");
	releaseGuestConnectionData(guest_connection);
	serverLog(LL_ERROR, "setGuestResultErr ");
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	serverLog(LL_ERROR, "ig_DeletePinRequest_deinit ");
	ig_DeletePinRequest_deinit(request);
	serverLog(LL_ERROR, "releaseGuestConnection ");
	ret = releaseGuestConnection(&guest_connection);
	if (ret)
	{
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	serverLog(LL_ERROR, "gattlib_adapter_close ");
}

static int handleDeletePinResponce(const uint8_t* data, int data_length,void* user_data)
{
	serverLog(LL_NOTICE, "handleDeletePinResponce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = 
		(guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size)
	{
		int ret;
		serverLog(LL_NOTICE, "handleDeletePinResponce RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_DELETEPINREQUEST_RESULT;

		size_t messageLen = 
			guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative(
				guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen)
		{ 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			goto DELETE_PINREQUEST_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen %d", responceLen);

		IgDeletePinResponse responce;
		ig_DeletePinResponse_init(&responce);
		IgSerializerError err = ig_DeletePinResponse_decode(
				responceBytes, responceLen, &responce, 0
				);
		if (err)
		{
			serverLog(LL_NOTICE, "ig_DeletePinResponse_decode err %d", err);
			guest_connection->receive_err = 1;
			goto DELETE_PINREQUEST_RESPONCE_EXIT;
		}

		serverLog(LL_NOTICE, "has delete pin has result %d result %d",
				responce.has_result, responce.result);
		if (responce.has_result)
		{
			serverLog(LL_NOTICE, "set guest result to success");
			ble_data->ble_result->result = responce.result;
		}
		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");
		setGuestResultCMDResponse(
				ble_data->ble_result, &responce, sizeof(IgDeletePinResponse));
		//bleSetBleResult( ble_data, ble_data->ble_result);

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");

		serverLog(LL_NOTICE, "GetLogs_RESULT_EXIT--------------------------------");

DELETE_PINREQUEST_RESPONCE_EXIT:
		if (responceBytes) {
			free(responceBytes);
			responceBytes = NULL;
		}
		g_main_loop_quit(task_node->loop);
	}
}


// ------------------------ get battery level -----------------------------

fsm_table_t guest_get_battery_level_fsm_table[GUEST_DELETE_PIN_REQUEST_SM_TABLE_LEN] = {
	{BLE_GUEST_BEGIN,         guest_connection_and_do_cmd,   BLE_GUEST_STEP2},
	{BLE_GUEST_STEP2,         waiting_guest_step2,          BLE_GUEST_STEP3},
	{BLE_GUEST_STEP3,         write_guest_step3,            BLE_GUEST_STEP4},
	{BLE_GUEST_STEP4,   waiting_guest_step4,          BLE_GUEST_GETBATTERYLEVEL_REQUEST},
	{BLE_GUEST_GETBATTERYLEVEL_REQUEST,   writeGetBatteryLevelRequest,   BLE_GUEST_GETBATTERYLEVEL_RESULT},
	{BLE_GUEST_GETBATTERYLEVEL_RESULT,  waitingGetBatteryLevelResult,        BLE_GUEST_GETBATTERYLEVEL_DONE},
};

fsm_table_t *getGuestGetBatteryLevelFsmTable() {
	return guest_get_battery_level_fsm_table;
}

int getGuestGetBatteryLevelFsmTableLen() {
	return GUEST_GET_BATTERY_LEVEL_SM_TABLE_LEN;
}

static int writeGetBatteryLevelRequest(void *arg) {
	serverLog(LL_NOTICE, "writeGetBatteryLevelRequest start --------");
	int ret = 0;
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;

	srand(time(0));
	int requestID = rand() % 2147483647;
	time_t cur_timestamp = time(NULL);
	size_t buf_size = 64;
	size_t encode_size = 0;
	uint8_t buf[buf_size];
	int retvalLen;
	uint8_t *retvalBytes = NULL;;
	uint8_t *encryptPayloadBytes = NULL;
	size_t encryptPayloadBytes_len;

	IgGetBatteryLevelRequest request;
	ig_GetBatteryLevelRequest_init(&request);
	serverLog(LL_NOTICE, "requestID: %d", requestID);
	ig_GetBatteryLevelRequest_set_operation_id( &request, requestID);
	ig_GetBatteryLevelRequest_set_password( &request, guest_connection->lock->password, guest_connection->lock->password_size);
	if (!ig_GetBatteryLevelRequest_is_valid(&request)) {
		serverLog(LL_ERROR, "request invalid");
		goto GET_BATTERY_ERROR;
	}

	IgSerializerError IgErr = ig_GetBatteryLevelRequest_encode( &request, buf, buf_size, &encode_size);
	if (IgErr) {
		serverLog(LL_ERROR, "ig_GetBatteryLevelRequest_encode err %d", IgErr);
		goto GET_BATTERY_ERROR;
	}
	serverLog(LL_NOTICE, "ig_GetBatteryLevelRequest_encode success size: %d", encode_size);

	retvalLen = igloohome_ble_lock_crypto_GuestConnection_encryptNative( guest_connection->lock->connectionID, buf, encode_size, &retvalBytes);
	if (!retvalLen) {
		serverLog(LL_ERROR, "failed in igloohome_ble_lock_crypto_GuestConnection_encryptNative");
		goto GET_BATTERY_ERROR;
	}
	serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_encryptNative success, %d", retvalLen );

	if (!build_msg_payload( &encryptPayloadBytes, &encryptPayloadBytes_len, retvalBytes, retvalLen)) {
		serverLog(LL_ERROR, "failed in build_msg_payload");
		goto GET_BATTERY_ERROR;
	}
	serverLog(LL_NOTICE, "build_msg_payload success");

	ret = write_char_by_uuid_multi_atts(
			guest_connection->gatt_connection, &guest_connection->guest_uuid, 
			encryptPayloadBytes, encryptPayloadBytes_len);
	if (ret != GATTLIB_SUCCESS) {
		serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		goto GET_BATTERY_ERROR;
	}
	serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success");

	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}

	ig_GetBatteryLevelRequest_deinit(&request);
	guest_connection->guest_step = BLE_GUEST_GETBATTERYLEVEL_REQUEST;
	return 0;

	// 出错处理
GET_BATTERY_ERROR:
	serverLog(LL_ERROR, "DELETE_PIN_ERROR");
	ig_GetBatteryLevelRequest_deinit(&request);
	if (encryptPayloadBytes) {
		free(encryptPayloadBytes);
		encryptPayloadBytes = NULL;
	}
	if (retvalBytes) {
		free(retvalBytes);
		retvalBytes = NULL;
	}
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "releaseGuestConnection success");
	serverLog(LL_NOTICE, "gattlib_adapter_close success");
	return 1;
}

static int waitingGetBatteryLevelResult(void *arg) {
	serverLog(LL_NOTICE, "waiting GetBatteryLevel RequestResult -------------------------------");
	task_node_t *task_node = (task_node_t *)arg;
	ble_data_t *ble_data = (ble_data_t *)(task_node->ble_data);
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	IgDeletePinRequest *request = guest_connection->cmd_request;

	serverLog(LL_NOTICE, "waitingDeletePinRequestResult new loop waiting");
	g_main_loop_run(task_node->loop);
	if (guest_connection->waiting_err || guest_connection->receive_err)
		goto WAITING_BATTERY_ERROR;
	g_source_remove(task_node->timeout_id);
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;

	serverLog(LL_NOTICE, "waitingGetLockStatusResult exit task_node->loop");

	releaseGuestConnectionData(guest_connection);
	int ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "waitingGetLockStatusResult releaseGuestConnection error");
		return ret;
	}
	return 0;

WAITING_BATTERY_ERROR:
	// Bug? corrupted double-linked list
	// Aborted
	serverLog(LL_ERROR, "WAITING_DELETE_PIN_ERROR ");
	g_main_loop_unref(task_node->loop);
	task_node->loop = NULL;
	serverLog(LL_ERROR, "releaseGuestConnectionData ");
	releaseGuestConnectionData(guest_connection);
	serverLog(LL_ERROR, "setGuestResultErr ");
	setGuestResultErr(ble_data->ble_result, 1);
	//bleSetBleResult(ble_data, ble_data->ble_result);
	serverLog(LL_ERROR, "ig_DeletePinRequest_deinit ");
	ig_DeletePinRequest_deinit(request);
	serverLog(LL_ERROR, "releaseGuestConnection ");
	ret = releaseGuestConnection(&guest_connection);
	if (ret) {
		serverLog(LL_ERROR, "guest_connection_and_do_cmd releaseGuestConnection error");
		return ret;
	}
	serverLog(LL_NOTICE, "guest_connection_and_do_cmd releaseGuestConnection success");
	serverLog(LL_ERROR, "gattlib_adapter_close ");
	return -1;

}

static int handleGetBatteryLevelResponce(const uint8_t* data, int data_length,void* user_data)
{
	serverLog(LL_NOTICE, "handleGetBatteryLevelResponce--------------------------------");
	task_node_t *task_node = (task_node_t *)user_data;
	ble_data_t *ble_data = task_node->ble_data;
	guest_connection_t *guest_connection = (guest_connection_t *)ble_data->ble_connection;
	int responceLen;
	uint8_t *responceBytes = NULL;

	save_message_data(data, data_length, user_data);

	if (guest_connection->step_max_size == guest_connection->step_cur_size) {
		int ret;
		serverLog(LL_NOTICE, "handleGetBatteryLevelResponce RECV step2 data finished");
		guest_connection->guest_step = BLE_GUEST_GETBATTERYLEVEL_RESULT;

		size_t messageLen = guest_connection->step_max_size - guest_connection->n_size_byte;
		uint8_t *data_start = guest_connection->step_data + guest_connection->n_size_byte;
		uint8_t messageBytes[messageLen];
		memcpy(messageBytes, data_start, messageLen);

		responceLen = igloohome_ble_lock_crypto_GuestConnection_decryptNative( guest_connection->lock->connectionID, messageBytes, messageLen, &responceBytes);
		if (!responceLen) { 
			serverLog(LL_ERROR, "igloohome_ble_lock_crypto_GuestConnection_decryptNative error");
			guest_connection->receive_err = 1;
			goto DELETE_PINREQUEST_RESPONCE_EXIT;
		}
		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_decryptNative responceLen %d", responceLen);

		IgGetBatteryLevelResponse responce;
		ig_GetBatteryLevelResponse_init(&responce);
		IgSerializerError err = ig_GetBatteryLevelResponse_decode( responceBytes, responceLen, &responce, 0);
		if (err) {
			serverLog(LL_NOTICE, "ig_GetLockStatusResponse_decode err %d", err);
			guest_connection->receive_err = 1;
			goto DELETE_PINREQUEST_RESPONCE_EXIT;
		}

		serverLog(LL_NOTICE, "has get battery level response %d error %d", responce.has_result, responce.result);
		if (responce.has_result) {
			serverLog(LL_NOTICE, "get battery result[%d]", responce.result);
			ble_data->ble_result->result = responce.result;
		}
		if (responce.has_battery_level){
			serverLog(LL_NOTICE, "get battery level[%d]", responce.battery_level);
			BleSetBatteryLRes(ble_data, responce.battery_level); 
		}

		// 返回参数给调用进程
		serverLog(LL_NOTICE, "handle_step4_message //bleSetBleResult to ble data");
		setGuestResultCMDResponse( ble_data->ble_result, &responce, sizeof(IgDeletePinResponse));
		//bleSetBleResult(ble_data, ble_data->ble_result);

		igloohome_ble_lock_crypto_GuestConnection_endConnection((guest_connection->lock)->connectionID);

		serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_GuestConnection_endConnection success");
		serverLog(LL_NOTICE, "GetLogs_RESULT_EXIT--------------------------------");

DELETE_PINREQUEST_RESPONCE_EXIT:
		if (responceBytes) {
			free(responceBytes);
			responceBytes = NULL;
		}
		g_main_loop_quit(task_node->loop);
	}
}
*/

//-------------------------- param
int bleInitGuestParam(ble_guest_param_t *guest_param)
{
  memset(guest_param, 0, sizeof(ble_guest_param_t));
  return 0;
}

int bleReleaseGuestParam(ble_guest_param_t **pp_guest_param)
{
  ble_guest_param_t *guest_param = *pp_guest_param;
  if (!pp_guest_param) return 1;
  if (!guest_param) return 1;
  if (guest_param->lock)
  {
    serverLog(LL_NOTICE, "bleReleaseGuestParam");
    releaseLock(&guest_param->lock);
    guest_param->lock = NULL;
  }
  if (guest_param->cmd_request)
  {
    free(guest_param->cmd_request);
    guest_param->cmd_request_size = 0;
    guest_param->cmd_request = NULL;
  }
  free(*pp_guest_param );
  *pp_guest_param = NULL;
  return 0;
}

int bleSetGuestParam(ble_guest_param_t *guest_param, igm_lock_t *lock)
{
  releaseLock(&guest_param->lock);
  guest_param->lock = calloc(sizeof(igm_lock_t), 1);
  copyLock(guest_param->lock, lock);
  return 0;
}

int bleSetGuestRequest( ble_guest_param_t *guest_param, void *cmd_request, size_t cmd_request_size) {
    if (!cmd_request) return 1;
    if (guest_param->cmd_request)
    {
        free(guest_param->cmd_request);
        guest_param->cmd_request_size = 0;
    }
    guest_param->cmd_request = malloc(cmd_request_size);
    memset(guest_param->cmd_request, 0, cmd_request_size);
    guest_param->cmd_request_size = cmd_request_size;
    memcpy(guest_param->cmd_request, cmd_request, cmd_request_size);
    return 0;
}

// -------------------------- result operation----------------------------------
int initGuestResult(ble_guest_result_t *result) {
	memset(result, 0, sizeof(ble_guest_result_t));
	return 0;
}

int setGuestResultAddr(ble_guest_result_t *result, char *addr, size_t addr_len) {
	memset(result->addr,0, MAX_DEVICE_ADDR);
	memcpy(result->addr, addr, addr_len>MAX_DEVICE_ADDR?MAX_DEVICE_ADDR:addr_len);
	return 0;
}

int isGuestSuccess(ble_guest_result_t *result) {
	return !result->result;
}

void bleReleaseGuestResult(ble_guest_result_t **pp_result) {
	if (!pp_result) return;
	if (!*pp_result) return;
	if(pp_result) {
		free(*pp_result);
		*pp_result = NULL;
	}
}

void setGuestResultErr(ble_guest_result_t *ble_result, int err) {
	if(NULL == ble_result){
		serverLog(LL_NOTICE,"in setGuestResultErr result is NULL!");
		return;
	}
	ble_result->result = err;
	return;
}


void setGuestResultCMDResponse( ble_guest_result_t *result, void *cmd_response, size_t cmd_response_size) {
	if (result->cmd_response) 
		releaseGuestResultCMDResponse(result);
	result->cmd_response = malloc(cmd_response_size);
	memset(result->cmd_response, 0, cmd_response_size);
	memcpy(result->cmd_response, cmd_response, cmd_response_size);
	return;
}

void releaseGuestResultCMDResponse(ble_guest_result_t *result) {
	if (!result) return;
	if (!result->cmd_response) return;

	if(result->cmd_response) {
		free(result->cmd_response);
		result->cmd_response = NULL;
		result->cmd_response_size = 0;
	}
}

void releaseGuestResult(ble_guest_result_t **pp_result) {
	// 主要释放里面内容,不释放自己
	if (!pp_result) return;
	if (!*pp_result) return;
	releaseGuestResultCMDResponse(*pp_result);
}


