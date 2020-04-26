#ifndef _TASK_H_
#define _TASK_H_
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/list.h>
#include <bridge/bridge_main/fsm.h>

typedef struct task_node {
    struct list_head list;
    sysinfo_t *sysif;
    char lock_id[32];
    void* lockinfo; // 外部数据?
	//struct task_node *next;
	unsigned int msg_id;
    void * dataBLE; // 外部数据, 外部释放?
    void * dataMQTT; // 外部数据, 外部释放?
    //void * dataBtn;
    unsigned int start_time;
    fsm_table_t* p_sm_table; // 外部数据, 如何保证生命周期? 其实要用weak_ptr模式保证声明周期
    unsigned char cur_state;
}task_node_t;



#endif