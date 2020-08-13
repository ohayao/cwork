#ifndef _WIFI_INFO_H_
#define _WIFI_INFO_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define SSID_MAX_NUM (32)
#define PASSWORD_MAX_NUM (63)
typedef struct SetWIFIInfoRequest{
  bool has_ssid;
  char *ssid;
  size_t ssid_len;
  bool has_password;
  char *password;
  size_t password_len;
  bool has_token;
  char *token;
  size_t token_len;
}SetWIFIInfoRequest;

int getWifiInfoRequest(SetWIFIInfoRequest **pp_wifi_info);
void initWifiInfoRequest(SetWIFIInfoRequest * wifi_info);
void deinitWifiInfoRequest(SetWIFIInfoRequest * wifi_info);
int setWifiInfoRequestSSID(SetWIFIInfoRequest * wifi_info, char *ssid, size_t ssid_len);
int setWifiInfoRequestPassword(SetWIFIInfoRequest * wifi_info, char *password, size_t password_len);
int setWifiInfoRequestToken(SetWIFIInfoRequest * wifi_info, char *password, size_t password_len);
int encodeWifiInfoRequest(SetWIFIInfoRequest * obj, uint8_t *retval,size_t length,size_t *written_length);
int decodeWifiInfoRequest(uint8_t *buf,size_t buf_size, SetWIFIInfoRequest *retval,size_t index);
int isWifiInfoRequestValid(SetWIFIInfoRequest * obj);

#endif