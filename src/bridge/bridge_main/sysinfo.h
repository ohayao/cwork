#ifndef _SYSINFO_H_
#define _SYSINFO_H_
#include <pthread.h>
#include <MQTTClient.h>
//#include "bridge/ble/ble_admin.h"

enum BLE_TASK {
    BLE_NONE_TASK = 0, // 没有任务
    BLE_DISCOVER_LOCK = 1, // 扫描锁
};

typedef struct LockInfo {
	unsigned char lock_addr_size;
    char lock_addr[32];
	unsigned char lock_ekey_size;
    char lock_ekey[32];
	unsigned char lock_token_size;
    char lock_token[128];
	unsigned char lock_passwd_size;
    char lock_passwd[32];
	unsigned char lock_id_size;
    char lock_id[32];
	void* gatt_connection;
	void* gatt_adapter;
}LockInfo_t;

typedef struct ble_addr{
	char name[32];
	char addr[32];
}ble_addr_t;

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
    unsigned char lock_total;
    LockInfo_t* lockinfo;
	void* ble_adapter;
	ble_addr_t* ble_list;
	unsigned char ble_list_n;
}sysinfo_t;

// 操作都会使用互斥锁

int sysinfoInit(sysinfo_t *sysinfo);

#endif
