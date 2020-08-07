/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Instituto Nokia de Tecnologia - INdT
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
// https://blog.csdn.net/csdn_zyp2015/article/details/73089380

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus/gdbus.h"

#include "bridge/wifi_service/error.h"
#include "bridge/wifi_service/SetWifiInfoRequest.h"
#include "bridge/wifi_service/pairing.h"
#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/wifi_service/ad.h"

#define GATT_MGR_IFACE			"org.bluez.GattManager1"
#define GATT_SERVICE_IFACE		"org.bluez.GattService1"
#define GATT_CHR_IFACE			"org.bluez.GattCharacteristic1"
#define GATT_DESCRIPTOR_IFACE		"org.bluez.GattDescriptor1"
#define LE_AD_MGR_IFACE 			"org.bluez.LEAdvertisingManager1"
#define ADPTER_IFACE					"org.bluez.Adapter1"

#define SET_CRYPT_PORT 13140
#define SET_CRYPT_IP "127.0.1.1"

/* Immediate Alert Service UUID */
// wifi request service
#define WIFI_SERVICE_UUID	"87654321-0000-1000-8000-00805f9b34fb"
#define WIFI_SERVICE_CHR_UUID		"87654321-0000-1000-8000-00805f9b34fb"

/* Random UUID for testing purpose */
#define READ_WRITE_DESCRIPTOR_UUID	"8260c653-1a54-426b-9e36-e84c238bc669"

static GMainLoop *main_loop;
static GSList *services;
static DBusConnection *connection;
static SetWIFIInfoRequest *wifi_info = NULL;
static size_t wifi_message_len = 0;
static size_t wifi_receive_len = 0;

struct characteristic {
	char *service;
	char *uuid;
	char *path;
	uint8_t *value;
	int vlen;
	const char **props;
	bool notifying;
	uint16_t mtu;
	struct io *write_io;
	struct io *notify_io;
	RecvData *recv_pairing_data;
	uint8_t *pairing_value;
	uint8_t event;
};

struct descriptor {
	struct characteristic *chr;
	char *uuid;
	char *path;
	uint8_t *value;
	int vlen;
	const char **props;
};
static GDBusProxy *adapter_proxy;
static GDBusProxy *ad_proxy;

int read_socket;
pthread_mutex_t crypt_mutex = PTHREAD_MUTEX_INITIALIZER;
Crypt *set_wifi_crypt;
size_t set_wifi_crypt_len;
struct sockaddr_in server_addr;
pthread_t server_tid;
#define MAX_CLIENT 5 //连接队列中的个数
#define BUF_SIZE   1024
int fd[MAX_CLIENT];
int conn_amount; //当前的连接数

#define _lockCrypt() pthread_mutex_lock(&crypt_mutex)

#define _unlockCrypt() pthread_mutex_unlock(&crypt_mutex)

// 状态机结构体
// struct connection
/*
 * Alert Level support Write Without Response only. Supported
 * properties are defined at doc/gatt-api.txt. See "Flags"
 * property of the GattCharacteristic1.
 */
static const char *pairing_info_props[] = { "write-without-response", "notify", NULL };
static const char *pairing_desc_props[] = { "read", "write", NULL };

// functions
static void chr_write(struct characteristic *chr, const uint8_t *value, int len);
void cmd_advertise();
void cmd_discoverable();
static int parse_options(DBusMessageIter *iter, const char **device);
static bool chr_read(struct characteristic *chr, DBusMessageIter *iter);
static int parse_value(DBusMessageIter *iter, const uint8_t **value, int *len);

// 状态机相关代码
static FSM *fsm = NULL;

void *serverThread(void *arg)
{
	serverLog(LL_NOTICE, "serverThread start");
	// pthread_detach(pthread_self());
	set_wifi_crypt = (Crypt *)malloc(sizeof(Crypt));
	int yes = 1;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buf[BUF_SIZE];
	int nbytes;
	int fdmax;
	fd_set read_fds;
	int newfd;
	int addrlen;
	struct timeval tv;
	int ret;

	read_socket = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == read_socket)
	{
		serverLog(LL_ERROR, "createServer socket");
		pthread_exit(NULL);
	}
	serverLog(LL_NOTICE, "socket success");

	/*"address already in use" error message */
	if(setsockopt(read_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("Server-setsockopt() error lol!");
		pthread_exit(NULL);
	}

	serverLog(LL_NOTICE, "setsockopt() success\n");

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(SET_CRYPT_PORT);
	(server_addr.sin_addr).s_addr=htonl(INADDR_ANY);

	if(-1==bind(read_socket,(struct sockaddr *)&server_addr,sizeof(server_addr)))  //套接字与端口绑定
	{
		serverLog(LL_ERROR, "bind errer");
		pthread_exit(NULL);
	}
	serverLog(LL_NOTICE, "bind success");

	if(-1==listen(read_socket, MAX_CLIENT)) //5是最大连接数，指服务器最多连接5个用户
	{
		serverLog(LL_ERROR, "listen");
		pthread_exit(NULL);
	}
	serverLog(LL_NOTICE, "listen() success\n");

	fdmax = read_socket; /* so far, it's this one*/
	conn_amount = 0;

	for(;;)
	{
		FD_ZERO(&read_fds); //清除描述符集
		FD_SET(read_socket, &read_fds); //把sock_fd加入描述符集
		tv.tv_sec = 30;
		tv.tv_usec =0;
		for (int i=0;i<MAX_CLIENT;i++) 
    {
      if(fd[i]!=0)
      {
        FD_SET(fd[i], &read_fds);
      }
    }
		ret = select(fdmax+1, &read_fds, NULL, NULL, &tv);
		if( ret == -1)
		{
			serverLog(LL_ERROR, "Server-select() error");
			pthread_exit(NULL);
		}
		else if (ret == 0)
		{
			serverLog(LL_ERROR, "timeout error");
			continue;
		}

		serverLog(LL_NOTICE, "select() success\n");
		/*run through the existing connections looking for data to be read*/
		serverLog(LL_NOTICE, "start to check new connection\n");
		if (FD_ISSET(read_socket, &read_fds))
		{
			newfd = accept(read_socket, (struct sockaddr *)&client_addr, &addrlen);
			if (newfd <= 0)
			{
				serverLog(LL_ERROR,"accept error");
				continue;
			}
			if (conn_amount < MAX_CLIENT)
			{
				for (int i = 0; i < MAX_CLIENT; ++i)
				{
					if (fd[i] == 0)
					{
						fd[i] = newfd;
						break;
					}
				}
				++conn_amount;
				serverLog(LL_NOTICE, "new connection client[%d]%s:%d", conn_amount,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
				if (newfd > fdmax)
				{
					fdmax = newfd;
				}
			}
			else
			{
				serverLog(LL_ERROR, "max connections arrive ,exit");
				close(newfd);
				continue;
			}
			// show client
			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				serverLog(LL_NOTICE, "[%d]: %d ",i,fd[i]);
			}
		}

		serverLog(LL_NOTICE, "start to check connect data");
		for(int i = 0; i < conn_amount; i++)
		{
			if (FD_ISSET(fd[i], &read_fds))
			{
				ret = recv(fd[i], buf, sizeof(buf),0);
				if (ret <= 0)
				{
					serverLog(LL_NOTICE, "client[%d] close\n",i);
					close(fd[i]);
					FD_CLR(fd[i], &read_fds);
					fd[i]=0;
					conn_amount--;
				}
				else
				{
					if(ret < BUF_SIZE)
					{
						serverLog(LL_NOTICE, "received data:");
						// for (int i = 0; i < ret; ++i)
						// {
						// 	printf(" %x", buf[i]);
						// }
						// printf("\n");
						_lockCrypt();
						serverLog(LL_NOTICE, "get client[%d] data", i);
						if (decodeCrypt(buf, ret, set_wifi_crypt, 0))
						{
							serverLog(LL_ERROR, "decodeCrypt error");
							continue;
						}
						serverLog(LL_NOTICE, "decodeCrypt success");

						serverLog(LL_NOTICE, "Crypt Client nonce");
						if (set_wifi_crypt->has_client_nonce)
						{
							for (int i = 0; i < set_wifi_crypt->client_nonce_len; ++i)
							{
								printf("%2x", set_wifi_crypt->client_nonce[i]);
							}
							printf("\n");
						}

						serverLog(LL_NOTICE, "Crypt Server nonce");
						if (set_wifi_crypt->has_server_nonce)
						{
							for (int i = 0; i < set_wifi_crypt->server_nonce_len; ++i)
							{
								printf("%2x", set_wifi_crypt->server_nonce[i]);
							}
							printf("\n");
						}

						serverLog(LL_NOTICE, "Crypt admin key");
						if (set_wifi_crypt->has_server_pairing_admin_key)
						{
							for (int i = 0; i < set_wifi_crypt->server_pairing_admin_key_len; ++i)
							{
								printf("%2x", set_wifi_crypt->server_pairing_admin_key[i]);
							}
							printf("\n");
						}
						_unlockCrypt();
						// 多线程了, 还没加锁, 出错再加
						decideEvent();
						handleClientEvent();
					}
					
				}
			}
			
		}
	}
	exit(0);
}

void createServerThread()
{
	if ( pthread_create(&server_tid,NULL,serverThread,NULL) == -1){

		serverLog(LL_ERROR, "pthread_create err");
		return;
	}
	serverLog(LL_NOTICE, "createServerThread success");
}

// 对 完成 pairing 后, set wifi request 的处理
int handleSetWifiRequest(void *arg)
{	
	serverLog(LL_NOTICE, " handleSetWifiRequest");

	
	return 0;
}

// 记录相关Crypt信息
int handlePairingCrypt(void *arg)
{
	serverLog(LL_NOTICE, " handlePairingCrypt");

}

// 正确返回 response
int handleReplyWifiResponse(void *arg)
{	
	serverLog(LL_NOTICE, " handleSetWifiRequest");
	
	return 0;
}


int getGlobalFSM()
{
	if(getFSM(&fsm))
  {
    serverLog(LL_ERROR, "getFSM err");
    return 1;
  }
	serverLog(LL_NOTICE, "getFSM success");
	return 0;
}

// 写死的 fsm 过程, 所以不需要参数设置
// 然后, 需要适配以下数据的不同
// 在server 当中, 是什么数据呢
// 在这儿, 初始化状态机的处理函数
int initWifiRequestFsm()
{
	uint8_t max_trans_num;
	FSMTransform trans_item;
	max_trans_num = 10;

  if (getFSMTransTable(fsm, max_trans_num))
  {
    serverLog(LL_ERROR, "getFSMTransTable err");
    return 1;
  }
  serverLog(LL_NOTICE, "getFSMTransTable success");

	// 
	// 1 SET_WIFI_REQUEST to SET_WIFI_COMPLETE
	if (fillTransItem(&trans_item, 
    S_RECV_PAIRING_CRYPT, SET_WIFI_NOTPAIR, handlePairingCrypt, SET_WIFI_BEGIN))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event C_WRITE_COMMIT");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  
	

	// 2 PAIRING_BEGIN to SET_WIFI_REQUEST
	if (fillTransItem(&trans_item, 
    C_WRITE_WIFI_REQUEST, SET_WIFI_BEGIN, handleSetWifiRequest, SET_WIFI_REQUEST))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event C_WRITE_COMMIT");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }

	// 3 SET_WIFI_REQUEST to SET_WIFI_COMPLETE
	if (fillTransItem(&trans_item, 
    S_REPLY_WIFI_RESPONSE, SET_WIFI_REQUEST, handleReplyWifiResponse, SET_WIFI_COMPLETE))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event C_WRITE_COMMIT");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }

	// 把虚拟机, 弄成 PAIRING_BEGIN, 也就是空闲的意思啦
	initFSMCurState(fsm, SET_WIFI_BEGIN);
}




static gboolean desc_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &desc->uuid);

	return TRUE;
}

static gboolean desc_get_characteristic(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH,
						&desc->chr->path);

	return TRUE;
}

static bool desc_read(struct descriptor *desc, DBusMessageIter *iter)
{
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &array);

	if (desc->vlen && desc->value)
		dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
						&desc->value, desc->vlen);

	dbus_message_iter_close_container(iter, &array);

	return true;
}

static gboolean desc_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	printf("Descriptor(%s): Get(\"Value\")\n", desc->uuid);

	return desc_read(desc, iter);
}

static void desc_write(struct descriptor *desc, const uint8_t *value, int len)
{
	g_free(desc->value);
	desc->value = g_memdup(value, len);
	desc->vlen = len;

	g_dbus_emit_property_changed(connection, desc->path,
					GATT_DESCRIPTOR_IFACE, "Value");
}

static int parse_value(DBusMessageIter *iter, const uint8_t **value, int *len)
{
	DBusMessageIter array;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &array);
	dbus_message_iter_get_fixed_array(&array, value, len);

	return 0;
}

static void desc_set_value(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct descriptor *desc = user_data;
	const uint8_t *value;
	int len;

	printf("Descriptor(%s): Set(\"Value\", ...)\n", desc->uuid);

	if (parse_value(iter, &value, &len)) {
		printf("Invalid value for Set('Value'...)\n");
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	desc_write(desc, value, len);

	g_dbus_pending_property_success(id);
}

static gboolean desc_get_props(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	printf("desc_get_props\n");
	struct descriptor *desc = data;
	DBusMessageIter array;
	int i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &array);

	for (i = 0; desc->props[i]; i++)
		dbus_message_iter_append_basic(&array,
					DBUS_TYPE_STRING, &desc->props[i]);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static const GDBusPropertyTable desc_properties[] = {
	{ "UUID",		"s", desc_get_uuid },
	{ "Characteristic",	"o", desc_get_characteristic },
	{ "Value",		"ay", desc_get_value, desc_set_value, NULL },
	{ "Flags",		"as", desc_get_props, NULL, NULL },
	{ }
};

static const GDBusPropertyTable wifi_desc_properties[] = {
	{ "UUID",		"s", desc_get_uuid },
	{ "Characteristic",	"o", desc_get_characteristic },
	{ "Value",		"ay", desc_get_value, desc_set_value, NULL },
	{ "Flags",		"as", desc_get_props, NULL, NULL },
	{ }
};

static gboolean chr_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct characteristic *chr = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &chr->uuid);

	return TRUE;
}

static gboolean chr_get_service(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct characteristic *chr = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH,
							&chr->service);

	return TRUE;
}

// 仅仅是读取 chr 的值
static bool chr_read(struct characteristic *chr, DBusMessageIter *iter)
{

	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &array);

	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
						&chr->value, chr->vlen);

	dbus_message_iter_close_container(iter, &array);

	return true;
}

static gboolean chr_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	// 写特征值的时候会调用
	struct characteristic *chr = user_data;

	// printf("Characteristic(%s): Get(\"Value\")\n", chr->uuid);

	return chr_read(chr, iter);
}

static gboolean chrc_get_notifying(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	printf("chrc_get_notifying\n");
	struct characteristic *chrc = user_data;
	dbus_bool_t value;

	value = chrc->notifying ? TRUE : FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &value);
	// printf("chrc_get_notifying  ------------------ \n");
	return TRUE;
}

// iter 是用作什么的?
static gboolean chr_get_props(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	printf("chr_get_props\n");
	struct characteristic *chr = data;
	DBusMessageIter array;
	int i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &array);

	for (i = 0; chr->props[i]; i++)
		dbus_message_iter_append_basic(&array,
					DBUS_TYPE_STRING, &chr->props[i]);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static void chr_set_value(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	fprintf(stdout, "-------------------------------------chr_set_value\n");
	struct characteristic *chr = user_data;
	const uint8_t *value;
	int len;

	printf("Characteristic(%s): Set('Value', ...)\n", chr->uuid);

	// parse_value 是什么?
	if (!parse_value(iter, &value, &len)) {
		printf("Invalid value for Set('Value'...)\n");
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}
	if (!len) return;
	
	chr_write(chr, value, len);

	g_dbus_pending_property_success(id);
}

// struct GDBusPropertyTable {
// 	const char *name;
// 	const char *type;
// 	GDBusPropertyGetter get;
// 	GDBusPropertySetter set;
// 	GDBusPropertyExists exists;
// 	GDBusPropertyFlags flags;
// };
static const GDBusPropertyTable chr_properties[] = {
	{ "UUID",	"s", chr_get_uuid },
	{ "Service",	"o", chr_get_service },
	{ "Value",	"ay", chr_get_value, chr_set_value, NULL },
	{ "Flags",	"as", chr_get_props, NULL, NULL },
	{ "Notifying", "b", chrc_get_notifying, NULL, NULL },
	{ }
};

static gboolean service_get_primary(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	dbus_bool_t primary = TRUE;

	printf("Get Primary: %s\n", primary ? "True" : "False");

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &primary);

	return TRUE;
}

static gboolean service_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	const char *uuid = user_data;

	printf("Get UUID: %s\n", uuid);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);

	return TRUE;
}

static gboolean service_get_includes(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	const char *uuid = user_data;

	printf("Get Includes: %s\n", uuid);

	return TRUE;
}

static gboolean service_exist_includes(const GDBusPropertyTable *property,
							void *user_data)
{
	const char *uuid = user_data;

	printf("Exist Includes: %s\n", uuid);

	return FALSE;
}

static const GDBusPropertyTable service_properties[] = {
	{ "Primary", "b", service_get_primary },
	{ "UUID", "s", service_get_uuid },
	{ "Includes", "ao", service_get_includes, NULL,
					service_exist_includes },
	{ }
};

static void chr_iface_destroy(gpointer user_data)
{
	struct characteristic *chr = user_data;

	g_free(chr->uuid);
	g_free(chr->service);
	g_free(chr->value);
	g_free(chr->path);
	g_free(chr);
}

static void desc_iface_destroy(gpointer user_data)
{
	struct descriptor *desc = user_data;

	g_free(desc->uuid);
	g_free(desc->value);
	g_free(desc->path);
	g_free(desc);
}

static int parse_options(DBusMessageIter *iter, const char **device)
{
	DBusMessageIter dict;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;
		int var;

		dbus_message_iter_recurse(&dict, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		var = dbus_message_iter_get_arg_type(&value);
		if (strcasecmp(key, "device") == 0) {
			if (var != DBUS_TYPE_OBJECT_PATH)
				return -EINVAL;
			dbus_message_iter_get_basic(&value, device);
			printf("Device: %s\n", *device);
		}

		dbus_message_iter_next(&dict);
	}

	return 0;
}

// 在 descriptor里面是可以读的
static DBusMessage *chr_read_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	fprintf(stdout, "chr_read_value\n");
	struct characteristic *chr = user_data;
	DBusMessage *reply;
	DBusMessageIter iter;
	const char *device;

	fprintf(stdout, "dbus_message_iter_init\n");
	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	fprintf(stdout, "parse_options\n");
	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	fprintf(stdout, "dbus_message_new_method_return\n");
	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return g_dbus_create_error(msg, DBUS_ERROR_NO_MEMORY,
							"No Memory");

	fprintf(stdout, "dbus_message_iter_init_append\n");
	dbus_message_iter_init_append(reply, &iter);
	fprintf(stdout, "chr\n");
	chr_read(chr, &iter);

	return reply;
}

static void chr_write(struct characteristic *chr, const uint8_t *value, int len)
{
	printf("chr_write state machine start");
	g_free(chr->value);
	chr->value = g_memdup(value, len);
	chr->vlen = len;
	printf("-------------g_dbus_emit_property_changed\n");
	g_dbus_emit_property_changed(connection, chr->path, GATT_CHR_IFACE,
								"Value");
}

// 根据 FSM的当前状态, 给出现在应该发送的时间
// 大概的想法就是, 
// 
void decideEvent(void *arg)
{
	struct characteristic *chr = (struct characteristic *)arg;
	switch(fsm->cur_state)
	{
		case SET_WIFI_NOTPAIR:
		{
			serverLog(LL_NOTICE, "decideEvent SET_WIFI_NOTPAIR");
			chr->event = S_RECV_PAIRING_CRYPT;
			break;
		}
		case SET_WIFI_BEGIN:
		{
			serverLog(LL_NOTICE, "decideEvent PAIRING_BEGIN");
			chr->event = C_WRITE_WIFI_REQUEST;
			break;
		}
		default:
		{
			serverLog(LL_ERROR, "decideEvent default error");
			break;
		}
	}
}

void handleClientEvent(void *arg)
{
	struct characteristic *chr = arg;
	int handle_res = 0;
	switch (chr->event)
	{
	case S_RECV_PAIRING_CRYPT:
	{
		serverLog(LL_NOTICE, "handleClientEvent S_RECV_PAIRING_CRYPT cur_state %d", fsm->cur_state);
		handle_res = handleEvent(fsm, chr->event, NULL);
		if (handle_res)
		{
			serverLog(LL_ERROR, "handle event C_WRITE_STEP1 error");
		}
		serverLog(LL_NOTICE, "handleClientEvent S_RECV_PAIRING_CRYPT cur_state %d", fsm->cur_state);
		break;
	}
	case C_WRITE_WIFI_REQUEST:
		// 首先转换状态, 到 step1
		serverLog(LL_NOTICE, "handleClientEvent C_WRITE_WIFI_REQUEST cur_state %d", fsm->cur_state);
		handle_res = handleEvent(fsm, chr->event, NULL);
		if (handle_res)
		{
			serverLog(LL_ERROR, "handle event C_WRITE_STEP1 error");
		}
		serverLog(LL_NOTICE, "handleClientEvent C_WRITE_STEP1 cur_state %d", fsm->cur_state);
		
		/* code */
		break;
	default:
		serverLog(LL_ERROR, "default error event");
		break;
	}
}

// 因为 process_message method->function的调用, 就是调的这个函数, 所以我直接在这里写状态机
// 这个主体, 先用于进行一个派对的实现.
// DBusConnection conn 是一个 dbus 的连接
// DBusMessage *msg,  对方传递过来的消息
// void *user_data, struct characteristic 的数据
// chr_write_value, 是 写到 chr 里面 的, 发送过来进行接收的函数
static DBusMessage *chr_write_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	// 初始化变量状态
	struct characteristic *chr = user_data;
	DBusMessageIter iter;
	const uint8_t *value = NULL;
	const uint8_t *wifi_info = NULL;
	int len = 0;
	const char *device;

	// 初始化一个迭代器样的东西, 对于这个消息
	dbus_message_iter_init(msg, &iter);

	if (parse_value(&iter, &value, &len))
	{
		printf("parse_value(&iter, &value, &len)\n");
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");
	}

	if (parse_options(&iter, &device))
	{
		printf("parse_options(&iter, &device)\n");
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");
	}

	if (!len) {
		printf("len is zero\n");
		return dbus_message_new_method_return(msg);
	}
	// want't to shut the warming up
	serverLog(LL_NOTICE, " chr_write_value recv data");
	recvData(chr->recv_pairing_data, (uint8_t *)value, len);
	// printf("value len %d----------\n", len);
	// printf("------------------- write value: \n");
	// for (int i = 0; i < len; ++i)
	// {
	// 	printf(" %x", value[i]);
	// }
	// printf("\n");

	//确实在这儿写状态机, 上面是不断的接受写过来的值,
	// 会分开几个包, 
	// 
	// 这里就是状态机开始?
	if(isRecvFullPkg(chr->recv_pairing_data))
	{
		// 这样就只会返回一个恢复,所以要弄好
		serverLog(LL_NOTICE, "-------- get full pkg");
		// decideEvent(user_data);
		// handleClientEvent(user_data);
		// chr_write(chr, value, len);
	}
	

	return dbus_message_new_method_return(msg);
}

static DBusMessage *chr_acquire_notify(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	printf("chr_acquire_notify 1\n");
	struct characteristic *chrc = user_data;
	DBusMessageIter iter;
	DBusMessage *reply;

	dbus_message_iter_init(msg, &iter);

	// if (chrc->notify_io)
	// 	return g_dbus_create_error(msg,
	// 				"org.bluez.Error.NotPermitted",
	// 				NULL);

	// if (parse_options(&iter, chrc))
	// 	return g_dbus_create_error(msg,
	// 				"org.bluez.Error.InvalidArguments",
	// 				NULL);

	// reply = chrc_create_pipe(chrc, msg);

	if (chrc->notify_io)
		g_dbus_emit_property_changed(conn, chrc->path, GATT_CHR_IFACE,
							"NotifyAcquired");

	return reply;
}

static DBusMessage *chr_start_notify(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	printf("chr_start_notify 1 \n");
	struct characteristic *chrc = user_data;
	if (!chrc->notifying)
		return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
	printf("chr_start_notify 2 \n");
	chrc->notifying = true;

	g_dbus_emit_property_changed(conn, chrc->path, GATT_CHR_IFACE,
							"Notifying");
	
	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static DBusMessage *chr_stop_notify(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	printf("chr_stop_notify 1 \n");
	struct characteristic *chrc = user_data;
	printf("path: %s \n", chrc->path);
	if (chrc->notifying)
	{
		printf("chrc->notifying \n");
		return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
	}
		
	
	chrc->notifying = false;

	printf("g_dbus_create_reply \n");
	g_dbus_emit_property_changed(conn, chrc->path, GATT_CHR_IFACE,
							"Notifying");

	printf("g_dbus_create_reply ---------------\n");
	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

// 定义特征的方法
static const GDBusMethodTable chr_methods[] = {
	{ GDBUS_ASYNC_METHOD("ReadValue", GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					chr_read_value) },
	{ GDBUS_ASYNC_METHOD("WriteValue", GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL, chr_write_value) },
	{ GDBUS_METHOD("AcquireNotify", GDBUS_ARGS({ "options", "a{sv}" }),
					NULL, chr_acquire_notify) },
	{ GDBUS_ASYNC_METHOD("StartNotify", NULL, NULL, chr_start_notify) },
	{ GDBUS_METHOD("StopNotify", NULL, NULL, chr_stop_notify) },
	{ }
};

// 设置 wifi 的数据结构
// desc



// select-attribute 8260c653-1a54-426b-9e36-e84c238bc669
// read 命令
// 会回调这个函数
static DBusMessage *desc_read_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	fprintf(stdout, "desc_read_value\n");
	struct descriptor *desc = user_data;
	DBusMessage *reply;
	DBusMessageIter iter;
	const char *device;

	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return g_dbus_create_error(msg, DBUS_ERROR_NO_MEMORY,
							"No Memory");

	dbus_message_iter_init_append(reply, &iter);

	desc_read(desc, &iter);

	return reply;
}

static DBusMessage *desc_write_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	fprintf(stdout, "desc_write_value\n");
	struct descriptor *desc = user_data;
	DBusMessageIter iter;
	const char *device;
	const uint8_t *value;
	int len;

	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_value(&iter, &value, &len))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	desc_write(desc, value, len);

	return dbus_message_new_method_return(msg);
}

// descriptor 所能够用的方法
static const GDBusMethodTable desc_methods[] = {
	{ GDBUS_ASYNC_METHOD("ReadValue", GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					desc_read_value) },
	{ GDBUS_ASYNC_METHOD("WriteValue", GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL, desc_write_value) },
	{ }
};

// 注册特征
static gboolean register_characteristic(const char *chr_uuid,
						const uint8_t *value, int vlen,
						const char **props,
						const char *desc_uuid,
						const char **desc_props,
						const char *service_path)
{
	struct characteristic *chr;
	struct descriptor *desc;
	static int id = 1;

	chr = g_new0(struct characteristic, 1);
	chr->uuid = g_strdup(chr_uuid);
	chr->value = g_memdup(value, vlen);
	chr->vlen = vlen;
	chr->props = props;
	chr->service = g_strdup(service_path);
	chr->path = g_strdup_printf("%s/characteristic%d", service_path, id++);
	// 初始化自己的数据结构
	getRecvData(&chr->recv_pairing_data);
	initRecvData(chr->recv_pairing_data);

	// 获得 fsm
	if(getFSM(&fsm))
  {
    serverLog(LL_ERROR, "getFSM err");
    return 1;
  }
  serverLog(LL_NOTICE, "getFSM success");

	// 初始化全局的 fsm
	serverLog(LL_NOTICE, "---------- initWifiRequestFsm success");
	initWifiRequestFsm();
	
	if (!g_dbus_register_interface(connection, chr->path, GATT_CHR_IFACE,
					chr_methods, NULL, chr_properties,
					chr, chr_iface_destroy)) {
		printf("Couldn't register characteristic interface\n");
		chr_iface_destroy(chr);
		return FALSE;
	}

	if (!desc_uuid)
		return TRUE;

	desc = g_new0(struct descriptor, 1);
	desc->uuid = g_strdup(desc_uuid);
	desc->chr = chr;
	desc->props = desc_props;
	desc->path = g_strdup_printf("%s/descriptor%d", chr->path, id++);

	if (!g_dbus_register_interface(connection, desc->path,
					GATT_DESCRIPTOR_IFACE,
					desc_methods, NULL, desc_properties,
					desc, desc_iface_destroy)) {
		printf("Couldn't register descriptor interface\n");
		g_dbus_unregister_interface(connection, chr->path,
							GATT_CHR_IFACE);

		desc_iface_destroy(desc);
		return FALSE;
	}

	return TRUE;
}

// 如何注册一个interface?
// g_dbus_register_interface
// 	object_path_ref 获取一个通用的数据结构, 代表这个dbus连接的
// 	find_interface 如果已经添加, 就会失败
// 	add_interface 主要的添加接口
// 		struct generic_data *data 的数据结构, 是传入进去的, 是通过object_path_ref所获得的
// 		新建 interface_data 结构体
//    把上面的 interface data 数据结构, 添加到 data
static char *register_service(const char *uuid)
{
	static int id = 1;
	char *path;

	path = g_strdup_printf("/service%d", id++);
	if (!g_dbus_register_interface(connection, path, GATT_SERVICE_IFACE,
				NULL, NULL, service_properties,
				g_strdup(uuid), g_free)) {
		printf("Couldn't register service interface\n");
		g_free(path);
		return NULL;
	}

	return path;
}

// 创建服务的具体函数
// 这是一个注册 pairing services
static void create_services()
{
	char *service_path;
	uint8_t level = 0;

	uint8_t *set_wifi_info_request = malloc(sizeof(SetWIFIInfoRequest));
	initWifiInfoRequest((SetWIFIInfoRequest*)set_wifi_info_request);

	service_path = register_service(WIFI_SERVICE_UUID);
	if (!service_path)
		return;

	/* Add Alert Level Characteristic to Immediate Alert Service */
	// 添加wifi 的特征进取
	if (!register_characteristic(WIFI_SERVICE_CHR_UUID,
						set_wifi_info_request, sizeof(SetWIFIInfoRequest),
						pairing_info_props,	// chr wifi 的属性?
						READ_WRITE_DESCRIPTOR_UUID,
						pairing_desc_props,				// chr wifi desc props
						service_path)) {
		printf("Couldn't register Alert Level characteristic (IAS)\n");
		g_dbus_unregister_interface(connection, service_path,
							GATT_SERVICE_IFACE);
		g_free(service_path);
		free(wifi_info);
		return;
	}

	services = g_slist_prepend(services, service_path);
	printf("Registered service: %s\n", service_path);
	free(wifi_info);
}

static void register_app_reply(DBusMessage *reply, void *user_data)
{
	DBusError derr;

	dbus_error_init(&derr);
	dbus_set_error_from_message(&derr, reply);

	if (dbus_error_is_set(&derr))
		printf("RegisterApplication: %s\n", derr.message);
	else
		printf("RegisterApplication: OK\n");

	dbus_error_free(&derr);
}

static void register_app_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = "/";
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

	/* TODO: Add options dictionary */
	dbus_message_iter_close_container(iter, &dict);
}

static void register_app(GDBusProxy *proxy)
{
	if (!g_dbus_proxy_method_call(proxy, "RegisterApplication",
					register_app_setup, register_app_reply,
					NULL, NULL)) {
		printf("Unable to call RegisterApplication\n");
		return;
	}
}

static void proxy_added_cb(GDBusProxy *proxy, void *user_data)
{
	
	const char *iface;

	iface = g_dbus_proxy_get_interface(proxy);

	if (!g_strcmp0(iface, LE_AD_MGR_IFACE))
	{
		ad_proxy = proxy;
		cmd_advertise();
	}
	else if (!g_strcmp0(iface, ADPTER_IFACE))
	{
		adapter_proxy = proxy;
		// 仅仅是开启几秒
		cmd_discoverable();
	}	

	if (g_strcmp0(iface, GATT_MGR_IFACE))
		return;

	register_app(proxy);
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static bool __terminated = false;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (!__terminated) {
			printf("Terminating\n");
			g_main_loop_quit(main_loop);
		}

		__terminated = true;
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static const char *ad_arguments[] = {
	"on",
	"off",
	"peripheral",
	"broadcast",
	NULL
};

// arg_table 解析参数的表
// value: 是否为真
// msg: 
static gboolean parse_argument(char *argv[], const char **arg_table, dbus_bool_t *value,
					const char **option)
{
	const char **opt;
	for (opt = arg_table; opt && *opt; opt++) {
		if (strcmp(argv[0], *opt) == 0) {
			*value = TRUE;
			*option = *opt;
			return TRUE;
		}
	}
	return FALSE;
}


void cmd_advertise()
{
	serverLog(LL_NOTICE, "--------------- cmd_advertise()");
	dbus_bool_t enable;
	const char *type;
	char *argv[1];
	argv[0] = "broadcast";

	if (!parse_argument(argv, ad_arguments, &enable, &type))
		return;
	serverLog(LL_NOTICE, " cmd_advertise enable: %d type: %s", enable, type);

	if (enable == TRUE)
	{
		ad_register(connection, ad_proxy, type);
	}
		
	return;
}

static void generic_callback(const DBusError *error, void *user_data)
{
	char *str = user_data;

	if (dbus_error_is_set(error))
		serverLog(LL_NOTICE, "Failed to set %s: %s\n", str, error->name);
	else
		serverLog(LL_NOTICE, "Changing %s succeeded\n", str);
}

// discoverble
void cmd_discoverable()
{
	dbus_bool_t discoverable = true;
	char *str = NULL;

	// 很有可能内存泄漏
	str = g_strdup_printf("discoverable %s",
				discoverable == TRUE ? "on" : "off");

	if (g_dbus_proxy_set_property_basic(adapter_proxy, "Discoverable",
					DBUS_TYPE_BOOLEAN, &discoverable,
					generic_callback, str, g_free) == TRUE)
		return;
	
	g_free(str);
}

int main(int argc, char *argv[])
{
	GDBusClient *client;
	guint signal;

	signal = setup_signalfd();
	if (signal == 0)
		return -errno;

	// // 其实只是对 dbus 消息的设置
	// // 还每有看到对 dbus 的订阅
	connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);


	main_loop = g_main_loop_new(NULL, FALSE);

	// 附加上  data 到 当前的connection 
	g_dbus_attach_object_manager(connection);

	// 获取当前的 service 的名字
	printf("gatt-service unique name: %s\n",
				dbus_bus_get_unique_name(connection));

	create_services();

	client = g_dbus_client_new(connection, "org.bluez", "/org/bluez");

	g_dbus_client_set_proxy_handlers(client, proxy_added_cb, NULL, NULL,
									NULL);
	createServerThread();
	g_main_loop_run(main_loop);

	g_dbus_client_unref(client);

	g_source_remove(signal);

	g_slist_free_full(services, g_free);
	dbus_connection_unref(connection);

	return 0;
}
