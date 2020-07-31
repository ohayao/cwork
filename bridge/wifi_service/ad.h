#ifndef _AD_H_
#define _AD_H_

#include "bridge/wifi_service/gdbus/gdbus.h"

void ad_register(DBusConnection *conn, GDBusProxy *manager, const char *type);

#endif