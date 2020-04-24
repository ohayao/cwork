#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "MQTTClient.h"
#include "mqtt_util.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "bridge.pb.h"

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define PUBLISH_CLIENT_ID "JasonPublishID"
#define CA_PATH "../igkey/"
#define TRUST_STORE "../igkey/ca.pem"
#define PRIVATE_KEY "../igkey/key.pem"
#define KEY_STORE "../igkey/cert.pem"


void testSubscribe();

void testSubscribe(){
    printf("Subscribe start join... ...\n");
    MQTTClient cli=util_initClients(HOST,"subscribedID",60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(cli==NULL) goto Over;
    int rc;
    if((rc=MQTTClient_subscribe(cli,"test",1))!=MQTTCLIENT_SUCCESS){
        goto Over;
    }
    while(true){
        char *topic=NULL;
        int len;
        MQTTClient_message *msg=NULL;
        if((rc=MQTTClient_receive(cli,&topic,&len,&msg,1e3))==MQTTCLIENT_SUCCESS){
            if(msg){
                printf("RECV [%s][%s]\n",topic,(char *)msg->payload);
            }
            MQTTClient_free(msg);
            MQTTClient_free(topic);
        }else goto Over;
        sleep(3);
    }
Over:
    if(cli!=NULL){
        MQTTClient_disconnect(cli,0);
        MQTTClient_destroy(&cli);
    }
}


int main()
{
    pthread_t recv;
    int stid=pthread_create(&recv,NULL,(void *)testSubscribe,NULL);
    if(stid>0){
        printf("create thread success\n");
        pthread_join(recv,NULL);    
    }else printf("create thread error\n");
    printf("Publish start join... ...\n");
    MQTTClient cli=util_initClients(HOST,PUBLISH_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(cli==NULL) goto Over;
    int rc;
    while(true){
        ign_LockLog lg={};
        strcpy(lg.bt_id,"0000-1111-MQTT-1234");

        char abc[1024];
        memset(abc,0,sizeof(abc));
        strcpy(abc,"Hello world");
        lg.log_data.size=strlen(abc);
        memcpy(lg.log_data.bytes,abc,strlen(abc));

        uint8_t buf[1024];
        memset(buf,0,sizeof(buf));
        pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
        if(pb_encode(&out,ign_LockLog_fields,&lg)){
            size_t len=out.bytes_written;
            if((rc=util_sendMessage(cli,"test",1,buf,(int)len)==MQTTCLIENT_SUCCESS)){
                printf("Publish Message OK [%s]!\n",buf);
                MQTTClient_yield();
            }else{
                printf("Publish Message Failed!!![%s]\n",buf);
                goto Over;
            }
        }else printf("encode error\n");
        MQTTClient_yield();
        sleep(10);
    }
Over:
    if(cli!=NULL){
        MQTTClient_disconnect(cli,0);
        MQTTClient_destroy(&cli);
    }

    return 0;
}

