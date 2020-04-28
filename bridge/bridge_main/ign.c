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

sysinfo_t g_sysif;

extern int WaitBtn(void *arg);

int FSMHandle(task_node_t* tn) {
    if(NULL == tn->task_sm_table) {
        serverLog(LL_ERROR, "sm_table is NULL.");
        return -1;
    }

	unsigned int table_max_num = sizeof(*tn->task_sm_table) / sizeof(fsm_table_t);
	int flag = 0;

    // 这儿是遍历所有的状态.
	for (int i = 0; i<table_max_num; i++) {
        serverLog(LL_NOTICE, "FSMHandle i %d ", i);
		if (tn->cur_state == tn->task_sm_table[i].cur_state) {
            serverLog(LL_NOTICE, "eventActFun begin");
			tn->task_sm_table[i].eventActFun(tn);
            serverLog(LL_NOTICE, "eventActFun end");
			tn->cur_state = tn->task_sm_table[i].next_state;

            flag = 1;
            serverLog(LL_NOTICE, "flag in for %d ", flag);
			break;
		}
	}
    serverLog(LL_NOTICE, "flag out for %d ", flag);
    serverLog(LL_NOTICE, "eventActFun end2 ");
    if (0 == flag) {
		// do nothing
        // sm or cur_state err
        serverLog(LL_ERROR, "NO state(%d).", tn->cur_state);
        return -1;
	}
    serverLog(LL_NOTICE, "FSMHandle end");
    return 0;
}

// int GetUserInfo(void* si) {
// 	//send request to server to get userinfo
// 	printf("send request to server to get userinfo!\n");
//     return 0;
// }

// int DealUserInfo(void* si) {
// 	//Recv UserInfo from server 
// 	printf("recv userinfo from server!\n");
//     return 0;
// }

// 首先实现这个扫描的 
int ScanLock(void* tn) {
	//Scan and connect with locks
	printf("scan & connect with locks!\n");
    return 0;
}

int UpdateLockState(void* tn) {
	//update lock status to server
	printf("update lock status to server!\n");
    return 0;
}

int UnLock(void* tn) {
	//unlock
	printf("unlock!\n");
    return 0;
}

int BLEParing(void* tn){
	printf("BLEParing!\n");
    return 0;
}


// int Init(void* tn) {

//     // get WiFi & user info from Mobile App
//     // get AWS pem by http

//     //task_node_t *ptn = (task_node_t*) tn; 
//     g_sysif.mqtt_c = util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
//     if(NULL == g_sysif.mqtt_c) {
//         //goto GoExit;
//         printf("util_initClients err, mqtt_c is NULL.\n");
//     }
//     int rc = MQTTClient_subscribe(g_sysif.mqtt_c, "test", 1);
//     if(MQTTCLIENT_SUCCESS != rc){
//         printf("Subscribe [abc] error with code [%d].\n", rc);
//     }

//     //InitBLE(si);
//     //InitBtn(si);
//     return 0;
// }


fsm_table_t g_fsm_table[] = {
    // {  CMD_INIT,                Init,               GET_WIFI_USER},
    // {  GET_WIFI_USER,           BLEParing,          CMD_REQ_USERINFO},
    // {  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
    // {  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
    {  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},
    {  }
};


// void GoExit(sysinfo_t *si) {
//     //BLE  
//     //MQTT
//     if(NULL != si->mqtt_c){
//         MQTTClient_disconnect(si->mqtt_c, 0);
//         MQTTClient_destroy(&si->mqtt_c);
//     }
// }

    /*
static int FSM(MQTTClient_message *msg){
    switch (msg) {
    case BTREE_TYPE_RBMT:
        err = ctree_tree_open(ctree, path, attr, fd);
        break;

    case BTREE_TYPE_BTRFS:
        err = ctree_bpt_open(ctree, path, attr, fd);
        break;

    case BTREE_TYPE_BITMAP:
        err = do_open_bitmap(ctree, path, attr, fd);
        break;

    default:
        err = -EINVAL;
    }
    return 0;
}
    */


// int HeartBeat(){
//     //send MQTT HB to Server
//     return 0;
// }

void WaitMQTT(void *arg){
    while(1){
        sleep(1);
//         //if (NULL == si->mqtt_c)
//         char *topic = NULL;
//         int topicLen;
//         MQTTClient_message *msg = NULL;
//         int rc = MQTTClient_receive(si->mqtt_c, &topic, &topicLen, &msg, 1e3);
//         if (0 != rc) {
//             //err log
//         }
//         if(msg){
//             //search task queue by msg_id
//             unsigned int msg_id = 1;
//             unsigned int current_state = 1;
//             task_node_t *ptn = NULL;
//             ptn = FindTaskByMsgID(msg_id, &waiting_task_head);
            
//             if (NULL!=ptn) {//move task_node into doing_list task queue
//                 printf("find task_node.msg_id[%u], current_state[%d].\n", ptn->msg_id, ptn->cur_state);
//                 //MoveTask();

//                 pthread_mutex_lock(g_sysif.mutex);
//                 MoveTask(&ptn->list, &doing_task_head);
//                 pthread_mutex_unlock(g_sysif.mutex);
//             }
//             else {//if not exist, add into task queue
//                 printf("find ptn==NULL.\n");
//                 pthread_mutex_lock(g_sysif.mutex);
//                 InsertTask(&doing_task_head, msg_id, current_state, NULL, NULL);
//                 pthread_mutex_unlock(g_sysif.mutex);
//             }

//             //decode get msg_id 
//             //task_node_t* tn = FindTaskByMsgID(msg_id, waiting_list)
            
//             MQTTClient_freeMessage(&msg);
//             MQTTClient_free(topic);
//         } else {
//             //err log
//             HeartBeat();
//         }
    }
}



int WaitBtn(void *arg){
    //if btn
    //add Init into doing_list
    for(;;) {
        sleep(1);
        serverLog(LL_DEBUG, "waiting for Btn...");
    }

    return 0;
}

// 添加扫描方式样例
// ble_data 里面全市
void addDiscoverTask()
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Discover task");
    serverLog(LL_NOTICE, "1. set ble parameters");
    ble_discover_param_t discover_param;
    serverLog(LL_NOTICE, "1. set scan_timeout to 3");
    discover_param.scan_timeout = 3;
    serverLog(LL_NOTICE, "2. set msg_id to 0(or anything you want)");
    int msg_id = 0;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, &discover_param, sizeof(ble_discover_param_t));
    // 与记有多少个结果, 30个锁
    serverLog(LL_NOTICE, "3. init ble result memory, suppose the max num of locks is 30");
    bleInitResults(ble_data, 30, sizeof(igm_lock_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_DISCOVER_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getDiscoverFsmTable(), getDiscoverFsmTableLen()
    );
    serverLog(LL_NOTICE, "5. Add Discover task.");
    return;
}

void visitScanResult(ble_data_t *ble_data)
{
    serverLog(LL_NOTICE, "6. after the task is finished, we can get the data like this.");
    serverLog(LL_NOTICE, "7. get the result point.");
    int num_of_result = bleGetNumsOfResult(ble_data);
    void *result = ble_data->ble_result;
    for (int j=0; j < num_of_result; j++)
    {
        serverLog(LL_NOTICE, "8. get the j:% lock.", j);
        igm_lock_t *lock = bleGetNResult(ble_data, j, sizeof(igm_lock_t));
        serverLog(LL_NOTICE, "name %s  addr: %s", lock->name, lock->addr);
    }
    serverLog(LL_NOTICE, "9. Release the ble data");
    bleReleaseData(&ble_data);
}

int main() {
    serverLog(LL_NOTICE,"Ready to start.");

    //daemon(1, 0);
    // 有g_sysinfo,还需要这个吗?
    // sysinfo_t *si = (sysinfo_t *)malloc(sizeof(sysinfo_t));
    // sysinfoInit(si);
    //Init for paring
    /*int rc = Init(si);
    if (0 != rc) {
        GoExit(si);
        return -1;
    }*/

    // pthread_t mqtt_thread = Thread_start(WaitMQTT, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", mqtt_thread);
    // pthread_t ble_thread = Thread_start(WaitBLE, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", ble_thread);
    // pthread_t bt_thread = Thread_start(WaitBtn, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", bt_thread);
    addDiscoverTask();
    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsDEmpty()) {
            serverLog(LL_NOTICE,"doing_task_head is empty, ready to sleep.");
            // 应该去检查waiting list
            sleep(1);
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
                    serverLog(LL_NOTICE, "one mission error");
                }
                else
                {
                    serverLog(LL_NOTICE, "one mission finished, delete this task");
                    visitScanResult(ptn->ble_data);
                    DeleteDTask(&ptn); // 自动置 ptn 为 NULL
                }
                
                task_node_t *tmp = NextDTask(ptn);
                if (tmp)
                {
                    serverLog(LL_NOTICE, "NextDTask not NULL");
                }
                
                ptn = tmp;
            }
        }
    }

    return 0;
}

