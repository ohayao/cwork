/*
 *
 *  D-Bus helper library
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus/gdbus.h"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt...)

struct timeout_handler {
	guint id;
	DBusTimeout *timeout;
};

struct watch_info {
	guint id;
	DBusWatch *watch;
	DBusConnection *conn;
};

struct disconnect_data {
	GDBusWatchFunction function;
	void *user_data;
};

static gboolean disconnected_signal(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	struct disconnect_data *dc_data = data;

	error("Got disconnected from the system message bus");

	dc_data->function(conn, dc_data->user_data);

	dbus_connection_unref(conn);

	return TRUE;
}

static gboolean message_dispatch(void *data)
{
	DBusConnection *conn = data;

	/* Dispatch messages */
	// 不断分发数据
	while (dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS);

	dbus_connection_unref(conn);

	return FALSE;
}

// 
static inline void queue_dispatch(DBusConnection *conn,
						DBusDispatchStatus status)
{
	// 状态是仍然有数据
	if (status == DBUS_DISPATCH_DATA_REMAINS)
		g_idle_add(message_dispatch, dbus_connection_ref(conn));
}

static gboolean watch_func(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	struct watch_info *info = data;
	unsigned int flags = 0;
	DBusDispatchStatus status;
	DBusConnection *conn;

	if (cond & G_IO_IN)  flags |= DBUS_WATCH_READABLE;
	if (cond & G_IO_OUT) flags |= DBUS_WATCH_WRITABLE;
	if (cond & G_IO_HUP) flags |= DBUS_WATCH_HANGUP;
	if (cond & G_IO_ERR) flags |= DBUS_WATCH_ERROR;

	/* Protect connection from being destroyed by dbus_watch_handle */
	conn = dbus_connection_ref(info->conn);

	dbus_watch_handle(info->watch, flags);

	status = dbus_connection_get_dispatch_status(conn);
	queue_dispatch(conn, status);

	dbus_connection_unref(conn);

	return TRUE;
}

static void watch_info_free(void *data)
{
	struct watch_info *info = data;

	if (info->id > 0) {
		g_source_remove(info->id);
		info->id = 0;
	}

	dbus_connection_unref(info->conn);

	g_free(info);
}

// 添加一个watch, 所需要左的事情是
static dbus_bool_t add_watch(DBusWatch *watch, void *data)
{
	DBusConnection *conn = data;
	GIOCondition cond = G_IO_HUP | G_IO_ERR;
	GIOChannel *chan;
	struct watch_info *info;
	unsigned int flags;
	int fd;

	// 如果这个watch 被disable, 就会进入这个if
	if (!dbus_watch_get_enabled(watch))
		return TRUE;

	// 建立一个 watch_info
	info = g_new0(struct watch_info, 1);

	fd = dbus_watch_get_unix_fd(watch);
	chan = g_io_channel_unix_new(fd);

	info->watch = watch;
	info->conn = dbus_connection_ref(conn);

	// 设置 info 到 watch里面, 这样可以通过 dbus_watch_get_data(). 取的
	dbus_watch_set_data(watch, info, watch_info_free);

	// 获取标志, dbus_watch_get_flags
	// 应该在文件描述符上监视什么条件。
	flags = dbus_watch_get_flags(watch);

	if (flags & DBUS_WATCH_READABLE) cond |= G_IO_IN;
	if (flags & DBUS_WATCH_WRITABLE) cond |= G_IO_OUT;

	// g_io_add_watch
	// Adds the GIOChannel into the default main loop context with the default priority.
	info->id = g_io_add_watch(chan, cond, watch_func, info);

	g_io_channel_unref(chan);

	return TRUE;
}
// 取消所有的watch 的数据
static void remove_watch(DBusWatch *watch, void *data)
{
	if (dbus_watch_get_enabled(watch))
		return;

	/* will trigger watch_info_free() */
	dbus_watch_set_data(watch, NULL, NULL);
}

static void watch_toggled(DBusWatch *watch, void *data)
{
	/* Because we just exit on OOM, enable/disable is
	 * no different from add/remove */
	// 如果 watch 是被 enable , 那么就会 add_watch
	if (dbus_watch_get_enabled(watch))
		add_watch(watch, data);
	else
		remove_watch(watch, data);
}

static gboolean timeout_handler_dispatch(gpointer data)
{
	struct timeout_handler *handler = data;

	handler->id = 0;

	/* if not enabled should not be polled by the main loop */
	if (!dbus_timeout_get_enabled(handler->timeout))
		return FALSE;

	dbus_timeout_handle(handler->timeout);

	return FALSE;
}

static void timeout_handler_free(void *data)
{
	struct timeout_handler *handler = data;

	if (handler->id > 0) {
		g_source_remove(handler->id);
		handler->id = 0;
	}

	g_free(handler);
}

static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data)
{
	int interval = dbus_timeout_get_interval(timeout);
	struct timeout_handler *handler;

	if (!dbus_timeout_get_enabled(timeout))
		return TRUE;

	handler = g_new0(struct timeout_handler, 1);

	handler->timeout = timeout;

	dbus_timeout_set_data(timeout, handler, timeout_handler_free);

	handler->id = g_timeout_add(interval, timeout_handler_dispatch,
								handler);

	return TRUE;
}

static void remove_timeout(DBusTimeout *timeout, void *data)
{
	/* will trigger timeout_handler_free() */
	dbus_timeout_set_data(timeout, NULL, NULL);
}

static void timeout_toggled(DBusTimeout *timeout, void *data)
{
	if (dbus_timeout_get_enabled(timeout))
		add_timeout(timeout, data);
	else
		remove_timeout(timeout, data);
}

static void dispatch_status(DBusConnection *conn,
					DBusDispatchStatus status, void *data)
{
	// 如果没有连接, 就返回
	if (!dbus_connection_get_is_connected(conn))
		return;

	// 
	queue_dispatch(conn, status);
}

// setup_dbus_with_main_loop 的主要作用

static inline void setup_dbus_with_main_loop(DBusConnection *conn)
{
	// 添加三个函数到dbus 里面
	// 这些函数负责使应用程序的主循环意识到需要使用select（）或poll（）监视事件的文件描述符。
	// add_watch: Called when libdbus needs a new watch to be monitored by the main loop.
	// remove_watch: Called when libdbus no longer needs a watch to be monitored by the main loop.
	// watch_toggled:: Called when dbus_watch_get_enabled() may return a different value than it did before.
	dbus_connection_set_watch_functions(conn, add_watch, remove_watch,
						watch_toggled, conn, NULL);

	// 如果添加 time out 的三个函数, 和上面类似
	dbus_connection_set_timeout_functions(conn, add_timeout, remove_timeout,
						timeout_toggled, NULL, NULL);

	// dispatch status 改变的时候, 会需要调用下面的函数
	dbus_connection_set_dispatch_status_function(conn, dispatch_status,
								NULL, NULL);
}

// setup_bus 的主要作用, conn 是已经建立的连接, name 是 NULL, err是错误的返回
// 1. 判断是否获取名字
static gboolean setup_bus(DBusConnection *conn, const char *name,
						DBusError *error)
{
	gboolean result;
	// Indicates the status of incoming data on a DBusConnection.
	// This determines whether dbus_connection_dispatch() needs to be called.
	DBusDispatchStatus status;

	// 如果名字不为 NULL, 那么, 就获取这个名字返回
	if (name != NULL) {
		result = g_dbus_request_name(conn, name, error);

		if (error != NULL) {
			if (dbus_error_is_set(error) == TRUE)
				return FALSE;
		}

		if (result == FALSE)
			return FALSE;
	}

	// 设置一些回调函数
	setup_dbus_with_main_loop(conn);

	// 获取当前的状态
	status = dbus_connection_get_dispatch_status(conn);
	// 调用一次分配
	queue_dispatch(conn, status);

	return TRUE;
}

// g_dbus_setup_bus的主要作用:
// 	dbus_bus_get: 获得一个连接
// 	判断是否有错误
// 	setup_bus: 连接

DBusConnection *g_dbus_setup_bus(DBusBusType type, const char *name,
							DBusError *error)
{
	DBusConnection *conn;
	// dbus_bus_get 是库函数
	conn = dbus_bus_get(type, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return NULL;
	}

	if (conn == NULL)
		return NULL;

	// 设置 dbus 连接
	if (setup_bus(conn, name, error) == FALSE) {
		dbus_connection_unref(conn);
		return NULL;
	}

	return conn;
}

DBusConnection *g_dbus_setup_private(DBusBusType type, const char *name,
							DBusError *error)
{
	DBusConnection *conn;

	conn = dbus_bus_get_private(type, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return NULL;
	}

	if (conn == NULL)
		return NULL;

	if (setup_bus(conn, name, error) == FALSE) {
		dbus_connection_close(conn);
		dbus_connection_unref(conn);
		return NULL;
	}

	return conn;
}

// g_dbus_request_name 的作用
// 通过 dbus_bus_request_name 请求名字, 然后返回
gboolean g_dbus_request_name(DBusConnection *connection, const char *name,
							DBusError *error)
{
	int result;

	result = dbus_bus_request_name(connection, name,
					DBUS_NAME_FLAG_DO_NOT_QUEUE, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return FALSE;
	}

	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		if (error != NULL)
			dbus_set_error(error, name, "Name already in use");

		return FALSE;
	}

	return TRUE;
}

gboolean g_dbus_set_disconnect_function(DBusConnection *connection,
				GDBusWatchFunction function,
				void *user_data, DBusFreeFunction destroy)
{
	struct disconnect_data *dc_data;

	dc_data = g_new0(struct disconnect_data, 1);

	dc_data->function = function;
	dc_data->user_data = user_data;

	dbus_connection_set_exit_on_disconnect(connection, FALSE);

	if (g_dbus_add_signal_watch(connection, NULL, NULL,
				DBUS_INTERFACE_LOCAL, "Disconnected",
				disconnected_signal, dc_data, g_free) == 0) {
		error("Failed to add watch for D-Bus Disconnected signal");
		g_free(dc_data);
		return FALSE;
	}

	return TRUE;
}
