#ifndef _BLE_DISCOVER_H
#define _BLE_DISCOVER_H
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/fsm.h>

enum BLE_DISCOVER_STATE {
  BLE_DISCOVER_BEGIN = 0,     // 表示用户需要启动, 这是启动标识
  BLE_DISCOVER_DONE = 1,    // 完成
};

typedef struct BLEDiscoverParam {
  int scan_timeout;
}ble_discover_param_t;

typedef struct BLEDiscoverResult {
  igm_lock_t *lokcs;
  int lock_len;
}ble_discover_result_t;

fsm_table_t *getDiscoverFsmTable();
int getDiscoverFsmTableLen();
void ble_discovered_device(
  void *adapter, const char* addr, const char* name, void *user_data);



#endif

