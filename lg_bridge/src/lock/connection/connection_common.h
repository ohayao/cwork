#ifndef ANDROIDSDKTESTINGFORIGLOOCHIP_CONNECTION_COMMON_H
#define ANDROIDSDKTESTINGFORIGLOOCHIP_CONNECTION_COMMON_H

#include "encryption.h"
#include <cstdint>

//#ifdef NDEBUG
#define Log_d(tag, ...)
//#else
//#define Log_d(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
//#endif

const int kMaxConnections = 32;
const int kConnectionKeyLength = 16;

enum ConnectionType {
    kConnectionTypeNone,
    kConnectionTypeAdmin,
    kConnectionTypeGuest,
};

struct Connection {
    bool active;
    int connnectionId;
    ConnectionType type;
    uint8_t txNonce[kNonceLength];
    uint8_t rxNonce[kNonceLength];
    uint8_t key[kConnectionKeyLength];
};

// void generateRandomNonce(JNIEnv *env, jbyte *buffer, int nonceLength);
// int beginConnection(JNIEnv *env, jobject pThis, ConnectionType type, jbyteArray jKey);
// void endConnection(JNIEnv *env, jobject pThis, jint connectionId);
// jbyteArray encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext);
// jbyteArray decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage);
// Connection* getConnection(int connectionId);
void generateRandomNonce(int nonceLength, uint8_t resultNonceArrar[]);
int beginConnection(ConnectionType type, uint8_t *jKey, int keyLen);
Connection* getConnection(int connectionId);
int encryptNative(int connectionId, uint8_t *jPlaintext, int plaintextLen,
    uint8_t **retBytes);
int decryptNative(
    int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes);

    
#endif //ANDROIDSDKTESTINGFORIGLOOCHIP_CONNECTION_COMMON_H
