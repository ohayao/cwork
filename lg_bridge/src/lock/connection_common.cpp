#include "connection_common.h"

#include <cstdint>
#include <cstring>
#include <cstdio>

static const char *kTag = "JNI_connection_common";

static Connection connections_[kMaxConnections];    // auto initialized to 0

void generateRandomNonce(JNIEnv *env, jbyte *buffer, int nonceLength) {
    jclass cls = env->FindClass("java/security/SecureRandom");
    jmethodID initMethod = env->GetMethodID(cls, "<init>", "()V");
    jmethodID nextBytesMethod = env->GetMethodID(cls, "nextBytes", "([B)V");
    jobject secureRandom = env->NewObject(cls, initMethod);
    jbyteArray jByteArray = env->NewByteArray(nonceLength);
    env->CallVoidMethod(secureRandom, nextBytesMethod, jByteArray);
    env->GetByteArrayRegion(jByteArray, 0, kNonceLength, buffer);
}

jint beginConnection(JNIEnv *env, jobject pThis, ConnectionType type, jbyteArray jKey) {
    for (int i = 0; i < kMaxConnections; i++) {
        if (!connections_[i].active) {
            connections_[i].active = true;
            connections_[i].connnectionId = i;
            connections_[i].type = type;
            jsize keyLen = env->GetArrayLength(jKey);
            if (keyLen != kConnectionKeyLength) {
                return -1;
            }
            env->GetByteArrayRegion(jKey, 0, keyLen, (jbyte*)connections_[i].key);

            generateRandomNonce(env, (jbyte*)connections_[i].rxNonce, kNonceLength);
//            for (int j = 0; j < 16; j++) {
//                Log_d(kTag, "%02x", connections_[j].rxNonce);
//            }

            return i;
        }
    }
    return -1;
}

void endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
    if (connectionId > kMaxConnections || connectionId < 0) {
        return;
    }

    connections_[connectionId].active = false;
    connections_[connectionId].connnectionId = 0;
    connections_[connectionId].type = kConnectionTypeNone;

    memset(connections_->rxNonce, 0, sizeof(connections_->rxNonce));
}

jbyteArray encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext) {
    // validate arguments
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    Connection &connection = connections_[connectionId];

    jsize plaintextLen = env->GetArrayLength(jPlaintext);
    uint8_t *plaintext = (uint8_t*)(env->GetByteArrayElements(jPlaintext, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jPlaintext, (jbyte*)plaintext, 0);
    };

    jsize messageLen = encryptDataSize(plaintextLen);
    uint8_t messageBytes[messageLen];
    int32_t retvalLen = encryptData(plaintext, plaintextLen,
                                    messageBytes, messageLen,
                                    connection.key, kConnectionKeyLength,
                                    connection.txNonce, kNonceLength);
    incrementNonce(connection.txNonce);
    if (retvalLen <= 0 || retvalLen != messageLen) {
        freeJArrays();
        return NULL;
    }
    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)messageBytes);

    freeJArrays();
    return retval;
}

jbyteArray decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage) {
    // validate arguments
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    Connection &connection = connections_[connectionId];

    jsize messageLen = env->GetArrayLength(jMessage);
    uint8_t *messageBytes = (uint8_t*)(env->GetByteArrayElements(jMessage, NULL));
    auto freeJArrays = [&]() {
        env->ReleaseByteArrayElements(jMessage, (jbyte*)messageBytes, 0);
    };

    jsize plaintextLen = decryptDataSize(messageLen);
    uint8_t plaintext[plaintextLen];
    int32_t retvalLen = decryptData(messageBytes, messageLen,
                                    plaintext, plaintextLen,
                                    connection.key, kConnectionKeyLength,
                                    connection.rxNonce, kNonceLength);
//    {   // debug print nonce
//        char temp[kNonceLength*2+1];
//        for (int j = 0; j < kNonceLength; j++) {
//            sprintf(temp+j*2, "%02x", connection.rxNonce[j]);
//        }
//        Log_d(kTag, "decryptNative: nonce = %s", temp);
//    }
//    {   // debug print decrypted text
//        char temp[plaintextLen*2+1];
//        for (int j = 0; j < plaintextLen; j++) {
//            sprintf(temp+j*2, "%02x", plaintext[j]);
//        }
//        Log_d(kTag, "decryptNative: decrypted = %s", temp);
//    }

    incrementNonce(connection.rxNonce);
    if (retvalLen <= 0 || retvalLen != plaintextLen) {
        freeJArrays();
        return NULL;
    }
    jbyteArray retval = env->NewByteArray(retvalLen);
    env->SetByteArrayRegion(retval, 0, retvalLen, (jbyte*)plaintext);

    freeJArrays();
    return retval;
}

Connection* getConnection(int connectionId) {
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    Connection *connection = &(connections_[connectionId]);
    return connection;
}
