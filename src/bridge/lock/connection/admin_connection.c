#include <bridge/lock/connection/admin_connection.h>
#include <bridge/lock/cifra/modes.h>
#include <bridge/lock/cifra/aes.h>
#include <bridge/lock/connection/encryption.h>
#include <bridge/lock/connection/connection_common.h>
#include <bridge/lock/messages/AdminConnectionStep1.h>
#include <bridge/lock/messages/AdminConnectionStep2.h>
#include <bridge/lock/messages/AdminConnectionStep3.h>
#include <stdio.h>
#include <bridge/bridge_main/log.h>

int igloohome_ble_lock_crypto_AdminConnection_beginConnection(uint8_t *jKey, int keyLen) {
  return beginConnection(kConnectionTypeAdmin, jKey, keyLen);
}

int igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native(int connectionId, 
		uint8_t *jConnectionStep1, int step1Len, uint8_t **retBytes) {
	Connection *connection = getConnection(connectionId);
	if (!connection) {
		serverLog(LL_ERROR, "!connection");
		return 0;
	}

	if (!jConnectionStep1) {
		serverLog(LL_ERROR, "!jConnectionStep1" );
		return 0;
	}

	uint8_t step1Bytes[step1Len];
	memcpy(step1Bytes, jConnectionStep1, step1Len);

	IgAdminConnectionStep1 step1;
	ig_AdminConnectionStep1_init(&step1);
	IgSerializerError step1_err = ig_AdminConnectionStep1_decode( step1Bytes, (size_t)step1Len, &step1, 0);
	if (step1_err || !ig_AdminConnectionStep1_is_valid(&step1) || step1.nonce_size != kNonceLength) {
		ig_AdminConnectionStep1_deinit(&step1);
		serverLog(LL_ERROR, "step1_err");
		return 0;
	}

	memcpy(connection->txNonce, step1.nonce, step1.nonce_size);

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
		serverLog(LL_ERROR, "ig_AdminConnectionStep2_encode error");
		return 0;
	}

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
		ig_AdminConnectionStep1_deinit(&step1);
		ig_AdminConnectionStep2_deinit(&step2);
		free(retvalBytes);
		serverLog(LL_ERROR, "incrementNonce error");
		return 0;
	}
	ig_AdminConnectionStep1_deinit(&step1);
	ig_AdminConnectionStep2_deinit(&step2);
	*retBytes = retvalBytes;
	return retvalLen;
}

bool igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(
		int connectionId, uint8_t *jConnectionStep3, int messageLen)
{
	Connection *connection = getConnection(connectionId);
	if (!connection)
		return false;
	if (!jConnectionStep3)
		return false;
	uint8_t *messageBytes = jConnectionStep3;

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
		return false;
	}

	IgAdminConnectionStep3 step3;
	ig_AdminConnectionStep3_init(&step3);
	ig_AdminConnectionStep3_decode(step3Bytes, (size_t)step3Len, &step3, 0);
	if (!ig_AdminConnectionStep3_is_valid(&step3) || !step3.success) {
		ig_AdminConnectionStep3_deinit(&step3);
		return false;
	}

	ig_AdminConnectionStep3_deinit(&step3);
	return true;
}

int AdminConnection_encryptNative(
		int connectionId, uint8_t *jPlaintext, int plaintextLen, uint8_t **retBytes) {
	return encryptNative(connectionId, jPlaintext, plaintextLen, retBytes);
}

int AdminConnection_decryptNative(
		int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes) {
	return decryptNative(connectionId, jMessage, messageLen, retBytes);
}

void AdminConnection_endConnection(int connectionId) {
	endConnection(connectionId);
}



