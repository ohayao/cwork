#ifndef _SYSINFO_H_
#define _SYSINFO_H_
#include <mutex>

class SysInfo {
public:
    int inited;
    // MQTTClient mqtt_c;
    std::mutex mutex;
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
    SysInfo();
};

typedef SysInfo sysinfo_t;

#endif