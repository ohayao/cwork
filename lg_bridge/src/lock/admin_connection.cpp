#include <jni.h>
#include <string.h>
#include <cstring>
#include <android/log.h>
#include "cifra/modes.h"
#include "cifra/aes.h"

#include "encryption.h"
#include "connection_common.h"
#include "messages/AdminConnectionStep1.h"
#include "messages/AdminConnectionStep2.h"
#include "messages/AdminConnectionStep3.h"

static const char *kTag = "JNI_admin_connection";

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_beginConnection(JNIEnv *env, jobject pThis, jbyteArray jKey) {
    return beginConnection(env, pThis, kConnectionTypeAdmin, jKey);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jConnectionStep1) {
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return NULL;
    if (!jConnectionStep1)
        return NULL;

    jsize step1Len = env->GetArrayLength(jConnectionStep1);
    uint8_t *step1Bytes = (uint8_t*)(env->GetByteArrayElements(jConnectionStep1, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jConnectionStep1, (jbyte *)step1Bytes, 0);
    };

    IgAdminConnectionStep1 step1;
    ig_AdminConnectionStep1_init(&step1);
    IgSerializerError step1_err = ig_AdminConnectionStep1_decode(step1Bytes, (size_t)step1Len, &step1, 0);
    if (step1_err || !ig_AdminConnectionStep1_is_valid(&step1) || step1.nonce_size != kNonceLength) {
        ig_AdminConnectionStep1_deinit(&step1);
        freeJArrays();
        return NULL;
    }
    memcpy(connection->txNonce, step1.nonce, step1.nonce_size);
//    {   // debug print nonce
//        char temp[step1.nonce_size*2+1];
//        for (int j = 0; j < step1.nonce_size; j++) {
//            sprintf(temp+j*2, "%02x", step1.nonce[j]);
//        }
//        Log_d(kTag, "genConnStep2Native: nonce = %s", temp);
//    }

    // create step2 message to return
    IgAdminConnectionStep2 step2;
    ig_AdminConnectionStep2_init(&step2);
    ig_AdminConnectionStep2_set_nonce(&step2, connection->rxNonce, kNonceLength);

    size_t step2MaxLen = ig_AdminConnectionStep2_get_max_payload_in_bytes(&step2);
    uint8_t plaintextBytes[step2MaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_AdminConnectionStep2_encode(&step2, plaintextBytes, step2MaxLen, &plaintextLen);
    if (err != IgSerializerNoError) {
        ig_AdminConnectionStep1_deinit(&step1);
        ig_AdminConnectionStep2_deinit(&step2);
        freeJArrays();
        return NULL;
    }
//    {   // debug print
//        char temp[plaintextLen*2+1];
//        for (int j = 0; j < plaintextLen; j++) {
//            sprintf(temp+j*2, "%02x", plaintextBytes[j]);
//        }
//        Log_d(kTag, "genConnStep2Native: plaintext = %s", temp);
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
        ig_AdminConnectionStep1_deinit(&step1);
        ig_AdminConnectionStep2_deinit(&step2);
        freeJArrays();
        return NULL;
    }

    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)retvalBytes);

    ig_AdminConnectionStep1_deinit(&step1);
    ig_AdminConnectionStep2_deinit(&step2);
    freeJArrays();
    return retval;
}

JNIEXPORT jboolean JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jConnectionStep3) {
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return false;
    if (!jConnectionStep3)
        return false;

    jsize messageLen = env->GetArrayLength(jConnectionStep3);
    uint8_t *messageBytes = (uint8_t*)(env->GetByteArrayElements(jConnectionStep3, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jConnectionStep3, (jbyte *)messageBytes, 0);
    };

    // decrypt
    uint32_t step3MaxLen = decryptDataSize(messageLen);
    uint8_t step3Bytes[step3MaxLen];
    int32_t step3Len = decryptData(
            messageBytes, messageLen,
            step3Bytes, step3MaxLen,
            connection->key, kConnectionKeyLength,
            connection->rxNonce, kNonceLength
    );
    incrementNonce(connection->rxNonce);
    if (step3Len < 0) {
        freeJArrays();
        return false;
    }

    // decode step 3
    IgAdminConnectionStep3 step3;
    ig_AdminConnectionStep3_init(&step3);
    ig_AdminConnectionStep3_decode(step3Bytes, (size_t)step3Len, &step3, 0);
    if (!ig_AdminConnectionStep3_is_valid(&step3) || !step3.success) {
        ig_AdminConnectionStep3_deinit(&step3);
        freeJArrays();
        return false;
    }

    ig_AdminConnectionStep3_deinit(&step3);
    freeJArrays();
    return true;
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
    endConnection(env, pThis, connectionId);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext) {
    return encryptNative(env, pThis, connectionId, jPlaintext);
}

JNIEXPORT jbyteArray JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage) {
    return decryptNative(env, pThis, connectionId, jMessage);
}

JNIEXPORT void JNICALL
Java_co_igloohome_ble_lock_crypto_AdminConnection_setRxNonce(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jNonce) {
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return;

    jsize nonceLen = env->GetArrayLength(jNonce);
    env->GetByteArrayRegion(jNonce, 0, nonceLen, (jbyte*)connection->rxNonce);
}

#ifdef __cplusplus
}
#endif
