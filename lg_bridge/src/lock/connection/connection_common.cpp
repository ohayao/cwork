#include "connection_common.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <random>

static const char *kTag = "JNI_connection_common";

static Connection connections_[kMaxConnections];    // auto initialized to 0

void generateRandomNonce(int nonceLength, uint8_t resultNonceArrar[]) {
    std::random_device r;
    std::default_random_engine char_random_engine(r());
    std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    for (int i = 0; i < nonceLength; i++)
    {
        resultNonceArrar[i] = uniform_dist(char_random_engine);
    }
    return;
}

int beginConnection(ConnectionType type, uint8_t *jKey, int keyLen) {
    for (int i = 0; i < kMaxConnections; i++) {
        if (!connections_[i].active) {
            connections_[i].active = true;
            connections_[i].connnectionId = i;
            connections_[i].type = type;
            if (keyLen != kConnectionKeyLength) {
                return ERROR_CONNECTION_ID;
            }
            memcpy((uint8_t*)(connections_[i].key), jKey, keyLen);
            generateRandomNonce( kNonceLength, (uint8_t*)connections_[i].rxNonce);
            printf("\n");
            return i;
        }
    }
    printf("beginConnection end\n");
    return ERROR_CONNECTION_ID;
}

// void endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
//     if (connectionId > kMaxConnections || connectionId < 0) {
//         return;
//     }

//     connections_[connectionId].active = false;
//     connections_[connectionId].connnectionId = 0;
//     connections_[connectionId].type = kConnectionTypeNone;

//     memset(connections_->rxNonce, 0, sizeof(connections_->rxNonce));
// }


int encryptNative(int connectionId, uint8_t *jPlaintext, int plaintextLen,
    uint8_t **retBytes) {
    // validate arguments
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    Connection &connection = connections_[connectionId];

    uint8_t *plaintext = jPlaintext;

    int messageLen = encryptDataSize(plaintextLen);
    uint8_t messageBytes[messageLen];
    int32_t retvalLen = encryptData(
        plaintext, plaintextLen,
        messageBytes, messageLen,
        connection.key, kConnectionKeyLength,
        connection.txNonce, kNonceLength);
    incrementNonce(connection.txNonce);
    if (retvalLen <= 0 || retvalLen != messageLen) {
        return 0;
    }
    (*retBytes) = (uint8_t *)calloc(retvalLen, 1);
    memcpy(*retBytes, messageBytes, retvalLen);
    return retvalLen;
}

int decryptNative(
    int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes) {
    // validate arguments
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    Connection &connection = connections_[connectionId];
    uint8_t *messageBytes = jMessage;

    int plaintextLen = decryptDataSize(messageLen);
    uint8_t plaintext[plaintextLen];
    int32_t retvalLen = decryptData(messageBytes, messageLen,
                                    plaintext, plaintextLen,
                                    connection.key, kConnectionKeyLength,
                                    connection.rxNonce, kNonceLength);

    incrementNonce(connection.rxNonce);
    if (retvalLen <= 0 || retvalLen != plaintextLen) {
        return NULL;
    }
    *retBytes = (uint8_t*)calloc(retvalLen, 1);
    memcpy(retBytes, plaintext, retvalLen);
    return retvalLen;
}

Connection* getConnection(int connectionId)
{
    if (connectionId > kMaxConnections || connectionId < 0)
        return NULL;
    return (&(connections_[connectionId]));
}

