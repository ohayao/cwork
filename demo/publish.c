#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>


#include "MQTTClient.h"
#include "util.h"

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define PUBLISH_CLIENT_ID "JasonPublishID"
#define CA_PATH "/root/project/cwbak/igkey/"
#define TRUST_STORE "/root/project/cwbak/igkey/ca.pem"
#define PRIVATE_KEY "/root/project/cwbak/igkey/key.pem"
#define KEY_STORE "/root/project/cwbak/igkey/cert.pem"



int main(){
    MQTTClient publish = util_initClients(HOST,PUBLISH_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(publish==NULL) goto GoExit;
    int count=0;
    while(1==1){
        count++;
        char end[1024];
        memset(end,0,sizeof(end));
        char *content="广东深圳天气-23~c^**&24()*)234";
        sprintf(end,"%03d-%s",count,content);
        
        int rc=util_sendMessage(publish,"abc",1,end,(int)strlen(end));
        if(rc!=MQTTCLIENT_SUCCESS){
            printf("Send msg error with code[%d] and exit soon\n",rc);
            goto GoExit;
        }
        printf("Wait 5s send next one \n");
        sleep(5);
    }
GoExit:
    if(publish!=NULL){
        MQTTClient_disconnect(publish,0);
        MQTTClient_destroy(&publish);
    }
	return 0;
}
