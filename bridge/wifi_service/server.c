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

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus/gdbus.h"

#include "bridge/wifi_service/error.h"
#include "bridge/wifi_service/SetWifiInfoRequest.h"
#include "bridge/wifi_service/pairing.h"
#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"

#define GATT_MGR_IFACE			"org.bluez.GattManager1"
#define GATT_SERVICE_IFACE		"org.bluez.GattService1"
#define GATT_CHR_IFACE			"org.bluez.GattCharacteristic1"
#define GATT_DESCRIPTOR_IFACE		"org.bluez.GattDescriptor1"

/* Immediate Alert Service UUID */
// wifi service
#define WIFI_SERVICE_UUID	"12345678-0000-1000-8000-00805f9b34fb"
#define WIFI_SERVICE_CHR_UUID		"12345678-0000-1000-8000-00805f9b34fb"

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
};

struct descriptor {
	struct characteristic *chr;
	char *uuid;
	char *path;
	uint8_t *value;
	int vlen;
	const char **props;
};

/*
 * Alert Level support Write Without Response only. Supported
 * properties are defined at doc/gatt-api.txt. See "Flags"
 * property of the GattCharacteristic1.
 */
static const char *wifi_info_props[] = { "write-without-response", "notify", NULL };
static const char *desc_props[] = { "read", "write", NULL };

static FSM *fsm = NULL;

int getGlobalFSM()
{
	if(getFSM(&fsm))
  {
    serverLog(LL_ERROR, "getFSM err");
    return 1;
  }
	return 0;
}

// 写死的 fsm 过程, 所以不需要参数设置
// 然后, 需要适配以下数据的不同
// 在server 当中, 是什么数据呢
int initPairingFsm()
{
	uint8_t max_trans_num;
	max_trans_num = 7;
  if (getFSMTransTable(fsm, max_trans_num))
  {
    serverLog(LL_ERROR, "getFSMTransTable err");
    return 1;
  }
  serverLog(LL_NOTICE, "getFSMTransTable success");


}


// functions
static void chr_write(struct characteristic *chr, const uint8_t *value, int len);

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
	fprintf(stdout, "chr_get_value\n");
	// 写特征值的时候会调用
	struct characteristic *chr = user_data;

	printf("Characteristic(%s): Get(\"Value\")\n", chr->uuid);

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
	printf("chrc_get_notifying  ------------------ \n");
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
	fprintf(stdout, "chr_set_value\n");
	struct characteristic *chr = user_data;
	const uint8_t *value;
	int len;

	printf("Characteristic(%s): Set('Value', ...)\n", chr->uuid);

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
	printf("g_dbus_emit_property_changed\n");
	g_dbus_emit_property_changed(connection, chr->path, GATT_CHR_IFACE,
								"Value");
}

// 因为 process_message method->function的调用, 就是调的这个函数, 所以我直接在这里写状态机
// 这个主体, 先用于进行一个派对的实现.
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
	printf("value len %d----------\n", len);

	// 这里就是状态机开始?
	chr_write(chr, value, len);

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
						wifi_info_props,	// chr wifi 的属性?
						READ_WRITE_DESCRIPTOR_UUID,
						desc_props,				// chr wifi desc props
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

int main(int argc, char *argv[])
{
	GDBusClient *client;
	guint signal;

	signal = setup_signalfd();
	if (signal == 0)
		return -errno;

	// 其实只是对 dbus 消息的设置
	// 还每有看到对 dbus 的订阅
	connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);

	// 附加上  data 到 当前的connection 
	g_dbus_attach_object_manager(connection);

	// 获取当前的 service 的名字
	printf("gatt-service unique name: %s\n",
				dbus_bus_get_unique_name(connection));

	create_services();

	client = g_dbus_client_new(connection, "org.bluez", "/");

	g_dbus_client_set_proxy_handlers(client, proxy_added_cb, NULL, NULL,
									NULL);

	g_main_loop_run(main_loop);

	g_dbus_client_unref(client);

	g_source_remove(signal);

	g_slist_free_full(services, g_free);
	dbus_connection_unref(connection);

	return 0;
}
