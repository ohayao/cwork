#include "token.h"

#include "encryption.h"
#include "connection_common.h"
#include <jni.h>
#include <android/log.h>

static const char *kTag = "JNI_token";
static const int kTokenKeyLength = 32;


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_Token_createTokenNative(JNIEnv *env, jobject pThis, jbyteArray jKey, jbyteArray jPlaintext) {
    // arguments already validated at kotlin level
//    if (!jKey || !jPlaintext)
//        return NULL;

    // read java byte arrays
    jsize keyLen = env->GetArrayLength(jKey);
    jsize plaintextLen = env->GetArrayLength(jPlaintext);
    uint8_t keyBytes[keyLen];
    uint8_t plaintextBytes[plaintextLen];
    env->GetByteArrayRegion(jKey, 0, keyLen, (jbyte*)keyBytes);
    env->GetByteArrayRegion(jPlaintext, 0, plaintextLen, (jbyte*)plaintextBytes);

//    Log_d(kTag, "step2 bytes = ");
//    for (int j = 0; j < step2Len; j++) {
//        Log_d(kTag, "%02x", step2Bytes[j]);
//    }

    uint32_t messageSize = encryptDataSize(plaintextLen);
    uint8_t tokenBytes[messageSize + kNonceLength];

    // generate nonce
    generateRandomNonce(env, (jbyte*)tokenBytes, kNonceLength);

    // write token bytes
    int32_t encryptDataLen = encryptData(
            plaintextBytes, plaintextLen,
            tokenBytes + kNonceLength, messageSize,
            keyBytes, keyLen,
            tokenBytes, kNonceLength
    );
//    Log_d(kTag, "encryptDataLen = %i", encryptDataLen);
    if (encryptDataLen < 0) {
        return NULL;
    }

    // return java ByteArray
    jbyteArray retval = env->NewByteArray(sizeof(tokenBytes));
    env->SetByteArrayRegion(retval, 0, sizeof(tokenBytes), (jbyte*)tokenBytes);

    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_Token_decryptTokenNative(JNIEnv *env, jobject pThis, jbyteArray jKey, jbyteArray jToken) {
    // arguments already validated at kotlin level
//    if (!jKey || !jPlaintext)
//        return NULL;

    // read java byte arrays
    jsize keyLen = env->GetArrayLength(jKey);
    jsize tokenLen = env->GetArrayLength(jToken);
    uint8_t keyBytes[keyLen];
    uint8_t tokenBytes[tokenLen];
    env->GetByteArrayRegion(jKey, 0, keyLen, (jbyte*)keyBytes);
    env->GetByteArrayRegion(jToken, 0, tokenLen, (jbyte*)tokenBytes);

//    Log_d(kTag, "step2 bytes = ");
//    for (int j = 0; j < step2Len; j++) {
//        Log_d(kTag, "%02x", step2Bytes[j]);
//    }

    uint32_t messageSize = tokenLen - kNonceLength;
    uint32_t plaintextSize = decryptDataSize(messageSize);
    uint8_t retvalBytes[plaintextSize];

    // generate token bytes
    int32_t decryptDataLen = decryptData(
            tokenBytes + kNonceLength, messageSize,
            retvalBytes, plaintextSize,
            keyBytes, keyLen,
            tokenBytes, kNonceLength
    );
    if (decryptDataLen < 0) {
        return NULL;
    }

    // return java ByteArray
    jbyteArray retval = env->NewByteArray(sizeof(retvalBytes));
    env->SetByteArrayRegion(retval, 0, sizeof(retvalBytes), (jbyte*)retvalBytes);

    return retval;
}

#ifdef __cplusplus
}
#endif
