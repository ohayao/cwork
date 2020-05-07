#ifndef _BLE_PAIRING_H_
#define _BLE_PAIRING_H_

#include <bridge/gattlib/gattlib.h>
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/fsm.h>

enum BLE_PAIRING_STATE {
  BLE_PAIRING_BEGIN = 0,     // 表示用户需要启动, 这是启动标识
  BLE_PAIRING_STEP1 = 1,    // 完成
  BLE_PAIRING_STEP2 = 2,
  BLE_PAIRING_STEP3 = 3,
  BLE_PAIRING_STEP4 = 5,
  BLE_PAIRING_COMMIT = 6,
  BLE_WAITING_COMMIT = 7,
  BLE_PAIRING_DONE = 8
};

// 总得让我知道需要pairing 哪把锁
typedef struct BLEPairingParam {
  // 别用指针, 深拷贝问题,会相当麻烦,如果里面还有指针
  // 会疯
  igm_lock_t *lock;
}ble_pairing_param_t;

int bleInitPairingParam(ble_pairing_param_t *pairing_param);
int bleReleaseParingParam(ble_pairing_param_t *pairing_param);
int bleSetPairingParam(ble_pairing_param_t *pairing_param, igm_lock_t *lock);

typedef struct BLEPairingResult {
  char addr[MAX_DEVICE_ADDR];
  int pairing_successed;
  int has_admin_key;
  uint8_t *admin_key;
  size_t admin_key_len;
  int has_password;
  uint8_t *password;
  size_t password_size;
}ble_pairing_result_t;

int bleInitPairingResult(ble_pairing_result_t *result);
int bleSetPairingSuccess(ble_pairing_result_t *pairing_result, int s);
int bleReleaseResultAdminKey(ble_pairing_result_t *result);
int bleReleaseResultPassword(ble_pairing_result_t *result);
int bleReleasePairingResult(ble_pairing_result_t **pp_result);
int bleSetPairingResultAdminKey(ble_pairing_result_t *result, 
  uint8_t *admin_key, int admin_key_len);
int bleSetPairingResultPassword(ble_pairing_result_t *result, 
  uint8_t *password, int password_size);
int bleGetPairingResultAdminKey(ble_pairing_result_t *result, 
  uint8_t *admin_key, int *p_admin_key_len);
int bleGetPairingResultPassword(ble_pairing_result_t *result, 
  uint8_t *password, int *p_password_size);
int bleSetPairingResultAddr(ble_pairing_result_t *result, 
  char *addr, size_t addr_len);

fsm_table_t *getPairingFsmTable();
int getPairingFsmTableLen();


#endif