#ifndef _AD_H_
#define _AD_H_
#include <stdbool.h>
#include <stdint.h>
#include "bridge/wifi_service/gdbus/gdbus.h"


void ad_register(DBusConnection *conn, GDBusProxy *manager, const char *type);
void ad_advertise_uuids(DBusConnection *conn, char *argv[]);
void ad_advertise_tx_power(DBusConnection *conn, bool value);
void ad_advertise_name(DBusConnection *conn, bool value);
void ad_advertise_timeout(DBusConnection *conn, uint16_t value);
void ad_advertise_duration(DBusConnection *conn, uint16_t value);
void ad_advertise_local_appearance(DBusConnection *conn, uint16_t value);
void ad_advertise_appearance(DBusConnection *conn, bool value);
#endif