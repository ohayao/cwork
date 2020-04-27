#ifndef _SYSINFO_H_
#define _SYSINFO_H_
#include <pthread.h>

enum BLE_TASK {
    BLE_NONE_TASK = 0, // 没有任务
    BLE_DISCOVER_LOCK = 1, // 扫描锁
};

typedef struct SysInfo {
    int inited;
    // MQTTClient mqtt_c;
    // 去除 这个mutex, 建议所有操作, 都写到相应的.c文件, 每次修改获取都自动加锁,
    // 以免某程序忘记加锁,减少出错
    char wifi_ssid[128];
    char wifi_pswd[32];
    char user_id[64];
    char user_pswd[32];
    void* userinfo;
    char lock1_id[32];
    char lock2_id[32];
    char lock3_id[32];
    char lock4_id[32];
    char lock5_id[32];
    void* lock1info;
    void* lock2info;
    void* lock3info;
    void* lock4info;
    void* lock5info;
    int ble_task; // task标示不同的任务,
    int ble_task_state; // 不同task里面的不同状态
    // 参数, 不同的任务,参数不同
    // 这些使用完,设置为Null,使用前, 不是Null,都释放?保证不会泄漏内存.?
    void *ble_param;
    int ble_param_len;    
    void *ble_result;   // 结果, 不同的任务,解析结果的方式不同,
    int ble_result_len
}sysinfo_t;

// 操作都会使用互斥锁

int sysinfoInit(sysinfo_t *sysinfo);

// 获取param长度
int sysinfoGetBleParamLen(sysinfo_t *sysinfo, int *result_len);
// 自备上述长度内存, 拷贝内容到自己相应区域
int sysinfoGetBleParam(sysinfo_t *sysinfo, void *result);
// 释放已经设置的内容
int sysinfoReleaseBleParam(sysinfo_t *sysinfo);
// 设置相应的参数内容
int sysinfoSetBleParam(sysinfo_t *sysinfo, void *ble_param, int ble_param_len);

// 获取长度
int sysinfoGetBleResultLen(sysinfo_t *sysinfo, int *result_len);
// 自备上诉长度内存, 拷贝结果内容到自备内存
int sysinfoGetBleResult(sysinfo_t *sysinfo, void *result);
// 释放Ble,结果的内容
int sysinfoReleaseBleResult(sysinfo_t *sysinfo);
// 设置相应结果
int sysinfoSetBleResult(sysinfo_t *sysinfo, void *ble_result, int ble_result_len);

#endif