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

enum BLE_ADMIN_LOCK_STATE {
  BLE_ADMIN_LOCK_REQUEST = 11, // 发送请求
  BLE_ADMIN_LOCK_RESULT = 12,  // 等待结果
  BLE_ADMIN_LOCK_DONE = 13     // 完成
};

typedef struct BLEAdminParam {
  // 需要addr, 需要key和password
  igm_lock_t *lock;
}ble_admin_param_t;

int bleInitAdminParam(ble_admin_param_t *unpair_param);
int bleReleaseAdminParam(ble_admin_param_t **punpair_param);
int bleSetAdminParam(ble_admin_param_t *unpair_param, igm_lock_t *lock);

typedef struct BLEAdminResult {
  char addr[MAX_DEVICE_ADDR];
  int admin_result;               // ***_result, 简单判断是否有出错
  int lock_result;
  int unlock_result;
  int unpair_result;
  int getlogs_result;
  int getlockstatus_result;
  size_t cmd_response_size;     // ****_response 就是相关的response的结构体
  void *cmd_response;     // 纯粹用来把相关的结构体,拷贝出去
}ble_admin_result_t;

int initAdminResult(ble_admin_result_t *result);
int setAdminResultAddr(ble_admin_result_t *result, 
  char *addr, size_t addr_len);
void bleReleaseAdminResult(ble_admin_result_t **pp_result);
void setAdminResultErr(ble_admin_result_t *result, int err);
void setAdminResultUnlockErr(ble_admin_result_t *result, int err);
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

// admin lock
int getAdminLockFsmTableLen();
fsm_table_t *getAdminLockFsmTable();

#endif