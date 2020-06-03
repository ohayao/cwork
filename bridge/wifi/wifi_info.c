#include "wifi_info.h"
#include <string.h>

int getWifiInfo(igm_wifi_info_t **pp_wifi_info)
{
  if(!pp_wifi_info) return 1;
  if (*pp_wifi_info) free(*pp_wifi_info);
  *pp_wifi_info = NULL;
  *pp_wifi_info = malloc(sizeof(igm_wifi_info_t));
  return 0;
}

void initWifiInfo(igm_wifi_info_t * wifi_info)
{
  memset(wifi_info, 0, sizeof(igm_wifi_info_t));
}

void deinitWifiInfo(igm_wifi_info_t * wifi_info)
{
  
}

int setWifiInfoSSID(igm_wifi_info_t * wifi_info, char *ssid, size_t ssid_len)
{
  memset(wifi_info->ssid,0, sizeof(wifi_info->ssid));
  memcpy(wifi_info->ssid, ssid, ssid_len);
  return 0;
}

int setWifiInfoPassword(igm_wifi_info_t * wifi_info, char *password, size_t password_len)
{
  memset(wifi_info->password,0, sizeof(wifi_info->password));
  memcpy(wifi_info->password, password, password_len);
  return 0;
}