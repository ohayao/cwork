#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "MQTTClient.h"
#include "util.h"

#include "proto.pb-c.h"
#include "proto.h"

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID"
#define CA_PATH "/root/project/cwbak/igkey/"
#define TRUST_STORE "/root/project/cwbak/igkey/ca.pem"
#define PRIVATE_KEY "/root/project/cwbak/igkey/key.pem"
#define KEY_STORE "/root/project/cwbak/igkey/cert.pem"


int main()
{
    MQTTClient subscribe=util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(subscribe==NULL) goto GoExit;
    int rc;
    if((rc=MQTTClient_subscribe(subscribe,"abc",1))!=MQTTCLIENT_SUCCESS){
        printf("Subscribe [abc] error with code [%d]\n",rc);
        goto GoExit;
    }
    if((rc=MQTTClient_subscribe(subscribe,"proto",1))!=MQTTCLIENT_SUCCESS){
        printf("Subscribe [proto] eerror whih code[%d]\n",rc);
        goto GoExit;
    }
    printf("Subscribe [abc|proto] success! per 3s try recv one ^_^\n");
    char *topic_abc="abc";
    char *topic_proto="proto";
    while(1==1){
        char *topic;
        int topicLen;
        MQTTClient_message *msg=NULL;
        MQTTClient_receive(subscribe,&topic,&topicLen,&msg,1e3);

        if(topic!=NULL){
            if(strcmp(topic,topic_abc)==0){
                printf("recv [%s:%d][%s]\n",topic,msg->payloadlen,(char*)msg->payload);
            }else if(strcmp(topic,topic_proto)==0){
                printf("recv [%s:%d]",topic,msg->payloadlen);
                Proto__Question* qst = proto_unPackQuestion(msg->payload,msg->payloadlen);
                if(qst!=NULL){
                    printf("[topic=%s][tips=%s]",qst->topic,qst->tips);
                }else{
                    printf(" pack faild !!!");
                }
                printf("\n");
            }else{
                printf("recv undefined topic [%s] !!!\n",topic);
            }
        }
        if(msg!=NULL){
            MQTTClient_freeMessage(&msg);
            MQTTClient_free(topic);
        }
        sleep(3);
    }
GoExit:
    if(subscribe!=NULL){
        MQTTClient_disconnect(subscribe,0);
        MQTTClient_destroy(&subscribe);
    }
    return 0;
}

