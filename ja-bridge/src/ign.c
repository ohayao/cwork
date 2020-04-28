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
#include "ign.h"

#include "pb_encode.h"
#include "pb_decode.h"
#include "ign.pb.h"
#include "cJSON.h"

//临时定义发布和订阅的TOPIC
#define PUB_TOPIC "igh/dev/abcdef"
#define SUB_TOPIC "igh/srv/abcdef"

//定义订阅和发布web端TOPIC
#define SUB_WEBDEMO "/WEBSOCKET_DEMO_SUB"
#define PUB_WEBDEMO "/WEBSOCKET_DEMO_PUB"

LIST_HEAD(waiting_task_head);
LIST_HEAD(doing_task_head);
 
sysinfo_t g_sysif;

int DoWebMsg(char *topic,void *payload);


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
    
    int rc = MQTTClient_subscribe(g_sysif.mqtt_c, SUB_TOPIC, 1);
    if(MQTTCLIENT_SUCCESS != rc){
        printf("Subscribe [%s] error with code [%d].\n",SUB_TOPIC, rc);
    }else{
        printf("Subscribe [%s] success!!!\n",SUB_TOPIC);
    }
    rc=MQTTClient_subscribe(g_sysif.mqtt_c,SUB_WEBDEMO,1);
    if(rc!=MQTTCLIENT_SUCCESS){
        printf("Subscribe [%s] error with code[%d]！！！\n",SUB_WEBDEMO,rc);
    }else{
        printf("Subscribe [%s] success!!!\n",SUB_WEBDEMO);
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

//发送心跳间隔计数
int hbInterval=0;
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
    ign_BridgeProfile bp={};

    char temp[100];
    memset(temp,0,sizeof(temp));
    strcpy(temp,"abcdef");
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
    bed.has_profile=true;
    bed.profile=bp;
    hb.has_bridge_data=true;
    hb.bridge_data=bed;


    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode_ex(&out,ign_MsgInfo_fields,&hb,PB_ENCODE_DELIMITED)){
        size_t len=out.bytes_written;
        if((publish_result=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
            printf("SEND MQTT HB ERROR WITH CODE[%d]\n",publish_result);
        }else{
            printf("SEND MQTT HB SUCCESS\n");
        }
    }else{
        printf("ENCODE MQTT HB ERROR\n");
    }
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
            if(strcmp(topic,SUB_WEBDEMO)==0){
                DoWebMsg(topic,msg->payload);
            }else{
                //decode msg
                //printf("start decode mqtt msg\n");
                //printf("%s\n",(char*)msg->payload);
                //printf("end decode mqtt msg\n");
                ign_MsgInfo imsg={};
                pb_istream_t in=pb_istream_from_buffer(msg->payload,(size_t)msg->payloadlen);
                if(pb_decode_ex(&in,ign_MsgInfo_fields,&imsg,PB_DECODE_DELIMITED)){
                    printf("DECODE SUCCESS-----------------------------------------------\n");
                    switch(imsg.event_type){
                        case ign_EventType_HEARTBEAT:
                            printf("RECV MQTT HB msg\n");
                            goto gomqttfree;
                        break;
                        case ign_EventType_GET_USER_INFO:
                            printf("RECV MQTT GETUSERINFO msg LEN=%d\n",msg->payloadlen);
                            printf("RECV msgid=%d,signal=%d\n",imsg.msg_id,imsg.bridge_data.profile.wifi_signal);
                            printf("RECV profile_bt_id=%s,bridege_name=%s\n",imsg.bridge_data.profile.bt_id.bytes,imsg.bridge_data.profile.name.bytes);
                            goto gomqttfree;
                        break;
                        default:
                            printf("RECV MQTT %u msg\n",imsg.event_type);
                            goto gomqttfree;
                        break;
                    }


                    //search task queue by msg_id
                    unsigned int msg_id = 1;
                    unsigned int current_state = 1;
                    task_node_t *ptn = NULL;
                    ptn = FindTaskByMsgID(msg_id, &waiting_task_head);
            
                    if (NULL!=ptn) {//move task_node into doing_list task queue
                        printf("find task_node.msg_id[%u], current_state[%d].\n", ptn->msg_id, ptn->cur_state);
                        //MoveTask();
                        pthread_mutex_lock(g_sysif.mutex);
                        MoveTask(&ptn->list, &doing_task_head);
                        pthread_mutex_unlock(g_sysif.mutex);
                    }
                    else {//if not exist, add into task queue
                        printf("find ptn==NULL.\n");
                        pthread_mutex_lock(g_sysif.mutex);
                        InsertTask(&doing_task_head, msg_id, current_state, NULL, NULL);
                        pthread_mutex_unlock(g_sysif.mutex);
                    }
                //decode get msg_id 
                //task_node_t* tn = FindTaskByMsgID(msg_id, waiting_list)
                gomqttfree:            
                    MQTTClient_freeMessage(&msg);
                    MQTTClient_free(topic);
                }else{
                    printf("MQTT MSG DECODE ERROR!!!!\n");
                }
            }
                       
        } else {
            //err log
            HeartBeat();
        }
    }
}

//处理web端消息
int DoWebMsg(char *topic,void *payload){
    printf("=============================================================\n");
    cJSON *root=NULL;
    root=cJSON_Parse((char *)payload);
    if(root==NULL){
        cJSON_Delete(root);
        printf("反序列化失败\n");
        return 0;
    }
    cJSON *cmd= cJSON_GetObjectItem(root,"cmd");
    cJSON *lockId=cJSON_GetObjectItem(root,"lockId");
    cJSON *value=cJSON_GetObjectItem(root,"value");
    printf("recv CMD=%s,LOCKID=%s Value=%s\n",cmd->valuestring,lockId->valuestring,value->valuestring);
    if(strcmp("list",cmd->valuestring)==0){
        printf("RECV WEB REQUEST [list]\n");
        ign_MsgInfo msg={};
        msg.event_type=ign_EventType_GET_USER_INFO;
        msg.time=get_ustime();
        msg.msg_id=get_ustime();
        ign_BridgeEventData bed={};
        ign_BridgeProfile bp={};
        bp.os_info=ign_OSType_LINUX;
        char temp[100];
        memset(temp,0,sizeof(temp));
        strcpy(temp,"abcdef");
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
        bed.has_profile=true;
        bed.profile=bp;
        msg.has_bridge_data=true;
        msg.bridge_data=bed;
        printf("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
        printf("%s %u %d\n",bp.bt_id.bytes,msg.bridge_data.profile.os_info,msg.bridge_data.profile.wifi_signal);
        printf("=================bp_id=%s,bp_name=%s \n",msg.bridge_data.profile.bt_id.bytes,msg.bridge_data.profile.name.bytes);
        printf("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");

        int pubResult;
        uint8_t buf[1024];
        memset(buf,0,sizeof(buf));
        pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
        if(pb_encode_ex(&out,ign_MsgInfo_fields,&msg,PB_ENCODE_DELIMITED)){
        //if(pb_encode(&out,ign_MsgInfo_fields,&msg)){
            size_t len=out.bytes_written;
            if((pubResult=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
                printf("SEND MQTT [GET_USER_INFO] ERROR WITH CODE[%d]\n",pubResult);
            }else{
                printf("SEND MQTT [GET_USER_INFO] SUCCESS\n");
            }
            printf("SEND LEN=%ld\n",len);
            util_sendMessage(g_sysif.mqtt_c,SUB_TOPIC,1,buf,(int)len);
        }else{
            printf("ENCODE GETUSERINFO ERROR\n");
        }
        
    }
    printf("=============================================================\n");
    cJSON_Delete(root);
    return 0;
}

int WaitBLE(sysinfo_t *si){
    //Thread_start(wait_BLE, sysinfo)
    for(;;) {
        sleep(1);
//        serverLog(LL_DEBUG, "waiting for BLE...");
    }
    return 0;
}

int WaitBtn(sysinfo_t *si){
    //if btn
    //add Init into doing_list
    for(;;) {
        sleep(1);
//        serverLog(LL_DEBUG, "waiting for Btn...");
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

    //初始化并连接MQTT SERVER
    int _temp=0;
    Init(&_temp);

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

                    pthread_mutex_lock(g_sysif.mutex);
                    MoveTask(&ptn->list, &waiting_task_head);
                    pthread_mutex_unlock(g_sysif.mutex);
                }
            }
        }
    }

    return 0;
}
