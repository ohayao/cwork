#ifndef ANDROIDSDKTESTINGFORIGLOOCHIP_ENCRYPTION_H
#define ANDROIDSDKTESTINGFORIGLOOCHIP_ENCRYPTION_H

#include <stdint.h>

enum {
  kNonceLength = 12
};

void incrementNonce(uint8_t *nonce);
uint32_t encryptDataSize(uint32_t dataLen);
int32_t encryptData(const uint8_t *dataIn, uint32_t dataInLen,
                    uint8_t *dataOut, uint32_t dataOutLen,
                    const uint8_t *key, uint32_t keyLen,
                    uint8_t *nonce, uint32_t nonceLen);
uint32_t decryptDataSize(uint32_t dataLen);
int32_t decryptData(uint8_t *dataIn, uint32_t dataInLen,
                    uint8_t *dataOut, uint32_t dataOutLen,
                    const uint8_t *key, uint32_t keyLen,
                    uint8_t *nonce, uint32_t nonceLen);

#endif //ANDROIDSDKTESTINGFORIGLOOCHIP_ENCRYPTION_H