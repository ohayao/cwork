#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>


#include "MQTTClient.h"


int pub_init();


int main(){
	pub_init();
    while(1==1){
        printf("Wait 15s\n");
        sleep(15);
    }
	return 0;
}

int pub_init(){
    printf("publish init start\n");
    MQTTClient publish;
    MQTTClient_connectOptions conn_opts=MQTTClient_connectOptions_initializer;

    MQTTClient_create(&publish,"ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883","JasonPublishID",MQTTCLIENT_PERSISTENCE_NONE,NULL);
    conn_opts.keepAliveInterval=60;
    conn_opts.cleansession=1;
    MQTTClient_SSLOptions ssl=MQTTClient_SSLOptions_initializer;
    ssl.CApath="/root/project/cwbak/igkey/";
    ssl.trustStore="/root/project/cwbak/igkey/ca.pem";
    ssl.privateKey="/root/project/cwbak/igkey/key.pem";
    ssl.keyStore="/root/project/cwbak/igkey/cert.pem";
    conn_opts.ssl=&ssl;
    int rc;
    if((rc=MQTTClient_connect(publish,&conn_opts))!=MQTTCLIENT_SUCCESS){
        printf("error connect %d and exit\n",rc);
        return 1;
    }
    printf("connect mqtt server ok !!!!\n");
    
    MQTTClient_message msg=MQTTClient_message_initializer;
    char *content="Hi, this is paho.mqtt.c's message";
    while(1==1){
        msg.payload=content;
        msg.qos=1;
        msg.payloadlen=strlen(content);
        MQTTClient_publishMessage(publish,"abc",&msg,NULL);
        sleep(10);
    }
    

    return 0;
}
