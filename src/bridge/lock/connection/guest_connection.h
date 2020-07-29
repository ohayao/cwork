
#include <string.h>
#include <stdint.h>
#include "connection_common.h"

int igloohome_ble_lock_crypto_GuestConnection_beginConnection(uint8_t *jKey, int keyLen);
int igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native(int connectionId, uint8_t* guest_token, unsigned int guest_token_len, uint8_t **retBytes);
int igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native(int connectionId, uint8_t *step2Bytes, int step2Len, uint8_t **retBytes);
bool igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native(int connectionId, uint8_t *jConnectionStep3, int messageLen);
void igloohome_ble_lock_crypto_GuestConnection_endConnection(int connectionId);
int igloohome_ble_lock_crypto_GuestConnection_encryptNative(int connectionId, uint8_t *jPlaintext, int plaintextLen, uint8_t **retBytes);
int igloohome_ble_lock_crypto_GuestConnection_decryptNative(int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes);
