#ifndef _BLE_UNPAIR_H_
#define _BLE_UNPAIR_H_
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/fsm.h>

enum BLE_ADMIN_UNPAIR_STATE {
  BLE_ADMIN_UNPAIR_BEGIN = 0,          // 注册一个改变通知
  BLE_ADMIN_UNPAIR_STEP1 = 1,          // 收听锁的第一步信息
  BLE_ADMIN_UNPAIR_STEP2 = 2,          // 写给锁的第二步信息
  BLE_ADMIN_UNPAIR_ESTABLISHED = 3,     // 此时已经建立
  BLE_ADMIN_UNPAIR_REQUEST = 4, // 发送请求
  BLE_ADMIN_UNPAIR_RESULT = 5,  // 等待结果
  BLE_ADMIN_UNPAIR_DONE = 6     // 完成
};


typedef struct BLEAdminUnpairParam {
  // 需要addr, 需要key和password
  igm_lock_t *lock;
}ble_admin_unpair_param_t;

int bleInitAdminUnpairParam(ble_admin_unpair_param_t *unpair_param);
int bleReleaseAdminUnpairParam(ble_admin_unpair_param_t *unpair_param);
int bleSetAdminUnpairParam(ble_admin_unpair_param_t *unpair_param, igm_lock_t *lock);

typedef struct BLEAdminUnpairResult {
  char addr[MAX_DEVICE_ADDR];
  int unpair_successed;
}ble_admin_unpair_result_t;

int bleInitAdminUnpairResult(ble_admin_unpair_result_t *result);
int bleSetAdminUnpairResultAddr(ble_admin_unpair_result_t *result, 
  char *addr, size_t addr_len);
int bleIsAdminUnpairSuccess(ble_admin_unpair_result_t *result);

fsm_table_t *getAdminUnpairFsmTable();
int getAdminUnpairFsmTableLen();
#endif