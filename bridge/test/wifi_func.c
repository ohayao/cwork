#include <stdio.h>

#include "bridge/wifi/SeWifiInfoRequest.h"
#include "bridge/bridge_main/log.h"

int main(int argc, char *argv[])
{
  int err = 0;
  serverLog(LL_NOTICE, "create igm_wifi_info_t pointer");
  SetWIFIInfoRequest *wifi_info_request = NULL;

  serverLog(LL_NOTICE, "--------------------- use getWifiInfoRequest to create igm_wifi_info_t in heap ---------------------");
  if (getWifiInfoRequest(&wifi_info_request))
  {
    serverLog(LL_ERROR, "getWifiInfoRequest failed");
    return 1;
  }
  serverLog(LL_NOTICE, "getWifiInfoRequest success");

  serverLog(LL_NOTICE, "---------------------use initWifiInfoRequest to init wifi_info_request to zero, this func won't fail ---------------------");
  initWifiInfoRequest(wifi_info_request);

  serverLog(LL_NOTICE, "---------------------use setWifiInfoRequestSSID to set wifi ssid ---------------------");
  char ssid[] = "Google";
  setWifiInfoRequestSSID(wifi_info_request, ssid, strlen(ssid));
  if (!wifi_info_request->has_ssid)
  {
    serverLog(LL_ERROR, "setWifiInfoRequestSSID failed");
    return 1;
  }
  serverLog(LL_NOTICE, "wifi info has ssid [%d]", 
                                        wifi_info_request->has_ssid);
  serverLog(LL_NOTICE, "wifi info ssid [%s]", 
                                        wifi_info_request->ssid);
  serverLog(LL_NOTICE, "wifi info ssid len [%d]", 
                                        wifi_info_request->ssid_len);

  serverLog(LL_NOTICE, "---------------------use setWifiInfoRequestPassword to set wifi password---------------------");

  char password[] = "12345678";
  setWifiInfoRequestPassword(wifi_info_request, password, strlen(password));
  if (!wifi_info_request->has_password)
  {
    serverLog(LL_ERROR, "setWifiInfoRequestPassword failed");
    return 1;
  }
  serverLog(LL_NOTICE, "wifi info has password [%d]", 
    wifi_info_request->has_password);
  serverLog(LL_NOTICE, "wifi info password [%s]", 
                            wifi_info_request->password);
  serverLog(LL_NOTICE, "wifi info password lend [%d]", 
                            wifi_info_request->password_len);

  serverLog(LL_NOTICE, "--------------------- use encodeWifiInfoRequest to encode request ---------------------");
  err = 0;
  size_t cbor_buff_len = 200;
  uint8_t cbor_buff[cbor_buff_len];
  size_t encode_len = 0;
  err = encodeWifiInfoRequest(wifi_info_request, cbor_buff, cbor_buff_len, &encode_len);
  if (err )
  {
    serverLog(LL_ERROR, "encodeWifiInfoRequest failed, err %d", err);
    return 1;
  }
  serverLog(LL_NOTICE, "encodeWifiInfoRequest success! encode size %d", encode_len);
  for (int j = 0; j < encode_len; j++)
  {
    printf("%x ", cbor_buff[j]);
  }
  printf("\n");

  serverLog(LL_NOTICE, "--------------------- use decodeWifiInfo to decode cbor request, encode_len [%d] ---------------------", encode_len);
  err = 0;
  SetWIFIInfoRequest decode_wifi_info_request;
  initWifiInfoRequest(&decode_wifi_info_request);
  err = decodeWifiInfoRequest(cbor_buff, encode_len, &decode_wifi_info_request, 0);
  if (err)
  {
    serverLog(LL_ERROR, "decodeWifiInfoRequest failed, err %d", err);
    return 1;
  }
  serverLog(LL_NOTICE, "decodeWifiInfoRequest success!");
  
  if (decode_wifi_info_request.has_ssid)
  {
    serverLog(LL_NOTICE, "decode_wifi_info_request hass ssid");
    serverLog(LL_NOTICE, "decode_wifi_info_request ssid len [%lu]", decode_wifi_info_request.ssid_len);
    serverLog(LL_NOTICE, "decode_wifi_info_request ssid [%s]", decode_wifi_info_request.ssid);
  }
  else
  {
    serverLog(LL_NOTICE, "decode_wifi_info_request don't have ssid! err");
    return 1;
  }
  
  if (decode_wifi_info_request.has_password)
  {
    serverLog(LL_NOTICE, "decode_wifi_info_request password len [%lu]", decode_wifi_info_request.password_len);
    serverLog(LL_NOTICE, "decode_wifi_info_request password [%s]", decode_wifi_info_request.password);
  }
  else
  {
    serverLog(LL_NOTICE, "decode_wifi_info_request don't have password! err");
    return 1;
  }
  
  


  serverLog(LL_NOTICE, "--------------------- use deinitWifiInfo to deinit igm_wifi_info_t heap memory, won't fail ---------------------");
  deinitWifiInfoRequest(wifi_info_request);

  serverLog(LL_NOTICE, "--------------------- use free to release wifi_info in heap memory, won't fail ---------------------");
  free(wifi_info_request);

  serverLog(LL_NOTICE, "************************* wifi info test Done *************************");
}