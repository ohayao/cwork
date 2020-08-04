#ifndef _CRYPT_H_
#define _CRYPT_H_
#include <stdint.h>
#include <string.h>

typedef struct
{
  bool has_server_pairing_admin_key;
  uint8_t *server_pairing_admin_key;
  size_t server_pairing_admin_key_len;
  bool has_client_nonce;
  uint8_t *client_nonce;
  size_t client_nonce_len;
  bool has_server_nonce;
  uint8_t *server_nonce;
  size_t server_nonce_len;
} Crypt;


#endif