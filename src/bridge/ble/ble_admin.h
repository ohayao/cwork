#ifndef _BLE_ADMIN_H_
#define _BLE_ADMIN_H_
#include "bridge/ble/lock.h"
#include "bridge/bridge_main/fsm.h"

enum BLE_ADMIN_STATE {
  BLE_ADMIN_BEGIN = 0,          // 注册一个改变通知
  BLE_ADMIN_STEP1 = 1,          // 收听锁的第一步信息
  BLE_ADMIN_STEP2 = 2,          // 写给锁的第二步信息
  BLE_ADMIN_ESTABLISHED = 3,     // 此时已经建立
  BLE_ADMIN_DONE = 4,    // 完成admin connection标志
};

enum BLE_ADMIN_UNPAIR_STATE {
  BLE_ADMIN_UNPAIR_REQUEST = 5, // 发送请求
  BLE_ADMIN_UNPAIR_RESULT = 6,  // 等待结果
  BLE_ADMIN_UNPAIR_DONE = 7     // 完成
};

enum BLE_ADMIN_UNLOCK_STATE {
  BLE_ADMIN_UNLOCK_REQUEST = 8, // 发送请求
  BLE_ADMIN_UNLOCK_RESULT = 9,  // 等待结果
  BLE_ADMIN_UNLOCK_DONE = 10     // 完成
};

enum BLE_ADMIN_GETLOGS_STATE {
  BLE_ADMIN_GETLOGS_REQUEST = 14, // 发送请求
  BLE_ADMIN_GETLOGS_RESULT = 15,  // 等待结果
  BLE_ADMIN_GETLOGS_DONE = 16     // 完成
};

enum BLE_ADMIN_GETLOCKSTATUS_STATE {
  BLE_ADMIN_GETLOCKSTATUS_REQUEST = 17, // 发送请求
  BLE_ADMIN_GETLOCKSTATUS_RESULT = 18,  // 等待结果
  BLE_ADMIN_GETLOCKSTATUS_DONE = 19     // 完成
};

enum BLE_ADMIN_CREATEPINREQUEST_STATE {
  BLE_ADMIN_CREATEPINREQUEST_REQUEST = 20, // 发送请求
  BLE_ADMIN_CREATEPINREQUEST_RESULT = 21,  // 等待结果
  BLE_ADMIN_CREATEPINREQUEST_DONE = 22     // 完成
};

enum BLE_ADMIN_DELETEPINREQUEST_STATE {
  BLE_ADMIN_DELETEPINREQUEST_REQUEST = 23, // 发送请求
  BLE_ADMIN_DELETEPINREQUEST_RESULT = 24,  // 等待结果
  BLE_ADMIN_DELETEPINREQUEST_DONE = 25     // 完成
};

enum BLE_ADMIN_GETBATTERYLEVEL_STATE {
  BLE_ADMIN_GETBATTERYLEVEL_REQUEST = 26, // 发送请求
  BLE_ADMIN_GETBATTERYLEVEL_RESULT = 27,  // 等待结果
  BLE_ADMIN_GETBATTERYLEVEL_DONE = 28     // 完成
};

enum BLE_ADMIN_SETTIME_STATE {
  BLE_ADMIN_SETTIME_REQUEST = 29, // 发送请求
  BLE_ADMIN_SETTIME_RESULT = 30,  // 等待结果
  BLE_ADMIN_SETTIME_DONE = 31     // 完成
};

enum BLE_ADMIN_LOCK_STATE {
  BLE_ADMIN_LOCK_REQUEST = 11, // 发送请求
  BLE_ADMIN_LOCK_RESULT = 12,  // 等待结果
  BLE_ADMIN_LOCK_DONE = 13     // 完成
};

typedef struct BLEAdminParam {
  igm_lock_t *lock;
  size_t cmd_request_size; // 某些请求太多了参数, 用一个指针来传入
  void *cmd_request;
}ble_admin_param_t;

int bleInitAdminParam(ble_admin_param_t *unpair_param);
int bleReleaseAdminParam(ble_admin_param_t **pp_admin_param);
int bleSetAdminParam(ble_admin_param_t *unpair_param, igm_lock_t *lock);
int bleSetAdminRequest(
  ble_admin_param_t *admin_param, void *cmd_request, size_t cmd_request_size);

typedef struct BLEAdminResult {
  char addr[MAX_DEVICE_ADDR];
  int result;               // ***_result, 简单判断是否有出错
  int lock_result;
  int unlock_result;
  int unpair_result;
  int getlogs_result;
  int getlockstatus_result;
  int create_pin_request_result;
  int delete_pin_request_result;
  int get_battery_level_result;
  int set_time_result;
  size_t cmd_response_size;     // ****_response 就是相关的response的结构体
  void *cmd_response;     // 纯粹用来把相关的结构体,拷贝出去
}ble_admin_result_t;

int initAdminResult(ble_admin_result_t *result);
int setAdminResultAddr(ble_admin_result_t *result, 
  char *addr, size_t addr_len);
void bleReleaseAdminResult(ble_admin_result_t **pp_result);
void setAdminResultErr(ble_admin_result_t *result, int err);
void setAdminResultCreatePinRequestErr(ble_admin_result_t *result, int err);
void setAdminResultUnlockErr(ble_admin_result_t *result, int err);
void setAdminResultLockErr(ble_admin_result_t *result, int err);
void setAdminResultGetLogsErr(ble_admin_result_t *result, int err);
void setAdminResultDeletePinRequestErr(ble_admin_result_t *result, int err);
void setAdminResultSetTimeErr(ble_admin_result_t *result, int err);
void setAdminResultCMDResponse(
      ble_admin_result_t *result, void *cmd_response, size_t cmd_response_size);
void releaseAdminResultCMDResponse(ble_admin_result_t *result);
void releaseAdminResult(ble_admin_result_t **pp_result);

int isAdminSuccess(ble_admin_result_t *result);
// admin
fsm_table_t *getAdminFsmTable();
int getAdminFsmTableLen();

// admin unpair
fsm_table_t *getAdminUnpairFsmTable();
int getAdminUnpairFsmTableLen();

// admin unlock
fsm_table_t *getAdminUnlockFsmTable();
int getAdminUnlockFsmTableLen();

// admin get logs
fsm_table_t *getAdminGetLogsFsmTable();
int getAdminGetLogsFsmTableLen();

// get lock status
fsm_table_t *getAdminGetLockStatusFsmTable();
int getAdminGetLockStatusFsmTableLen();

// create pin request
fsm_table_t *getAdminCreatePinRequestFsmTable();
int getAdminCreatePinRequestFsmTableLen();

// delete pin request
fsm_table_t *getAdminDeletePinRequestFsmTable();
int getAdminDeletePinRequestFsmTableLen();

// get battery level
fsm_table_t *getAdminGetBatteryLevelFsmTable();
int getAdminGetBatteryLevelFsmTableLen();

// set time
fsm_table_t *getAdminSetTimeFsmTable();
int getAdminSetTimeFsmTableLen();


// admin lock
int getAdminLockFsmTableLen();
fsm_table_t *getAdminLockFsmTable();

#endif
