#include <stdio.h>

#include "bridge/gattlib/gattlib.h"
#include "bridge/wifi/SeWifiInfoRequest.h"
#include "bridge/bridge_main/log.h"
#include "bridge/ble/lock.h"
#include "bridge/ble/ble_operation.h"
static char wifi_inf_str[] = "12345678-0000-1000-8000-00805f9b34fb";

int main(int argc, char *argv[])
{

  if (argc != 4) {
    serverLog(LL_NOTICE, "%s <device_address> <ssid> <password>\n", argv[0]);
    return 1;
  }


  int ret;
  gatt_connection_t* gatt_connection;
  igm_lock_t *lock = NULL;
  uuid_t wifi_inf_uuid;
  uint8_t *payloadBytes = NULL;
  size_t payload_len = 0;
  SetWIFIInfoRequest wifi_info;

  getLock(&lock);
  initLock(lock);
  setLockAddr(lock, argv[1], strlen(argv[1]));
  
  gatt_connection = gattlib_connect(
    NULL, lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);

  if (gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		return 1;
	}
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );

  if (
    gattlib_string_to_uuid(
      wifi_inf_str, strlen(wifi_inf_str), &(wifi_inf_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );

  setWifiInfoRequestSSID(&wifi_info, argv[2], strlen(argv[2]));
  serverLog(LL_NOTICE, "setWifiInfoRequestSSID success." );
  setWifiInfoRequestPassword(&wifi_info, argv[3], strlen(argv[3]));
  serverLog(LL_NOTICE, "setWifiInfoRequestPassword success." );

  payloadBytes = (uint8_t *)&wifi_info;
  payload_len = sizeof(wifi_info);

  ret = write_char_by_uuid_multi_atts(
    gatt_connection, &wifi_inf_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
	  return 1;
	}
  serverLog(LL_ERROR, "write_char_by_uuid_multi_atts success");

  deinitWifiInfoRequest(&wifi_info);
  releaseLock(&lock);
  return 0;
}