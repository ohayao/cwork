#include "bridge/bridge_main/log.h"
#include "bridge/wifi_service/SetWifiInfoRequest.h"
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/gattlib/gattlib.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/wifi_service/pairing.h"
#include "bridge/lock/cifra/drbg.h"
#include "bridge/lock/cifra/sha1.h"
#include "bridge/lock/micro-ecc/uECC.h"
#include "bridge/wifi_service/crypt.h"
#include "bridge/lock/connection/encryption.h"

#include <glib.h>
#include <ctype.h>
static char wifi_setting_str[] = "87654321-0000-1000-8000-00805f9b34fb";
uuid_t wifi_setting_uuid = {};

gatt_connection_t* gatt_connection = NULL;
char *bridge_addr = "DC:A6:32:10:C7:DC";
char *SSID = "Google";
char *SSID_PASSWD = "12345678";
char *SSID_TOKEN = "12345678";
bool is_registered = false;

uint8_t admin_key[16+1];
uint8_t client_nonce[12+1];
uint8_t server_nonce[12+1];

GMainLoop *loop = NULL;

SET_WIFI_STATUS status;

int write_wifi_request(){
  serverLog(LL_NOTICE, "--------- write_wifi_request --------");
  int ret;

  SetWIFIInfoRequest request;
  initWifiInfoRequest(&request);

  if (setWifiInfoRequestSSID(&request, SSID, sizeof(SSID)))
  {
    serverLog(LL_ERROR, "setWifiInfoRequestSSID error");
    return 1;
  }
  serverLog(LL_NOTICE, " setWifiInfoRequestSSID success");

  if (setWifiInfoRequestPassword(&request, SSID_PASSWD, sizeof(SSID_PASSWD)))
  {
    serverLog(LL_ERROR, "setWifiInfoRequestPassword error");
    return 1;
  }
  serverLog(LL_NOTICE, " setWifiInfoRequestPassword success");

  if (setWifiInfoRequestToken(&request, SSID_TOKEN, sizeof(SSID_TOKEN)))
  {
    serverLog(LL_ERROR, "setWifiInfoRequestToken error");
    return 1;
  }
  serverLog(LL_NOTICE, " setWifiInfoRequestToken success");

  
  if (request.has_ssid)
  {
    serverLog(LL_NOTICE, "request ssid:");
    for (int i = 0; i < request.ssid_len; ++i)
    {
      printf("%c", request.ssid[i]);
    }
    printf("\n");
  }

  if (request.has_password)
  {
    serverLog(LL_NOTICE, "request password:");
    for (int i = 0; i < request.password_len; ++i)
    {
      printf("%c", request.password[i]);
    }
    printf("\n");
  }
  
  if (request.has_token)
  {
    serverLog(LL_NOTICE, "request token:");
    for (int i = 0; i < request.token_len; ++i)
    {
      printf("%c", request.token[i]);
    }
    printf("\n");
  }

  size_t len = 100;
  uint8_t encoded_request[len];
  size_t encoded_write_len = 0;
  if (encodeWifiInfoRequest(&request, encoded_request, len, &encoded_write_len))
  {
    serverLog(LL_ERROR, "encodeWifiInfoRequest error");
    return 1;
  }
  serverLog(LL_NOTICE, " encodeWifiInfoRequest success, size: %lu", encoded_write_len);

  // test decode
  uint8_t deencoded_request[len];
  size_t deencoded_write_len = 0;
  SetWIFIInfoRequest decode_request;
  if (decodeWifiInfoRequest(encoded_request, encoded_write_len, &decode_request, 0))
  {
    serverLog(LL_ERROR, "decodeWifiInfoRequest error");
    return 1;
  }
  else
  {
    serverLog(LL_NOTICE, " decodeWifiInfoRequest success");
    if (decode_request.has_ssid)
    {
      serverLog(LL_NOTICE, "request ssid:");
      for (int i = 0; i < decode_request.ssid_len; ++i)
      {
        printf("%c", decode_request.ssid[i]);
      }
      printf("\n");
    }

    if (decode_request.has_password)
    {
      serverLog(LL_NOTICE, "request password:");
      for (int i = 0; i < decode_request.password_len; ++i)
      {
        printf("%c", decode_request.password[i]);
      }
      printf("\n");
    }
    
    if (decode_request.has_token)
    {
      serverLog(LL_NOTICE, "request token:");
      for (int i = 0; i < decode_request.token_len; ++i)
      {
        printf("%c", decode_request.token[i]);
      }
      printf("\n");
    }
  }

  int encrypted_bytes_written_len = 0;
  size_t max_encrypt_len = 200;
  uint8_t encrypt_bytes[max_encrypt_len];

  encrypted_bytes_written_len = encryptData(
    encoded_request, encoded_write_len, encrypt_bytes, max_encrypt_len,
    admin_key, 16, client_nonce, 12);
  if (encrypted_bytes_written_len <= 0)
  {
    serverLog(LL_ERROR, "client encryptDataSize error");
    return 1;
  }

  serverLog(LL_NOTICE, "encryptData success %lu", encrypted_bytes_written_len);
  for (int i = 0; i < encrypted_bytes_written_len; ++i)
  {
    printf(" %x", encrypt_bytes[i]);
  }
  printf("\n");

  size_t payload_len = 0;
  uint8_t *payloadBytes = NULL;

  if (!build_msg_payload(&payloadBytes, &payload_len, encrypt_bytes, encrypted_bytes_written_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		return 1;
	}
  serverLog(LL_NOTICE, "success in build_msg_payload");

  ret = write_char_by_uuid_multi_atts(
    gatt_connection, &wifi_setting_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		return 1;
	}
  serverLog(LL_NOTICE, 
    "write_char_by_uuid_multi_atts success in writing packages, len %u ", payload_len);
  return 0;
}

static void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {

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
      wifi_setting_str, strlen(wifi_setting_str), &(wifi_setting_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );

  // 注册, 并且 start 了
  gattlib_register_notification(
    gatt_connection, message_handler, NULL);
  ret = gattlib_notification_start(
      gatt_connection, &wifi_setting_uuid);
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
    ret = gattlib_notification_stop(gatt_connection, &wifi_setting_uuid);
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

int hexStrToByte(const char* source, uint8_t* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
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

int main(int argc, char *argv[])
{
  if (argc != 4) {
    serverLog(LL_NOTICE, "%s <MAC> <client_nonce> <server_nonce> <admin_key>\n", argv[0]);
    return 1;
  }

  uint8_t tmp_buff[100];
  memset(tmp_buff, 0, sizeof(tmp_buff));
  int client_nonce_len = hexStrToByte(argv[1], tmp_buff, strlen(argv[1]));
  memcpy(client_nonce, tmp_buff, client_nonce_len);
  serverLog(LL_NOTICE, "client_nonce: ");
  for (int i = 0; i < client_nonce_len; ++i)
  {
    printf(" %x", tmp_buff[i]);
  }
  printf("\n");


  memset(tmp_buff, 0, sizeof(tmp_buff));
  int server_nonce_len = hexStrToByte(argv[2], tmp_buff, strlen(argv[2]));
  memcpy(server_nonce, tmp_buff, server_nonce_len);
  serverLog(LL_NOTICE, "server_nonce: ");
  for (int i = 0; i < server_nonce_len; ++i)
  {
    printf(" %x", tmp_buff[i]);
  }
  printf("\n");

  memset(tmp_buff, 0, sizeof(tmp_buff));
  int admin_key_len = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
  printf("%d \n", admin_key_len);
  memcpy(admin_key, tmp_buff, admin_key_len);
  serverLog(LL_NOTICE, "admin_key: ");
  for (int i = 0; i < admin_key_len; ++i)
  {
    printf(" %x", tmp_buff[i]);
  }
  printf("\n");

  // 先注册
  if (register_pairing_notfication(NULL))
  {
    serverLog(LL_ERROR, "register_pairing_notfication error");
  }

  // 写第一步阿
  if(write_wifi_request())
  {
    serverLog(LL_ERROR, "write_wifi_request error");
  }
  // waiting_pairing_step2();
  endGattConnection();
}