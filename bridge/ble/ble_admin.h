#ifndef _BLE_ADMIN_H_
#define _BLE_ADMIN_H_
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/fsm.h>

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
  int admin_successed;
  int unlock_result;
  int unpair_result;
}ble_admin_result_t;

int bleInitAdminResult(ble_admin_result_t *result);
int bleSetAdminResultAddr(ble_admin_result_t *result, 
  char *addr, size_t addr_len);
int bleIsAdminSuccess(ble_admin_result_t *result);
int bleSetAdminResultGattConnection(ble_admin_result_t *result, void* gatt_connection);
void bleReleaseAdminResult(ble_admin_result_t **pp_result);

// admin
fsm_table_t *getAdminFsmTable();
int getAdminFsmTableLen();

// admin unpair
fsm_table_t *getAdminUnpairFsmTable();
int getAdminUnpairFsmTableLen();

// admin unlock
fsm_table_t *getAdminUnlockFsmTable();
int getAdminUnlockFsmTableLen();

// admin lock
int getAdminLockFsmTableLen();
fsm_table_t *getAdminLockFsmTable();

#endif