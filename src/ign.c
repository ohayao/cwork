#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "MQTTClient.h"
#include "util.h"

//#include "proto.pb-c.h"
//#include "proto.h"

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID"
#define CA_PATH "/tmp/igkey/"
#define TRUST_STORE "/tmp/igkey/ca.pem"
#define PRIVATE_KEY "/tmp/igkey/key.pem"
#define KEY_STORE "/tmp/igkey/cert.pem"

#define thread_type pthread_t
#define mutex_type pthread_mutex_t*
typedef struct {
    int inited;
    MQTTClient mqtt_c;
    //ssize_t (*read)(struct blockif *, void *buf, size_t count, uint32_t blk);
    //struct index *idx;
}sysinfo_t;

int Init(sysinfo_t *si) {

    // get WiFi & user info from Mobile App
    // get AWS pem by http
   
    si->mqtt_c = util_initClients(HOST,SUBSCRIBE_CLIENT_ID,60,1,CA_PATH,TRUST_STORE,PRIVATE_KEY,KEY_STORE);
    if(NULL == si->mqtt_c) {
        //goto GoExit;
        printf("util_initClients err, mqtt_c is NULL.\n");
        return -1;
    }
    int rc = MQTTClient_subscribe(si->mqtt_c, "test", 1);
    if(MQTTCLIENT_SUCCESS != rc){
        printf("Subscribe [abc] error with code [%d].\n", rc);
    }

    //InitBLE(si);
    //InitBtn(si);
 
    return 0;
}

void GoExit(sysinfo_t *si) {
    //BLE  
    //MQTT
    if(NULL != si->mqtt_c){
        MQTTClient_disconnect(si->mqtt_c, 0);
        MQTTClient_destroy(&si->mqtt_c);
    }
}

static int FSM(MQTTClient_message *msg){
    /*
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
    */
    return 0;
}

void RunMQTT(sysinfo_t *si){
    while(1){
        char *topic = NULL;
        int topicLen;
        MQTTClient_message *msg = NULL;
        int rc = MQTTClient_receive(si->mqtt_c, &topic, &topicLen, &msg, 1e3);
        if (0 != rc) {
            //err log
        }
        if(msg){
            //state machine
            FSM(msg);
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
        } else {
            //err log
        }
    }
}

thread_type Thread_start(void* fn, void* parameter)
{
    thread_type thread = 0;
    pthread_attr_t attr;

    //FUNC_ENTRY
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, fn, parameter) != 0)
        thread = 0;
    pthread_attr_destroy(&attr);
    //FUNC_EXIT;
    return thread;
}

mutex_type Thread_create_mutex(void)
{
    mutex_type mutex = NULL;

    //FUNC_ENTRY;
    mutex = malloc(sizeof(pthread_mutex_t));
    int rc = pthread_mutex_init(mutex, NULL);
    //FUNC_EXIT_RC(rc);
    return mutex;
}
void Thread_destroy_mutex(mutex_type mutex)
{
    //int rc = 0;
    //FUNC_ENTRY;
    pthread_mutex_destroy(mutex);
    free(mutex);
    //FUNC_EXIT_RC(rc);
}

//thread_type thread = Thread_start(thread_fn fn, void* parameter);
//mutex_type mutex = Thread_create_mutex();
//rc = pthread_mutex_lock(mutex);
//rc = pthread_mutex_unlock(mutex);
//pthread_self();
//pthread_detach(p);??????


int RunBLE(){
    //Thread_start(wait_BLE, sysinfo)
    return 0;
}

int main()
{
    //daemon(1, 0);
    sysinfo_t *si = malloc(sizeof(sysinfo_t));
    //Init for paring
    int rc = Init(si);
    if (0 != rc) {
        GoExit(si);
        return -1;
    }

    RunMQTT(si);
    //RunBLE();
    //BtnInit();

    return 0;
}

