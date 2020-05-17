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
#include <bridge/ble/ble_admin.h>
#include <bridge/ble/ble_pairing.h>
#include "bridge/mqtt/mqtt_constant.h"
#include "bridge/mqtt/mqtt_util.h"
#include "bridge/mqtt/cJSON.h"
#include <bridge/proto/ign.pb.h>
#include <bridge/proto/pb_encode.h>
#include <bridge/proto/pb_decode.h>
sysinfo_t g_sysif;

extern int WaitBtn(void *arg);
ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID);
void addPairingTask(igm_lock_t *lock, int msg_id);
void addDiscoverTask(int msg_id);
void addAdminTask(igm_lock_t *lock, int msg_id);

int FSMHandle(task_node_t* tn) {
    if(NULL == tn->task_sm_table) {
        serverLog(LL_ERROR, "sm_table is NULL.");
        return -1;
    }
    // sizeof 在定义某数组的时候, sizeof 指针 返回整个数组的字节数
    // sizeof 对指针的时候, 只能返回指针大小
    // sizeof 对指针所知数组的某个内容的时候, 只能返回该项大小, 
    // 也就是只会返回 sm_table 当中某一项的大小
    // 所以我手动返回了长度.
	unsigned int table_max_num = tn->sm_table_len;
    serverLog(LL_NOTICE, "table_max_num %d", table_max_num);
	int flag = 0;
    // 这儿是遍历所有的状态.
	for (int i = 0; i<table_max_num; i++) {
        serverLog(LL_NOTICE, "FSMHandle i %d ", i);
		if (tn->cur_state == tn->task_sm_table[i].cur_state) {
            serverLog(LL_NOTICE, "eventActFun begin---------------");
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
            serverLog(LL_NOTICE, "eventActFun end-----------------");
            
		}
	}
    serverLog(LL_NOTICE, "FSMHandle out for end-----------------");
    if (0 == flag) {
		// do nothing
        // sm or cur_state err
        serverLog(LL_ERROR, "something wrogin int fsm, one of the event function error, state(%d) error.", tn->cur_state);
        return -1;
	}
    serverLog(LL_NOTICE, "FSMHandle end");
    return 0;
}

ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID){
    ign_BridgeProfile bp={};
    bp.os_info=ign_OSType_LINUX;
    char temp[100];
    memset(temp,0,sizeof(temp));
    strcpy(temp,bridgeID);
    bp.bt_id.size=strlen(temp);
    memcpy(bp.bt_id.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"mac_addr");
    bp.mac_addr.size=strlen(temp);
    memcpy(bp.mac_addr.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"local_ip");
    bp.local_ip.size=strlen(temp);
    memcpy(bp.local_ip.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"public_ip");
    bp.public_ip.size=strlen(temp);
    memcpy(bp.public_ip.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"sys_statics");
    bp.sys_statics.size=strlen(temp);
    memcpy(bp.sys_statics.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"wifi_ssid");
    bp.wifi_ssid.size=strlen(temp);
    memcpy(bp.wifi_ssid.bytes,temp,strlen(temp));
    bp.wifi_signal=2;
    bp.inited_time=get_ustime();

    memset(temp,0,sizeof(temp));
    strcpy(temp,"bridge_name");
    bp.name.size=strlen(temp);
    memcpy(bp.name.bytes,temp,strlen(temp));
    return bp;
}

static int hbInterval=0;
int HeartBeat(){
    //send MQTT HB to Server
    hbInterval++;
    hbInterval=hbInterval%10;
    if(hbInterval>0) return 0;
    ign_MsgInfo hb={};
    hb.event_type=ign_EventType_HEARTBEAT;
    hb.time=get_ustime();
    hb.msg_id=get_ustime();
    ign_BridgeEventData bed={};
    bed.has_profile=true;
    bed.profile=Create_IgnBridgeProfile("abcdef");
    hb.has_bridge_data=true;
    hb.bridge_data=bed;


    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,ign_MsgInfo_fields,&hb)){
        size_t len=out.bytes_written;
        if((publish_result=sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
            serverLog(LL_ERROR, "SEND MQTT HB ERROR WITH CODE[%d]", publish_result);
        }else{
            serverLog(LL_NOTICE, "SEND MQTT HB SUCCESS");
        }
    }else{
        serverLog(LL_ERROR, "ENCODE MQTT HB ERROR");
    }
    return 0;
}

int GetUserInfo(void* si) {
	//send request to server to get userinfo
    serverLog(LL_NOTICE, "send request to server to get userinfo!");
    return 0;
}

// int DealUserInfo(void* si) {
// 	//Recv UserInfo from server 
// 	printf("recv userinfo from server!\n");
//     return 0;
// }

// 首先实现这个扫描的 
int ScanLock(void* tn) {
	//Scan and connect with locks
    serverLog(LL_NOTICE, "scan & connect with locks!");
    return 0;
}

int UpdateLockState(void* tn) {
	//update lock status to server
    serverLog(LL_NOTICE, "update lock status to server!");
    return 0;
}

int UnLock(void* tn) {
	//unlock
    serverLog(LL_NOTICE, "unlock!");
    return 0;
}

int BLEParing(void* tn){
    serverLog(LL_NOTICE, "BLEParing!");
    return 0;
}


int Init(void* tn) {
    serverLog(LL_NOTICE, "Init mqtt Clients");
    g_sysif.mqtt_c = initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(NULL == g_sysif.mqtt_c) {
        //goto GoExit;
        serverLog(LL_ERROR, "util_initClients err, mqtt_c is NULL.");
        return 1;
    }
    serverLog(LL_NOTICE, "init mqtt Clients success");
    
    int rc = MQTTClient_subscribe(g_sysif.mqtt_c, SUB_TOPIC, 1);
    if(MQTTCLIENT_SUCCESS != rc){
        serverLog(LL_ERROR, "Subscribe [%s] error with code [%d].", SUB_TOPIC, rc);
        return 1;
    }
    serverLog(LL_NOTICE, "Subscribe [%s] success!!!", SUB_TOPIC);
    

    rc = MQTTClient_subscribe(g_sysif.mqtt_c,PUB_WEBDEMO,1);
    if(rc!=MQTTCLIENT_SUCCESS){
        serverLog(LL_ERROR, "Subscribe [%s] error with code [%d].", PUB_WEBDEMO, rc);
        return 1;
    }
    serverLog(LL_NOTICE, "Subscribe [%s] success!!!", PUB_WEBDEMO);


    //InitBLE(si);
    //InitBtn(si);
    return 0;
}


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

//处理web端消息
int DoWebMsg(char *topic,void *payload){
    printf("=============================================================\n");
    cJSON *root=NULL;
    root=cJSON_Parse((char *)payload);
    if(root==NULL){
        cJSON_Delete(root);
        return 0;
    }
    cJSON *cmd= cJSON_GetObjectItem(root,"cmd");
    cJSON *bridgeId=cJSON_GetObjectItem(root,"bridge_id");
    cJSON *value=cJSON_GetObjectItem(root,"value");
    printf("recv CMD=%s,BRIDGEID=%s Value=%s\n",cmd->valuestring,bridgeId->valuestring,value->valuestring);
    if(strcmp("getUserInfo",cmd->valuestring)==0){
        GetUserInfo(bridgeId->valuestring);
    }else if(strcmp("unlock",cmd->valuestring)==0){
        char* lockID=value->valuestring;
        UnLock(lockID);
    }
    printf("=============================================================\n");
    cJSON_Delete(root);
    int msg_id = rand() % 25532;
    serverLog(LL_NOTICE, "addDiscoverTask msg_id %d", msg_id);
    igm_lock_t lock;
    lockSetName(&lock, "IGM303e31a5c", strlen("IGM303e31a5c"));
    // addPairingTask(&lock, msg_id);
    addAdminTask(&lock, msg_id);
    // addPairingTask(&lock, msg_id);
    return 0;
}

void WaitMQTT(void *arg){
    sysinfo_t *si = (sysinfo_t *)arg;
    while(1){
        sleep(1);
        char *topic = NULL;
        int topicLen;
        MQTTClient_message *msg = NULL;
        int rc = MQTTClient_receive(si->mqtt_c, &topic, &topicLen, &msg, 1e3);
        if (0 != rc) {
            serverLog(LL_ERROR, "MQTTClient_receive msg error");
            continue;
        }
        // serverLog(LL_NOTICE, "MQTTClient_receive msg success");
        if (!msg)
        {
            HeartBeat();
            continue;
        }

        if (strcmp(topic,PUB_WEBDEMO) == 0)
        {
            DoWebMsg(topic,msg->payload);;
        }
        serverLog(LL_NOTICE, "topic %s", topic);
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
        // addPairingTask(lock);
    }
    serverLog(LL_NOTICE, "9. Release the ble data");
    bleReleaseData(&ble_data);
}

// 添加扫描方式样例
// ble_data 里面全市
void addDiscoverTask(int msg_id)
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Discover task");
    serverLog(LL_NOTICE, "1. set ble parameters");
    ble_discover_param_t discover_param;
    serverLog(LL_NOTICE, "1. set scan_timeout to 3");
    discover_param.scan_timeout = 2;
    serverLog(LL_NOTICE, "2. set msg_id to 0(or anything you want)");
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
    InsertBle2DTail(msg_id, BLE_DISCOVER_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getDiscoverFsmTable(), getDiscoverFsmTableLen(), TASK_BLE_DISCOVER
    );
    serverLog(LL_NOTICE, "5. Add Discover task.");
    return;
}

igm_lock_t *checkLockIsDiscovered(igm_lock_t *lock)
{
    int n_try_scan = 3;
    igm_lock_t *lock_nearby;
    while (n_try_scan--)
    {
        lock_nearby = findLockByName(lock->name);
        if (!lock_nearby)
        {
            serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge, bridge scan first");
            contnueDiscoverLock();
        }
    }
    if (!lock_nearby)
    {
        serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge");
        return NULL;
    }
    return lock_nearby;
}

igm_lock_t *checkLockIsPaired(igm_lock_t *lock)
{
    igm_lock_t *paired_lock;
    int n_try_paired = 3;
    int msg_id = 233;
    while (n_try_paired--)
    {
        if (lock && lock->paired)
        {
            return lock;
        }
        addPairingTask(lock, msg_id);
        sleep(1);
    }
    return NULL;
}

void addPairingTask(igm_lock_t *lock, int msg_id)
{

    igm_lock_t *lock_nearby = checkLockIsDiscovered(lock);
    if (!lock_nearby)
    {
        serverLog(LL_ERROR, "can't not find lock nearby");
        return;
    }

    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Pairing task");
    serverLog(LL_NOTICE, "1. set ble pairing parameters");
    ble_pairing_param_t *pairing_param = (ble_pairing_param_t *)calloc(sizeof(ble_pairing_param_t), 1);
    serverLog(LL_NOTICE, "1. set pairing param lock to name %s addr %s", lock_nearby->name, lock_nearby->addr);
    bleSetPairingParam(pairing_param, lock_nearby);
    serverLog(LL_NOTICE, "2. set msg_id to 1(or anything you want)");
    // int msg_id = 1;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, pairing_param, sizeof(ble_pairing_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DTail(msg_id, BLE_PAIRING_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getPairingFsmTable(), getPairingFsmTableLen(), TASK_BLE_PAIRING);
    serverLog(LL_NOTICE, "5. Add Pairing task.");
    return;
}

void addAdminTask(igm_lock_t *lock, int msg_id)
{
    igm_lock_t *lock_nearby  = checkLockIsDiscovered(lock);
    if (!lock_nearby)
    {
        return;
    }
    
    lock_nearby = checkLockIsPaired(lock_nearby);
    if (!lock_nearby)
    {
        serverLog(LL_ERROR, "can't not paired lock nearby");
        return;
    }
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Admin task");
    serverLog(LL_NOTICE, "1. set ble admin parameters");
    ble_admin_param_t *admin_param = (ble_admin_param_t *)calloc(sizeof(ble_admin_param_t), 1);
    serverLog(LL_NOTICE, "1. set pairing param lock to name %s addr %s", lock->name, lock->addr);
    bleSetAdminParam(admin_param, lock);
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_PAIRING_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getAdminFsmTable(), getAdminFsmTableLen(), TASK_BLE_ADMIN_CONNECTION);
    serverLog(LL_NOTICE, "5. Add admin task.");
    return;
}

void addAdminUnpairTask(igm_lock_t *lock)
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Admin Unpair task");
    serverLog(LL_NOTICE, "1. set ble admin unpair parameters");
    ble_admin_param_t *admin_unpair_param = (ble_admin_param_t *)calloc(sizeof(ble_admin_param_t), 1);
    serverLog(LL_NOTICE, "1. set admin unpair param lock to name %s addr %s", lock->name, lock->addr);
    bleSetAdminParam(admin_unpair_param, lock);
    serverLog(LL_NOTICE, "2. set msg_id to 3(or anything you want)");
    int msg_id = 3;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, admin_unpair_param, sizeof(ble_admin_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_ADMIN_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getAdminUnpairFsmTable(), getAdminUnpairFsmTableLen(), TASK_BLE_ADMIN_UNPAIR);
    serverLog(LL_NOTICE, "5. Add admin unpair task.");
    return;
}

void addAdminUnlockTask(igm_lock_t *lock)
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Admin Unlock task");
    serverLog(LL_NOTICE, "1. set ble admin Unlock parameters");
    ble_admin_param_t *admin_param = (ble_admin_param_t *)calloc(sizeof(ble_admin_param_t), 1);
    serverLog(LL_NOTICE, "1. set admin Unlock param lock to name %s addr %s", lock->name, lock->addr);
    bleSetAdminParam(admin_param, lock);
    serverLog(LL_NOTICE, "2. set msg_id to 4(or anything you want)");
    int msg_id = 4;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_ADMIN_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getAdminUnlockFsmTable(), getAdminUnlockFsmTableLen(), TASK_BLE_ADMIN_UNLOCK);
    serverLog(LL_NOTICE, "5. Add admin unlock task.");
    return;
}

void addAdminLockTask(igm_lock_t *lock)
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Admin lock task");
    serverLog(LL_NOTICE, "1. set ble admin lock parameters");
    ble_admin_param_t *admin_param = (ble_admin_param_t *)calloc(sizeof(ble_admin_param_t), 1);
    serverLog(LL_NOTICE, "1. set admin lock param lock to name %s addr %s", lock->name, lock->addr);
    bleSetAdminParam(admin_param, lock);
    serverLog(LL_NOTICE, "2. set msg_id to 5(or anything you want)");
    int msg_id = 5;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_ADMIN_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getAdminLockFsmTable(), getAdminLockFsmTableLen(), TASK_BLE_ADMIN_LOCK);
    serverLog(LL_NOTICE, "5. Add admin unlock task.");
    return;
}

void saveTaskData(task_node_t *ptn)
{
    if (!ptn) return;

    if(ptn->ble_data && ptn->ble_data_len)
    {
        ble_data_t *ble_data = ptn->ble_data;
        int task_type = ptn->task_type;
        switch (task_type)
        {
        case TASK_BLE_DISCOVER:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_DISCOVER data");
            int num_of_result = bleGetNumsOfResult(ble_data);
            void *result = ble_data->ble_result;
            for (int j=0; j < num_of_result; j++)
            {
                igm_lock_t *lock = bleGetNResult(ble_data, j, sizeof(igm_lock_t));
                serverLog(LL_NOTICE, "name %s  addr: %s", lock->name, lock->addr);
                insertLock(lock);
                // test 需要, 
                // if (!lock->paired)
                // {
                //     serverLog(LL_NOTICE, "try to pair name %s  addr: %s", lock->name, lock->addr);
                //     addPairingTask(lock);
                // }              
            }
            break;
        }
        case TASK_BLE_PAIRING:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_PAIRING data");
            ble_pairing_result_t *pairing_result = (ble_pairing_result_t *)ble_data->ble_result;
            igm_lock_t *lock = findLockByAddr(pairing_result->addr);
            // 只有找到这把锁, 并且匹配成功, 成保存称为Paired
            // 添加任务, 纯属测试需要
            if (lock && pairing_result->pairing_successed)
            {
                serverLog(LL_NOTICE, "set name %s addr %s to paired", lock->name, lock->addr);
                setLockPaired(lock);
                setLockAdminKey(lock, pairing_result->admin_key, pairing_result->admin_key_len);
                setLockPassword(lock, pairing_result->password, pairing_result->password_size);
                // addAdminTask(lock, 2);
                // addAdminUnpairTask(lock);
                // addAdminUnlockTask(lock);
                // addAdminLockTask(lock);
            }
            break;
        }
        case TASK_BLE_ADMIN_CONNECTION:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_CONNECTION data");
        }
        case TASK_BLE_ADMIN_UNPAIR:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_UNPAIR data");
            ble_admin_result_t *admin_unpair_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        case TASK_BLE_ADMIN_UNLOCK:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_UNLOCK data");
            ble_admin_result_t *admin_lock_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        case TASK_BLE_ADMIN_LOCK:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_LOCK data");
            ble_admin_result_t *admin_lock_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        default:
            break;
        }
    }
}

int main() {
    Init(NULL);
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

    pthread_t mqtt_thread = Thread_start(WaitMQTT, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", mqtt_thread);
    // pthread_t ble_thread = Thread_start(WaitBLE, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", ble_thread);
    // pthread_t bt_thread = Thread_start(WaitBtn, &g_sysif);
    // serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", bt_thread);
    // addDiscoverTask();
    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsDEmpty()) {
            serverLog(LL_NOTICE,"doing_task_head is empty, check Lock list.");
            // 应该去检查waiting list
            printLockList();
            serverLog(LL_NOTICE,"doing_task_head is empty, ready to sleep.");
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
                }
                saveTaskData(ptn);
                DeleteDTask(&ptn); // 自动置 ptn 为 NULL
                
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

