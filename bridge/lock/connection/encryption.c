
#include <bridge/lock/connection/encryption.h>
#include <bridge/lock/connection/encryption.h>
#include <string.h>

#include <bridge/lock/cifra/modes.h>
#include <bridge/lock/cifra/aes.h>
#include <bridge/bridge_main/log.h>

static const char *kTag = "JNI encryption";

enum {
    kCcmInternalCounterLength = 3,
    kCcmTagLength = 8
};

void incrementNonce(uint8_t *nonce) {
    // nonce interpreted as big endian (most significant byte has lowest address/index)
    for (int i = kNonceLength-1; i >= 0; i--) {
        if (nonce[i] == 255) {
            nonce[i] = 0;
        }
        else {
            nonce[i]++;
            break;
        }
    }
}

uint32_t encryptDataSize(uint32_t dataLen) {
    // TODO: remove non-encrypted version
//    return dataLen;

    return dataLen + kCcmTagLength;
}

// encrypt data
int32_t encryptData(const uint8_t *dataIn, uint32_t dataInLen,
                     uint8_t *dataOut, uint32_t dataOutLen,
                     const uint8_t *key, uint32_t keyLen,
                     uint8_t *nonce, uint32_t nonceLen) {
    if (dataInLen > 4096)
        return -1;
    if (dataOutLen < encryptDataSize(dataInLen))
        return -1;
    if (keyLen != 16 && keyLen != 24 && keyLen != 32)
        return -1;
    if (nonceLen != kNonceLength)
        return -1;

    cf_aes_context aes_ctx;
    cf_aes_init(&aes_ctx, key, (size_t)keyLen);

    uint8_t tag[kCcmTagLength];
    uint8_t ciphertext[dataInLen];
    cf_ccm_encrypt(&cf_aes, &aes_ctx, dataIn, dataInLen, kCcmInternalCounterLength, NULL, 0, nonce, kNonceLength, ciphertext, tag, kCcmTagLength);
    memcpy(dataOut, ciphertext, dataInLen);
    memcpy(dataOut+dataInLen, tag, kCcmTagLength);
    uint32_t bytesWritten = encryptDataSize(dataInLen);
    cf_aes_finish(&aes_ctx);

    // TODO: remove non-encrypted version
//    memcpy(dataOut, dataIn, dataInLen);
//    bytesWritten = dataInLen;

    return bytesWritten;
}

uint32_t decryptDataSize(uint32_t dataLen) {
    // TODO: remove non-encrypted version
//    return dataLen;

    return dataLen - kCcmTagLength;
}

// decrypt data
int32_t decryptData(uint8_t *dataIn, uint32_t dataInLen, uint8_t *dataOut,
                     uint32_t dataOutLen, const uint8_t *key, uint32_t keyLen,
                     uint8_t *nonce, uint32_t nonceLen) {
    if (dataInLen > 4096)
    {
        serverLog(LL_ERROR, "decryptData dataInLen > 4096");
        return -1;
    }
        
    if (dataOutLen < decryptDataSize(dataInLen))
    {
        serverLog(LL_ERROR, "decryptData dataOutLen < decryptDataSize(dataInLen)");
        return -1;
    }
    if (keyLen != 16 && keyLen != 24 && keyLen != 32)
    {
        serverLog(LL_ERROR, "decryptData keyLen != 16 && keyLen != 24 && keyLen != 32");
        return -1;
    }
    if (nonceLen != kNonceLength)
    {
        serverLog(LL_ERROR, "decryptData nonceLen != kNonceLength");
        return -1;
    }
        

    cf_aes_context aes_ctx;
    cf_aes_init(&aes_ctx, key, keyLen);

    uint8_t *ciphertext = dataIn;
    size_t ciphertextLen = decryptDataSize(dataInLen);
    uint8_t *tag = dataIn + ciphertextLen;
    size_t tagLen = kCcmTagLength;
    serverLog(LL_NOTICE, "decryptData result %x", ciphertext);
    int result = cf_ccm_decrypt(&cf_aes, &aes_ctx, ciphertext, ciphertextLen, kCcmInternalCounterLength, NULL, 0, nonce, kNonceLength, tag, tagLen, dataOut);
    serverLog(LL_NOTICE, "decryptData result %d", result);
//    Log_d(kTag, "decrypt result = %i", result);
    uint32_t bytesWritten = decryptDataSize(dataInLen);
    cf_aes_finish(&aes_ctx);
    if (result) {
        return -1;
    }

    // TODO: remove non-encrypted version
//    memcpy(dataOut, dataIn, dataInLen);
//    bytesWritten = dataInLen;


    return bytesWritten;
}

// JNIEXPORT jbyteArray JNICALL
// Java_co_igloohome_ble_lock_crypto_Encryption_encrypt(JNIEnv *env, jobject pThis, jbyteArray jPlaintext, jbyteArray jKey, jbyteArray jNonce) {
//     jsize plaintextLen = env->GetArrayLength(jPlaintext);
//     uint8_t *plaintext = (uint8_t*)(env->GetByteArrayElements(jPlaintext, NULL));
//     jsize keyLen = env->GetArrayLength(jKey);
//     uint8_t *key = (uint8_t*)(env->GetByteArrayElements(jKey, NULL));
//     jsize nonceLen = env->GetArrayLength(jNonce);
//     uint8_t *nonce = (uint8_t*)(env->GetByteArrayElements(jNonce, NULL));
//     auto freeJArrays = [&]() {
//         env->ReleaseByteArrayElements(jPlaintext, (jbyte*)plaintext, 0);
//         env->ReleaseByteArrayElements(jKey, (jbyte*)key, 0);
//         env->ReleaseByteArrayElements(jNonce, (jbyte*)nonce, 0);
//     };

//     jsize ciphertextLen = encryptDataSize(plaintextLen);
//     uint8_t ciphertext[ciphertextLen];
//     uint32_t retvalLen = encryptData(plaintext, plaintextLen,
//                                      ciphertext, ciphertextLen,
//                                      key, keyLen, nonce, nonceLen);
//     incrementNonce(nonce);
//     if (retvalLen == 0) {
//         freeJArrays();
//         return NULL;
//     }
//     if (retvalLen != ciphertextLen) {
//         freeJArrays();
//         return NULL;
//     }
//     jbyteArray retval = env->NewByteArray(retvalLen);
//     env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)ciphertext);

//     freeJArrays();
//     return retval;
// }

// JNIEXPORT jbyteArray JNICALL
// Java_co_igloohome_ble_lock_crypto_Encryption_decrypt(JNIEnv *env, jobject pThis, jbyteArray jMessage, jbyteArray jKey, jbyteArray jNonce) {
//     jsize messageLen = env->GetArrayLength(jMessage);
//     uint8_t *messageBytes = (uint8_t*)(env->GetByteArrayElements(jMessage, NULL));
//     jsize keyLen = env->GetArrayLength(jKey);
//     uint8_t *key = (uint8_t*)(env->GetByteArrayElements(jKey, NULL));
//     jsize nonceLen = env->GetArrayLength(jNonce);
//     uint8_t *nonce = (uint8_t*)(env->GetByteArrayElements(jNonce, NULL));
//     auto freeJArrays = [&]() {
//         env->ReleaseByteArrayElements(jMessage, (jbyte*)messageBytes, 0);
//         env->ReleaseByteArrayElements(jKey, (jbyte*)key, 0);
//         env->ReleaseByteArrayElements(jNonce, (jbyte*)nonce, 0);
//     };

//     jsize plaintextLen = decryptDataSize(messageLen);
//     uint8_t plaintext[plaintextLen];
//     uint32_t retvalLen = decryptData(messageBytes, messageLen,
//                                      plaintext, plaintextLen,
//                                      key, keyLen, nonce, nonceLen);
//     incrementNonce(nonce);
//     if (retvalLen == 0) {
//         freeJArrays();
//         return NULL;
//     }
//     if (retvalLen != plaintextLen) {
//         freeJArrays();
//         return NULL;
//     }
//     jbyteArray retval = env->NewByteArray(retvalLen);
//     env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)plaintext);

//     freeJArrays();
//     return retval;
// }

#ifdef __cplusplus
}
#endif

