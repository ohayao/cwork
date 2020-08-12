#ifndef __IGN__
#define __IGN__

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdlib.h>
#include <assert.h> 

#include <pthread.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h> //for struct ifreq

#include "bridge/gattlib/gattlib.h"
//#include "bridge/ble/ble_admin.h"

// #include "MQTTClient.h"

#define MAX_LOCK_COUNT		5
#define BLE_SCAN_TIMEOUT	6

atomic_int g_msg_id;

//LIST_HEAD(task_head);
//INIT_LIST_HEAD(task_head);

int GetMacAddr(char * mac, int len_limit) {
    struct ifreq ifreq;
    int sock, ret;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    strcpy (ifreq.ifr_name, "wlan0");    //Currently, only get eth0
    ret = ioctl (sock, SIOCGIFHWADDR, &ifreq);
    if (ret) {
        printf("get eth0 err[%d]\n", ret);
    } else {
        printf("get eth0 succ\n");
    }

    if (NULL == ifreq.ifr_hwaddr.sa_data) {
        strcpy (ifreq.ifr_name, "wlp3s0");    //Currently, only get eth0
        ret = ioctl (sock, SIOCGIFHWADDR, &ifreq);
        if (ret) {
            printf("get wlp3s0 err[%d]\n", ret);
        } else {
            printf("get wlp3s0 succ\n");
        }

        if (NULL == ifreq.ifr_hwaddr.sa_data) {
            printf("get mac falied.\n");
            return -3;
        }
    }

    return snprintf (mac, len_limit, "%X%X%X%X%X%X", 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[0], 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[1], 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[2], 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[3], 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[4], 
        (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}

unsigned int GetMsgID() {
INC:
    atomic_fetch_add_explicit(&g_msg_id, 1, memory_order_relaxed);
	if(0 == g_msg_id % 2){
		goto INC;
	}
	return g_msg_id;
}

//int Init_MQTT(MQTTClient* p_mqtt);

int hexStrToByte(const char* source, uint8_t* dest, int sourceLen) {
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2) {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);
        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;
 
        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return sourceLen /2 ;
}

void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen) {
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i++) {
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;

		highByte += 0x30;

		if (highByte > 0x39)
			dest[i * 2] = highByte + 0x07;
		else
			dest[i * 2] = highByte;

		lowByte += 0x30;
		if (lowByte > 0x39)
			dest[i * 2 + 1] = lowByte + 0x07;
		else
			dest[i * 2 + 1] = lowByte;
	}
	return;
}
/*
int create_gatt_connection(const char* addr, gatt_connection_t** gatt_connection, void** gatt_adapter) {
	//blue connection
	int ret = gattlib_adapter_open(addr, gatt_adapter);
	if (ret) {
		serverLog(LL_ERROR, "register_admin_notfication Failed to open adapter, ret[%d].", ret);
		return -1;
		//??goto ADMIN_ERROR_EXIT;
	}

	serverLog(LL_NOTICE, "register_admin_notfication ready to connection lock ble addr[%s]", addr);
	//optimise this short connection to long!
	//should use asyc connect interface
	*gatt_connection = gattlib_connect(*gatt_adapter, addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (NULL == gatt_connection) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device.");
		return -2;
		//goto ADMIN_ERROR_EXIT;
	}
	serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );
	return 0;
}
*/

char* get_file_content(char *path){
    char* buf;
    FILE *csr=fopen(path,"r");
    fseek(csr,0,SEEK_END);
    int len=ftell(csr);
    buf=(char*)malloc(len+1);
    rewind(csr);
    fread(buf,1,len,csr);
    buf[len]=0;
    fclose(csr);
    return buf;
}
void write_file_content(char *path,char *content){
    FILE *csr=fopen(path,"w");
    fwrite(content,sizeof(char),strlen(content),csr);
    fclose(csr);
}



// thread_type Thread_start(void* fn, void* parameter) {
//     thread_type thread = 0;
//     pthread_attr_t attr;

//     //FUNC_ENTRY
//     pthread_attr_init(&attr);
//     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
//     if (pthread_create(&thread, &attr, fn, parameter) != 0)
//         thread = 0;
//     pthread_attr_destroy(&attr);
//     //FUNC_EXIT;
//     return thread;
// }

// mutex_type Thread_create_mutex(void)
// {
//     mutex_type mutex = NULL;

//     //FUNC_ENTRY;
//     mutex = malloc(sizeof(pthread_mutex_t));
//     int rc = pthread_mutex_init(mutex, NULL);
//     if (0>rc)
//         serverLog(LL_ERROR,"create thread error, rc[%d].", rc);
//     else
//         serverLog(LL_NOTICE,"create thread rc[%d].", rc);
//     //FUNC_EXIT_RC(rc);
//     return mutex;
// }

// void Thread_destroy_mutex(mutex_type mutex)
// {
//     //int rc = 0;
//     //FUNC_ENTRY;
//     pthread_mutex_destroy(mutex);
//     free(mutex);
//     //FUNC_EXIT_RC(rc);
// }

//thread_type thread = Thread_start(thread_fn fn, void* parameter);
//mutex_type mutex = Thread_create_mutex();
//rc = pthread_mutex_lock(mutex);
//rc = pthread_mutex_unlock(mutex);
//pthread_self();
//pthread_detach(p);??????


#endif
