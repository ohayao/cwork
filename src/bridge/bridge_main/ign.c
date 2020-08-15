#include <bridge/bridge_main/ign.h>
#include <bridge/bridge_main/ign_constants.h>
#include <bridge/bridge_main/log.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/task.h>
#include <bridge/bridge_main/thread_helper.h>
#include <bridge/bridge_main/mutex_helper.h>
#include <bridge/bridge_main/task_queue.h>
#include <bridge/bridge_main/wait_ble.h>
#include "bridge/bridge_main/profile.h"
#include <bridge/ble/ble_discover.h>
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/lock_list.h>
#include <bridge/ble/ble_guest.h>
//#include <bridge/ble/ble_pairing.h>
#include <bridge/mqtt/mqtt_constant.h>
#include <bridge/mqtt/mqtt_util.h>
#include <bridge/mqtt/cJSON.h>
#include <bridge/proto/ign.pb.h>
#include <bridge/proto/pb_encode.h>
#include <bridge/proto/pb_decode.h>
#include "bridge/lock/messages/CreatePinRequest.h"
#include "bridge/lock/messages/DeletePinRequest.h"
#include "bridge/lock/messages/GetLogsResponse.h"
//#include "bridge/https_client/https.h"
#include <stdio.h>                                                             
#include <linux/input.h>                                                       
#include <stdlib.h>                                                            
#include <sys/types.h>                                                          
#include <sys/stat.h>                                            
#include <fcntl.h>   

static sysinfo_t g_sysif;
static ble_addr_t g_ble_addr;
static char TOPIC_SUB[32];
static char TOPIC_PUB[32];

//LIST_HEAD(waiting_task_head);
//LIST_HEAD(doing_task_head);

extern int WaitBtn(void *arg);
ign_BridgeProfile Create_IgnBridgeProfile(sysinfo_t* si);
void addDiscoverTask(int msg_id);
void addAdminDoLockTask(igm_lock_t *lock);
int Init_MQTT(MQTTClient* p_mqtt);


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

int h_downloadcsr(sysinfo_t* si, char *bridgeToken){
    char res[4096],url[200];
    char *localCSR = get_file_content(LOCAL_CSR);
	if(NULL == localCSR) {
		printf("get_file_content ./certificate/local.csr err.\n");
		return -1;
	}
    memset(res,0,sizeof(res));
    memset(url,0,sizeof(url));
    HTTP_INFO hi;
    sprintf(url,"https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/devices/bridge/%s", si->mac);

    printf("h_downloadcsr,request_url=[%s]\n",url);
    cJSON *root;
    root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"csr",localCSR);
	if(NULL == root) {
		serverLog(LL_ERROR, "root is NULL err.");
		return -1;
	}
    char *sdata=cJSON_PrintUnformatted(root);
	if(NULL == sdata) {
		serverLog(LL_ERROR, "sdata is NULL err.");
		return -1;
	}

    cJSON_Delete(root);
    printf("________________Request BridgeToken\n%s\nBody Content\n%s\n",bridgeToken,sdata);
    int ret=http_post_with_auth(&hi,url,bridgeToken,sdata,res,sizeof(res));
    //printf("----->downloadcsr response[%s]\n",res);
    if(200 != ret) {
        printf("h_downloadcsr error [%d:%s]\n",ret,res);
        return -1;
    }
	printf("res size[%d] [%s]\n", sizeof(res),res);
    root=cJSON_Parse(res);
    if(NULL == root) {
        serverLog(LL_ERROR, "h_downloadcsr content not json type\n");
        cJSON_Delete(root);
        return -1;
    }
	printf("root[%s]\n", root);
    cJSON *pem = cJSON_GetObjectItem(root,"pem");
	if (NULL == pem) {
		serverLog(LL_ERROR, "cJSON_GetObjectItem return pem is NULL err.");
		return -1;
	}
    printf("pem=[%s]\n",pem->valuestring);
    write_file_content(SERVICE_PEM, pem->valuestring);
	cJSON_Delete(root);
	printf("=====>>>>>DOWNLOAD-CSR Over!!!!!\n");
	return 0;
}

int download_ca(sysinfo_t* si, int tryTimes){
	if(-1 != (access(SERVICE_PEM, F_OK))){   
        printf("file mytest.c exist.\n");   
		return 0;
    }   

    int ret;
    int _tt=0;
    char userToken[2048],bridgeToken[2048];
    memset(userToken,0,sizeof(userToken));
    memset(bridgeToken,0,sizeof(bridgeToken));
    while((ret=h_GetUserToken(userToken))!=0){
        printf("GetUserTokenError [%d]\n",ret);
        if(tryTimes>0) _tt++;
        else if(_tt>0 && _tt>=tryTimes) return -1;
    }
    _tt=0;
    while((ret=h_GetBridgeToken(userToken,bridgeToken))!=0){
        printf("GetBridgetTokenError [%d]\n",ret);
        if(tryTimes>0) _tt++;
        if(_tt>0 && _tt>=tryTimes) return -1;
    }
    _tt=0;
    while((ret=h_downloadcsr(si,bridgeToken))!=0){
        printf("DownloadCSR Error [%d]\n",ret);
        if(tryTimes>0) _tt++;
        if(_tt>0 && _tt>=tryTimes) return -1;
    }
    return 0;
}


int SendMQTTMsg(ign_MsgInfo* msg, char* topic) {
	if (NULL == g_sysif.mqtt_c) {
		serverLog(LL_ERROR, "mqtt is NULL, no available connection.");
		printf("mqtt is NULL, no available connection.\n");
		return -1;
	}
    int ret = 0;
    uint8_t buf[1024];
    memset(buf, 0, sizeof(buf));
    pb_ostream_t out = pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out, ign_MsgInfo_fields, msg)){
        size_t len=out.bytes_written;
        if(MQTTCLIENT_SUCCESS != (ret = MQTT_sendMessage(g_sysif.mqtt_c, topic, 1, buf, (int)len))){
            serverLog(LL_ERROR, "MQTT_sendMessage err[%d], do reconnection.", ret);
            do {
                ret = Init_MQTT(&g_sysif.mqtt_c);
            } while (NULL == g_sysif.mqtt_c); //0 != ret);

			ret = MQTTClient_subscribe(g_sysif.mqtt_c, TOPIC_SUB, 1);
			if(MQTTCLIENT_SUCCESS != ret){
				serverLog(LL_ERROR, "re-Subscribe [%s] error with code [%d].", TOPIC_SUB, ret);
			}
			serverLog(LL_NOTICE, "Re-Subscribe [%s] success!!!", TOPIC_SUB);
			//demo use
			ret = MQTTClient_subscribe(g_sysif.mqtt_c, PUB_WEBDEMO, 1);
			if(MQTTCLIENT_SUCCESS != ret){
				serverLog(LL_ERROR, "Re-Subscribe [%s] error with code [%d].", PUB_WEBDEMO, ret);
			}
		} else {
			printf("<<<< Send MQTT to[%s] len[%d]\n", topic, len);
		}

    }else{
        serverLog(LL_ERROR, "pb_encode failed.");
        printf("ENCODE UPDATEUSERINFO ERROR\n");
    }
    return 0;
}

static int hbInterval=0;
int HeartBeat(){
    hbInterval++;
    hbInterval = hbInterval%10;
    if(hbInterval>0)
		 return 0;
    ign_MsgInfo hb={};
    hb.event_type=ign_EventType_HEARTBEAT;
    hb.time=get_ustime();
    hb.msg_id=GetMsgID();
    hb.has_bridge_data=true;
    hb.bridge_data.has_profile=true;
    hb.bridge_data.profile=Create_IgnBridgeProfile(&g_sysif);

    SendMQTTMsg(&hb, TOPIC_PUB);
    return 0;
}

int Sync_Battery(char* lock_id, int battery_l){
    ign_MsgInfo battery={};
    battery.event_type=ign_EventType_DEMO_UPDATE_LOCK_BATTERY;
    battery.time=get_ustime();
    battery.msg_id=GetMsgID();

    battery.has_bridge_data=true;
    ign_BridgeEventData *pbed= &battery.bridge_data;
    strncpy(pbed->demo_lockId, lock_id, sizeof(pbed->demo_lockId));
    pbed->has_profile=true;
	pbed->profile = Create_IgnBridgeProfile(&g_sysif);

    pbed->has_demo_update_lock_battery = true;
    pbed->demo_update_lock_battery.battery = battery_l;
   
    SendMQTTMsg(&battery, TOPIC_PUB);
    return 0;
}

int Sync_Status(char* lock_id, int lock_status){
	ign_MsgInfo status={};
    status.event_type=ign_EventType_DEMO_UPDATE_LOCK_STATUS;
    status.time=get_ustime();
    status.msg_id=GetMsgID();
    
    status.has_bridge_data=true;
    ign_BridgeEventData *pbed = &status.bridge_data;
    strncpy(pbed->demo_lockId, lock_id, sizeof(pbed->demo_lockId));
    pbed->has_profile=true;
	pbed->profile=Create_IgnBridgeProfile(&g_sysif);
    pbed->has_demo_update_lock_status=true;
    pbed->demo_update_lock_status.status = lock_status;

    SendMQTTMsg(&status, TOPIC_PUB);
    return 0;
}

int Sync_Activities(char* lock_id, char* logs, unsigned int logs_size){
    ign_MsgInfo log_msg = {};
    log_msg.event_type=ign_EventType_DEMO_UPDATE_LOCK_ACTIVITIES;
    log_msg.time=get_ustime();
    log_msg.msg_id=GetMsgID();

    log_msg.has_bridge_data=true;
    ign_BridgeEventData *pbed = &log_msg.bridge_data;
    strncpy(pbed->demo_lockId, lock_id, sizeof(pbed->demo_lockId));
    pbed->has_profile=true;
    pbed->profile=Create_IgnBridgeProfile(&g_sysif);

    pbed->has_demo_update_lock_activities=true;
    ign_DemoUpdateLockActivities dula={};
    dula.log.size = logs_size;//strlen(logs);
    memcpy(dula.log.bytes, logs, logs_size);
    pbed->demo_update_lock_activities = dula;

    SendMQTTMsg(&log_msg, TOPIC_PUB);
    return 0;
}


void saveTaskData(task_node_t* ptn) {
	if (!ptn) return;

	printf( "in saveTaskData, ptn->task_type[%d], ble_data_len[%d]\n", ptn->task_type, ptn->ble_data_len);
	if(ptn->ble_data) {
		ble_data_t *ble_data = ptn->ble_data;
		switch (ptn->task_type) {
			case TASK_BLE_GUEST_GETLOCKSTATUS:
				{
					printf( "handle ble get lock status data.\n");
					ble_guest_result_t *guest_result = (ble_guest_result_t *)ble_data->ble_result;
					if(NULL == guest_result) {
						serverLog(LL_ERROR, "guest_result is NULL err.");
						return;
					}
					if(guest_result->result) {
						printf("get status error[%d].\n", guest_result->result);
					} else {
						printf("@@@get status success. lock_status[%d]\n", ble_data->lock_status);
						Sync_Status(ptn->lock_id, ble_data->lock_status);
					}
					break;
				}
			case TASK_BLE_GUEST_GET_BATTERY_LEVEL:
				{
					printf( "handle battery data.\n");
					ble_guest_result_t* guest_result = (ble_guest_result_t *)ble_data->ble_result;
					if(NULL == guest_result) {
						serverLog(LL_ERROR, "guest_result is NULL err.");
						return;
					}
					int ret = guest_result->result;
					if (ret) {
						printf("get battery error[%d].\n", ret);
					} else {
						printf("get battery success, battery_level[%d], lock_id[%s]\n", ble_data->battery_level, ptn->lock_id);
						Sync_Battery(ptn->lock_id, ble_data->battery_level);
					}

					break;
				}
			case TASK_BLE_GUEST_GETLOGS:
				{
					printf( "get ble response data of TASK_BLE_GUEST_GETLOGS\n");
					ble_guest_result_t *guest_get_logs_result = (ble_guest_result_t *)ble_data->ble_result;
					if(NULL == guest_get_logs_result) {
						serverLog(LL_ERROR, "guest_result is NULL err.");
						return;
					}
					int ret = guest_get_logs_result->result;
					if (ret) {
						printf( "get lock logs error\n");
					} else {
						IgGetLogsResponse *get_logs_response = guest_get_logs_result->cmd_response;
						printf( "get lock logs success size [%lu], data[%s]\n", get_logs_response->data_size, get_logs_response->data);
						// send get_logs_response.data;
						Sync_Activities(ptn->lock_id, get_logs_response->data, get_logs_response->data_size);
					}   
					break;
				}
			case TASK_BLE_GUEST_UNLOCK:
				{
					serverLog(LL_NOTICE, "saving ble TASK_BLE_GUEST_UNLOCK data");
					ble_guest_result_t *guest_lock_result = (ble_guest_result_t *)ble_data->ble_result;
					break;
				}
			case TASK_BLE_GUEST_LOCK:
				{
					serverLog(LL_NOTICE, "saving ble TASK_BLE_GUEST_LOCK data");
					ble_guest_result_t *guest_lock_result = (ble_guest_result_t *)ble_data->ble_result;
					break;
				}
			default:
				break;
		}
	}
}

int HandleLockCMD (sysinfo_t* si, igm_lock_t* lock, int cmd, void* request) {
    serverLog(LL_NOTICE, "in HandleLockCMD.");
	LIGHT_BLINK(b)
    ble_guest_param_t *guest_param = (ble_guest_param_t *)malloc(sizeof(ble_guest_param_t));
    bleInitGuestParam(guest_param);
    bleSetGuestParam(guest_param, lock);

	if (ign_DemoLockCommand_CREATE_PIN == cmd && NULL != request) {
		bleSetGuestRequest(guest_param, request, sizeof(IgCreatePinRequest));
	} else if (ign_DemoLockCommand_DELETE_PIN == cmd && NULL != request) {
		bleSetGuestRequest(guest_param, request, sizeof(IgDeletePinRequest));
	}

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, guest_param, sizeof(ble_guest_param_t));

	task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data_len = sizeof(task_node_t);
	tn->sysif = si;
	tn->ble_data = ble_data;
	memset(tn->lock_id, 0x0, sizeof(tn->lock_id));
	memcpy(tn->lock_id, lock->name, lock->name_len);

	if (ign_DemoLockCommand_LOCK == cmd) {
		//tn->sm_table_len = getGuestLockFsmTableLen();
		//tn->task_sm_table = getGuestLockFsmTable();
		tn->task_type = TASK_BLE_GUEST_LOCK;
	} else if (ign_DemoLockCommand_UNLOCK == cmd) {
		//tn->sm_table_len = getGuestUnlockFsmTableLen();
		//tn->task_sm_table = getGuestUnlockFsmTable();
		tn->task_type = TASK_BLE_GUEST_UNLOCK;
	} else if (ign_DemoLockCommand_GET_LOCK_STATUS == cmd) {
		//tn->sm_table_len = getGuestGetLockStatusFsmTableLen();
		//tn->task_sm_table = getGuestGetLockStatusFsmTable();
	    tn->task_type = TASK_BLE_GUEST_GETLOCKSTATUS;
	} else if (ign_DemoLockCommand_GET_BATTERY == cmd) {
		//tn->sm_table_len = getGuestGetBatteryLevelFsmTableLen();
		//tn->task_sm_table = getGuestGetBatteryLevelFsmTable();
		tn->task_type = TASK_BLE_GUEST_GET_BATTERY_LEVEL;
	} else if (ign_DemoLockCommand_GET_LOGS == cmd) {
		//tn->sm_table_len = getGuestGetLogsFsmTableLen();
		//tn->task_sm_table = getGuestGetLogsFsmTable();
	    tn->task_type = TASK_BLE_GUEST_GETLOGS;
	} else if (ign_DemoLockCommand_CREATE_PIN == cmd) {
		//tn->sm_table_len = getGuestCreatePinRequestFsmTableLen();
		//tn->task_sm_table = getGuestCreatePinRequestFsmTable();
	    tn->task_type = TASK_BLE_GUEST_CREATE_PIN;
	} else if (ign_DemoLockCommand_DELETE_PIN == cmd) {
		//tn->sm_table_len = getGuestDeletePinRequestFsmTableLen();
		//tn->task_sm_table = getGuestDeletePinRequestFsmTable();
        tn->task_type = TASK_BLE_GUEST_DEL_PIN;
    } else {
        serverLog(LL_ERROR, "cmd[%d] err.", cmd);
        return -1;
    }

/*
	int current_state = BLE_GUEST_BEGIN;
    for (int j = 0; j < tn->sm_table_len; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                serverLog(LL_ERROR, "ign sm err, step[%d] event_result[%d].", j, event_result);
                return -1;
            } else {
				current_state = tn->task_sm_table[j].next_state;
            }
		}
    }*/

    int ret = guest_connection_and_do_cmd(tn);
    if(ret) {
        serverLog(LL_ERROR, "guest_connection_and_do_cmd err[%d].", ret);
        return -1;
    }

    saveTaskData(tn);

    ble_guest_result_t *guest_unlock_result = (ble_guest_result_t *)ble_data->ble_result;
    releaseGuestResult(&guest_unlock_result);
    bleReleaseBleResult(ble_data);
    free(ble_data); ble_data = NULL;
    free(tn); tn = NULL;
    free(guest_param); guest_param = NULL;
    serverLog(LL_NOTICE, "HandleLockCMD end-------");
	LIGHT_ON(g)
    return 0;
}


/*
int download_ca(){
    char *url;
    char data[1024], response[4096];
    int  i, ret, size;

    printf("=====>>>>>Step1. Get User login token\n");
    HTTP_INFO hi1;
    http_init(&hi1, TRUE);
    url = "https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/login";
    sprintf(data,"{\"email\":\"cs.lim+bridge@igloohome.co\",\"password\":\"igloohome\"}");
    ret = http_post(&hi1, url, data, response, sizeof(response));
    http_close(&hi1);
    if(ret!=200) return -1;
    char userToken[2048];
    memset(userToken,0,sizeof(userToken));
    strncpy(userToken,response+16,strlen(response)-18);
    printf("UserToken=[%s]\n",userToken);
    printf("=====>>>>>Step2. Get Bridge token\n");
    memset(data,0,sizeof(data));
    memset(response,0,sizeof(response));
    HTTP_INFO hi2;
    url="https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/token";
    ret=http_get_with_auth(&hi2,url,userToken,response,sizeof(response));
    http_close(&hi2);
    if(ret!=200) return -2;
    char biridgeToken[2048];
    memset(biridgeToken,0,sizeof(biridgeToken));
    strncpy(biridgeToken,response+18,strlen(response)-20);
    printf("BirdgeTOken=[%s]\n",biridgeToken);
    printf("=====>>>>>Step3. Download CA \n");
    char* localCSR=get_file_content("/root/project/gomvc_blog/ign/webign.csr");
    memset(data,0,sizeof(data));
    memset(response,0,sizeof(response));
    HTTP_INFO hi3;
    url="https://tkm70zar9f.execute-api.ap-southeast-1.amazonaws.com/development/devices/bridge/DCA63210C7DA";
    cJSON *root;
    root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"csr",localCSR);
    char *sdata=cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    printf("________________Request Body Content\n%s\n",sdata);
    ret=http_post_with_auth(&hi3,url,biridgeToken,sdata,response,sizeof(response));
    if(ret!=200) return -3;
    root=cJSON_Parse(response);
    if(root==NULL) return -4;
    cJSON *pem=cJSON_GetObjectItem(root,"pem");
    printf("pem=%s\n",pem->valuestring);
    write_file_content("./test_test_test_test.csr",pem->valuestring);
    cJSON_Delete(root);
    printf("=====>>>>>DOWNLOAD-CSR Over!!!!!\n");
    return 0;
}
*/
int FSMHandle(task_node_t* tn) {
    if(NULL == tn->task_sm_table) {
        serverLog(LL_ERROR, "sm_table is NULL.");
        return -1;
    }
    // sizeof å¨å®ä¹ææ°ç»çæ¶å, sizeof æé è¿åæ´ä¸ªæ°ç»çå­èæ°
    // sizeof å¯¹æéçæ¶å, åªè½è¿åæéå¤§å°
    // sizeof å¯¹æéæç¥æ°ç»çæä¸ªåå®¹çæ¶å, åªè½è¿åè¯¥é¡¹å¤§å°, 
    // ä¹å°±æ¯åªä¼è¿å sm_table å½ä¸­æä¸é¡¹çå¤§å°
    // æä»¥ææå¨è¿åäºé¿åº¦.
	unsigned int table_max_num = tn->sm_table_len;
    serverLog(LL_NOTICE, "table_max_num %d", table_max_num);
	int flag = 0;
    // è¿å¿æ¯éåææçç¶æ.
	for (int i = 0; i<table_max_num; i++) {
        serverLog(LL_NOTICE, "FSMHandle state[%d].", i);
		if (tn->cur_state == tn->task_sm_table[i].cur_state) {
			serverLog(LL_NOTICE, "eventActFun begin---------------, tn->task_sm_table[%d].cur_state[%d].", i, tn->task_sm_table[i].cur_state);
            // å¢å ä¸ä¸ªå¤æ­å½åå½æ°, æ¯å¦å½åå½æ°åºé. 0 è¡¨ç¤ºæ²¡é®é¢
			int event_result = tn->task_sm_table[i].eventActFun(tn);
            if (event_result)
            {
				serverLog(LL_ERROR, "tn->task_sm_table[%d].cur_state[%d] err[%d].", i, tn->task_sm_table[i].cur_state, event_result);
				//flag = 0;
				//break;
				return -1;
			}
			else
			{
				tn->cur_state = tn->task_sm_table[i].next_state;
				serverLog(LL_NOTICE, "update cur_state[%d].", tn->task_sm_table[i].next_state );
				//flag = 1;
			}
			serverLog(LL_NOTICE, "eventActFun end-----------------");
		}
	}
	serverLog(LL_NOTICE, "FSMHandle out for end-----------------");
	return 0;
}

ign_BridgeProfile Create_IgnBridgeProfile(sysinfo_t* si){
    ign_BridgeProfile bp = {};
    bp.os_info = ign_OSType_LINUX;
    char temp[100];
    memset(temp,0,sizeof(temp));
    bp.bt_id.size = snprintf(temp, sizeof(temp), "%s", si->mac);
    bp.mac_addr.size = bp.bt_id.size;
    memcpy(bp.bt_id.bytes, temp, strlen(temp));
    memcpy(bp.mac_addr.bytes, temp, strlen(temp));
    
    char localip[20],publicip[20];                                                                                                                                                                                         
    memset(localip,0,sizeof(localip));
    memset(publicip,0,sizeof(publicip));
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
    bp.name.size = strlen(temp);
    memcpy(bp.name.bytes, si->mac, strlen(si->mac));
    printf("[DEVICEINFO]LocalIP=%s PublicIP=%s Name=%s Sys_statics=%s Wifi_ssid=%s Wifi_signal=%d inited_time=%d\n",
            bp.local_ip.bytes,bp.public_ip.bytes,bp.name.bytes,bp.sys_statics.bytes,bp.wifi_ssid.bytes,bp.wifi_signal,bp.inited_time);
    return bp;
}

static ign_LockEntry glocks[5];
static int glock_index=0;

bool get_server_event_data(pb_istream_t *stream,const pb_field_t *field,void **arg){
    ign_LockEntry lock={};
    if(pb_decode(stream,ign_LockEntry_fields,&lock)){
        glocks[glock_index]=lock;
        glock_index++;
        return true;
    }    
    return false;
}

int GetUserInfo() {
	//send request to server to get userinfo
	printf("send request to server to get userinfo!\n");

	ign_MsgInfo msg = {};
	msg.event_type = ign_EventType_GET_USER_INFO;
	msg.time = get_ustime();
	msg.msg_id = GetMsgID();//get_ustime();

	msg.has_bridge_data = true;
	msg.bridge_data.has_profile = true;
	msg.bridge_data.profile = Create_IgnBridgeProfile(&g_sysif);
    SendMQTTMsg(&msg, TOPIC_PUB);
	return 0;
}

int Init_MQTT(MQTTClient* p_mqtt){
    printf("====>CAPATH=%s;TRUST_STORE=%s;PRIVATE_KEY=%s;KEY_STORE=%s\n",CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    *p_mqtt = MQTT_initClients(HOST, g_sysif.mac, 60, 1, CA_PATH, TRUST_STORE, PRIVATE_KEY, KEY_STORE);
    if(NULL == *p_mqtt) {
        serverLog(LL_ERROR, "MQTT_initClients err, mqtt_c is NULL.");
        return -1;
    }
	serverLog(LL_DEBUG, "MQTT_initClients succ, mqtt_c[%x], addr[%x]\n", p_mqtt, &p_mqtt);
	return 0;
}

int Init_Ble(sysinfo_t* si) {
	si->lockinfo = (LockInfo_t*) malloc(sizeof(LockInfo_t)*MAX_LOCK_COUNT);
	InitLockinfo(si);
	//memset(si->lockinfo, 0, sizeof(LockInfo_t)*MAX_LOCK_COUNT);
	//si->lock_total = 0;

    int ret = gattlib_adapter_open(NULL, &si->ble_adapter);
    if (ret) {
        serverLog(LL_ERROR, "discoverLock ERROR: Failed to open adapter.\n");
		Close(si);
        return -1;
    }

/*
	LockInfo_t *li = (LockInfo_t*) malloc(sizeof(LockInfo_t));
	if (NULL==li) {
		return -1;
	}
*/
/*
	char lock_id[] = "IGM3037f4b09";
	char device_address[] = "EC:09:02:7F:4B:09";
	char admin_key[] = "96eb72d2852d41df94dac37eb3241caa";
	char passwd[] = "63c5bd7dd34fe863";

	LockInfo_t li;
	memset(&li, 0, sizeof(LockInfo_t));
	memcpy(&li.lock_id, lock_id, strlen(lock_id));
	memcpy(&li.lock_addr, device_address, strlen(device_address));
	memcpy(&li.lock_ekey, admin_key, strlen(admin_key));
	memcpy(&li.lock_passwd, passwd, strlen(passwd));

	AddLockinfo(si, &li);
	PrintLockinfo(si);

*/
	//@@@ need async connect!!!
	/*
	int ret = create_gatt_connection(li->lock_addr, &(li->gatt_connection), &(li->gatt_adapter));
	if(ret) {
		serverLog("create_gatt_connection err[%d].", ret);
		return -2;
	}
	*/

	/*??
	static char admin_str[] = "5c3a659f-897e-45e1-b016-007107c96df6";
	if ( gattlib_string_to_uuid( admin_str, strlen(admin_str), &(admin_connection->admin_uuid))<0) {
		serverLog(LL_ERROR, "gattlib_string_to_uuid to admin_uuid fail");
		//goto ADMIN_ERROR_EXIT;
	}
	serverLog(LL_NOTICE, "gattlib_string_to_uuid to admin_uuid success." );

    ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL);
    if (ret) {
        serverLog(LL_NOTICE, "ERROR: Failed to scan.");
        //goto EXIT;
    }
	*/

	return 0;
}

char* GetAddrBle(sysinfo_t* si, char* lock_id) {
	for(int i=0; i<si->ble_list_n; i++){
		if(!strcmp(lock_id, si->ble_list[i].name)) {
			return si->ble_list[i].addr;
		}
	}
	return NULL;
}

void InitLockinfo(sysinfo_t* si) {
	si->lock_total = 0;
	memset(si->lockinfo, 0x0, sizeof(LockInfo_t)*MAX_LOCK_COUNT);
}

int AddLockinfo(sysinfo_t* si, LockInfo_t* li) {
	if(si->lock_total < MAX_LOCK_COUNT-1) {
		LockInfo_t* p = si->lockinfo + (si->lock_total);
		//get addr
		char* lock_addr = GetAddrBle(si, li->lock_id);
		if(NULL != lock_addr) {
			memcpy(p, li, sizeof(LockInfo_t)); 
			p->lock_addr_size = strlen(lock_addr);
			if(32 > p->lock_addr_size){
				memcpy(p->lock_addr, lock_addr, p->lock_addr_size);
			}
			si->lock_total++;
		}else{
			printf("GetAddrBle get NULL.\n");
			p->lock_addr_size = 0;
			//jusg for test
			{
				if(!strcmp(li->lock_id, "IGM3037f4b09")) {
					memcpy(p, li, sizeof(LockInfo_t)); 
					memcpy(p->lock_addr, "EC:09:02:7F:4B:09", sizeof("EC:09:02:7F:4B:09")); 
					si->lock_total++;
				} else if(!strcmp(li->lock_id, "IGP105cc2684")) {
					memcpy(p, li, sizeof(LockInfo_t)); 
					memcpy(p->lock_addr, "ED:67:F0:CC:26:84", sizeof("ED:67:F0:CC:26:84")); 
					si->lock_total++;
				}
			}
		}
		return 0;
	}
	return -1;
}

void PrintLockinfo(sysinfo_t* si) {
	for(int i =0; i<si->lock_total; i++){
		LockInfo_t* pi = si->lockinfo + i;
		printf("[%d], id[%s],id_size[%d],key[%X],key_size[%d],passwd[%x],passwd_size[%d],token[%x],token_size[%d],addr[%s]\n",
			i, pi->lock_id, pi->lock_id_size, pi->lock_ekey, pi->lock_ekey_size, pi->lock_passwd, pi->lock_passwd_size, pi->lock_token, pi->lock_token_size, pi->lock_addr);
	}
}

LockInfo_t* SearchLockInfo(sysinfo_t* si, char* lock_id){
	for(int i =0; i<si->lock_total; i++){
		LockInfo_t* pi = si->lockinfo + i;
		if(!strcmp(pi->lock_id, lock_id)) {
			return pi;
		}else{
			//printf("\tpi->lock_id[%s] != lock_id[%s]\n", pi->lock_id, lock_id);
		}
	}
	return NULL;
}

void ble_discovered_cb(void *adapter, const char* addr, const char* name, void* si) {
	ble_addr_t* bl = ((sysinfo_t*)si)->ble_list;
	unsigned char ble_total = ((sysinfo_t*)si)->ble_list_n; 
    if (name) {
        printf("Discovered [%s] - [%s]\n", addr, name);
		((sysinfo_t*)si)->ble_list = realloc(bl, sizeof(ble_addr_t)*(ble_total+1));
		ble_addr_t* nbl = ((sysinfo_t*)si)->ble_list + ble_total;
		memset(nbl, 0x0, sizeof(ble_addr_t));
		memcpy(nbl->name, name, strlen(name));
		memcpy(nbl->addr, addr, strlen(addr));
		((sysinfo_t*)si)->ble_list_n++;
    } else {
        printf("Discovered [%s]\n", addr);
    }
}

int ScanBLE(sysinfo_t* si) {
	if (NULL == si || NULL == si->ble_adapter) {
		serverLog(LL_ERROR, "in ScanBLE, si is NULL err. ");
		return -1;
	}
	si->ble_list_n = 0;
    //do scan BLE
    int ret = gattlib_adapter_scan_enable(si->ble_adapter, ble_discovered_cb, BLE_SCAN_TIMEOUT, si);
    if (ret) {
        fprintf(stderr, "ERROR: Failed to scan.\n");
		Close(si);
		return -1;
    }
	printf("ble_addr_n[%d]\n", si->ble_list_n);
	for(int i =0; i<si->ble_list_n; i++) {
		printf("[%d],name[%s],addr[%s]\n", i, si->ble_list[i].name, si->ble_list[i].addr);
	}
/*
	printf("g_ble_addr.size[%d]\n",g_ble_addr.size());
	for(map<string, string>::iterator it = g_ble_addr.begin(); g_ble_addr.end()!= it; it++) {
		prtinf("name[%s],addr[%s]\n", it->first.c_str(), it->second.c_str());
	}
*/
    gattlib_adapter_scan_disable(si->ble_adapter);
    //if have the BLE addr, do init for demo
    return 0;
}

void Close(sysinfo_t* si) {
	if(si->ble_adapter) {
		gattlib_adapter_close(si->ble_adapter);
	}
	if(si->ble_list) {
		free(si->ble_list);
	}
}

int WifiConnection(){
    // benny
    return 0;
}

int Init(void* tn) {
	LIGHT_ON(w)

	int ret = 0;

	ret = GetMacAddr(g_sysif.mac, sizeof(g_sysif.mac));
    if(ret < 0) {
        serverLog(LL_ERROR, "Init GetMacAddr err[%d].", ret);
        return -1;
    }

    ret = Init_Ble(&g_sysif);
	if(ret) {
        serverLog(LL_ERROR, "Init_Ble err[%d].", ret);
		return -2;
	}

    ret = ScanBLE(&g_sysif);
	if(ret) {
        serverLog(LL_ERROR, "ScanBle err[%d].", ret);
		return -3;
	}

	memset(TOPIC_PUB, 0, sizeof(TOPIC_PUB));
	memset(TOPIC_SUB, 0, sizeof(TOPIC_SUB));
	snprintf(TOPIC_PUB, sizeof(TOPIC_PUB), "%s%s", PUB_TOPIC_PREFIX, g_sysif.mac);
	snprintf(TOPIC_SUB, sizeof(TOPIC_SUB), "%s%s", SUB_TOPIC_PREFIX, g_sysif.mac);
    printf("Init Mac as Device ID[%s], TOPIC_PUB[%s], TOPIC_SUB[%s].\n", g_sysif.mac, TOPIC_PUB, TOPIC_SUB);
    serverLog(LL_NOTICE, "Init Mac as Device ID[%s], TOPIC_PUB[%s], TOPIC_SUB[%s].", g_sysif.mac, TOPIC_PUB, TOPIC_SUB);

    do{
        ret = WifiConnection();
        sleep(0.5);
        //benny, if connection fail, set the light : can not connect wifi
    } while(0 != ret);
    serverLog(LL_NOTICE, "init Wifi connection success");

	//system("openssl req -new -nodes -newkey rsa:2048 -keyout local.key -out local.csr");
	//if no pem, do it;  if there is pem, pass this
    //ret = download_ca();

    //尝试下载证书的次数 默认0 直到下载成功停止
    int try_time=0;
    /*
    if(0 == download_ca(&g_sysif, try_time)){
        printf("_______DOWNLOAD CSR SUCCESS\n");
    }else{
        printf("_______DOWNLOAD CSR FAILED\n");
    }*/

	LIGHT_BLINK(b)
	do {
		ret = Init_MQTT(&g_sysif.mqtt_c);
        sleep(0.5);
        //benny, if connection fail, set the light : can not connect internet
	} while (0 != ret);
    serverLog(LL_NOTICE, "init mqtt Clients success");
	LIGHT_ON(g)
	
    ret = MQTTClient_subscribe(g_sysif.mqtt_c, TOPIC_SUB, 1);
    if(MQTTCLIENT_SUCCESS != ret){
        serverLog(LL_ERROR, "Subscribe [%s] error with code [%d].", TOPIC_SUB, ret);
        return -2;
    }
    serverLog(LL_NOTICE, "Subscribe [%s] success!!!", TOPIC_SUB);

    //demo use
    ret = MQTTClient_subscribe(g_sysif.mqtt_c, PUB_WEBDEMO, 1);
	if(MQTTCLIENT_SUCCESS != ret){
        serverLog(LL_ERROR, "Subscribe [%s] error with code [%d].", PUB_WEBDEMO, ret);
        return -3;
    }
    serverLog(LL_NOTICE, "Subscribe [%s] success!!!", PUB_WEBDEMO);

	//
    GetUserInfo();

	printf("Init Finish!\n");
    return 0;
}

//å¤çwebç«¯æ¶æ¯
int DoWebMsg(char *topic,void *payload){
    printf("^^^^^^^^^^^^^^^^web msg^^^^^^^^^^^^^^^\n");
    cJSON *root=NULL;
    root=cJSON_Parse((char *)payload);
    if(NULL == root){
        cJSON_Delete(root);
        return 0;
    }
    cJSON *cmd= cJSON_GetObjectItem(root,"cmd");
    cJSON *bridgeId=cJSON_GetObjectItem(root,"bridge_id");
    cJSON *value=cJSON_GetObjectItem(root,"value");
    printf("recv CMD=%s,BRIDGEID=%s Value=%s\n",cmd->valuestring,bridgeId->valuestring,value->valuestring);
	//handle request CMD
    if(0 == strcmp("getUserInfo",cmd->valuestring)){
		printf("will do getUserInfo.\n");
        GetUserInfo();//bridgeId->valuestring);
	}
    /*
    }else if(0 == strcmp("unlock",cmd->valuestring)){
		printf("will do UnLock.\n");
        char* lockID=value->valuestring;
        UnLock(lockID);
    }*/
    printf("vvvvvvvvvvvvvvv web msgvvvvvvvvvvvvvvv\n");
	cJSON_Delete(root);

	return 0;
}

int DealCMD(sysinfo_t *si, ign_MsgInfo imsg) {
	printf("after pb decode, type[%d]:\n", (imsg.event_type));
	//create node,add into list, bred
	switch(imsg.event_type){
		case ign_EventType_HEARTBEAT:
			{
				printf("RECV MQTT HB msg\n");
				return 0;
			}
		case ign_EventType_UPDATE_USER_INFO: 
			{
				InitLockinfo(si);
				if(imsg.has_server_data){
					/*
					   for(int i=0;i<glock_index;i++){
					   printf("%02d bt_id=%s\n",i,glocks[i].bt_id);
					   }*/
					for(int j=0; j<imsg.server_data.lock_entries_count; j++) {
						LockInfo_t li;
						memset(&li, 0, sizeof(LockInfo_t));
						li.lock_id_size = strlen(imsg.server_data.lock_entries[j].bt_id);
						li.lock_ekey_size = imsg.server_data.lock_entries[j].ekey.guest_aes_key.size;
						li.lock_token_size = imsg.server_data.lock_entries[j].ekey.guest_token.size;
						li.lock_passwd_size = imsg.server_data.lock_entries[j].ekey.password.size;
						memcpy(li.lock_ekey, imsg.server_data.lock_entries[j].ekey.guest_aes_key.bytes, sizeof(li.lock_ekey));
						memcpy(li.lock_token, imsg.server_data.lock_entries[j].ekey.guest_token.bytes, sizeof(li.lock_token));
						memcpy(li.lock_passwd, imsg.server_data.lock_entries[j].ekey.password.bytes, sizeof(li.lock_passwd));
						memcpy(li.lock_id, imsg.server_data.lock_entries[j].bt_id, sizeof(li.lock_id));
						int ret = AddLockinfo(si, &li);
						if (ret) {
							printf("AddLockinfo err, lock_total[%d]\n", si->lock_total);
						}

						printf("[%d], bt_id[%s], guest_aes_ekey size[%d] [", j, imsg.server_data.lock_entries[j].bt_id, imsg.server_data.lock_entries[j].ekey.guest_aes_key.size);
						for(int k=0;k<imsg.server_data.lock_entries[j].ekey.guest_aes_key.size; k++) {
							printf("%02x", imsg.server_data.lock_entries[j].ekey.guest_aes_key.bytes[k]);
						}
						printf("]\n");
						printf("guest_token size[%d] [", imsg.server_data.lock_entries[j].ekey.guest_token.size);
						for(int k=0;k<imsg.server_data.lock_entries[j].ekey.guest_token.size; k++) {
							printf("%02x", imsg.server_data.lock_entries[j].ekey.guest_token.bytes[k]);
						}
						printf("]\n");
						printf("password size[%d] [", imsg.server_data.lock_entries[j].ekey.password.size);
						for(int k=0;k<imsg.server_data.lock_entries[j].ekey.password.size; k++) {
							printf("%02x", imsg.server_data.lock_entries[j].ekey.password.bytes[k]);
						}
						printf("]\n");
					}
				}

				/*/fake lock data
				char lock_id[] = "IGP105cc2684";
				char device_address[] = "ED:67:F0:CC:26:84";
				char guest_key[] = "7df6d0dc000873150e94deb03ccd18cb";
				char passwd[] = "06F3E8FC48D256CA";
				char token[] = "5294087c99c653ca277d5d42074572c83a853ba7c14c0a8fe72f74473777504aeb999ec892f30ab961e2d18354d02708bd6a15bcf73a6103e491dca642873b0367478e08f3a5";

				uint8_t tmp_buff[256] = {0};
				memset(tmp_buff, 0, sizeof(tmp_buff));

				LockInfo_t li;
				memset(&li, 0, sizeof(LockInfo_t));
				memcpy(&li.lock_id, lock_id, strlen(lock_id));
				li.lock_id_size = strlen(lock_id);
				memcpy(&li.lock_addr, device_address, strlen(device_address));
				li.lock_addr_size = sizeof(device_address);

				memset(tmp_buff, 0, sizeof(tmp_buff));
				int ekey_len = hexStrToByte(guest_key, tmp_buff, strlen(guest_key));
				memcpy(&li.lock_ekey, guest_key, ekey_len);
				li.lock_ekey_size = ekey_len;

				memset(tmp_buff, 0, sizeof(tmp_buff));
				int passwd_len = hexStrToByte(passwd, tmp_buff, strlen(passwd));
				memcpy(&li.lock_passwd, tmp_buff, passwd_len);
				li.lock_passwd_size = passwd_len;

				memset(tmp_buff, 0, sizeof(tmp_buff));
				int token_len = hexStrToByte(token, tmp_buff, strlen(token));
				memcpy(&li.lock_token, tmp_buff, token_len);
				li.lock_token_size = token_len;

				int ret = AddLockinfo(si, &li);
				if (ret) {
					printf("AddLockinfo err, lock_total[%d]\n", si->lock_total);
				}*/

				PrintLockinfo(si);

				//for web client
				ign_MsgInfo tmsg = {};
				tmsg.event_type = ign_EventType_UPDATE_USER_INFO;
				tmsg.has_bridge_data = true;
				ign_BridgeEventData *pbed = &tmsg.bridge_data;
				pbed->has_profile = true;
				pbed->profile = Create_IgnBridgeProfile(&g_sysif);
				tmsg.has_server_data = true;
				ign_ServerEventData *psd = &tmsg.server_data;
				psd->lock_entries_count = imsg.server_data.lock_entries_count;
				for(int i=0; i<imsg.server_data.lock_entries_count; i++){
					psd->lock_entries[i] = imsg.server_data.lock_entries[i];
				}
				/*
				if(5 > psd->lock_entries_count){
					memcpy(psd->lock_entries[psd->lock_entries_count].bt_id, li.lock_id, li.lock_id_size); 
					psd->lock_entries[psd->lock_entries_count].has_ekey = 1;
					psd->lock_entries[psd->lock_entries_count].ekey.guest_aes_key.size = li.lock_ekey_size;
					memcpy(psd->lock_entries[psd->lock_entries_count].ekey.guest_aes_key.bytes, li.lock_ekey, li.lock_ekey_size);
					psd->lock_entries[psd->lock_entries_count].ekey.guest_token.size = li.lock_token_size;
					memcpy(psd->lock_entries[psd->lock_entries_count].ekey.guest_token.bytes, li.lock_token, li.lock_token_size);
					psd->lock_entries[psd->lock_entries_count].ekey.password.size = li.lock_passwd_size;
					memcpy(psd->lock_entries[psd->lock_entries_count].ekey.password.bytes, li.lock_passwd, li.lock_passwd_size); 
					psd->lock_entries[psd->lock_entries_count].ekey.keyId = psd->lock_entries_count;
					psd->lock_entries_count++;
				}*/

				SendMQTTMsg(&tmsg, SUB_WEBDEMO);
				return 0;
			}
		case ign_EventType_NEW_JOB_NOTIFY:
			{
				//@@@test
				/*
				   printf("RECV[NEW_JOB_NOTIFY]:bt_id[%s],lock_cmd_size[%d],lock_cmd[",
				   imsg.server_data.job.bt_id,
				   (int)imsg.server_data.job.lock_cmd.size);
				   for(int i=0;i<imsg.server_data.job.lock_cmd.size;i++){
				   printf("%x", imsg.server_data.job.lock_cmd.bytes[i]);
				   }
				   printf("]\nlock_entries_count[%u]:", imsg.server_data.lock_entries_count);
				   for(int j=0; j<imsg.server_data.lock_entries_count; j++) {
				   printf("[%d], bt_id[%s], guest_aes_ekey size[%d] [", j, imsg.server_data.lock_entries[j].bt_id, imsg.server_data.lock_entries[j].ekey.guest_aes_key.size);
				   for(int k=0;k<imsg.server_data.lock_entries[j].ekey.guest_aes_key.size; k++) {
				   printf("%x", imsg.server_data.lock_entries[j].ekey.guest_aes_key.bytes[k]);
				   }
				   printf("]");
				   printf(", guest_token size[%d] [", imsg.server_data.lock_entries[j].ekey.guest_token.size);
				   for(int k=0;k<imsg.server_data.lock_entries[j].ekey.guest_token.size; k++) {
				   printf("%x", imsg.server_data.lock_entries[j].ekey.guest_token.bytes[k]);
				   }
				   printf("]");
				   printf(", password size[%d] [", imsg.server_data.lock_entries[j].ekey.password.size);
				   for(int k=0;k<imsg.server_data.lock_entries[j].ekey.password.size; k++) {
				   printf("%x", imsg.server_data.lock_entries[j].ekey.password.bytes[k]);
				   }
				   printf("]");
				   }
				   printf("\n");
				 */
				LockInfo_t* pli = SearchLockInfo(si, imsg.server_data.demo_job.bt_id);
				if (NULL == pli) {
					printf("SearchLockInfo err. lock_id[%s], si->lock_total[%d].\n", imsg.server_data.demo_job.bt_id, si->lock_total);
					return -1;
				}

				printf("@@@ si->lock_total[%d], pli[%x][%s][%s],demo_job.bt_id[%s], op_cmd[%d], pin[",
						si->lock_total, pli, pli->lock_id, pli->lock_addr,
						imsg.server_data.demo_job.bt_id, imsg.server_data.demo_job.op_cmd);
				for(int n=0;n<imsg.server_data.demo_job.pin.size; n++) {
					printf("%x", imsg.server_data.demo_job.pin.bytes[n]);
				}
				printf("]\n");
				//handle lock_cmd
				//char device_address[] = "EC:09:02:7F:4B:09";
				//char admin_key[] = "96eb72d2852d41df94dac37eb3241caa";
				//char passwd[] = "63c5bd7dd34fe863";

				igm_lock_t* lock = NULL;
				getLock(&lock);
				initLock(lock);
				setLockName(lock, pli->lock_id, strlen(pli->lock_id));//imsg.server_data.demo_job.bt_id, strlen(imsg.server_data.demo_job.bt_id));
				setLockAddr(lock, pli->lock_addr, strlen(pli->lock_addr));//device_address, strlen(device_address));
				setLockKey(lock, pli->lock_ekey, pli->lock_ekey_size);
				setLockPassword(lock, pli->lock_passwd, pli->lock_passwd_size);
				setLockToken(lock, pli->lock_token, pli->lock_token_size);
				//setLockCmd(&lock, imsg.server_data.job.lock_cmd.bytes, imsg.server_data.job.lock_cmd.size);
				void* request = NULL;
				if (ign_DemoLockCommand_CREATE_PIN == imsg.server_data.demo_job.op_cmd) {
					IgCreatePinRequest create_pin_request;
					ig_CreatePinRequest_init(&create_pin_request);
					ig_CreatePinRequest_set_password(&create_pin_request, pli->lock_passwd, pli->lock_passwd_size);
					ig_CreatePinRequest_set_new_pin(&create_pin_request, imsg.server_data.demo_job.pin.bytes, imsg.server_data.demo_job.pin.size);
					printf( "set new_pin[%s]\n", create_pin_request.new_pin);
					ig_CreatePinRequest_set_start_date(&create_pin_request, time(0));
					ig_CreatePinRequest_set_end_date(&create_pin_request, time(0)+10000);
					ig_CreatePinRequest_set_pin_type(&create_pin_request, 2);
					ig_CreatePinRequest_set_operation_id(&create_pin_request, 1);
					request = &create_pin_request;
					//int res = testCreatePin(lock, &create_pin_request);
					//releaseLock(&lock);
				}
				else if (ign_DemoLockCommand_DELETE_PIN == imsg.server_data.demo_job.op_cmd) {
					IgDeletePinRequest delete_pin_request;
					ig_DeletePinRequest_init(&delete_pin_request);
					ig_DeletePinRequest_set_password(&delete_pin_request, pli->lock_passwd, pli->lock_passwd_size);

					ig_DeletePinRequest_set_old_pin(&delete_pin_request, imsg.server_data.demo_job.pin.bytes, imsg.server_data.demo_job.pin.size);
					printf( "set old_pin[%s]\n", delete_pin_request.old_pin);
					ig_DeletePinRequest_set_operation_id(&delete_pin_request, 2);
					request = &delete_pin_request;
					//int res = testDeletePin(lock, &delete_pin_request);
				}
				int ret = HandleLockCMD(si, lock, imsg.server_data.demo_job.op_cmd, request);
				if(ret) {
					printf( "HandleLockCMD ret[%d].\n", ret);
				}
				return 0;
			}
		default:
			{
				printf("RECV MQTT err type[%u].\n", imsg.event_type);
				return -1;
			}
	}
	return -1;
}

void WaitMQTT(sysinfo_t *si) {
	printf("do Waiting MQTT...\n");
	int ret = 0;
	while(1){
		//if (NULL == si->mqtt_c)
		char *topic = NULL;
		int topicLen;
		MQTTClient_message *msg = NULL;
		//printf("will do MQTTClient_receive, mqtt_c[%x], addr[%s]\n", si->mqtt_c, &si->mqtt_c);
		int rc = MQTTClient_receive(si->mqtt_c, &topic, &topicLen, &msg, 1e3);
		if (0 != rc) {
			serverLog(LL_ERROR, "MQTTClient_receive err[%d], topic[%s].", rc, topic);
		}
		if(msg){
			printf(">>>> MQTTClient_receive msg[%u] from server, topic[%s].\n", msg->payloadlen, topic);
			if(0 == strcmp(topic, PUB_WEBDEMO)){
				//web simulator request
				DoWebMsg(topic,msg->payload);
			}else{
				//from server msg
				memset(glocks,0,sizeof(glocks));
				glock_index=0;
				ign_MsgInfo imsg={};
				pb_istream_t in = pb_istream_from_buffer(msg->payload,(size_t)msg->payloadlen);
				//imsg.server_data.lock_entries.funcs.decode=&get_server_event_data;
				ret = pb_decode(&in, ign_MsgInfo_fields, &imsg);
				if(true != ret) {
			        serverLog(LL_ERROR, "pb_decode err[%d].", ret);
					printf("MQTT MSG DECODE ERROR[%d]!\n", ret);
				}else{
					int ret = DealCMD(si, imsg);
					if (ret) {
						serverLog(LL_ERROR, "DealCMD err[%d].", ret);
					}
				}
			} 
			MQTTClient_freeMessage(&msg);
			MQTTClient_free(topic);

			/*
			//search task queue by msg_id
			unsigned int current_state = 1;
			task_node_t *ptn = NULL;
			ptn = FindTaskByMsgID(imsg.msg_id, &waiting_task_head);

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
			InsertTask(&doing_task_head, imsg.msg_id, current_state, NULL, NULL);
			pthread_mutex_unlock(g_sysif.mutex);
			}
			 */
		} else {
			HeartBeat();
		}
	}
}

int StartBLE() {
	pid_t pid;
	if((pid=vfork()) < 0) {
		printf("vfork error!\n");
		return -1;
	} else if(0 == pid) {
		printf("Child process PID: [%d].\n", getpid());
		//char *argv[ ]={"ls", "-al", "/home", NULL}; 
		//char *envp[ ]={"PATH=/bin", NULL};
		if(execl("./", "wifi_service") < 0) {
			printf("subprocess[./wifi_service] start error");
			return -1;
		}
	} else {
		printf("Parent process PID: %d.\n", getpid());
	}
	return 0;
}

int WaitBtn(void *arg){
	//if btn
	//add Init into doing_list
	serverLog(LL_NOTICE, "waiting for Btn...");
	int keys_fd;
	struct input_event t;
	keys_fd=open(DEV_PATH, O_RDONLY);
	if(keys_fd <= 0) {
		serverLog(LL_ERROR, "open /dev/input/event0 device error!\n");
		return -1;
	}

	while(1) {                                               
		if(read(keys_fd, &t, sizeof(t)) == sizeof(t)) {  
			if(t.type==EV_KEY) {                     
				if(t.value==0 || t.value==1) {   
					printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
					if(!t.value) {
						LIGHT_BLINK(g)
						StartBLE();
					} else {
						printf("Btn trigger.\n");
						//system("echo 1 > /sys/class/leds/g/trigger");
					}
				}                                              
			}                                                      
		}                                                              
	}                                                                      
	close(keys_fd);                                                        

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

// æ·»å æ«ææ¹å¼æ ·ä¾
// ble_data éé¢å¨å¸
void addDiscoverTask(int msg_id)
{
    // è®¾ç½®éè¦çåæ°
    serverLog(LL_NOTICE, "Add Discover task");
    serverLog(LL_NOTICE, "1. set ble parameters");
    ble_discover_param_t discover_param;
    serverLog(LL_NOTICE, "1. set scan_timeout to 3");
    discover_param.scan_timeout = 2;
    serverLog(LL_NOTICE, "2. set msg_id to 0(or anything you want)");
    // æåæ°åå¥data, å½åæä¸ªé®é¢å°±æ¯, ä½¿ç¨å®, å¾è®¿é®çäººè®°çéæ¾.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, &discover_param, sizeof(ble_discover_param_t));
    // ä¸è®°æå¤å°ä¸ªç»æ, 30ä¸ªé
    serverLog(LL_NOTICE, "3. init ble result memory, suppose the max num of locks is 30");
    bleInitResults(ble_data, 30, sizeof(igm_lock_t));

    // æå¥ç³»ç»çéå
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DTail(msg_id, BLE_DISCOVER_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getDiscoverFsmTable(), getDiscoverFsmTableLen(), TASK_BLE_DISCOVER
    );
    serverLog(LL_NOTICE, "5. Add Discover task.");
    return;
}

igm_lock_t* checkLockIsDiscovered(igm_lock_t *lock)
{
    int n_try_scan = 3;
    igm_lock_t *lock_nearby;
    while (n_try_scan--) {
        lock_nearby = findLockByName(lock->name);
        if (!lock_nearby) {
            serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge, bridge scan first");
            continueDiscoverLock();
        }
    }
    if (!lock_nearby) {
        serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge");
        return NULL;
    }
    return lock_nearby;
}


int main() {
    int rc = Init(NULL);
	assert(0 == rc);
    serverLog(LL_NOTICE,"Ready to start.");

    //daemon(1, 0);

    //sysinfo_t *si = (sysinfo_t *)malloc(sizeof(sysinfo_t));
    //sysinfoInit(si);
    //Init for paring
    /*int rc = Init(si);
      if (0 != rc) {
      GoExit(si);
      return -1;
      } */

    //g_sysif.mutex = Thread_create_mutex();

    //INIT_LIST_HEAD(&waiting_task_head);
    //INIT_LIST_HEAD(&doing_task_head);
 
    pthread_t mqtt_thread = Thread_start(WaitMQTT, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", mqtt_thread);
    
    //benny, Btn monitor to update wifi info and do reconnection
    pthread_t bt_thread = Thread_start(WaitBtn, &g_sysif);
    serverLog(LL_NOTICE,"new thread to WaitMQTT[%u].", bt_thread);

    //check req list, start a new thread to work, bred
    //after work delete node

    // addDiscoverTask();
    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsDEmpty()) {
            //serverLog(LL_NOTICE,"doing_task_head is empty, check Lock list.");
            //printLockList();
            sleep(1);
        } else {
            serverLog(LL_NOTICE,"doing_task_head not empty, do it.-----------");
        // if doing list has task
            // è·åå½ådoing list çå¤´é¨
            task_node_t *ptn = GetDHeadNode();
            while (ptn) {
                // TODO, å½ä»»å¡å®æ,éè¦æä¹å¤ç?
                int ret = FSMHandle(ptn);
                if(ret) {
                    serverLog(LL_NOTICE, "one mission error[%d].", ret);
                } else {
                    serverLog(LL_NOTICE, "one mission finished, delete this task");
                }
                saveTaskData(ptn);
                DeleteDTask(&ptn); // èªå¨ç½® ptn ä¸º NULL
                
                task_node_t *tmp = NextDTask(ptn);
                if (tmp) {
                    serverLog(LL_NOTICE, "NextDTask not NULL.");
                }
                ptn = tmp;
            }
            serverLog(LL_NOTICE,"doing_task_head not empty, do end.-----------");
        }
    }

    return 0;
}



/*
void addAdminDoLockTask(igm_lock_t *lock) {
    // è®¾ç½®éè¦çåæ°
    serverLog(LL_NOTICE, "Add Admin Unlock task");
    serverLog(LL_NOTICE, "1. set ble admin Unlock parameters");
    ble_admin_param_t *admin_param = (ble_admin_param_t *)calloc(sizeof(ble_admin_param_t), 1);
    serverLog(LL_NOTICE, "1. set admin Unlock param lock to name[%s] addr[%s]", lock->name, lock->addr);
    bleSetAdminParam(admin_param, lock);
    serverLog(LL_NOTICE, "2. set msg_id to 4(or anything you want)");
	int msg_id = GetMsgID();
    // æåæ°åå¥data, å½åæä¸ªé®é¢å°±æ¯, ä½¿ç¨å®, å¾è®¿é®çäººè®°çéæ¾.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    // æå¥ç³»ç»çéå
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DFront(msg_id, BLE_ADMIN_BEGIN, 
        ble_data, sizeof(ble_data_t), lock->lock_cmd, lock->lock_cmd_size,
        getAdminUnlockFsmTable(), getAdminUnlockFsmTableLen(), TASK_BLE_ADMIN_UNLOCK);
    serverLog(LL_NOTICE, "5. Add admin unlock task.");
    return;
}

int testGetLockStatus(igm_lock_t *lock) {
	printf("get lock status cmd ask invoker to release the lock.\n");
	ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *get_lock_status_fsm = getAdminGetLockStatusFsmTable();
    int fsm_max_n = getAdminGetLockStatusFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;
    tn->ble_data_len = sizeof(task_node_t);

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = get_lock_status_fsm;

    tn->task_type = TASK_BLE_ADMIN_GETLOCKSTATUS;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                printf("%d step error[%d]\n", j, event_result);
                error = 1;
                break;
            } else {
                current_state = tn->task_sm_table[j].next_state;
            }
		}
    }
    if (error) {
        printf("process error.\n");
        return error;
    }

    saveTaskData(tn);
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    printf( "lock end-------\n");
    return 0;
}

int testGetLockBattery(igm_lock_t *lock) {
	printf("get lock battery cmd ask invoker to release the lock.\n");

	ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *get_lock_fsm = getAdminGetBatteryLevelFsmTable();
    int fsm_max_n = getAdminGetBatteryLevelFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = get_lock_fsm;

    tn->task_type = TASK_BLE_ADMIN_GET_BATTERY_LEVEL;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // å¢å ä¸ä¸ªå¤æ­å½åå½æ°, æ¯å¦å½åå½æ°åºé. 0 è¡¨ç¤ºæ²¡é®é¢
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                printf("%d step error[%d]\n", j, event_result);
                error = 1;
                break;
            } else {
                current_state = tn->task_sm_table[j].next_state;
            }
		}
    }
    if (error) {
        printf("process error.\n");
        return error;
    }

    saveTaskData(tn);
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    printf( "lock end-------\n");
    return 0;
}

int testLock(igm_lock_t *lock) {
    serverLog(LL_NOTICE,"Lock cmd ask invoker to release the lock.");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *unlock_fsm = getAdminLockFsmTable();
    int fsm_max_n = getAdminLockFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = unlock_fsm;

    tn->task_type = TASK_BLE_ADMIN_LOCK;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // å¢å ä¸ä¸ªå¤æ­å½åå½æ°, æ¯å¦å½åå½æ°åºé. 0 è¡¨ç¤ºæ²¡é®é¢
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                serverLog(LL_ERROR, "%d step error", j);
                error = 1;
                break;
            } else {
				current_state = tn->task_sm_table[j].next_state;
            }
		}
    }
    if (error) {
        serverLog(LL_ERROR, "lock error");
        return error;
    }

    //saveTaskData(tn);
    ble_admin_result_t *admin_unlock_result = (ble_admin_result_t *)ble_data->ble_result;
    ig_AdminLockResponse_deinit(admin_unlock_result->cmd_response);
    // è¿å¿æ¯éæ¾ç»æ, å ä¸ºå·²ç»ç¨å®
    releaseAdminResult(&admin_unlock_result);
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    serverLog(LL_NOTICE, "lock end-------");
    return 0;
}

int testCreatePin(igm_lock_t *lock, IgCreatePinRequest *request) {
    printf("create pin request cmd ask invoker to release the lock.\n");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);
    bleSetAdminRequest(admin_param, request, sizeof(IgCreatePinRequest));

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *create_pin_fsm = getAdminCreatePinRequestFsmTable();
    int fsm_max_n = getAdminCreatePinRequestFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = create_pin_fsm;

    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // å¢å ä¸ä¸ªå¤æ­å½åå½æ°, æ¯å¦å½åå½æ°åºé. 0 è¡¨ç¤ºæ²¡é®é¢
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                printf("%d step error.\n", j);
                error = 1;
                break;
            } else {
                current_state = tn->task_sm_table[j].next_state;
            }
		}
    }
    if (error) {
        printf("lock error.\n");
        return error;
    }

    //saveTaskData(tn);
    ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
    //ig_CreatePinResponse_deinit(result->cmd_response);
    free(result->cmd_response);
    result->cmd_response = NULL;
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    // ä¸å®è¦ç­å°ç°å¨æè½éæ¾, å ä¸ºéé¢çåå®¹, æ¯ä¼
    // è¢«å¤å¶, ç¶åä½¿ç¨
    free(admin_param);
    admin_param = NULL;
    printf( "lock end-------\n");
    return 0;
}

int testDeletePin(igm_lock_t *lock, IgDeletePinRequest *request) {
    printf("delete pin request cmd ask invoker to release the lock.\n");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);
    bleSetAdminRequest(admin_param, request, sizeof(IgDeletePinRequest));

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *delete_pin_fsm = getAdminDeletePinRequestFsmTable();
    int fsm_max_n = getAdminDeletePinRequestFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = delete_pin_fsm;

    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // å¢å ä¸ä¸ªå¤æ­å½åå½æ°, æ¯å¦å½åå½æ°åºé. 0 è¡¨ç¤ºæ²¡é®é¢
            int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                printf("[%d] step error.\n", j);
                error = 1;
                break;
            } else {
                current_state = tn->task_sm_table[j].next_state;
            }
        }
    }
    if (error) {
        printf("lock error.\n");
        return error;
    }

    //saveTaskData(tn);
    ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
    //ig_DeletePinResponse_deinit(result->cmd_response);
    free(result->cmd_response);
    result->cmd_response = NULL;
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    printf( "lock end-------.\n");
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

fsm_table_t g_sm_table[] = {
    {  CMD_INIT,                Init,               GET_WIFI_USER},
    {  GET_WIFI_USER,           BLEParing,          CMD_REQ_USERINFO},
    {  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
    {  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
    {  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},
};

//admin
int HandleLockCMD (igm_lock_t *lock, int cmd, void* request) {
    serverLog(LL_NOTICE, "in HandleLockCMD.");
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);

	if (ign_DemoLockCommand_CREATE_PIN == cmd && NULL != request) {
		bleSetAdminRequest(admin_param, request, sizeof(IgCreatePinRequest));
	} else if (ign_DemoLockCommand_DELETE_PIN == cmd && NULL != request) {
		bleSetAdminRequest(admin_param, request, sizeof(IgDeletePinRequest));
	}

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

	task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
	tn->ble_data = ble_data;
	memset(tn->lock_id, 0x0, sizeof(tn->lock_id));
	memcpy(tn->lock_id, lock->name, lock->name_len);

	if (ign_DemoLockCommand_LOCK == cmd) {
		tn->sm_table_len = getAdminLockFsmTableLen();
		tn->task_sm_table = getAdminLockFsmTable();
		tn->task_type = TASK_BLE_ADMIN_LOCK;
	} else if (ign_DemoLockCommand_UNLOCK == cmd) {
		tn->sm_table_len = getAdminUnlockFsmTableLen();
		tn->task_sm_table = getAdminUnlockFsmTable();
		tn->task_type = TASK_BLE_ADMIN_UNLOCK;
	} else if (ign_DemoLockCommand_GET_LOCK_STATUS == cmd) {
		tn->sm_table_len = getAdminGetLockStatusFsmTableLen();
		tn->task_sm_table = getAdminGetLockStatusFsmTable();
	    tn->task_type = TASK_BLE_ADMIN_GETLOCKSTATUS;
	} else if (ign_DemoLockCommand_GET_BATTERY == cmd) {
		tn->sm_table_len = getAdminGetBatteryLevelFsmTableLen();
		tn->task_sm_table = getAdminGetBatteryLevelFsmTable();
		tn->task_type = TASK_BLE_ADMIN_GET_BATTERY_LEVEL;
	} else if (ign_DemoLockCommand_GET_LOGS == cmd) {
		tn->sm_table_len = getAdminGetLogsFsmTableLen();
		tn->task_sm_table = getAdminGetLogsFsmTable();
	    tn->task_type = TASK_BLE_ADMIN_GETLOGS;
	} else if (ign_DemoLockCommand_CREATE_PIN == cmd) {
		tn->sm_table_len = getAdminCreatePinRequestFsmTableLen();
		tn->task_sm_table = getAdminCreatePinRequestFsmTable();
	    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;
	} else if (ign_DemoLockCommand_DELETE_PIN == cmd) {
		tn->sm_table_len = getAdminDeletePinRequestFsmTableLen();
		tn->task_sm_table = getAdminDeletePinRequestFsmTable();
	    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;
	} 

	int current_state = BLE_ADMIN_BEGIN;
    for (int j = 0; j < tn->sm_table_len; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                serverLog(LL_ERROR, "[%d] step error", j);
                return -1;
                break;
            } else {
				current_state = tn->task_sm_table[j].next_state;
            }
		}
    }

    saveTaskData(tn);
    ble_admin_result_t *admin_unlock_result = (ble_admin_result_t *)ble_data->ble_result;
    //ig_AdminLockResponse_deinit(admin_unlock_result->cmd_response);
    releaseAdminResult(&admin_unlock_result);
    bleReleaseBleResult(ble_data);
    free(ble_data); ble_data = NULL;
    free(tn); tn = NULL;
    free(admin_param); admin_param = NULL;
    serverLog(LL_NOTICE, "HandleLockCMD end-------");
    return 0;
}


*/
