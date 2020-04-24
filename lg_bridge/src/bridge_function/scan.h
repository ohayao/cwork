
#include <gattlib/include/gattlib.h>
#include <lock/connection/pairing_connection.h>
#include <iostream>

#include <assert.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <regex>

#define DEFAULT_BLE_SCAN_TIMEOUT 5

// 08:00:20:0A:8C:6D
#define DEVICE_ADDR_LEN (18)
// IGM401671ba7
#define DEVICE_NAME_LEN (12)

typedef void (*ble_discovered_device_t)(const char* addr, const char* name);

// uuids
uuid_t pairing_uuid;
char pairing_str[] = "5c3a659e-897e-45e1-b016-007107c96df6";

int start_scan(void* adapter, const int &scan_timeout);
int stop_scan(void* adapter);