#ifndef _CRYPT_H_
#define _CRYPT_H_
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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

int initCrypt(Crypt *obj);
int getCrypt(Crypt **pobj);
int setCryptAdminKey(Crypt *data, uint8_t *admin_key, size_t admin_key_len);
int setCryptClientNonce(Crypt *data, uint8_t *client_nonce, size_t client_nonce_len);
int setCryptServerNonce(Crypt *data, uint8_t *server_nonce, size_t server_nonce_len);
int isCryptInvalid(Crypt * obj);
int encodeCrypt(Crypt * obj, uint8_t *retval, size_t length,size_t *written_length);
int decodeCrypt(uint8_t *buf,size_t buf_size, Crypt *retval, size_t index);
#endif