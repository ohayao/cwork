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
#include "profile.h"

#include "pb_encode.h"
#include "pb_decode.h"
#include "ign.pb.h"
#include "cJSON.h"
#include "../../src/bridge/https_client/https.h"

//临时定义发布和订阅的TOPIC
#define PUB_TOPIC "igh/dev/DCA63210C7DA"
#define SUB_TOPIC "igh/srv/DCA63210C7DA"

//定义订阅和发布web端TOPIC
#define SUB_WEBDEMO "/WEBSOCKET_DEMO_SUB"
#define PUB_WEBDEMO "/WEBSOCKET_DEMO_PUB"

#define RUN_SUCCESS 0
#define RUN_FAIL -1

LIST_HEAD(waiting_task_head);
LIST_HEAD(doing_task_head);
 
sysinfo_t g_sysif;

char* get_file_content(char *path);
void write_file_content(char *path,char *content);

char* get_file_content(char *path){
    char* buf;
    FILE *csr=fopen(path,"r");
    fseek(csr,0,SEEK_END);
    int len=ftell(csr);
    buf=(char*)malloc(len+1);
    rewind(csr);
    fread(buf,1,len,csr);
    buf[len]=0;
    fclose(csr);
    return buf;
}
void write_file_content(char *path,char *content){
    FILE *csr=fopen(path,"w");
    fwrite(content,sizeof(char),strlen(content),csr);
    fclose(csr);
}

int h_GetUserToken(char *userToken);
int h_GetUserToken(char *userToken){
    char data[1024],res[4096];
    memset(data,0,sizeof(data));
    memset(res,0,sizeof(res));
    HTTP_INFO hi;
    http_init(&hi, TRUE);
    char *url = "https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/login";
    sprintf(data,"{\"email\":\"cs.lim+bridge@igloohome.co\",\"password\":\"igloohome\"}");
    int ret = http_post(&hi, url, data, res, sizeof(res));
    http_close(&hi);
    if(ret!=200){ 
        printf("h_GetUserToken error [%d:%s]\n",ret,res);
        return -1;
    }
    strncpy(userToken,res+16,strlen(res)-18);
    return 0;
}

int h_GetBridgeToken(char *userToken,char *bridgeToken);
int h_GetBridgeToken(char *userToken,char *bridgeToken){
    char res[4096];
    memset(res,0,sizeof(res));
    HTTP_INFO hi;
    http_init(&hi,TRUE);
    char *url="https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/token";
    int ret=http_get_with_auth(&hi,url,userToken,res,4096);
    //printf("----->> res=%s\n",res);
    if(ret!=200) {
        printf("h_GetBridgeToken error [%d:%s]\n",ret,res);
        return -1;
    }
    strncpy(bridgeToken,res+18,strlen(res)-20);
    return 0;

}
int h_downloadcsr(char *bridgeToken);
int h_downloadcsr(char *bridgeToken){
    char res[4096],url[200],bridgeID[20];
    char *localCSR=get_file_content("./domain.csr");
    memset(res,0,sizeof(res));
    memset(url,0,sizeof(url));
    memset(bridgeID,0,sizeof(bridgeID));
    Pro_GetMacAddrs(bridgeID);
    HTTP_INFO hi;
    sprintf(url,"https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/devices/bridge/%s",bridgeID);

    printf("h_downloadcsr,request_url=%s\n",url);
    cJSON *root;
    root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"csr",localCSR);
    char *sdata=cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    printf("________________Request BridgeToken\n%s\nBody Content\n%s\n",bridgeToken,sdata);
    int ret=http_post_with_auth(&hi,url,bridgeToken,sdata,res,sizeof(res));
    //printf("----->downloadcsr response[%s]\n",res);
    if(ret!=200){
        printf("h_downloadcsr error [%d:%s]\n",ret,res);
        return -1;
    }
    root=cJSON_Parse(res);
    if(root==NULL) {
        printf("h_downloadcsr content not json type\n");
        cJSON_Delete(root);
        return -1;
    }
    cJSON *pem=cJSON_GetObjectItem(root,"pem");
    printf("pem=%s\n",pem->valuestring);
    write_file_content("./test_test_test_test.csr",pem->valuestring);
    cJSON_Delete(root);
    printf("=====>>>>>DOWNLOAD-CSR Over!!!!!\n");
    return 0;
}
int download_ca();
int download_ca(){
    int ret;
    char userToken[2048],bridgeToken[2048];
    memset(userToken,0,sizeof(userToken));
    memset(bridgeToken,0,sizeof(bridgeToken));
    if((ret=h_GetUserToken(userToken))!=0){
        printf("GetUserTokenError [%d]\n",ret);
        return -1;
    }

    if((ret=h_GetBridgeToken(userToken,bridgeToken))!=0){
        printf("GetBridgeTokenError [%d]\n",ret);
        return -1;
    }
    
    if((ret=h_downloadcsr(bridgeToken))!=0){
        printf("DownloadCSR Error[%d]\n",ret);
        return -1;
    }
    return 0;
}

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

//create birdge profile
ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID);
    ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID){
    ign_BridgeProfile bp={};
    bp.os_info=ign_OSType_LINUX;
    char temp[100];
    memset(temp,0,sizeof(temp));
    strcpy(temp,bridgeID);
    bp.bt_id.size=strlen(temp);
    memcpy(bp.bt_id.bytes,temp,strlen(temp));

    char localip[20],publicip[20],ma[20];                                                                                                                                                                                         
    memset(localip,0,sizeof(localip));
    memset(publicip,0,sizeof(publicip));
    memset(ma,0,sizeof(ma));
    Pro_GetLocalIP(localip);
    Pro_GetPublicIP(publicip);

    bp.local_ip.size = strlen(localip);
    memcpy(bp.local_ip.bytes, localip, strlen(localip));

    bp.public_ip.size=strlen(publicip);
    memcpy(bp.public_ip.bytes,publicip,strlen(publicip));
        
    PRO_DISK_INFO *di=Pro_GetDiskInfo();
    PRO_MEMORY_INFO *mi=Pro_GetMemoryInfo();
    char cpu[200];
    memset(cpu,0,sizeof(cpu));
    sprintf(cpu,"CR:%.4f;MT:%d,MF:%d,MUR:%.4f;DT:%d,DU:%d,DUR:%.4f",
            Pro_GetCpuRate(),
            mi->total,mi->free,mi->used_rate,
            di->total,di->used,di->used_rate);
    bp.sys_statics.size=strlen(cpu);
    memcpy(bp.sys_statics.bytes,cpu,strlen(cpu));

    PRO_WIFI_INFO *wf=Pro_GetWifiInfo();
    bp.wifi_ssid.size = strlen(wf->ssid);
    memcpy(bp.wifi_ssid.bytes,wf->ssid,strlen(wf->ssid));
    bp.wifi_signal = wf->signal;
    bp.inited_time = Pro_GetInitedTime();
    Pro_GetMacAddrs(ma);
    bp.name.size = strlen(temp);
    memcpy(bp.name.bytes, ma, strlen(ma));
    printf("==>>>>> LIP=%s PIP=%s NM=%s \nSYS=%s\n WIFI=%s SIGNAL=%d INITEDTIME=%d \n",
           bp.local_ip.bytes,bp.public_ip.bytes,bp.name.bytes,bp.sys_statics.bytes,bp.wifi_ssid.bytes,bp.wifi_signal,bp.inited_time);
    return bp;
}

int GetUserInfo(void* si) {
	//send request to server to get userinfo
	printf("send request to server to get userinfo!\n");
    
    ign_MsgInfo msg={};
        msg.event_type=ign_EventType_GET_USER_INFO;
        msg.time=get_ustime();
        msg.msg_id=get_ustime();
        ign_BridgeEventData bed={};
        bed.has_profile=true;
        bed.profile=Create_IgnBridgeProfile((char*)si);
        msg.has_bridge_data=true;
        msg.bridge_data=bed;

        int pubResult;
        uint8_t buf[1024];
        memset(buf,0,sizeof(buf));
        pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
        if(pb_encode(&out,ign_MsgInfo_fields,&msg)){
            size_t len=out.bytes_written;
            if((pubResult=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
                printf("SEND MQTT [GET_USER_INFO] ERROR WITH CODE[%d]\n",pubResult);
            }else{
                printf("SEND MQTT [GET_USER_INFO] SUCCESS\n");
            }
        }else{
            printf("ENCODE GETUSERINFO ERROR\n");
        }

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
	printf("unlock %s!\n",(char*)tn);
    return 0;
}

int BLEParing(void* tn){
	printf("BLEParing!\n");
    return 0;
}

int Sync_Battery();
int Sync_Battery(){
    ign_MsgInfo battery={};
    battery.event_type=ign_EventType_DEMO_UPDATE_LOCK_BATTERY;
    battery.time=get_ustime();
    battery.msg_id=GetMsgID();
    ign_BridgeEventData bed={};
    sprintf(bed.demo_lockId,"IGM303a316e7");
    bed.has_profile=true;
    bed.profile=Create_IgnBridgeProfile("DCA63210C7DA");

    bed.has_demo_update_lock_battery=true;
    ign_DemoUpdateLockBattery dulb={};
    dulb.battery=66;
    bed.demo_update_lock_battery=dulb;

    battery.has_bridge_data=true;
    battery.bridge_data=bed;
    

    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,ign_MsgInfo_fields,&battery)){
        size_t len=out.bytes_written;

        if((publish_result=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
            printf("SEND MQTT BATTERY ERROR WITH CODE[%d]\n",publish_result);
        }else{
            printf("SEND MQTT BATTERY SUCCESS\n");
        }
    }else{
        printf("ENCODE MQTT BATTERY ERROR\n");
    }
    return 0;
}
int Sync_Status();
int Sync_Status(){
ign_MsgInfo battery={};
    battery.event_type=ign_EventType_DEMO_UPDATE_LOCK_STATUS;
    battery.time=get_ustime();
    battery.msg_id=GetMsgID();
    ign_BridgeEventData bed={};
    sprintf(bed.demo_lockId,"IGM303a316e7");
    bed.has_profile=true;
    bed.profile=Create_IgnBridgeProfile("DCA63210C7DA");

    bed.has_demo_update_lock_status=true;
    ign_DemoUpdateLockStatus duls={};
    duls.status=1;
    bed.demo_update_lock_status=duls;

    battery.has_bridge_data=true;
    battery.bridge_data=bed;
    

    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,ign_MsgInfo_fields,&battery)){
        size_t len=out.bytes_written;

        if((publish_result=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
            printf("SEND MQTT STATUS ERROR WITH CODE[%d]\n",publish_result);
        }else{
            printf("SEND MQTT STATUS SUCCESS\n");
        }
    }else{
        printf("ENCODE MQTT STATUS ERROR\n");
    }
    return 0;

}
int Sync_Activities();
int Sync_Activities(){
    ign_MsgInfo battery={};
    battery.event_type=ign_EventType_DEMO_UPDATE_LOCK_ACTIVITIES;
    battery.time=get_ustime();
    battery.msg_id=GetMsgID();
    ign_BridgeEventData bed={};
    sprintf(bed.demo_lockId,"IGM303a316e7");
    bed.has_profile=true;
    bed.profile=Create_IgnBridgeProfile("DCA63210C7DA");

    bed.has_demo_update_lock_activities=true;
    ign_DemoUpdateLockActivities dula={};
    char logs[500];
    memset(logs,0,sizeof(logs));
    strcpy(logs,"log log log log");
    dula.log.size=strlen(logs);
    memcpy(dula.log.bytes,logs,strlen(logs));
    bed.demo_update_lock_activities=dula;

    battery.has_bridge_data=true;
    battery.bridge_data=bed;
    

    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,ign_MsgInfo_fields,&battery)){
        size_t len=out.bytes_written;

        if((publish_result=util_sendMessage(g_sysif.mqtt_c,PUB_TOPIC,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
            printf("SEND MQTT ACTIVITIES ERROR WITH CODE[%d]\n",publish_result);
        }else{
            printf("SEND MQTT ACTIVITIES SUCCESS\n");
        }
    }else{
        printf("ENCODE MQTT ACTIVITIES ERROR\n");
    }
    return 0;

}


int Init(void* tn) {
    // get WiFi & user info from Mobile App
    // get AWS pem by http

    //task_node_t *ptn = (task_node_t*) tn; 
    g_sysif.mqtt_c = util_initClients(HOST, SUBSCRIBE_CLIENT_ID, 60, 1, CA_PATH, TRUST_STORE, PRIVATE_KEY, KEY_STORE);
    if(NULL == g_sysif.mqtt_c) {
        //goto GoExit;
        printf("util_initClients err, mqtt_c is NULL.\n");
    }
    
    int rc = MQTTClient_subscribe(g_sysif.mqtt_c, SUB_TOPIC, 1);
    if(MQTTCLIENT_SUCCESS != rc){
        printf("Subscribe [%s] error with code [%d].\n", SUB_TOPIC, rc);
    }else{
        printf("Subscribe [%s] success!!!\n", SUB_TOPIC);
    }
    rc=MQTTClient_subscribe(g_sysif.mqtt_c,PUB_WEBDEMO, 1);
    if(rc!=MQTTCLIENT_SUCCESS){
        printf("Subscribe [%s] error with code[%d]！！！\n", PUB_WEBDEMO, rc);
    }else{
        printf("Subscribe [%s] success!!!\n",PUB_WEBDEMO);
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
    bed.has_profile=true;
    bed.profile=Create_IgnBridgeProfile("DCA63210C7DA");
    hb.has_bridge_data=true;
    hb.bridge_data=bed;


    int publish_result;
    uint8_t buf[1024];
    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,ign_MsgInfo_fields,&hb)){
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

static ign_LockEntry glocks[5];
static int glock_index=0;

bool get_server_event_data(pb_istream_t *stream,const pb_field_t *field,void **arg);
bool get_server_event_data(pb_istream_t *stream,const pb_field_t *field,void **arg){
    ign_LockEntry lock={};
    if(pb_decode(stream,ign_LockEntry_fields,&lock)){
        glocks[glock_index]=lock;
        glock_index++;
        return true;
    }    
    return false;
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
			serverLog(LL_ERROR, "MQTTClient_receive err[%d], topic[%s].", rc, topic);
        }
		printf("MQTTClient_receive msg from server, topic[%s].\n", topic);
        if(msg){
            if(strcmp(topic,PUB_WEBDEMO)==0){
				//web simulator request
                DoWebMsg(topic,msg->payload);
            }else{
                //decode msg
                memset(glocks,0,sizeof(glocks));
                glock_index=0;
                ign_MsgInfo imsg={};
                pb_istream_t in=pb_istream_from_buffer(msg->payload,(size_t)msg->payloadlen);
               // imsg.server_data.lockEntries.funcs.decode=&get_server_event_data;
                if(pb_decode(&in,ign_MsgInfo_fields,&imsg)){
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
                        case ign_EventType_UPDATE_USER_INFO:
                            if(imsg.has_server_data){
                                for(int i=0;i<glock_index;i++){
                                    printf("%02d bt_id=%s\n",i,glocks[i].bt_id);
                                }
                            }
       printf("=======> RECV UPDATE_USER_INFO\n");
       ign_MsgInfo tmsg={};
       tmsg.event_type=ign_EventType_UPDATE_USER_INFO;
       tmsg.has_bridge_data=true;
       ign_BridgeEventData tbed={};
       tbed.has_profile=true;
       tbed.profile=Create_IgnBridgeProfile("DCA63210C7DA");
       tmsg.bridge_data=tbed;
       ign_ServerEventData tsd={};
       tsd.lock_entries_count=5;
       int tl=0;
       for(int i=0;i<5;i++){
           if(strlen(imsg.server_data.lock_entries[i].bt_id)>0){
               tl++;
               strcpy(tsd.lock_entries[i].bt_id,imsg.server_data.lock_entries[i].bt_id);
               //strcpy(tsd.lock_entries[i].ekey.bytes,imsg.server_data.lock_entries[i].ekey.bytes);
               tsd.lock_entries[i].has_ekey=true;
               strcpy(tsd.lock_entries[i].ekey.guest_token.bytes,imsg.server_data.lock_entries[i].ekey.guest_token.bytes);
               strcpy(tsd.lock_entries[i].ekey.guest_aes_key.bytes,imsg.server_data.lock_entries[i].ekey.guest_aes_key.bytes);
               strcpy(tsd.lock_entries[i].ekey.password.bytes,imsg.server_data.lock_entries[i].ekey.password.bytes);
               tsd.lock_entries[i].ekey.keyId=imsg.server_data.lock_entries[i].ekey.keyId;
           }
       }
       tsd.lock_entries_count=tl;
       tmsg.has_server_data=true;
       tmsg.server_data=tsd;

      
       int pubResult;
       uint8_t buf[1024];
       memset(buf,0,sizeof(buf));
       pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
       if(pb_encode(&out,ign_MsgInfo_fields,&tmsg)){
           size_t len=out.bytes_written;
           if((pubResult=util_sendMessage(g_sysif.mqtt_c,SUB_WEBDEMO,1,buf,(int)len))!=MQTTCLIENT_SUCCESS){
               printf("TRANS TO WEB [UPDATE_USER_INFO] ERROR WITH CODE[%d]\n",pubResult);
           }else{
               printf("TRANS TO WEB [UPDATE_USER_INFO] SUCCESS\n");
           }
       }else{
           printf("ENCODE UPDATEUSERINFO ERROR\n");
       }





                            //util_sendMessage(g_sysif.mqtt_c,SUB_WEBDEMO,1,msg->payload,msg->payloadlen);
                            goto gomqttfree;
                        break;
                        case ign_EventType_NEW_JOB_NOTIFY:
                            printf("RECV[NEW_JOB_NOTIFY]\n\tbt_id=%s\tlock_cmd_size=%d\tlock_cmd=%s\n",imsg.server_data.job.bt_id,(int)imsg.server_data.job.lock_cmd.size,imsg.server_data.job.lock_cmd.bytes);
                            for(int i=0;i<imsg.server_data.job.lock_cmd.size;i++){
                                char b=(char)imsg.server_data.job.lock_cmd.bytes[i];
                                printf("%x",b);
                            }
                            printf("\n");
                            if(imsg.has_server_data&&imsg.server_data.has_demo_job){
                                switch(imsg.server_data.demo_job.op_cmd){
                                case ign_DemoLockCommand_GET_BATTERY:
                                    Sync_Battery();
                                    break;
                                case ign_DemoLockCommand_GET_LOCK_STATUS:
                                    Sync_Status();
                                    break;
                                case ign_DemoLockCommand_GET_LOGS:
                                    Sync_Activities();
                                    break;
                                }
                            }
                            //util_sendMessage(g_sysif.mqtt_c,SUB_WEBDEMO,1,msg->payload,msg->payloadlen);
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
            printf("message null\n");
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
    int try_time=0;
    while(1){
        int ret=download_ca();
        try_time++;
        if(ret==0||try_time>2){
            break;
        }
        sleep(1);
    }
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
