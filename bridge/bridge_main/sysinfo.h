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
    pthread_mutex_t *p_mutex;
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
    void *ble_result;   // 结果, 不同的任务,解析结果的方式不同,
}sysinfo_t;

#endif