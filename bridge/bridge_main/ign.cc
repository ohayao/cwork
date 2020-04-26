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

// #include "util.h"
// #include "task_queue.h"
#include "ign.h"
#include "bridge/bridge_main/sysinfo.h"
#include <bridge/bridge_main/fsm.h>
#include <bridge/bridge_main/task.h>

// LIST_HEAD(waiting_task_head);
// LIST_HEAD(doing_task_head);
 

sysinfo_t g_sysif;


// int FSMHandle(task_node_t* tn) {
//     if(NULL == tn->p_sm_table) {
//         serverLog(LL_ERROR, "sm_table is NULL.");
//         return -1;
//     }

// 	unsigned int table_max_num = sizeof(*tn->p_sm_table) / sizeof(fsm_table_t);
// 	int flag = 0;

// 	for (int i = 0; i<table_max_num; i++) {
// 		if (tn->cur_state == tn->p_sm_table[i].cur_state) {
// 			tn->p_sm_table[i].eventActFun(tn);
// 			tn->cur_state = tn->p_sm_table[i].next_state;
//             flag = 1;
// 			break;
// 		}
// 	}

//     if (0 == flag) {
// 		// do nothing
//         // sm or cur_state err
//         serverLog(LL_ERROR, "NO state(%d).", tn->cur_state);
//         return -1;
// 	}
//     return 0;
// }


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

fsm_table_t g_sm_table[] = {
    // {  CMD_INIT,                Init,               GET_WIFI_USER},
    // {  GET_WIFI_USER,           BLEParing,          CMD_REQ_USERINFO},
    // {  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
    // {  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
    {  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},
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

// void WaitMQTT(sysinfo_t *si){
//     while(1){
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
//     }
// }

// int WaitBLE(sysinfo_t *si){
//     //Thread_start(wait_BLE, sysinfo)
//     for(;;) {
//         sleep(1);
//         serverLog(LL_DEBUG, "waiting for BLE...");
//     }
//     return 0;
// }

// int WaitBtn(sysinfo_t *si){
//     //if btn
//     //add Init into doing_list
//     for(;;) {
//         sleep(1);
//         serverLog(LL_DEBUG, "waiting for Btn...");
//     }

//     return 0;
// }

int main() {
    serverLog(LL_NOTICE,"Ready to start.");
    //daemon(1, 0);
    sysinfo_t *si = (sysinfo_t *)malloc(sizeof(sysinfo_t));
    sysinfoInit(si);
    //Init for paring
    /*int rc = Init(si);
    if (0 != rc) {
        GoExit(si);
        return -1;
    }*/

    // 这句可以不用了, 因为用了std::mutex, 不用再创建了
    // g_sysif.mutex = Thread_create_mutex();

    // 使用
    // INIT_LIST_HEAD(&waiting_task_head);
    // INIT_LIST_HEAD(&doing_task_head);


    // thread_type thread = Thread_start(WaitMQTT, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", thread);
    // thread = Thread_start(WaitBLE, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitBLE[%u].", thread);
    // thread = Thread_start(WaitBtn, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitBtn[%u].", thread);

    // while(1) {
    //     //if empty, sleep(0.5);
    //     //do it , after set into waiting_list
    //     if (IsEmpty(&doing_task_head)) {
    //         serverLog(LL_NOTICE,"doing_task_head is empty, ready to sleep.");
    //         sleep(1);
    //     } else {
    //         task_node_t *ptn=NULL;
    //         list_head_t* po=&doing_task_head;
    //         for(; po != NULL; po = &ptn->list) { 
    //             ptn = NextTask(po, &doing_task_head);
    //             if (NULL != ptn) {
    //                 FSMHandle(ptn);
    //                 //move ptn to waiting_task_head

    //                 pthread_mutex_lock(g_sysif.mutex);
    //                 MoveTask(&ptn->list, &waiting_task_head);
    //                 pthread_mutex_unlock(g_sysif.mutex);
    //             }
    //         }
    //     }
    // }

    return 0;
}

