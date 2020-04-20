#include <jni.h>
#include <string.h>
#include <cstring>
#include <android/log.h>
#include "cifra/modes.h"
#include "cifra/aes.h"

#include "encryption.h"
#include "connection_common.h"

#include "messages/GuestConnectionStep1.h"
#include "messages/GuestConnectionStep2.h"
#include "messages/GuestConnectionStep3.h"
#include "messages/GuestConnectionStep4.h"
#include "messages/GuestToken.h"

static const char *kTag = "JNI_guest_connection";

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_test(JNIEnv *env, jobject pThis, jstring test) {
    return NULL;
}

JNIEXPORT jint JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_beginConnection(JNIEnv *env, jobject pThis, jbyteArray jKey) {
    return beginConnection(env, pThis, kConnectionTypeGuest, jKey);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jGuestToken) {
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return NULL;
    if (!jGuestToken)
        return NULL;

    // get guest token raw bytes
    jsize guestTokenLen = env->GetArrayLength(jGuestToken);
    uint8_t *guestTokenBytes = (uint8_t*)(env->GetByteArrayElements(jGuestToken, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jGuestToken, (jbyte *)guestTokenBytes, 0);
    };

    // create step 1
    IgGuestConnectionStep1 step1;
    ig_GuestConnectionStep1_init(&step1);
    ig_GuestConnectionStep1_set_token(&step1, guestTokenBytes, guestTokenLen);
    size_t retvalMaxLen = ig_GuestConnectionStep1_get_max_payload_in_bytes(&step1);
    uint8_t retvalBytes[retvalMaxLen];
    size_t retvalLen = 0;
    IgSerializerError err = ig_GuestConnectionStep1_encode(&step1, retvalBytes, retvalMaxLen, &retvalLen);
    if (err != IgSerializerNoError) {
        ig_GuestConnectionStep1_deinit(&step1);
        freeJArrays();
        return NULL;
    }

    // create JVM ByteArray
    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    ig_GuestConnectionStep1_deinit(&step1);
    freeJArrays();
    return retval;
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jConnectionStep2) {
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return NULL;
    if (!jConnectionStep2)
        return NULL;

    // decode step 2
    jsize step2Len = env->GetArrayLength(jConnectionStep2);
    uint8_t *step2Bytes = (uint8_t*)(env->GetByteArrayElements(jConnectionStep2, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jConnectionStep2, (jbyte *)step2Bytes, 0);
    };
//    Log_d(kTag, "step2 bytes = ");
//    for (int j = 0; j < step2Len; j++) {
//        Log_d(kTag, "%02x", step2Bytes[j]);
//    }

    IgGuestConnectionStep2 step2;
    ig_GuestConnectionStep2_init(&step2);
    IgSerializerError step2_err = ig_GuestConnectionStep2_decode(step2Bytes, (size_t)step2Len, &step2, 0);
//    Log_d(kTag, "step2_err = %i", step2_err);
    if (step2_err || !ig_GuestConnectionStep2_is_valid(&step2) || step2.nonce_size != kNonceLength) {
        ig_GuestConnectionStep2_deinit(&step2);
        freeJArrays();
        return NULL;
    }
    memcpy(connection->txNonce, step2.nonce, step2.nonce_size);
//    Log_d(kTag, "step2 nonce = ");
//    for (int j = 0; j < step2.nonce_size; j++) {
//        Log_d(kTag, "%02x", step2.nonce[j]);
//    }

    // create step2 message to return
    IgGuestConnectionStep3 step3;
    ig_GuestConnectionStep3_init(&step3);
    ig_GuestConnectionStep3_set_nonce(&step3, connection->rxNonce, kNonceLength);

    size_t step3MaxLen = ig_GuestConnectionStep3_get_max_payload_in_bytes(&step3);
    uint8_t plaintextBytes[step3MaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_GuestConnectionStep3_encode(&step3, plaintextBytes, step3MaxLen, &plaintextLen);
    if (err != IgSerializerNoError) {
        ig_GuestConnectionStep2_deinit(&step2);
        ig_GuestConnectionStep3_deinit(&step3);
        freeJArrays();
        return NULL;
    }
//    Log_d(kTag, "encoded step3 = ");
//    for (int j = 0; j < plaintextLen; j++) {
//        Log_d(kTag, "%02x", plaintextBytes[j]);
//    }

    // encrypt
    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
    uint8_t retvalBytes[retvalMaxLen];
    int32_t retvalLen = encryptData(
            plaintextBytes, plaintextLen,
            retvalBytes, retvalMaxLen,
            connection->key, kConnectionKeyLength,
            connection->txNonce, kNonceLength
    );
    incrementNonce(connection->txNonce);
    if (retvalLen < 0) {
        ig_GuestConnectionStep2_deinit(&step2);
        ig_GuestConnectionStep3_deinit(&step3);
        freeJArrays();
        return NULL;
    }

    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    ig_GuestConnectionStep2_deinit(&step2);
    ig_GuestConnectionStep3_deinit(&step3);
    freeJArrays();
    return retval;
}

JNIEXPORT jboolean JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jConnectionStep4) {
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return false;
    if (!jConnectionStep4)
        return false;

    jsize messageLen = env->GetArrayLength(jConnectionStep4);
    uint8_t *messageBytes = (uint8_t*)(env->GetByteArrayElements(jConnectionStep4, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jConnectionStep4, (jbyte *)messageBytes, 0);
    };

    // decrypt
    uint32_t step4MaxLen = decryptDataSize(messageLen);
    uint8_t step4Bytes[step4MaxLen];
    int32_t step4Len = decryptData(
            messageBytes, messageLen,
            step4Bytes, step4MaxLen,
            connection->key, kConnectionKeyLength,
            connection->rxNonce, kNonceLength
    );
    incrementNonce(connection->rxNonce);
    if (step4Len < 0) {
        freeJArrays();
        return false;
    }

    // decode step 4
    IgGuestConnectionStep4 step4;
    ig_GuestConnectionStep4_init(&step4);
    ig_GuestConnectionStep4_decode(step4Bytes, (size_t)step4Len, &step4, 0);
    if (!ig_GuestConnectionStep4_is_valid(&step4) || !step4.success) {
        ig_GuestConnectionStep4_deinit(&step4);
        freeJArrays();
        return false;
    }

    ig_GuestConnectionStep4_deinit(&step4);
    freeJArrays();
    return true;
}


JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
    endConnection(env, pThis, connectionId);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext) {
    return encryptNative(env, pThis, connectionId, jPlaintext);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage) {
    return decryptNative(env, pThis, connectionId, jMessage);
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_GuestConnection_setRxNonce(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jNonce) {
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return;

    jsize nonceLen = env->GetArrayLength(jNonce);
    env->GetByteArrayRegion(jNonce, 0, nonceLen, (jbyte*)connection->rxNonce);
}

#ifdef __cplusplus
}
#endif
