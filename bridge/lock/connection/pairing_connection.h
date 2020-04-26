#ifndef PAIRING_CONNECTION_H
#define PAIRING_CONNECTION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

bool igloohome_ble_lock_crypto_PairingConnection_beginConnection();
int igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(uint8_t **ret);
int igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
    int step2Len, uint8_t jPairingStep2[], uint8_t **ret);
int igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
    int messageLen, uint8_t jPairingStep4[], uint8_t **ret);
int igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(
    int currentTime, uint8_t **ret);
int igloohome_ble_lock_crypto_PairingConnection_getAdminKeyNative(
    int currentTime, uint8_t **ret);
#ifdef __cplusplus
}
#endif



#endif