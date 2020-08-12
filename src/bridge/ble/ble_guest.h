#ifndef _BLE_GUEST_H_
#define _BLE_GUEST_H_
#include "bridge/ble/lock.h"
#include "bridge/bridge_main/fsm.h"

enum ERR_GUEST_STATE {
	ERR_LOGIC = -1000,
	ERR_CONNECTION,
	ERR_WAITING,
	ERR_SEND,
	ERR_GATT,
	ERR_GLIB_LOOP,
	ERR_STEP1,
	ERR_STEP2,
	ERR_CMD,
};
enum BLE_GUEST_STATE {
  BLE_GUEST_BEGIN = 0,          // 注册一个改变通知
  BLE_GUEST_STEP2 = 1,          // 收听锁的第一步信息
  BLE_GUEST_STEP3 = 2,          // 写给锁的第二步信息
  BLE_GUEST_STEP4 = 3,     // 此时已经建立
  BLE_GUEST_DONE = 4,    // 完成guest connection标志
  BLE_GUEST_CMD_RESULT = 5,
};

enum BLE_GUEST_UNPAIR_STATE {
  BLE_GUEST_UNPAIR_REQUEST = 5, // 发送请求
  BLE_GUEST_UNPAIR_RESULT = 6,  // 等待结果
  BLE_GUEST_UNPAIR_DONE = 7     // 完成
};

enum BLE_GUEST_UNLOCK_STATE {
  BLE_GUEST_UNLOCK_REQUEST = 8, // 发送请求
  BLE_GUEST_UNLOCK_RESULT = 9,  // 等待结果
  BLE_GUEST_UNLOCK_DONE = 10     // 完成
};

enum BLE_GUEST_GETLOGS_STATE {
  BLE_GUEST_GETLOGS_REQUEST = 14, // 发送请求
  BLE_GUEST_GETLOGS_RESULT = 15,  // 等待结果
  BLE_GUEST_GETLOGS_DONE = 16     // 完成
};

enum BLE_GUEST_GETLOCKSTATUS_STATE {
  BLE_GUEST_GETLOCKSTATUS_REQUEST = 17, // 发送请求
  BLE_GUEST_GETLOCKSTATUS_RESULT = 18,  // 等待结果
  BLE_GUEST_GETLOCKSTATUS_DONE = 19     // 完成
};

enum BLE_GUEST_CREATEPINREQUEST_STATE {
  BLE_GUEST_CREATEPINREQUEST_REQUEST = 20, // 发送请求
  BLE_GUEST_CREATEPINREQUEST_RESULT = 21,  // 等待结果
  BLE_GUEST_CREATEPINREQUEST_DONE = 22     // 完成
};

enum BLE_GUEST_DELETEPINREQUEST_STATE {
  BLE_GUEST_DELETEPINREQUEST_REQUEST = 23, // 发送请求
  BLE_GUEST_DELETEPINREQUEST_RESULT = 24,  // 等待结果
  BLE_GUEST_DELETEPINREQUEST_DONE = 25     // 完成
};

enum BLE_GUEST_GETBATTERYLEVEL_STATE {
  BLE_GUEST_GETBATTERYLEVEL_REQUEST = 26, // 发送请求
  BLE_GUEST_GETBATTERYLEVEL_RESULT = 27,  // 等待结果
  BLE_GUEST_GETBATTERYLEVEL_DONE = 28     // 完成
};

enum BLE_GUEST_SETTIME_STATE {
  BLE_GUEST_SETTIME_REQUEST = 29, // 发送请求
  BLE_GUEST_SETTIME_RESULT = 30,  // 等待结果
  BLE_GUEST_SETTIME_DONE = 31     // 完成
};

enum BLE_GUEST_LOCK_STATE {
  BLE_GUEST_LOCK_REQUEST = 11, // 发送请求
  BLE_GUEST_LOCK_RESULT = 12,  // 等待结果
  BLE_GUEST_LOCK_DONE = 13     // 完成
};

typedef struct BLEGuestParam {
  igm_lock_t *lock;
  size_t cmd_request_size; // 某些请求太多了参数, 用一个指针来传入
  void *cmd_request;
}ble_guest_param_t;

int bleInitGuestParam(ble_guest_param_t *unpair_param);
int bleReleaseGuestParam(ble_guest_param_t **pp_guest_param);
int bleSetGuestParam(ble_guest_param_t *unpair_param, igm_lock_t *lock);
int bleSetGuestRequest(
  ble_guest_param_t *guest_param, void *cmd_request, size_t cmd_request_size);

typedef struct BLEGuestResult {
  char addr[MAX_DEVICE_ADDR];
  int result;               // ***_result, 简单判断是否有出错
/*  int lock_result;
  int unlock_result;
  int unpair_result;
  int getlogs_result;
  int getlockstatus_result;
  int create_pin_request_result;
  int delete_pin_request_result;
  int get_battery_level_result;
  int set_time_result;
*/
  size_t cmd_response_size;     // ****_response 就是相关的response的结构体
  void *cmd_response;     // 纯粹用来把相关的结构体,拷贝出去
}ble_guest_result_t;

int initGuestResult(ble_guest_result_t *result);
int setGuestResultAddr(ble_guest_result_t *result, 
  char *addr, size_t addr_len);
void bleReleaseGuestResult(ble_guest_result_t **pp_result);
void setGuestResultErr(ble_guest_result_t *result, int err);
void setGuestResultCreatePinRequestErr(ble_guest_result_t *result, int err);
void setGuestResultUnlockErr(ble_guest_result_t *result, int err);
void setGuestResultLockErr(ble_guest_result_t *result, int err);
void setGuestResultGetLogsErr(ble_guest_result_t *result, int err);
void setGuestResultDeletePinRequestErr(ble_guest_result_t *result, int err);
void setGuestResultSetTimeErr(ble_guest_result_t *result, int err);
void setGuestResultCMDResponse(
      ble_guest_result_t *result, void *cmd_response, size_t cmd_response_size);
void releaseGuestResultCMDResponse(ble_guest_result_t *result);
void releaseGuestResult(ble_guest_result_t **pp_result);

int isGuestSuccess(ble_guest_result_t *result);
// guest
fsm_table_t *getGuestFsmTable();
int getGuestFsmTableLen();

// guest unpair
fsm_table_t *getGuestUnpairFsmTable();
int getGuestUnpairFsmTableLen();

// guest unlock
fsm_table_t *getGuestUnlockFsmTable();
int getGuestUnlockFsmTableLen();

// guest get logs
fsm_table_t *getGuestGetLogsFsmTable();
int getGuestGetLogsFsmTableLen();

// get lock status
fsm_table_t *getGuestGetLockStatusFsmTable();
int getGuestGetLockStatusFsmTableLen();

// create pin request
fsm_table_t *getGuestCreatePinRequestFsmTable();
int getGuestCreatePinRequestFsmTableLen();

// delete pin request
fsm_table_t *getGuestDeletePinRequestFsmTable();
int getGuestDeletePinRequestFsmTableLen();

// get battery level
fsm_table_t *getGuestGetBatteryLevelFsmTable();
int getGuestGetBatteryLevelFsmTableLen();

// set time
fsm_table_t *getGuestSetTimeFsmTable();
int getGuestSetTimeFsmTableLen();


// guest lock
int getGuestLockFsmTableLen();
fsm_table_t *getGuestLockFsmTable();

#endif
