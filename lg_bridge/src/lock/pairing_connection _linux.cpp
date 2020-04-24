#include <jni.h>
#include <string.h>
#include <cstring>
#include <android/log.h>
#include "cifra/modes.h"
#include "cifra/aes.h"
#include "micro-ecc/uECC.h"

#include "encryption.h"
#include "connection_common.h"

#include "messages/PairingStep1.h"
#include "messages/PairingStep2.h"
#include "messages/PairingStep3.h"
#include "messages/PairingStep4.h"
#include "messages/PairingCommit.h"
#include "messages/GuestToken.h"

static const char *kTag = "JNI_pairing_connection";

static const int kPrivateKeyLength = 32;
static const int kPublicKeyLength = 64;

static uint8_t privateKey_[kPrivateKeyLength] = {0x00};
static uint8_t publicKey_[kPublicKeyLength] = {0x00};
static uint8_t otherPublicKey_[kPublicKeyLength] = {0x00};
static uint8_t txNonce_[kNonceLength] = {0x00};
static uint8_t rxNonce_[kNonceLength] = {0x00};
static uint8_t sharedKey_[kConnectionKeyLength] = {0x00};

#ifdef __cplusplus
extern "C" {
#endif

// WARNING used to pass the JNI env into the uECC RNG function before calling uECC
// functions that require the RNG
static JNIEnv *env_;
int rngFunction(uint8_t *dest, unsigned size) {
    jclass cls = env_->FindClass("java/security/SecureRandom");
    jmethodID initMethod = env_->GetMethodID(cls, "<init>", "()V");
    jmethodID nextBytesMethod = env_->GetMethodID(cls, "nextBytes", "([B)V");
    jobject secureRandom = env_->NewObject(cls, initMethod);
    jbyteArray jByteArray = env_->NewByteArray(size);
    env_->CallVoidMethod(secureRandom, nextBytesMethod, jByteArray);
    env_->GetByteArrayRegion(jByteArray, 0, kNonceLength, (jbyte*)dest);
    return 1;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_test(JNIEnv *env, jobject pThis, jstring test) {
//    uint8_t private_key1[kPrivateKeyLength] = {
//            0x66, 0x29, 0xED, 0xCE, 0xF5, 0x5A, 0xF2, 0x10,
//            0x86, 0xC2, 0x9C, 0x77, 0x22, 0x07, 0x11, 0xC2,
//            0x49, 0xF6, 0x51, 0xAA, 0x2D, 0x4C, 0x02, 0x2D,
//            0xFB, 0x42, 0xDB, 0xA6, 0xBC, 0x7E, 0x18, 0xC1
//    };
//    uint8_t public_key1[kPublicKeyLength] = {
//            0x6B, 0x41, 0xE6, 0x98, 0xF7, 0x16, 0x1B, 0x48,
//            0x64, 0xDB, 0x87, 0xBC, 0x50, 0x5F, 0x6E, 0x3B,
//            0xB0, 0x69, 0x96, 0x9B, 0x27, 0x51, 0xD8, 0x9D,
//            0xAE, 0xC4, 0x8A, 0x2E, 0xE7, 0x16, 0x15, 0x8B,
//            0x24, 0xDC, 0x49, 0xA5, 0x9B, 0x91, 0x50, 0xAB,
//            0x0F, 0xCB, 0xE3, 0x03, 0x0B, 0x64, 0xFB, 0x85,
//            0x84, 0x73, 0x03, 0xA3, 0x7E, 0xC3, 0x27, 0x05,
//            0x6B, 0xA4, 0xE1, 0x32, 0x1B, 0x4F, 0xE8, 0xB0
//    };
//    uint8_t private_key2[kPrivateKeyLength] = {
//            0x87, 0x7B, 0xB8, 0xC4, 0x11, 0x15, 0x06, 0x4D,
//            0xCB, 0x69, 0x62, 0xD6, 0xA9, 0xEB, 0x1D, 0x23,
//            0xC0, 0x55, 0xCC, 0x31, 0x8E, 0x00, 0xCF, 0xE1,
//            0x3E, 0x01, 0x04, 0x2C, 0xBB, 0x45, 0x74, 0xA9
//    };
//    uint8_t public_key2[kPublicKeyLength] = {
//            0x5E, 0x64, 0x8A, 0x6C, 0x7C, 0x8E, 0x81, 0x3C,
//            0x04, 0xAB, 0xF3, 0x4C, 0x45, 0x1C, 0x9B, 0x14,
//            0x7D, 0xD6, 0x3B, 0x3B, 0xA5, 0xC1, 0x3B, 0xE4,
//            0x66, 0x94, 0xCD, 0xE5, 0x24, 0x98, 0x78, 0x2B,
//            0x7C, 0x3F, 0xA2, 0x77, 0x19, 0xFB, 0x85, 0x67,
//            0xE5, 0x01, 0x1A, 0xD2, 0x86, 0xE5, 0xF5, 0xCF,
//            0xE5, 0xD2, 0x3B, 0x69, 0x80, 0x32, 0xA9, 0x80,
//            0x45, 0x65, 0x27, 0x4B, 0x1E, 0xC4, 0xE3, 0x47
//    };
//
//    uint8_t shared_secret1[kPrivateKeyLength] = {0};
//    uint8_t shared_secret2[kPrivateKeyLength] = {0};
//    uint8_t shared_secret_verify[] = {
//            0x0A, 0x88, 0xB5, 0x60, 0x00, 0x2E, 0x3E, 0x73,
//            0x24, 0x27, 0xCA, 0x7A, 0xC0, 0x84, 0x37, 0x76,
//            0xCA, 0xAA, 0x0C, 0x9C, 0x7B, 0x76, 0x7D, 0x41,
//            0x85, 0x02, 0xBC, 0x3E, 0x55, 0xAD, 0x46, 0xD4
//    };
//
//    const struct uECC_Curve_t * p_curve = uECC_secp256r1();
//    int ret = 0;
//
//    ret = uECC_shared_secret(public_key2, private_key1, shared_secret1, p_curve);
//    Log_d(kTag, "uECC_shared_secret ret = %i", ret);
//    for (int j = 0; j < sizeof(shared_secret1); j++) {
//        Log_d(kTag, "%02x", shared_secret1[j]);
//    }
//
//    ret = uECC_shared_secret(public_key1, private_key2, shared_secret2, p_curve);
//    Log_d(kTag, "uECC_shared_secret ret = %i", ret);
//    for (int j = 0; j < sizeof(shared_secret2); j++) {
//        Log_d(kTag, "%02x", shared_secret2[j]);
//    }


    uint8_t nonce[] = {
            0x0A, 0x88, 0xB5, 0x60, 0x00, 0x2E, 0x3E, 0x73, 0x24, 0x27,
            0xCA, 0x7A
    };
    uint8_t key[] = {
            0x3F, 0x11, 0xEB, 0xAA, 0x97, 0x57, 0xFC, 0x11, 0x93, 0x3D,
            0xA7, 0x24, 0x83, 0x8F, 0x5A, 0xE6
    };
    uint8_t plaintext[] = {
            0xA6, 0x00, 0x18, 0xCA, 0x0B, 0x19, 0x27, 0x75, 0x0C, 0x18,
            0x64, 0x0D, 0x1A, 0x5B, 0x87, 0x33, 0x80, 0x0E, 0x50, 0x05,
            0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07,
            0x07, 0x07, 0x07, 0x07, 0x08, 0x0F, 0x44, 0xFF, 0xFF, 0xFF,
            0xFF
    };
    uint32_t encryptedSize = encryptDataSize(sizeof(plaintext));
    uint8_t encryptedData[encryptedSize];

    encryptData(plaintext, sizeof(plaintext), encryptedData, sizeof(encryptedData), key, sizeof(key), nonce, sizeof(nonce));
    uint8_t token[sizeof(nonce) + encryptedSize];
    memcpy(token, nonce, sizeof(nonce));
    memcpy(token + sizeof(nonce), encryptedData, sizeof(encryptedData));

    // create JVM ByteArray
    jbyteArray retval = env->NewByteArray(sizeof(token));
    env->SetByteArrayRegion(retval, 0, sizeof(token), (jbyte*)token);

    return retval;
}

JNIEXPORT jboolean JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_beginConnection(JNIEnv *env, jobject pThis) {
    // generate random private/public key pair
    const struct uECC_Curve_t * p_curve = uECC_secp256r1();
    uECC_set_rng(rngFunction);
    // set env_ before calling make_key, because we have no other way to pass the env into the RNG function
    env_ = env;
    int makeKeyRetval = uECC_make_key(publicKey_, privateKey_, p_curve);
    if (makeKeyRetval == 0) return JNI_FALSE;

    // generate nonce
    generateRandomNonce(env, (jbyte*)rxNonce_, kNonceLength);
    // TEST: hardcoded nonce
//    memset(rxNonce_, 0x0E, sizeof(rxNonce_));

    return JNI_TRUE;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(JNIEnv *env, jobject pThis) {
    // TEST: hardcoded private/public key pair
//    uint8_t private_key1[kPrivateKeyLength] = {
//            0x87, 0x7B, 0xB8, 0xC4, 0x11, 0x15, 0x06, 0x4D,
//            0xCB, 0x69, 0x62, 0xD6, 0xA9, 0xEB, 0x1D, 0x23,
//            0xC0, 0x55, 0xCC, 0x31, 0x8E, 0x00, 0xCF, 0xE1,
//            0x3E, 0x01, 0x04, 0x2C, 0xBB, 0x45, 0x74, 0xA9
//    };
//    uint8_t public_key1[kPublicKeyLength] = {
//            0x5E, 0x64, 0x8A, 0x6C, 0x7C, 0x8E, 0x81, 0x3C,
//            0x04, 0xAB, 0xF3, 0x4C, 0x45, 0x1C, 0x9B, 0x14,
//            0x7D, 0xD6, 0x3B, 0x3B, 0xA5, 0xC1, 0x3B, 0xE4,
//            0x66, 0x94, 0xCD, 0xE5, 0x24, 0x98, 0x78, 0x2B,
//            0x7C, 0x3F, 0xA2, 0x77, 0x19, 0xFB, 0x85, 0x67,
//            0xE5, 0x01, 0x1A, 0xD2, 0x86, 0xE5, 0xF5, 0xCF,
//            0xE5, 0xD2, 0x3B, 0x69, 0x80, 0x32, 0xA9, 0x80,
//            0x45, 0x65, 0x27, 0x4B, 0x1E, 0xC4, 0xE3, 0x47
//    };
//    memcpy(privateKey_, private_key1, kPrivateKeyLength);
//    memcpy(publicKey_, public_key1, kPublicKeyLength);

    // create step 1
    IgPairingStep1 step1;
    ig_PairingStep1_init(&step1);
    ig_PairingStep1_set_public_key(&step1, publicKey_, sizeof(publicKey_));
    size_t retvalMaxLen = ig_PairingStep1_get_max_payload_in_bytes(&step1);
    uint8_t retvalBytes[retvalMaxLen];
    size_t retvalLen = 0;
    IgSerializerError err = ig_PairingStep1_encode(&step1, retvalBytes, retvalMaxLen, &retvalLen);
    if (err != IgSerializerNoError) {
        ig_PairingStep1_deinit(&step1);
        return NULL;
    }

    // create JVM ByteArray
    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    ig_PairingStep1_deinit(&step1);
    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(JNIEnv *env, jobject pThis, jbyteArray jPairingStep2, jbyteArray jStep3Params) {
    // validate arguments
    if (!jPairingStep2)
        return NULL;

    // read java byte arrays
    jsize step2Len = env->GetArrayLength(jPairingStep2);
    uint8_t step2Bytes[step2Len];
    env->GetByteArrayRegion(jPairingStep2, 0, step2Len, (jbyte*)step2Bytes);

    Log_d(kTag, "step2 bytes = ");
    for (int j = 0; j < step2Len; j++) {
        Log_d(kTag, "%02x", step2Bytes[j]);
    }

    // decode step 2
    IgPairingStep2 step2;
    ig_PairingStep2_init(&step2);
    IgSerializerError step2_err = ig_PairingStep2_decode(step2Bytes, (size_t)step2Len, &step2, 0);
    Log_d(kTag, "step2_err = %i", step2_err);
    if (step2_err || !ig_PairingStep2_is_valid(&step2) || step2.nonce_size != kNonceLength ||
            !step2.has_public_key || step2.public_key_size != kPublicKeyLength) {
        ig_PairingStep2_deinit(&step2);
        return NULL;
    }
    memcpy(txNonce_, step2.nonce, step2.nonce_size);
    memcpy(otherPublicKey_, step2.public_key, kPublicKeyLength);
    // TEST: hardcode nonce
//    memset(txNonce_, 0x0C, sizeof(txNonce_));


    // calculate shared key
    uint8_t sharedSecret[kPrivateKeyLength] = {0};
    const struct uECC_Curve_t * p_curve = uECC_secp256r1();
    env_ = env;
    int shared_secret_ret = uECC_shared_secret(otherPublicKey_, privateKey_, sharedSecret, p_curve);
    Log_d(kTag, "uECC_shared_secret ret = %i", shared_secret_ret);
    for (int j = 0; j < sizeof(sharedSecret); j++) {
        Log_d(kTag, "%02x", sharedSecret[j]);
    }
    if (shared_secret_ret != 1) {
        ig_PairingStep2_deinit(&step2);
        return NULL;
    }
    // TODO: calculate SHA256 hash
    uint8_t hashedSharedSecret[kPublicKeyLength];
    memcpy(hashedSharedSecret, sharedSecret, sizeof(sharedSecret));

    // truncate to get admin key
    memcpy(sharedKey_, hashedSharedSecret, sizeof(sharedKey_));

    // create step 3 message to return
    IgPairingStep3 step3;
    ig_PairingStep3_init(&step3);
    if (jStep3Params) {
        // start from the passed in step 3
        jsize paramsLen = env->GetArrayLength(jStep3Params);
        uint8_t *paramsBytes = (uint8_t*)(env->GetByteArrayElements(jStep3Params, NULL));
        ig_PairingStep3_decode(paramsBytes, paramsLen, &step3, 0);
    }
    ig_PairingStep3_set_nonce(&step3, rxNonce_, kNonceLength);

    size_t step3MaxLen = ig_PairingStep3_get_max_payload_in_bytes(&step3);
    uint8_t plaintextBytes[step3MaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_PairingStep3_encode(&step3, plaintextBytes, step3MaxLen, &plaintextLen);
    if (err != IgSerializerNoError) {
        ig_PairingStep2_deinit(&step2);
        ig_PairingStep3_deinit(&step3);
        return NULL;
    }
    Log_d(kTag, "encoded step3 = ");
    for (int j = 0; j < plaintextLen; j++) {
        Log_d(kTag, "%02x", plaintextBytes[j]);
    }

    // encrypt
    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
    uint8_t retvalBytes[retvalMaxLen];
    int32_t retvalLen = encryptData(
            plaintextBytes, plaintextLen,
            retvalBytes, retvalMaxLen,
            sharedKey_, kConnectionKeyLength,
            txNonce_, kNonceLength
    );
    incrementNonce(txNonce_);
    if (retvalLen < 0) {
        ig_PairingStep2_deinit(&step2);
        ig_PairingStep3_deinit(&step3);
        return NULL;
    }

    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    ig_PairingStep2_deinit(&step2);
    ig_PairingStep3_deinit(&step3);
    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(JNIEnv *env, jobject pThis, jbyteArray jPairingStep4) {
    // validate arguments
    if (!jPairingStep4)
        return NULL;

    jsize messageLen = env->GetArrayLength(jPairingStep4);
    uint8_t *messageBytes = (uint8_t*)(env->GetByteArrayElements(jPairingStep4, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jPairingStep4, (jbyte *)messageBytes, 0);
    };

    // decrypt
    uint32_t step4MaxLen = decryptDataSize(messageLen);
    uint8_t step4Bytes[step4MaxLen];
    int32_t step4Len = decryptData(
            messageBytes, messageLen,
            step4Bytes, step4MaxLen,
            sharedKey_, kConnectionKeyLength,
            rxNonce_, kNonceLength
    );
    incrementNonce(rxNonce_);
    Log_d(kTag, "recPairingStep4Native: step4Len = %i", step4Len);
    if (step4Len < 0) {
        freeJArrays();
        return NULL;
    }

    // decode step 4 to check result
    IgPairingStep4 step4;
    ig_PairingStep4_init(&step4);
    ig_PairingStep4_decode(step4Bytes, (size_t)step4Len, &step4, 0);
    Log_d(kTag, "recPairingStep4Native: step4.has_success = %i, step4.success = %i", step4.has_success, step4.success);
    if (!ig_PairingStep4_is_valid(&step4) || !step4.success) {
        ig_PairingStep4_deinit(&step4);
        freeJArrays();
        return NULL;
    }
    ig_PairingStep4_deinit(&step4);

    jbyteArray retval = env->NewByteArray(step4Len);
    env->SetByteArrayRegion(retval, 0, step4Len, (jbyte*)step4Bytes);

    freeJArrays();
    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(JNIEnv *env, jobject pThis, jint currentTime) {
    // create pairing commit
    IgPairingCommit pairingCommit;
    ig_PairingCommit_init(&pairingCommit);
    ig_PairingCommit_set_current_time(&pairingCommit, currentTime);

    size_t pairingCommitMaxLen = ig_PairingCommit_get_max_payload_in_bytes(&pairingCommit);
    uint8_t plaintextBytes[pairingCommitMaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_PairingCommit_encode(&pairingCommit, plaintextBytes, pairingCommitMaxLen, &plaintextLen);
    if (err != IgSerializerNoError) {
        ig_PairingCommit_deinit(&pairingCommit);
        return NULL;
    }
    Log_d(kTag, "encoded pairingCommit = ");
    for (int j = 0; j < plaintextLen; j++) {
        Log_d(kTag, "%02x", plaintextBytes[j]);
    }

    // encrypt
    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
    uint8_t retvalBytes[retvalMaxLen];
    int32_t retvalLen = encryptData(
            plaintextBytes, plaintextLen,
            retvalBytes, retvalMaxLen,
            sharedKey_, kConnectionKeyLength,
            txNonce_, kNonceLength
    );
    incrementNonce(txNonce_);
    if (retvalLen < 0) {
        ig_PairingCommit_deinit(&pairingCommit);
        return NULL;
    }

    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_getAdminKeyNative(JNIEnv *env, jobject pThis, jint currentTime) {
    jbyteArray retval = env->NewByteArray(kConnectionKeyLength);
    env->SetByteArrayRegion(retval, 0, kConnectionKeyLength, (jbyte*)sharedKey_);

    return retval;
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
    endConnection(env, pThis, connectionId);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext) {
    return encryptNative(env, pThis, connectionId, jPlaintext);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage) {
    return decryptNative(env, pThis, connectionId, jMessage);
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_setRxNonce(JNIEnv *env, jobject pThis, jbyteArray jNonce) {
    jsize nonceLen = env->GetArrayLength(jNonce);
    env->GetByteArrayRegion(jNonce, 0, nonceLen, (jbyte*)rxNonce_);
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_setPrivateKey(JNIEnv *env, jobject pThis, jbyteArray jPrivateKey) {
    jsize nonceLen = env->GetArrayLength(jPrivateKey);
    env->GetByteArrayRegion(jPrivateKey, 0, nonceLen, (jbyte*)privateKey_);
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_PairingConnection_setPublicKey(JNIEnv *env, jobject pThis, jbyteArray jPublicKey) {
    jsize nonceLen = env->GetArrayLength(jPublicKey);
    env->GetByteArrayRegion(jPublicKey, 0, nonceLen, (jbyte*)publicKey_);
}

#ifdef __cplusplus
}
#endif
