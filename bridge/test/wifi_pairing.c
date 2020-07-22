#include "bridge/bridge_main/log.h"
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/gattlib/gattlib.h"

static char wifi_pairing_str[] = "12345678-0000-1000-8000-00805f9b34fb";
uuid_t wifi_pairing_uuid = {};
gatt_connection_t* gatt_connection = NULL;
char *bridge_addr = "DC:A6:32:10:C7:DC";

static void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
  serverLog(LL_NOTICE, "----------- message_handler ---------------");
}

int register_pairing_notfication()
{
  serverLog(LL_NOTICE, "-------- register_pairing_notfication start --------");
  int ret;
  gatt_connection = gattlib_connect(
    NULL, bridge_addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		return 1;
	} 
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );

  if (
    gattlib_string_to_uuid(
      wifi_pairing_str, strlen(wifi_pairing_str), &(wifi_pairing_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );

  // 注册, 并且 start 了
  gattlib_register_notification(
    gatt_connection, message_handler, NULL);
  ret = gattlib_notification_start(
      gatt_connection, &wifi_pairing_uuid);
  if (ret) {
    serverLog(LL_ERROR, "Fail to start notification.");
		return 1;
	}
  serverLog(LL_NOTICE, "success to start notification" );

} 

void endConnection()
{
  int ret= 0;
  if(gatt_connection)
  {
    serverLog(LL_NOTICE, "endConnection gattlib_notification_stop");
    ret = gattlib_notification_stop(gatt_connection, &wifi_pairing_uuid);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_notification_stop error");
      return;
    }
    serverLog(LL_NOTICE, "endConnection gattlib_disconnect");
    ret = gattlib_disconnect(gatt_connection);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_disconnect error");
      return;
    }
    gatt_connection = NULL;
  }
  

  ret = gattlib_disconnect(gatt_connection);
  if (ret != GATTLIB_SUCCESS)
  {
    serverLog(LL_ERROR, "end_admin_gatt gattlib_disconnect error");
    return;
  }
  
}

// 对方一定要advertise, 否则不会收得到得.

int main(int argc, char *argv[])
{
  if (!register_pairing_notfication(NULL)){
    endConnection();
  }
  
}