#ifndef _TASK_H_
#define _TASK_H_

#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/fsm.h>
#include <bridge/bridge_main/mqttData.h>
#include <bridge/bridge_main/bleData.h>
#include <bridge/bridge_main/list.h>

typedef struct TaskNode {
    struct list_head list;
    char lock_id[32];
    // void* lockinfo; // 外部数据?
	//struct task_node *next;
	unsigned int msg_id;
    ble_data_t *dataBLE; // 外部数据, 外部释放?
    mqtt_data_t *dataMQTT; // 外部数据, 外部释放?
    //void * dataBtn;
    unsigned int start_time;
    unsigned char cur_state;
    sysinfo_t *sysif;
    fsm_table_t *p_sm_table; // 外部数据
}task_node_t;


#endif