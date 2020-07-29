#include <string.h>
#include <bridge/lock/cifra/modes.h>
#include <bridge/lock/cifra/aes.h>
#include <bridge/lock/connection/encryption.h>
#include <bridge/lock/connection/connection_common.h>

#include <bridge/lock/messages/GuestConnectionStep1.h>
#include <bridge/lock/messages/GuestConnectionStep2.h>
#include <bridge/lock/messages/GuestConnectionStep3.h>
#include <bridge/lock/messages/GuestConnectionStep4.h>
#include "messages/GuestToken.h"

#include <stdio.h>
#include <bridge/bridge_main/log.h>
#include <bridge/lock/connection/guest_connection.h>

//static const char *kTag = "JNI_guest_connection";

#ifdef __cplusplus
extern "C" {
#endif


int igloohome_ble_lock_crypto_GuestConnection_beginConnection(uint8_t *jKey, int keyLen) {
	return beginConnection(kConnectionTypeGuest, jKey, keyLen);
}

int igloohome_ble_lock_crypto_GuestConnection_genConnStep1Native(int connectionId, uint8_t* guest_token, unsigned int guest_token_len, uint8_t **retBytes) {
	// validate arguments
	Connection *connection = getConnection(connectionId);
	if (!connection) {
		serverLog(LL_ERROR, "!connection");
		return 0;
	}
	if (!guest_token) {
		serverLog(LL_ERROR, "!guest_token");
		return 0;
	}

    // create step 1
    IgGuestConnectionStep1 step1;
    ig_GuestConnectionStep1_init(&step1);
    ig_GuestConnectionStep1_set_token(&step1, guest_token, guest_token_len);
    size_t retvalMaxLen = ig_GuestConnectionStep1_get_max_payload_in_bytes(&step1);
    uint8_t* retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);
    size_t retvalLen = 0;
    IgSerializerError err = ig_GuestConnectionStep1_encode(&step1, retvalBytes, retvalMaxLen, &retvalLen);
    if (err != IgSerializerNoError) {
        ig_GuestConnectionStep1_deinit(&step1);
        free(retvalBytes);
        return 0;
    }

    ig_GuestConnectionStep1_deinit(&step1);
	*retBytes = retvalBytes;
	return retvalLen;
}

int igloohome_ble_lock_crypto_GuestConnection_genConnStep3Native(int connectionId, uint8_t *step2Bytes, int step2Len, uint8_t **retBytes) {
	Connection *connection = getConnection(connectionId);
	if (!connection) {
		serverLog(LL_ERROR, "!connection");
		return 0;
	}

	if (!step2Bytes) {
		serverLog(LL_ERROR, "!step2Bytes" );
		return 0;
	}

    IgGuestConnectionStep2 step2;
    ig_GuestConnectionStep2_init(&step2);
	IgSerializerError step2_err = ig_GuestConnectionStep2_decode(step2Bytes, (size_t)step2Len, &step2, 0);
	if (step2_err || !ig_GuestConnectionStep2_is_valid(&step2) || step2.nonce_size != kNonceLength) {
        ig_GuestConnectionStep2_deinit(&step2);
		serverLog(LL_ERROR, "ig_GuestConnectionStep2_decode err[%d] step2Len[%d]", step2_err, step2Len);
		return 0;
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
		serverLog(LL_ERROR, "ig_GuestConnectionStep3_encode error[%d]", err);
        return 0;
    }
//    Log_d(kTag, "encoded step3 = ");
//    for (int j = 0; j < plaintextLen; j++) {
//        Log_d(kTag, "%02x", plaintextBytes[j]);
//    }

    // encrypt
    uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
	uint8_t *retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);
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
		free(retvalBytes);
		serverLog(LL_ERROR, "encryptData error[%d]", retvalLen);
        return 0;
    }

    ig_GuestConnectionStep2_deinit(&step2);
    ig_GuestConnectionStep3_deinit(&step3);
	*retBytes = retvalBytes;
	return retvalLen;
}

bool igloohome_ble_lock_crypto_GuestConnection_recConnStep4Native(int connectionId, uint8_t *messageBytes, int messageLen){
    // validate arguments
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return false;
    if (!messageBytes)
        return false;

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
        return false;
    }

    // decode step 4
    IgGuestConnectionStep4 step4;
    ig_GuestConnectionStep4_init(&step4);
    ig_GuestConnectionStep4_decode(step4Bytes, (size_t)step4Len, &step4, 0);
    if (!ig_GuestConnectionStep4_is_valid(&step4) || !step4.success) {
        ig_GuestConnectionStep4_deinit(&step4);
        return false;
    }

    ig_GuestConnectionStep4_deinit(&step4);
    return true;
}


void igloohome_ble_lock_crypto_GuestConnection_endConnection(int connectionId) {
    endConnection(connectionId);
}

int igloohome_ble_lock_crypto_GuestConnection_encryptNative(int connectionId, uint8_t *jPlaintext, int plaintextLen, uint8_t **retBytes) {
    return encryptNative(connectionId, jPlaintext, plaintextLen, retBytes);
}

int igloohome_ble_lock_crypto_GuestConnection_decryptNative(int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes) {
    return decryptNative(connectionId, jMessage, messageLen, retBytes);
}

/*
void igloohome_ble_lock_crypto_GuestConnection_setRxNonce(JNIEnv *env, jobject pThis, jint connectionId, jbyteArray jNonce) {
    Connection *connection = getConnection(connectionId);
    if (!connection)
        return;

    jsize nonceLen = env->GetArrayLength(jNonce);
    env->GetByteArrayRegion(jNonce, 0, nonceLen, (jbyte*)connection->rxNonce);
}*/

#ifdef __cplusplus
}
#endif
