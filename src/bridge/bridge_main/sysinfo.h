#ifndef _SYSINFO_H_
#define _SYSINFO_H_
#include <pthread.h>
#include <MQTTClient.h>
#include "bridge/ble/ble_admin.h"

enum BLE_TASK {
    BLE_NONE_TASK = 0, // 没有任务
    BLE_DISCOVER_LOCK = 1, // 扫描锁
};

typedef struct LockInfo {
    char lock_addr[32];
    char lock_ekey[32];
    char lock_passwd[32];
    char lock_id[32];
	void* gatt_connection;
	void* gatt_adapter;
}LockInfo_t;

typedef struct SysInfo {
    int inited;
    MQTTClient mqtt_c;
    // 去除 这个mutex, 建议所有操作, 都写到相应的.c文件, 每次修改获取都自动加锁,
    // 以免某程序忘记加锁,减少出错
	//pthread_mutex_t* mutex;
    char mac[16];
    char wifi_ssid[128];
    char wifi_pswd[32];
    char user_id[64];
    char user_pswd[32];
    void* userinfo;
    char lock_total;
    LockInfo_t** lockinfo;
}sysinfo_t;

// 操作都会使用互斥锁

int sysinfoInit(sysinfo_t *sysinfo);

#endif
