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
  BLE_PAIRING_DONE = 7
};

// 总得让我知道需要pairing 哪把锁
typedef struct BLEPairingParam {
  igm_lock_t *lock;
}ble_pairing_param_t;

int bleInitPairingParam(ble_pairing_param_t *pairing_param);
int bleReleaseParingParam(ble_pairing_param_t *pairing_param);
int bleSetPairingParam(ble_pairing_param_t *pairing_param, igm_lock_t *lock);

typedef struct BLEPairingResult {
  int pairing_successed;
  uint8_t *admin_key;
  size_t admin_key_len;
  uint8_t *password;
  size_t password_size;
}ble_pairing_result_t;



#endif