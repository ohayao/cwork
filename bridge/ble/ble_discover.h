#ifndef _BLE_DISCOVER_H
#define _BLE_DISCOVER_H

enum BLE_DISCOVER_STATE {
  BLE_DISCOVER_WRONG = 0,     // 表示出错, 预留
  BLE_DISCOVER_BEGIN = 1,     // 表示用户需要启动, 这是启动标识
  BLE_DISCOVER_ACTIVE = 2,    // 正在发现, 因为蓝牙需要设定秒数
  BLE_DISCOVER_DONE = 3       // 发现完成
}

#endif

