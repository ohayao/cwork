#include <bridge/mqtt/mqtt_util.h>

MQTTClient initClients(
  char *host,char *clientID,int keepAliveInterval,int clearnSession,char *caPath,char *trustStore,char *privateKey,char *keyStore){
    MQTTClient client;
    MQTTClient_create(&client,host,clientID,MQTTCLIENT_PERSISTENCE_NONE,NULL);

    MQTTClient_connectOptions opts=configConnect(keepAliveInterval,clearnSession);
    MQTTClient_SSLOptions ssl=configSSL(caPath,trustStore,privateKey,keyStore);
    bindSSLToConnect(&opts,&ssl);

    int rc = MQTTClient_connect(client, &opts);
    if(MQTTCLIENT_SUCCESS != rc){
        printf("Connect server error with code[%d]!!!!!!\n",rc);
        return NULL;
    }
    printf("Connect server Success ^_^ -- ^_^\n");
    return client;
}

MQTTClient_SSLOptions configSSL(char *caPath,char *trustStore,char *privateKey,char *keyStore){
    MQTTClient_SSLOptions ssl=MQTTClient_SSLOptions_initializer;
    ssl.CApath=caPath;
    ssl.trustStore=trustStore;
    ssl.privateKey=privateKey;
    ssl.keyStore=keyStore;
    return ssl;
}

MQTTClient_connectOptions configConnect(int keepAliveInterval,int cleanSession){
    MQTTClient_connectOptions opts=MQTTClient_connectOptions_initializer;
    opts.keepAliveInterval=keepAliveInterval;
    opts.cleansession=cleanSession;
    return opts;
}

void bindSSLToConnect(MQTTClient_connectOptions *opts,MQTTClient_SSLOptions *ssl){
    opts->ssl=ssl;
}

int sendMessage(MQTTClient client,char *topic,int qos,void * message,int messageLength){
    //    MQTTClient_message msg=MQTTClient_message_initializer;
    //    msg.qos=qos;
    //    msg.payload=message;
    //    msg.payloadlen=messageLength;
    //    MQTTClient_publishMessage(client,topic,&msg,NULL);
    int rc=MQTTClient_publish(client,topic,messageLength,message,qos,0,NULL);
    //printf("send result %d\n",rc);
    return rc;
}