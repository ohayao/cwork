#include "bridge/wifi_service/crypt.h"
#include <stdio.h>
#include <stdlib.h>

int setCrypt(Crypt *data, char *admin_key, size_t admin_key_len)
{
  if (!data) return 1;
  data->server_pairing_admin_key = (uint8_t *)malloc(admin_key_len);
  memset(data->server_pairing_admin_key,0, admin_key_len);
  memcpy(data->server_pairing_admin_key, admin_key, admin_key_len);
  data->has_server_pairing_admin_key = true;
  data->server_pairing_admin_key_len = admin_key_len;
  return 0;
}