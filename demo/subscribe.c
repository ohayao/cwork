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


int myconnect(MQTTClient* client) {
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
	MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;
	int rc = 0;

	if (opts.verbose)
		printf("Connecting\n");

	if (opts.MQTTVersion == MQTTVERSION_5)
	{
		MQTTClient_connectOptions conn_opts5 = MQTTClient_connectOptions_initializer5;
		conn_opts = conn_opts5;
	}

	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.MQTTVersion = opts.MQTTVersion;

	if (opts.will_topic) 	/* will options */
	{
		will_opts.message = opts.will_payload;
		will_opts.topicName = opts.will_topic;
		will_opts.qos = opts.will_qos;
		will_opts.retained = opts.will_retain;
		conn_opts.will = &will_opts;
	}

	if (opts.connection && (strncmp(opts.connection, "ssl://", 6) == 0 ||
			strncmp(opts.connection, "wss://", 6) == 0))
	{
		if (opts.insecure)
			ssl_opts.verify = 0;
		else
			ssl_opts.verify = 1;
		ssl_opts.CApath = opts.capath;
		ssl_opts.keyStore = opts.cert;
		ssl_opts.trustStore = opts.cafile;
		ssl_opts.privateKey = opts.key;
		ssl_opts.privateKeyPassword = opts.keypass;
		ssl_opts.enabledCipherSuites = opts.ciphers;
		conn_opts.ssl = &ssl_opts;
	}

	if (opts.MQTTVersion == MQTTVERSION_5)
	{
		MQTTProperties props = MQTTProperties_initializer;
		MQTTProperties willProps = MQTTProperties_initializer;
		MQTTResponse response = MQTTResponse_initializer;

		conn_opts.cleanstart = 1;
		response = MQTTClient_connect5(client, &conn_opts, &props, &willProps);
		rc = response.reasonCode;
		MQTTResponse_free(response);
	}
	else
	{
		conn_opts.cleansession = 1;
		rc = MQTTClient_connect(client, &conn_opts);
	}

	if (opts.verbose && rc == MQTTCLIENT_SUCCESS)
		fprintf(stderr, "Connected\n");
	else if (rc != MQTTCLIENT_SUCCESS && !opts.quiet)
		fprintf(stderr, "Connect failed return code: %s\n", MQTTClient_strerror(rc));

	return rc;
}


int main()
{
    MQTTClient subscribe = util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(NULL == subscribe) {
        goto GoExit;
    }
    int rc = 0;
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
    while(1){
        char *topic = NULL;
        int topicLen;
        MQTTClient_message *msg = NULL;
        rc = MQTTClient_receive(subscribe, &topic, &topicLen, &msg, 1e3);
        if(msg){
            //state machine
            FSM(msg)
            /*
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
            */
            MQTTClient_freeMessage(&msg);
            MQTTClient_free(topic);
        }
		if (rc != 0) {
			//myconnect(&client);
            MQTTClient subscribe = util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
            if(NULL == subscribe) {
                goto GoExit;
            }
 
        }
        //sleep(3);
    }
GoExit:
    if(subscribe!=NULL){
        MQTTClient_disconnect(subscribe,0);
        MQTTClient_destroy(&subscribe);
    }
    return 0;
}

