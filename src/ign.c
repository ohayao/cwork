#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <syslog.h>

#include "util.h"
#include "task_queue.h"


LIST_HEAD(waiting_task_head);
LIST_HEAD(doing_task_head);
 
sysinfo_t g_sysif;


int FSMHandle(task_node_t* tn) {
    if(NULL == tn->p_sm_table) {
        serverLog(LL_ERROR, "sm_table is NULL.");
        return -1;
    }

	unsigned int table_max_num = sizeof(*tn->p_sm_table) / sizeof(fsm_table_t);
	int flag = 0;

	for (int i = 0; i<table_max_num; i++) {
		if (tn->cur_state == tn->p_sm_table[i].cur_state) {
			tn->p_sm_table[i].eventActFun(tn);
			tn->cur_state = tn->p_sm_table[i].next_state;
            flag = 1;
			break;
		}
	}

    if (0 == flag) {
		// do nothing
        // sm or cur_state err
        serverLog(LL_ERROR, "NO state(%d).", tn->cur_state);
        return -1;
	}
    return 0;
}


int GetUserInfo(void* si) {
	//send request to server to get userinfo
	printf("send request to server to get userinfo!\n");
    return 0;
}

int DealUserInfo(void* si) {
	//Recv UserInfo from server 
	printf("recv userinfo from server!\n");
    return 0;
}

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


int Init(void* tn) {

    // get WiFi & user info from Mobile App
    // get AWS pem by http

    //task_node_t *ptn = (task_node_t*) tn; 
    g_sysif.mqtt_c = util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(NULL == g_sysif.mqtt_c) {
        //goto GoExit;
        printf("util_initClients err, mqtt_c is NULL.\n");
    }
    int rc = MQTTClient_subscribe(g_sysif.mqtt_c, "test", 1);
    if(MQTTCLIENT_SUCCESS != rc){
        printf("Subscribe [abc] error with code [%d].\n", rc);
    }

    //InitBLE(si);
    //InitBtn(si);
    return 0;
}

fsm_table_t g_sm_table[] = {
    {  CMD_INIT,                Init,               GET_WIFI_USER},
    {  GET_WIFI_USER,           BLEParing,          CMD_REQ_USERINFO},
    {  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
    {  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
    {  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},
};


void GoExit(sysinfo_t *si) {
    //BLE  
    //MQTT
    if(NULL != si->mqtt_c){
        MQTTClient_disconnect(si->mqtt_c, 0);
        MQTTClient_destroy(&si->mqtt_c);
    }
}

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

thread_type Thread_start(void* fn, void* parameter) {
    thread_type thread = 0;
    pthread_attr_t attr;

    //FUNC_ENTRY
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, fn, parameter) != 0)
        thread = 0;
    pthread_attr_destroy(&attr);
    //FUNC_EXIT;
    return thread;
}

mutex_type Thread_create_mutex(void)
{
    mutex_type mutex = NULL;

    //FUNC_ENTRY;
    mutex = malloc(sizeof(pthread_mutex_t));
    int rc = pthread_mutex_init(mutex, NULL);
    if (0>rc)
        serverLog(LL_ERROR,"create thread error, rc[%d].", rc);
    else
        serverLog(LL_NOTICE,"create thread rc[%d].", rc);
    //FUNC_EXIT_RC(rc);
    return mutex;
}
void Thread_destroy_mutex(mutex_type mutex)
{
    //int rc = 0;
    //FUNC_ENTRY;
    pthread_mutex_destroy(mutex);
    free(mutex);
    //FUNC_EXIT_RC(rc);
}

//thread_type thread = Thread_start(thread_fn fn, void* parameter);
//mutex_type mutex = Thread_create_mutex();
//rc = pthread_mutex_lock(mutex);
//rc = pthread_mutex_unlock(mutex);
//pthread_self();
//pthread_detach(p);??????


int HeartBeat(){
    //send MQTT HB to Server
    return 0;
}

void WaitMQTT(sysinfo_t *si){
    while(1){
        //if (NULL == si->mqtt_c)
        char *topic = NULL;
        int topicLen;
        MQTTClient_message *msg = NULL;
        int rc = MQTTClient_receive(si->mqtt_c, &topic, &topicLen, &msg, 1e3);
        if (0 != rc) {
            //err log
        }
        if(msg){
            //search task queue by msg_id
            unsigned int msg_id = 1;
            unsigned int current_state = 1;
            task_node_t *ptn = NULL;
            ptn = FindTaskByMsgID(msg_id, &waiting_task_head);
            
            if (NULL!=ptn) {//move task_node into doing_list task queue
                printf("find task_node.msg_id[%u], current_state[%d].\n", ptn->msg_id, ptn->cur_state);
                //MoveTask();
                MoveTask(&ptn->list, &doing_task_head);
            }
            else {//if not exist, add into task queue
                printf("find ptn==NULL.\n");
                InsertTask(&doing_task_head, msg_id, current_state, NULL, NULL);
            }

            //decode get msg_id 
            //task_node_t* tn = FindTaskByMsgID(msg_id, waiting_list)
            
            MQTTClient_freeMessage(&msg);
            MQTTClient_free(topic);
        } else {
            //err log
            HeartBeat();
        }
    }
}

int WaitBLE(sysinfo_t *si){
    //Thread_start(wait_BLE, sysinfo)
    for(;;) {
        sleep(1);
        serverLog(LL_DEBUG, "waiting for BLE...");
    }
    return 0;
}

int WaitBtn(sysinfo_t *si){
    //if btn
    //add Init into doing_list
    for(;;) {
        sleep(1);
        serverLog(LL_DEBUG, "waiting for Btn...");
    }

    return 0;
}

int main() {
    serverLog(LL_NOTICE,"Ready to start.");
    //daemon(1, 0);
    //sysinfo_t *si = malloc(sizeof(sysinfo_t));
    memset(&g_sysif, 0, sizeof(sysinfo_t));
    //Init for paring
    /*int rc = Init(si);
    if (0 != rc) {
        GoExit(si);
        return -1;
    }*/
    g_sysif.mutex = Thread_create_mutex();

    INIT_LIST_HEAD(&waiting_task_head);
    INIT_LIST_HEAD(&doing_task_head);


    thread_type thread = Thread_start(WaitMQTT, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", thread);
    thread = Thread_start(WaitBLE, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitBLE[%u].", thread);
    thread = Thread_start(WaitBtn, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitBtn[%u].", thread);

    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsEmpty(&doing_task_head)) {
            serverLog(LL_NOTICE,"doing_task_head is empty, ready to sleep.");
            sleep(1);
        } else {
            task_node_t *ptn=NULL;
            list_head_t* po=&doing_task_head;
            for(; po != NULL; po = &ptn->list) { 
                ptn = NextTask(po, &doing_task_head);
                if (NULL != ptn) {
                    FSMHandle(ptn);
                    //move ptn to waiting_task_head
                    MoveTask(&ptn->list, &waiting_task_head);
                }
            }
        }
    }

    return 0;
}

