#include "bridge/bridge_main/log.h"
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/gattlib/gattlib.h"
#include "bridge/ble/ble_operation.h"

static char wifi_pairing_str[] = "12345678-0000-1000-8000-00805f9b34fb";
uuid_t wifi_pairing_uuid = {};
gatt_connection_t* gatt_connection = NULL;
char *bridge_addr = "DC:A6:32:10:C7:DC";
bool is_registered = false;
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
  is_registered = true;
  serverLog(LL_NOTICE, "success to start notification" );

} 

void endGattConnection()
{
  int ret= 0;
  if (is_registered && gatt_connection)
  {
    serverLog(LL_NOTICE, "endConnection gattlib_notification_stop");
    ret = gattlib_notification_stop(gatt_connection, &wifi_pairing_uuid);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_notification_stop error");
      return;
    }
    serverLog(LL_NOTICE, "endConnection gattlib_disconnect");
  }

  if(gatt_connection)
  {
    ret = gattlib_disconnect(gatt_connection);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_disconnect error");
      return;
    }
    gatt_connection = NULL;
  }

}

int write_pairing_step1()
{
  serverLog(LL_NOTICE, "write_pairing_step1 start --------");
  int ret;
  size_t step1Bytes_len, payload_len;
  uint8_t *step1Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  ret = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (!ret)
  {
    serverLog(LL_ERROR, "beginConnection fail");
    return 1;
  }
  serverLog(LL_NOTICE, "beginConnection success");

   step1Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			                                                            &step1Bytes);
  if (step1Bytes == NULL)
  {
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native fail");
    return 1;
  }
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native success");

  if (!build_msg_payload(&payloadBytes, &payload_len, step1Bytes, step1Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		return 1;
	}
  serverLog(LL_NOTICE, "success in build_msg_payload");

  ret = write_char_by_uuid_multi_atts(
    gatt_connection, &wifi_pairing_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		return 1;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success in writing packages");

  if (step1Bytes) 
  {
    free(step1Bytes);
    step1Bytes = NULL;
  }

  if (payloadBytes)
  {
    free(payloadBytes);
    payloadBytes =  NULL;
  }
  return 0;
}

// 对方一定要advertise, 否则不会收得到得.

int main(int argc, char *argv[])
{
  // 先注册
  if (register_pairing_notfication(NULL))
  {
    serverLog(LL_ERROR, "register_pairing_notfication error");

  }

  // 写第一步阿
  write_pairing_step1();

  endGattConnection();
}