#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdlib.h>
#include <bridge/bridge_main/ign_constants.h>
#include <bridge/bridge_main/log.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/ign.h>
#include <bridge/bridge_main/task.h>
#include <bridge/bridge_main/thread_helper.h>
#include <bridge/bridge_main/mutex_helper.h>
#include <bridge/bridge_main/task_queue.h>
#include <bridge/bridge_main/wait_ble.h>
#include <bridge/ble/ble_discover.h>
#include <bridge/ble/lock.h>
#include <bridge/ble/ble_pairing.h>
#include <bridge/bridge_main/lock_list.h>
//#include <bridge/ble/ble_guest.h>
#include <bridge/ble/ble_pairing.h>
#include <bridge/mqtt/mqtt_constant.h>
#include <bridge/mqtt/mqtt_util.h>
#include <bridge/mqtt/cJSON.h>
#include <bridge/proto/ign.pb.h>
#include <bridge/proto/pb_encode.h>
#include <bridge/proto/pb_decode.h>
sysinfo_t g_sysif;

void addDiscoverTask(int msg_id);
void addAdminTask(igm_lock_t *lock, int msg_id);

int FSMHandle(task_node_t* tn) {
    if(NULL == tn->task_sm_table) {
        printf( "sm_table is NULL.");
        return -1;
    }
    // sizeof 在定义某数组的时候, sizeof 指针 返回整个数组的字节数
    // sizeof 对指针的时候, 只能返回指针大小
    // sizeof 对指针所知数组的某个内容的时候, 只能返回该项大小, 
    // 也就是只会返回 sm_table 当中某一项的大小
    // 所以我手动返回了长度.
	unsigned int table_max_num = tn->sm_table_len;
    printf( "table_max_num %d", table_max_num);
	int flag = 0;
    // 这儿是遍历所有的状态.
	for (int i = 0; i<table_max_num; i++) {
        printf( "FSMHandle i %d ", i);
		if (tn->cur_state == tn->task_sm_table[i].cur_state) {
            printf( "eventActFun begin---------------");
            // 增加一个判断当前函数, 是否当前函数出错. 0 表示没问题
			int event_result = tn->task_sm_table[i].eventActFun(tn);
            if (event_result)
            {
                flag = 0;
                break;
            }
            else
            {
                tn->cur_state = tn->task_sm_table[i].next_state;
                flag = 1;
            }
            printf( "eventActFun end-----------------");
            
		}
	}
    printf( "FSMHandle out for end-----------------");
    if (0 == flag) {
		// do nothing
        // sm or cur_state err
        printf( "something wrogin int fsm, one of the event function error, state(%d) error.", tn->cur_state);
        return -1;
	}
    printf( "FSMHandle end");
    return 0;
}

static int hbInterval=0;


// 添加扫描方式样例
// ble_data 里面全市
void addDiscoverTask(int msg_id)
{
    // 设置需要的参数
    ble_discover_param_t discover_param;
    printf( "1. set scan_timeout to 5");
    discover_param.scan_timeout = 5;
    printf( "2. set msg_id to 0(or anything you want)");
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    printf( "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    bleInitData(ble_data);
    printf( "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, &discover_param, sizeof(ble_discover_param_t));
    // 与记有多少个结果, 30个锁
    printf( "3. init ble result memory, suppose the max num of locks is 30");
    bleInitResults(ble_data, 30, sizeof(igm_lock_t));

    // 插入系统的队列
    printf( "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DTail(msg_id, BLE_DISCOVER_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getDiscoverFsmTable(), getDiscoverFsmTableLen(), TASK_BLE_DISCOVER
    );
    printf( "5. Add Discover task.");
    return;
}

void saveTaskData(task_node_t *ptn) {
    if (!ptn) return;
    if(ptn->ble_data && ptn->ble_data_len) {
        ble_data_t *ble_data = ptn->ble_data;
        int task_type = ptn->task_type;
        switch (task_type) {
            case TASK_BLE_DISCOVER: {
                    printf( "saving ble TASK_BLE_DISCOVER data");
                    int num_of_result = bleGetNumsOfResult(ble_data);
                    void *result = ble_data->ble_result;
                    for (int j=0; j < num_of_result; j++) {
                        igm_lock_t *lock = bleGetNResult(ble_data, j, sizeof(igm_lock_t));
                        printf( "name %s  addr: %s", lock->name, lock->addr);
                        insertLock(lock);
                        // test 需要, 
                        // if (!lock->paired)
                        // {
                        //     printf( "try to pair name %s  addr: %s", lock->name, lock->addr);
                        //     addPairingTask(lock);
                        // }              
                    }
                    break;
                }
            default:
                break;
        }
    }
}

int main() {
    // Init(NULL);
    printf("test ble discover Ready to start.");
    int msg_id = 1;
    addDiscoverTask(msg_id);
    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsDEmpty()) {
            printf("doing_task_head is empty, check Lock list.");
            // 应该去检查waiting list
            printLockList();
            printf("doing_task_head is empty, ready to sleep.");
            // sleep(1);
            // 只是用于来扫描一次
            break;
        } 
        else {
        // if doing list has task
            // 获取当前doing list 的头部
            task_node_t *ptn=GetDHeadNode();
            while (ptn)
            {
                // TODO, 当任务完成,需要怎么处理?
                int ret = FSMHandle(ptn);
                if(ret)
                {
                    printf( "one mission error");
                }
                else
                {
                    printf( "one mission finished, delete this task");
                }
                saveTaskData(ptn);
                DeleteDTask(&ptn); // 自动置 ptn 为 NULL
                
                task_node_t *tmp = NextDTask(ptn);
                if (tmp)
                {
                    printf( "NextDTask not NULL");
                }
                
                ptn = tmp;
            }
        }
    }
    return 0;
}

