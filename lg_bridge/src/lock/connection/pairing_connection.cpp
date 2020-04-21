#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <iostream>
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

using namespace std;

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

bool igloohome_ble_lock_crypto_PairingConnection_beginConnection() {
    const struct uECC_Curve_t * p_curve = uECC_secp256r1();
    int makeKeyRetval = uECC_make_key(publicKey_, privateKey_, p_curve);
    if (makeKeyRetval == 0) return false;
    generateRandomNonce(kNonceLength, rxNonce_);
    return true;
}

int igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(uint8_t **ret)
{
    // create step 1
    IgPairingStep1 step1;
    ig_PairingStep1_init(&step1);
    ig_PairingStep1_set_public_key(&step1, publicKey_, sizeof(publicKey_));
    size_t retvalMaxLen = ig_PairingStep1_get_max_payload_in_bytes(&step1);
    uint8_t retvalBytes[retvalMaxLen];
    size_t retvalLen = 0;
    IgSerializerError err = ig_PairingStep1_encode(&step1, retvalBytes, retvalMaxLen, &retvalLen);
    if (err != IgSerializerNoError) {
        cout << "IgSerializerNoError" << endl;
        ig_PairingStep1_deinit(&step1);
        return 0;
    }
    uint8_t *retBytes = (uint8_t *)calloc(retvalLen, sizeof(uint8_t));
    memcpy(retBytes, retvalBytes, retvalLen);
    *ret = retBytes;
    return retvalLen;
}

int igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
    int step2Len, uint8_t jPairingStep2[], uint8_t **ret)
{
    if (!jPairingStep2 || step2Len == 0)
    {
        *ret = NULL;
        return 0;
    }

    uint8_t step2Bytes[step2Len];
    memcpy(step2Bytes, jPairingStep2, step2Len);

    IgPairingStep2 step2;
    ig_PairingStep2_init(&step2);
    IgSerializerError step2_err = ig_PairingStep2_decode(
        step2Bytes, (size_t)step2Len, &step2, 0);
    if (step2_err || !ig_PairingStep2_is_valid(&step2) || step2.nonce_size != kNonceLength ||
            !step2.has_public_key || step2.public_key_size != kPublicKeyLength) {
        cout << "step2_err" << step2_err << endl;
        ig_PairingStep2_deinit(&step2);
        *ret = NULL;
        return 0;
    }
    memcpy(txNonce_, step2.nonce, step2.nonce_size);
    memcpy(otherPublicKey_, step2.public_key, kPublicKeyLength);

    uint8_t sharedSecret[kPrivateKeyLength] = {0};
    const struct uECC_Curve_t * p_curve = uECC_secp256r1();
    int shared_secret_ret = uECC_shared_secret(otherPublicKey_, privateKey_, sharedSecret, p_curve);
    if (shared_secret_ret != 1) {
        cout << "shared_secret_ret" << endl;
        ig_PairingStep2_deinit(&step2);
        *ret = NULL;
        return 0;
    }
     uint8_t hashedSharedSecret[kPublicKeyLength];
     memcpy(hashedSharedSecret, sharedSecret, sizeof(sharedSecret));
     memcpy(sharedKey_, hashedSharedSecret, sizeof(sharedKey_));

     IgPairingStep3 step3;
     ig_PairingStep3_init(&step3);
    //  if (jStep3Params) {
    //     // start from the passed in step 3
    //     uint8_t *jStep3Params_end = jStep3Params;
    //     while (*jStep3Params_end != 0)
    //     {
    //         jStep3Params_end++;
    //     }
    //     size_t paramsLen = jStep3Params_end - jStep3Params;
    //     uint8_t *paramsBytes = jStep3Params;
    //     ig_PairingStep3_decode(paramsBytes, paramsLen, &step3, 0);
    // }

    //尝试设置其他的东西
    // step3.has_gmt_offset = true;
    // step3.gmt_offset = 28800;

    // 只设置
    ig_PairingStep3_set_nonce(&step3, rxNonce_, kNonceLength);
    size_t step3MaxLen = ig_PairingStep3_get_max_payload_in_bytes(&step3);
    uint8_t plaintextBytes[step3MaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_PairingStep3_encode(&step3, plaintextBytes, step3MaxLen, &plaintextLen);
    
    if (err != IgSerializerNoError) {
        ig_PairingStep2_deinit(&step2);
        ig_PairingStep3_deinit(&step3);
        *ret = NULL;
        return 0;
    }

    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
    uint8_t *retvalBytes = (uint8_t *)calloc(retvalMaxLen, sizeof(uint8_t));
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
        *ret = NULL;
        return 0;
    }

    ig_PairingStep2_deinit(&step2);
    ig_PairingStep3_deinit(&step3);
    *ret = retvalBytes;
    return retvalLen;
}

int igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
    int messageLen, uint8_t jPairingStep4[], uint8_t **ret
)
{
    if (!jPairingStep4)
    {
        *ret = NULL;
        return 0;
    }

    uint8_t messageBytes[messageLen];
    memcpy(messageBytes, jPairingStep4, messageLen);

    uint32_t step4MaxLen = decryptDataSize(messageLen);
    uint8_t step4Bytes[step4MaxLen];
    int32_t step4Len = decryptData(
            messageBytes, messageLen,
            step4Bytes, step4MaxLen,
            sharedKey_, kConnectionKeyLength,
            rxNonce_, kNonceLength
    );
    incrementNonce(rxNonce_);
    printf("recPairingStep4Native: step4Len = %i\n", step4Len);
    if (step4Len < 0) {
        *ret = NULL;
        return 0;
    }

    IgPairingStep4 step4;
    ig_PairingStep4_init(&step4);
    ig_PairingStep4_decode(step4Bytes, (size_t)step4Len, &step4, 0);
    printf("recPairingStep4Native: step4.has_success = %i, step4.success = %i\n", step4.has_success, step4.success);
    if (!ig_PairingStep4_is_valid(&step4) || !step4.success) {
        ig_PairingStep4_deinit(&step4);
        *ret = NULL;
        return 0;
    }

    ig_PairingStep4_deinit(&step4);
    *ret = (uint8_t *)malloc(step4Len);
    memcpy(*ret, step4Bytes, step4Len);
    return step4Len;
}

int igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(
    int currentTime, uint8_t **ret)
{
    IgPairingCommit pairingCommit;
    ig_PairingCommit_init(&pairingCommit);
    ig_PairingCommit_set_current_time(&pairingCommit, currentTime);

    size_t pairingCommitMaxLen = ig_PairingCommit_get_max_payload_in_bytes(&pairingCommit);
    uint8_t plaintextBytes[pairingCommitMaxLen];
    size_t plaintextLen = 0;
    IgSerializerError err = ig_PairingCommit_encode(&pairingCommit, plaintextBytes, pairingCommitMaxLen, &plaintextLen);
    if (err != IgSerializerNoError) {
        ig_PairingCommit_deinit(&pairingCommit);
        *ret = NULL;
        return 0;
    }

    printf("encoded pairingCommit = ");
    for (int j = 0; j < plaintextLen; j++) {
        printf("%02x ", plaintextBytes[j]);
    }
    printf("\n");

    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
    uint8_t retvalBytes[retvalMaxLen];
    int32_t retvalLen = encryptData(
            plaintextBytes, plaintextLen,
            retvalBytes, retvalMaxLen,
            sharedKey_, kConnectionKeyLength,
            txNonce_, kNonceLength
    );
    *ret = (uint8_t *)malloc(retvalLen);
    memcpy(*ret, retvalBytes, retvalLen);
    incrementNonce(txNonce_);
    if (retvalLen < 0) {
        ig_PairingCommit_deinit(&pairingCommit);
        *ret = NULL;
        return 0;
    }
    return retvalLen;
}

int igloohome_ble_lock_crypto_PairingConnection_getAdminKeyNative(
    int currentTime, uint8_t **ret)
{
    *ret = (uint8_t *)malloc(kConnectionKeyLength);
    for (int i = 0; i < kConnectionKeyLength; i++)
    {
        (*ret)[i] = sharedKey_[i];
    }
    return kConnectionKeyLength;
}


// JNIEXPORT void JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_endConnection(JNIEnv *env, jobject pThis, jint connectionId) {
//     endConnection(env, pThis, connectionId);
// }

// JNIEXPORT jbyteArray JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_encryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jPlaintext) {
//     return encryptNative(env, pThis, connectionId, jPlaintext);
// }

// JNIEXPORT jbyteArray JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_decryptNative(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jMessage) {
//     return decryptNative(env, pThis, connectionId, jMessage);
// }

// JNIEXPORT void JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_setRxNonce(JNIEnv *env, jobject pThis, jbyteArray jNonce) {
//     jsize nonceLen = env->GetArrayLength(jNonce);
//     env->GetByteArrayRegion(jNonce, 0, nonceLen, (jbyte*)rxNonce_);
// }

// JNIEXPORT void JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_setPrivateKey(JNIEnv *env, jobject pThis, jbyteArray jPrivateKey) {
//     jsize nonceLen = env->GetArrayLength(jPrivateKey);
//     env->GetByteArrayRegion(jPrivateKey, 0, nonceLen, (jbyte*)privateKey_);
// }

// JNIEXPORT void JNICALL
// Java_co_igloohome_ble_lock_crypto_PairingConnection_setPublicKey(JNIEnv *env, jobject pThis, jbyteArray jPublicKey) {
//     jsize nonceLen = env->GetArrayLength(jPublicKey);
//     env->GetByteArrayRegion(jPublicKey, 0, nonceLen, (jbyte*)publicKey_);
// }

#ifdef __cplusplus
}
#endif
