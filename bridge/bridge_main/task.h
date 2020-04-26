#ifndef _TASK_H_
#define _TASK_H_
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/fsm.h>
#include <memory>
#include <bridge/bridge_main/mqttData.h>
#include <bridge/bridge_main/bleData.h>
using std::shared_ptr;

class task_node {
public:
    char lock_id[32];
    // void* lockinfo; // 外部数据?
	//struct task_node *next;
	unsigned int msg_id;
    void * dataBLE; // 外部数据, 外部释放?
    // void * dataMQTT; // 外部数据, 外部释放?
    //void * dataBtn;
    unsigned int start_time;
    unsigned char cur_state;
    std::shared_ptr<sysinfo_t> sysif;
    std::shared_ptr<fsm_table_t> p_sm_table; // 外部数据, 如何保证生命周期? 其实要用weak_ptr模式保证声明周期
    
    task_node() = delete;
    task_node(unsigned int msg_id, MqttData p_dataMQTT);
    task_node(unsigned int msg_id, BleData p_dataBLE);
};

typedef task_node task_node_t;

#endif