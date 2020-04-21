#ifndef _UTIL_H_
#define _UTIL_H_

#include <string.h>
#include <strings.h>

#include "MQTTClient.h"


/*
 *  * 初始化MQTT客户端 
 *  * */
MQTTClient util_initClients(char *host,char *clientID,int keepAliveInterval,int clearnSession,char *caPath,char *trustStore,char *privateKey,char *keyStore);

/*
 *  * 初始化SSL配置
 *  * */
MQTTClient_SSLOptions util_configSSL(char *caPath,char *trustStore,char *privateKey,char *keyStore);

/*
 *  * 初始化连接配置
 *  * */
MQTTClient_connectOptions util_configConnect(int keepAliveInterval,int clearnSession);

/*
 *  * 配置SSL加密连接选项
 *  * */
void util_bindSSLToConnect(MQTTClient_connectOptions *opts,MQTTClient_SSLOptions *ssl);

/*
 *  * 发布消息
 *  * */
int util_sendMessage(MQTTClient client,char *topic,int qos,void * message,int messageLength);


/*
 *  * ***************************
 *  * *******************函数实现
 *  * ***************************
 *  * */

MQTTClient util_initClients(char *host,char *clientID,int keepAliveInterval,int clearnSession,char *caPath,char *trustStore,char *privateKey,char *keyStore){    
    MQTTClient client;
    MQTTClient_create(&client,host,clientID,MQTTCLIENT_PERSISTENCE_NONE,NULL);

    MQTTClient_connectOptions opts=util_configConnect(keepAliveInterval,clearnSession);
    MQTTClient_SSLOptions ssl=util_configSSL(caPath,trustStore,privateKey,keyStore);
    util_bindSSLToConnect(&opts,&ssl);

    int rc;
    if((rc=MQTTClient_connect(client,&opts))!=MQTTCLIENT_SUCCESS){
        printf("Connect server error with code[%d]!!!!!!\n",rc);
        return NULL;                                            
    }
    printf("Connect server Success ^_^ -- ^_^\n");
    return client;                                
}

MQTTClient_SSLOptions util_configSSL(char *caPath,char *trustStore,char *privateKey,char *keyStore){
    MQTTClient_SSLOptions ssl=MQTTClient_SSLOptions_initializer;
    ssl.CApath=caPath;
    ssl.trustStore=trustStore;
    ssl.privateKey=privateKey;
    ssl.keyStore=keyStore;
    return ssl;                        
}

MQTTClient_connectOptions util_configConnect(int keepAliveInterval,int cleanSession){
    MQTTClient_connectOptions opts=MQTTClient_connectOptions_initializer;
    opts.keepAliveInterval=keepAliveInterval;
    opts.cleansession=cleanSession;
    return opts;                
}

void util_bindSSLToConnect(MQTTClient_connectOptions *opts,MQTTClient_SSLOptions *ssl){
    opts->ssl=ssl;
}

int util_sendMessage(MQTTClient client,char *topic,int qos,void * message,int messageLength){
    //    MQTTClient_message msg=MQTTClient_message_initializer;
    //    msg.qos=qos;
    //    msg.payload=message;
    //    msg.payloadlen=messageLength;
    //    MQTTClient_publishMessage(client,topic,&msg,NULL);
    int rc=MQTTClient_publish(client,topic,messageLength,message,qos,0,NULL);
    printf("send result %d\n",rc);
    return rc;                            
}

#endif
