#ifndef _BLE_DISCOVER_H
#define _BLE_DISCOVER_H
#include <bridge/ble/lock.h>

enum BLE_DISCOVER_STATE {
  BLE_DISCOVER_WRONG = 0,     // 表示出错, 预留
  BLE_DISCOVER_BEGIN = 1,     // 表示用户需要启动, 这是启动标识
  BLE_DISCOVER_ACTIVE = 2,    // 正在发现, 因为蓝牙需要设定秒数
  BLE_DISCOVER_DONE = 3       // 发现完成
}

typedef struct BLEDiscoverParam {
  int scan_timeout;
}ble_discover_param_t;

typedef struct BLEDiscoverResult {
  igm_lock_t lokcs[];
  int lock_len;
}ble_discover_result_t;

#endif

