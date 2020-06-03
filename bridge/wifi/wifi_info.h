#ifndef _WIFI_INFO_H_
#define _WIFI_INFO_H_

#include <stdlib.h>
#include <stdbool.h>
#define SSID_MAX_NUM (32)
#define PASSWORD_MAX_NUM (63)
typedef struct WIFIInfo{
  bool has_ssid;
  char ssid[SSID_MAX_NUM + 1];
  bool has_password;
  char password[PASSWORD_MAX_NUM+1];
}igm_wifi_info_t;

int getWifiInfo(igm_wifi_info_t **pp_wifi_info);
void initWifiInfo(igm_wifi_info_t * wifi_info);
void deinitWifiInfo(igm_wifi_info_t * wifi_info);
int setWifiInfoSSID(igm_wifi_info_t * wifi_info, char *ssid, size_t ssid_len);
int setWifiInfoPassword(igm_wifi_info_t * wifi_info, char *password, size_t password_len);


#endif