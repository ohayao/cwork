#ifndef _MQTT_UTIL_H_
#define _MQTT_UTIL_H_

#include <string.h>
#include <strings.h>

#include <MQTTClient.h>
#include <MQTTClientPersistence.h>

/*
 * 初始化MQTT客户端
 * */
MQTTClient initClients(char *host,char *clientID,int keepAliveInterval,int clearnSession,char *caPath,char *trustStore,char *privateKey,char *keyStore);

/*
 * 初始化SSL配置
 * */
MQTTClient_SSLOptions configSSL(char *caPath,char *trustStore,char *privateKey,char *keyStore);

/*
 * 初始化连接配置
 * */
MQTTClient_connectOptions configConnect(int keepAliveInterval,int clearnSession);

/*
 * 配置SSL加密连接选项
 * */
void bindSSLToConnect(MQTTClient_connectOptions *opts,MQTTClient_SSLOptions *ssl);

/*
 * 发布消息
 * */
int sendMessage(MQTTClient client,char *topic,int qos,void * message,int messageLength);
#endif

