#include "igloo.h"

#include <string.h>
//#include "components/libraries/util/app_error.h"
//#include "components/softdevice/common/softdevice_handler/softdevice_handler.h"
//#include "components/libraries/timer/app_timer.h"
//#include "components/boards/boards.h"
//#include "components/drivers_nrf/delay/nrf_delay.h"
#include "cifra/aes.h"
#include "cifra/modes.h"
#include "cifra/hmac.h"
#include "cifra/sha2.h"

//#define NRF_LOG_ENABLED
// #define NRF_LOG_MODULE_NAME "APP"
// #include "components/libraries/log/nrf_log.h"
// #include "components/libraries/log/nrf_log_ctrl.h"
// #include "components/libraries/crypto/nrf_crypto.h"

#include "serializer.h"
/*
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/messages/AdminConnectionStep1.h"
#include "bridge/lock/messages/AdminConnectionStep2.h"
#include "bridge/lock/messages/AdminConnectionStep3.h"
#include "bridge/lock/messages/GuestConnectionStep1.h"
#include "bridge/lock/messages/GuestConnectionStep2.h"
#include "bridge/lock/messages/GuestConnectionStep3.h"
#include "bridge/lock/messages/GuestConnectionStep4.h"
#include "bridge/lock/messages/GuestToken.h"
*/

#include "messages/PairingStep1.h"
#include "messages/PairingStep2.h"
#include "messages/PairingStep3.h"
#include "messages/PairingStep4.h"
#include "messages/PairingCommit.h"
#include "messages/AdminConnectionStep1.h"
#include "messages/AdminConnectionStep2.h"
#include "messages/AdminConnectionStep3.h"
#include "messages/GuestConnectionStep1.h"
#include "messages/GuestConnectionStep2.h"
#include "messages/GuestConnectionStep3.h"
#include "messages/GuestConnectionStep4.h"
#include "messages/GuestToken.h"
#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define kMaxConnections 64
#define kNonceLength 12
#define kCcmTagLength 8
#define kCcmInternalCounterLength 3
#define kDstArrayLen 256
#define kSha256Len 32

static uint8_t pairing_admin_key[IG_KEY_LENGTH];
static uint8_t pairing_rx_nonce[kNonceLength];
static uint8_t pairing_tx_nonce[kNonceLength];
static uint8_t pairing_pin_key[IG_PIN_KEY_LENGTH];
static uint8_t pairing_password[IG_PASSWORD_LENGTH];
static uint32_t pairing_master_pin_length;
static uint8_t pairing_master_pin[IG_MASTER_PIN_MAX_LENGTH];
static int32_t pairing_gmt_offset;
static uint32_t pairing_dst_len;
static uint8_t pairing_dst[kDstArrayLen];

static void init_connections(void);
static void increment_nonce(uint8_t *nonce);
static int decrypt_data(uint8_t *aes_key, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written); 
static void encrypt_data(uint8_t *aes_key, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written);

// void ig_init() {
// 	nrf_crypto_init();
// 	nrf_crypto_rng_init();
// 	init_connections();
// }

// void ig_begin_pairing() {
// 	memset(pairing_admin_key, 0x00, IG_KEY_LENGTH);
// 	memset(pairing_pin_key, 0x00, IG_PIN_KEY_LENGTH);
// 	memset(pairing_password, 0x00, IG_PASSWORD_LENGTH);
// 	pairing_master_pin_length = 0;
// 	memset(pairing_master_pin, 0x00, IG_MASTER_PIN_MAX_LENGTH);
// }

// uint32_t ig_pairing_step2_size() {
// 	IgPairingStep2 step2;
// 	ig_PairingStep2_init(&step2);
// 	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// 	ig_PairingStep2_set_nonce(&step2, nonce, kNonceLength);
// 	uint8_t public_key[IG_KEY_EXCHANGE_PUBLIC_LENGTH] = {0x0};
// 	ig_PairingStep2_set_public_key(&step2, public_key, IG_KEY_EXCHANGE_PUBLIC_LENGTH);
// 	size_t step2_size = ig_PairingStep2_get_max_payload_in_bytes(&step2);

// 	ig_PairingStep2_deinit(&step2);
// 	return step2_size;
// }

// 生成步骤的运用
// data_in 传入的 step1 的数据, 而非带有长度的数据
// data_in_len 传入的 是 ste1 的长度, 
// data_out 我们输出的长度
// data_out_len 输出的buffer 的长度
// bytes_written 实际使用的长度有多少
IgErrorCode ig_pairing_step2(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	IgPairingStep1 step1;
// 	ig_PairingStep1_init(&step1);
// 	IgSerializerError step1_err = ig_PairingStep1_decode(data_in, data_in_len, &step1, 0);
// 	if (step1_err || !ig_PairingStep1_is_valid(&step1)) {
// 		ig_PairingStep1_deinit(&step1);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	if (ig_PairingStep1_get_public_key_size(&step1) != IG_KEY_EXCHANGE_PUBLIC_LENGTH) {
// 		ig_PairingStep1_deinit(&step1);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(public_key2, SECP256R1);
// 	memcpy(public_key2.p_value, step1.public_key, step1.public_key_size);
// 	// TEST: hardcoded phone public key
// //    uint8_t hardcoded_public_key2[IG_KEY_EXCHANGE_PUBLIC_LENGTH] = {
// //            0x5E, 0x64, 0x8A, 0x6C, 0x7C, 0x8E, 0x81, 0x3C,
// //            0x04, 0xAB, 0xF3, 0x4C, 0x45, 0x1C, 0x9B, 0x14,
// //            0x7D, 0xD6, 0x3B, 0x3B, 0xA5, 0xC1, 0x3B, 0xE4,
// //            0x66, 0x94, 0xCD, 0xE5, 0x24, 0x98, 0x78, 0x2B,
// //            0x7C, 0x3F, 0xA2, 0x77, 0x19, 0xFB, 0x85, 0x67,
// //            0xE5, 0x01, 0x1A, 0xD2, 0x86, 0xE5, 0xF5, 0xCF,
// //            0xE5, 0xD2, 0x3B, 0x69, 0x80, 0x32, 0xA9, 0x80,
// //            0x45, 0x65, 0x27, 0x4B, 0x1E, 0xC4, 0xE3, 0x47
// //    };
// //	memcpy(public_key2.p_value, hardcoded_public_key2, sizeof(hardcoded_public_key2));
// 	// END TEST


// 	IgPairingStep2 step2;
// 	ig_PairingStep2_init(&step2);
// 	// random nonce
// 	nrf_crypto_rng_vector_generate(pairing_rx_nonce, kNonceLength);
// 	ig_PairingStep2_set_nonce(&step2, pairing_rx_nonce, kNonceLength);
// 	// TEST: hardcoded nonce
// //	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// //	memset(nonce, 0x0C, kNonceLength);
// //	memcpy(pairing_rx_nonce, nonce, kNonceLength);
// //	ig_PairingStep2_set_nonce(&step2, pairing_rx_nonce, kNonceLength);
// 	// END TEST


// 	// generate random private and public keys
// 	NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE(private_key1, SECP256R1);
// 	NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(public_key1, SECP256R1);
// 	nrf_crypto_ecc_key_pair_generate(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key1, &public_key1);
// 	ig_PairingStep2_set_public_key(&step2, public_key1.p_value, IG_KEY_EXCHANGE_PUBLIC_LENGTH);
	
// 	// TEST: hardcoded lock public/private key
// //    uint8_t private_key_test[IG_KEY_EXCHANGE_PRIVATE_LENGTH] = {
// //            0x66, 0x29, 0xED, 0xCE, 0xF5, 0x5A, 0xF2, 0x10,
// //            0x86, 0xC2, 0x9C, 0x77, 0x22, 0x07, 0x11, 0xC2,
// //            0x49, 0xF6, 0x51, 0xAA, 0x2D, 0x4C, 0x02, 0x2D,
// //            0xFB, 0x42, 0xDB, 0xA6, 0xBC, 0x7E, 0x18, 0xC1
// //    };
// //    uint8_t public_key_test[IG_KEY_EXCHANGE_PUBLIC_LENGTH] = {
// //            0x6B, 0x41, 0xE6, 0x98, 0xF7, 0x16, 0x1B, 0x48,
// //            0x64, 0xDB, 0x87, 0xBC, 0x50, 0x5F, 0x6E, 0x3B,
// //            0xB0, 0x69, 0x96, 0x9B, 0x27, 0x51, 0xD8, 0x9D,
// //            0xAE, 0xC4, 0x8A, 0x2E, 0xE7, 0x16, 0x15, 0x8B,
// //            0x24, 0xDC, 0x49, 0xA5, 0x9B, 0x91, 0x50, 0xAB,
// //            0x0F, 0xCB, 0xE3, 0x03, 0x0B, 0x64, 0xFB, 0x85,
// //            0x84, 0x73, 0x03, 0xA3, 0x7E, 0xC3, 0x27, 0x05,
// //            0x6B, 0xA4, 0xE1, 0x32, 0x1B, 0x4F, 0xE8, 0xB0
// //    };
// //	memcpy(private_key1.p_value, private_key_test, private_key1.length);
// //	memcpy(public_key1.p_value, public_key_test, public_key1.length);
// //	ig_PairingStep2_set_public_key(&step2, public_key_test, IG_KEY_EXCHANGE_PUBLIC_LENGTH);
// 	// END TEST

	
// 	// TEST
// //	NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE(private_key1, SECP256R1);
// //	NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(public_key1, SECP256R1);
// //	NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE(private_key2, SECP256R1);
// //	NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(public_key2, SECP256R1);
// //	nrf_crypto_ecc_key_pair_generate(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key1, &public_key1);
// //	nrf_crypto_ecc_key_pair_generate(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key2, &public_key2);
// //	NRF_LOG_INFO("set 1\n");
// //	NRF_LOG_HEXDUMP_INFO(private_key1.p_value, private_key1.length);
// //	NRF_LOG_HEXDUMP_INFO(public_key1.p_value, public_key1.length);
// //	for (int i = 0; i < 1000000; i++) {volatile uint8_t a = 2+i;}
// //	NRF_LOG_INFO("set 2\n");
// //	NRF_LOG_HEXDUMP_INFO(private_key2.p_value, private_key2.length);
// //	NRF_LOG_HEXDUMP_INFO(public_key2.p_value, public_key2.length);
// //	ig_PairingStep2_set_public_key(&step2, public_key1.p_value, IG_KEY_EXCHANGE_PK_LENGTH);

// 	size_t step2_size = ig_PairingStep2_get_max_payload_in_bytes(&step2);
// 	if (data_out_len < step2_size) {
// 		ig_PairingStep1_deinit(&step1);
// 		ig_PairingStep2_deinit(&step2);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}

// 	size_t step2_written_bytes = 0;
// 	IgSerializerError err = ig_PairingStep2_encode(&step2, data_out, data_out_len, &step2_written_bytes);
// 	if (err != IgSerializerNoError) {
// 		ig_PairingStep1_deinit(&step1);
// 		ig_PairingStep2_deinit(&step2);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}
// 	*bytes_written = (uint32_t)step2_written_bytes;
	
// 	// calculate ECDH shared secret from both private keys
// 	NRF_CRYPTO_ECDH_SHARED_SECRET_INSTANCE_CREATE(shared_secret, SECP256R1);
// 	nrf_crypto_ecdh_shared_secret_compute(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key1, &public_key2, &shared_secret);

// 	// TODO: calculate SHA256 hash
// 	uint8_t hashed_shared_secret[IG_KEY_EXCHANGE_PRIVATE_LENGTH];
// 	memcpy(hashed_shared_secret, shared_secret.p_value, shared_secret.length);

// 	// truncate to get admin key
// 	memcpy(pairing_admin_key, hashed_shared_secret, IG_KEY_LENGTH);

// 	// TEST: calculate admin key from both private keys
// //	uint32_t result = 1;
// //	uint32_t shared_secret_size; 
// //	result = nrf_crypto_ecdh_shared_secret_size_get(NRF_CRYPTO_CURVE_SECP256R1, &shared_secret_size);
// //	NRF_CRYPTO_ECDH_SHARED_SECRET_INSTANCE_CREATE(shared_secret, SECP256R1);
// //	nrf_crypto_ecdh_shared_secret_compute(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key1, &public_key2, &shared_secret);
// //	NRF_CRYPTO_ECDH_SHARED_SECRET_INSTANCE_CREATE(shared_secret2, SECP256R1);
// //	nrf_crypto_ecdh_shared_secret_compute(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &private_key2, &public_key1, &shared_secret2);
// //	NRF_LOG_INFO("admin key 1, len = %i\n", shared_secret.length);
// //	NRF_LOG_HEXDUMP_INFO(shared_secret.p_value, shared_secret.length);
// //	NRF_LOG_INFO("admin key 2, len = %i\n", shared_secret2.length);
// //	NRF_LOG_HEXDUMP_INFO(shared_secret2.p_value, shared_secret2.length);

// 	ig_PairingStep1_deinit(&step1);
// 	ig_PairingStep2_deinit(&step2);
	return IG_ERROR_NONE;
}

uint32_t ig_pairing_step4_size() {
	IgPairingStep4 step4;
	ig_PairingStep4_init(&step4);
	ig_PairingStep4_set_success(&step4, true);
	ig_PairingStep4_set_pin_key(&step4, pairing_pin_key, sizeof(pairing_pin_key));
	ig_PairingStep4_set_password(&step4, pairing_password, sizeof(pairing_password));
	ig_PairingStep4_set_master_pin(&step4, pairing_master_pin, sizeof(pairing_master_pin));
	size_t step4_size = ig_PairingStep4_get_max_payload_in_bytes(&step4);

	ig_PairingStep4_deinit(&step4);
	return step4_size;
}

// IgErrorCode ig_pairing_step4(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	// decrypt key with calculated admin key
// 	uint32_t decrypted_data_in_len = ig_decrypt_data_size(data_in_len);
// 	uint8_t decrypted_data_in[decrypted_data_in_len];
// 	uint32_t decrypted_bytes_written = 0;
// 	int decrypt_result = decrypt_data(pairing_admin_key, data_in, data_in_len, decrypted_data_in, decrypted_data_in_len, pairing_rx_nonce, &decrypted_bytes_written);
// 	if (decrypt_result) {
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	increment_nonce(pairing_rx_nonce);
// 	// TEST: unencrypted
// //	uint8_t *decrypted_data_in = data_in;
// //	uint32_t decrypted_data_len = data_in_len;

// 	// decode step 3
// 	IgPairingStep3 step3;
// 	ig_PairingStep3_init(&step3);
// 	ig_PairingStep3_decode(decrypted_data_in, decrypted_bytes_written, &step3, 0);

// 	// validate data
// 	if (!ig_PairingStep3_is_valid(&step3) ||
// 		(step3.has_nonce && ig_PairingStep3_get_nonce_size(&step3) != kNonceLength) ||
// 		(step3.has_pin_key && ig_PairingStep3_get_pin_key_size(&step3) != IG_PIN_KEY_LENGTH) ||
// 		(step3.has_password && ig_PairingStep3_get_password_size(&step3) != IG_PASSWORD_LENGTH) ||
// 		(step3.has_master_pin && ig_PairingStep3_get_master_pin_size(&step3) > IG_MASTER_PIN_MAX_LENGTH) ||
// 		(step3.has_master_pin && ig_PairingStep3_get_master_pin_size(&step3) < IG_MASTER_PIN_MIN_LENGTH)) 
// 	{
// 		ig_PairingStep3_deinit(&step3);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}


// 	// copy key or generate random
// 	if (step3.has_pin_key) {
// 		memcpy(pairing_pin_key, step3.pin_key, ig_PairingStep3_get_pin_key_size(&step3));
// 	}
// 	else {
// 		nrf_crypto_rng_vector_generate(pairing_pin_key, sizeof(pairing_pin_key));
// 		// TEST: hardcoded value
// //		memset(pairing_pin_key, 0x2, sizeof(pairing_pin_key));
// 	}
// 	if (step3.has_password) {
// 		memcpy(pairing_password, step3.password, ig_PairingStep3_get_password_size(&step3));
// 	}
// 	else {
// 		nrf_crypto_rng_vector_generate(pairing_password, sizeof(pairing_password));
// 		// TEST: hardcoded value
// //		memset(pairing_password, 0x3, sizeof(pairing_password));
// 	}
// 	if (step3.has_master_pin && 
// 			ig_PairingStep3_get_master_pin_size(&step3) >= IG_MASTER_PIN_MIN_LENGTH && 
// 			ig_PairingStep3_get_master_pin_size(&step3) <= IG_MASTER_PIN_MAX_LENGTH) {
// 		pairing_master_pin_length = ig_PairingStep3_get_master_pin_size(&step3);
// 		memcpy(pairing_master_pin, step3.master_pin, pairing_master_pin_length);
// 	}
// 	else {
// 		pairing_master_pin_length = IG_MASTER_PIN_DEFAULT_LENGTH;
// 		nrf_crypto_rng_vector_generate(pairing_master_pin, IG_MASTER_PIN_DEFAULT_LENGTH);
// 		for (int i = 0; i < IG_MASTER_PIN_DEFAULT_LENGTH; ++i) {
// 			pairing_master_pin[i] = pairing_master_pin[i] % 10;
// 			pairing_master_pin[i] = pairing_master_pin[i] + 0x30;
// 		}
// 	}
// 	memcpy(pairing_tx_nonce, step3.nonce, step3.nonce_size);
// 	pairing_gmt_offset = step3.has_gmt_offset ? step3.gmt_offset : 0;
// 	if (step3.has_dst_times) {
// 	   	if (ig_PairingStep3_get_dst_times_size(&step3) <= kDstArrayLen) {
// 			pairing_dst_len = ig_PairingStep3_get_dst_times_size(&step3);
// 			memcpy(pairing_dst, step3.dst_times, pairing_dst_len);
// 		}
// 		else {
// 			return IG_ERROR_INTERNAL_ERROR;
// 		}
// 	}
// 	else {
// 		pairing_dst_len = 0;
// 	}

// 	IgPairingStep4 step4;
// 	ig_PairingStep4_init(&step4);
// 	ig_PairingStep4_set_success(&step4, true);
// 	ig_PairingStep4_set_pin_key(&step4, pairing_pin_key, sizeof(pairing_pin_key));
// 	ig_PairingStep4_set_password(&step4, pairing_password, sizeof(pairing_password));
// 	ig_PairingStep4_set_master_pin(&step4, pairing_master_pin, pairing_master_pin_length);
// 	ig_PairingStep4_set_gmt_offset(&step4, pairing_gmt_offset);

// 	size_t step4_size = ig_PairingStep4_get_max_payload_in_bytes(&step4);
// 	if (data_out_len < step4_size) {
// 		ig_PairingStep3_deinit(&step3);
// 		ig_PairingStep4_deinit(&step4);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}

// 	uint8_t step4_serialized[step4_size];
// 	size_t step4_written_bytes = 0;
// 	IgSerializerError err = ig_PairingStep4_encode(&step4, step4_serialized, step4_size, &step4_written_bytes);
// 	if (err != IgSerializerNoError) {
// 		ig_PairingStep3_deinit(&step3);
// 		ig_PairingStep4_deinit(&step4);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}

// 	// encrypt message
// 	uint32_t encrypted_bytes_written = 0;
// 	encrypt_data(pairing_admin_key, step4_serialized, step4_written_bytes, data_out, data_out_len, pairing_tx_nonce, &encrypted_bytes_written);
// 	*bytes_written = encrypted_bytes_written;
// 	// TEST: unencrypted message
// //	memcpy(data_out, step4_serialized, step4_written_bytes);
// //	*bytes_written = (uint32_t)step4_written_bytes;
	
// 	ig_PairingStep3_deinit(&step3);
// 	ig_PairingStep4_deinit(&step4);
// 	return IG_ERROR_NONE;
// }

// IgErrorCode ig_commit_pairing(uint8_t *data_in, uint32_t data_in_len, IgPairingResult *data_out) {
// 	// decrypt key with calculated admin key
// 	uint32_t decrypted_data_in_len = ig_decrypt_data_size(data_in_len);
// 	uint8_t decrypted_data_in[decrypted_data_in_len];
// 	uint32_t decrypted_bytes_written = 0;
// 	decrypt_data(pairing_admin_key, data_in, data_in_len, decrypted_data_in, decrypted_data_in_len, pairing_rx_nonce, &decrypted_bytes_written);
// 	increment_nonce(pairing_rx_nonce);
// 	// TEST: unencrpyted version
// //	uint8_t *decrypted_data_in = data_in;
// //	uint32_t decrypted_data_len = data_len;

// 	IgPairingCommit pairing_commit;
// 	ig_PairingCommit_init(&pairing_commit);
// 	ig_PairingCommit_decode(decrypted_data_in, decrypted_data_in_len, &pairing_commit, 0);
// 	if (!ig_PairingCommit_is_valid(&pairing_commit)) {
// 		ig_PairingCommit_deinit(&pairing_commit);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
	
// 	data_out->current_time = pairing_commit.current_time;
// 	memcpy(data_out->admin_key, pairing_admin_key, IG_KEY_LENGTH);
// 	memcpy(data_out->pin_key, pairing_pin_key, IG_PIN_KEY_LENGTH);
// 	memcpy(data_out->password, pairing_password, IG_PASSWORD_LENGTH);
// 	data_out->master_pin_length = pairing_master_pin_length;
// 	memcpy(data_out->master_pin, pairing_master_pin, pairing_master_pin_length);
// 	data_out->gmt_offset = pairing_gmt_offset;
// 	data_out->dst = pairing_dst;
// 	data_out->dst_length = pairing_dst_len;

// 	ig_PairingCommit_deinit(&pairing_commit);
// 	return IG_ERROR_NONE;
// }

// void ig_end_pairing() {
// 	memset(pairing_admin_key, 0x00, IG_KEY_LENGTH);
// 	memset(pairing_pin_key, 0x00, IG_PIN_KEY_LENGTH);
// 	memset(pairing_password, 0x00, IG_PASSWORD_LENGTH);
// 	pairing_master_pin_length = 0;
// 	memset(pairing_master_pin, 0x00, IG_MASTER_PIN_MAX_LENGTH);
// 	pairing_dst_len = 0;
// 	memset(pairing_dst, 0x00, kDstArrayLen); 
// }

// int ig_debug_get_pairing_state() {
// 	return 911555;
// }

// // --------------------------------------------------------------
// typedef struct ConnectionEntry {
// 	uint32_t conn_id;
// 	bool active;
// 	IgConnectionCharacteristicType type;
// 	uint8_t tx_nonce[kNonceLength];
// 	uint8_t rx_nonce[kNonceLength];
// 	uint8_t aes_key[IG_KEY_LENGTH];
// } ConnectionEntry;
// static ConnectionEntry connections_[kMaxConnections];
// static uint8_t admin_key[IG_KEY_LENGTH];

// static void init_connections() {
// 	memset(connections_, 0, sizeof(connections_));
// }

// void ig_set_admin_key(uint8_t key[IG_KEY_LENGTH]) {
// 	memcpy(admin_key, key, IG_KEY_LENGTH);
// }

// static uint32_t ig_begin_connection(IgConnectionCharacteristicType type, IgConnectionScratchpad *scratchpad) {
// 	// find free slot for connection
// 	int free_idx = -1;
// 	for (int i = 0; i < kMaxConnections; ++i) {
// 		if (!connections_[i].active) {
// 			free_idx = i;
// 			break;
// 		}
// 	}
	
// 	if (free_idx == -1) {
// 		return IG_ERROR_MAX_CONNECTIONS;
// 	}

// 	connections_[free_idx].active = true;
// 	memset(&(connections_[free_idx].tx_nonce), 0x0, sizeof(connections_[free_idx].tx_nonce));
// 	memset(&(connections_[free_idx].rx_nonce), 0x0, sizeof(connections_[free_idx].rx_nonce));
// 	connections_[free_idx].type = type;
// 	connections_[free_idx].conn_id = free_idx;
// 	if (type == IG_CONNECTION_TYPE_ADMIN)
// 		memcpy(connections_[free_idx].aes_key, admin_key, sizeof(admin_key));
// 	else
// 		memset(connections_[free_idx].aes_key, 0x0, sizeof(connections_[free_idx].aes_key));

// 	return free_idx;
// }

// static bool ig_end_connection(IgConnectionCharacteristicType type, uint32_t conn_id) {
// 	if (conn_id >= kMaxConnections) 
// 		return false;
// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return false;
// 	if (connection_entry->type != type) 
// 		return false;

// 	connections_[conn_id].active = false;
// 	return true;
// }

// uint32_t ig_admin_begin_connection(IgConnectionScratchpad *scratchpad) {
// 	return ig_begin_connection(IG_CONNECTION_TYPE_ADMIN, scratchpad);
// }

// uint32_t ig_admin_connect_step1_size() {
// 	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// 	IgAdminConnectionStep1 step1;
// 	ig_AdminConnectionStep1_init(&step1);
// 	ig_AdminConnectionStep1_set_nonce(&step1, nonce, kNonceLength);
// 	size_t step1_size = ig_AdminConnectionStep1_get_max_payload_in_bytes(&step1);
// 	ig_AdminConnectionStep1_deinit(&step1);
// 	return step1_size;
// }

// IgErrorCode ig_admin_connect_step1(uint32_t conn_id, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;

// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (connection_entry->type != IG_CONNECTION_TYPE_ADMIN)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	// random nonce
// 	nrf_crypto_rng_vector_generate(connection_entry->rx_nonce, kNonceLength);
// 	// TEST: hardcoded nonce
// //	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// //	memcpy(connection_entry->rx_nonce, nonce, kNonceLength);

// 	IgAdminConnectionStep1 step1;
// 	ig_AdminConnectionStep1_init(&step1);
// 	ig_AdminConnectionStep1_set_nonce(&step1, connection_entry->rx_nonce, kNonceLength);
// 	size_t step1_size = ig_AdminConnectionStep1_get_max_payload_in_bytes(&step1);
// 	if (data_out_len < step1_size) {
// 		ig_AdminConnectionStep1_deinit(&step1);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}
// 	size_t step1_bytes_written = 0;
// 	IgSerializerError err = ig_AdminConnectionStep1_encode(&step1, data_out, data_out_len, &step1_bytes_written);
// 	if (err != IgSerializerNoError) {
// 		ig_AdminConnectionStep1_deinit(&step1);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}
// 	*bytes_written = step1_bytes_written;

// 	ig_AdminConnectionStep1_deinit(&step1);
// 	return IG_ERROR_NONE;
// }

// uint32_t ig_admin_connect_step3_size() {
// 	IgAdminConnectionStep3 step3;
// 	ig_AdminConnectionStep3_init(&step3);
// 	ig_AdminConnectionStep3_set_success(&step3, true);
// 	size_t step3_size = ig_AdminConnectionStep3_get_max_payload_in_bytes(&step3);
// 	ig_AdminConnectionStep3_deinit(&step3);
// 	// TEST: unencrypted size
// //	return step3_size;

// 	uint32_t encrypted_size = ig_encrypt_data_size(step3_size);
// 	return encrypted_size;
// }

// IgErrorCode ig_admin_connect_step3(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;

// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (connection_entry->type != IG_CONNECTION_TYPE_ADMIN)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	// decrypt
// 	uint32_t decrypted_data_len = ig_decrypt_data_size(data_in_len);
// 	uint8_t decrypted_data_in[decrypted_data_len];
// 	uint32_t decrypted_bytes_written = 0;
// 	IgErrorCode decrypt_err = ig_decrypt_data(conn_id, data_in, data_in_len, decrypted_data_in, decrypted_data_len, &decrypted_bytes_written);
// 	if (decrypt_err != IG_ERROR_NONE) {
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}

// 	// decode step 2
// 	IgAdminConnectionStep2 step2;
// 	ig_AdminConnectionStep2_init(&step2);
// 	ig_AdminConnectionStep2_decode(decrypted_data_in, decrypted_bytes_written, &step2, 0);
// 	if (!ig_AdminConnectionStep2_is_valid(&step2) || step2.nonce_size != kNonceLength) {
// 		ig_AdminConnectionStep2_deinit(&step2);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	memcpy(connection_entry->tx_nonce, step2.nonce, step2.nonce_size);

// 	// create step 3
// 	IgAdminConnectionStep3 step3;
// 	ig_AdminConnectionStep3_init(&step3);
// 	ig_AdminConnectionStep3_set_success(&step3, true);
// 	size_t step3_size = ig_AdminConnectionStep3_get_max_payload_in_bytes(&step3);
// 	uint8_t step3_bytes[step3_size];
// 	size_t step3_bytes_written = 0;
// 	if (data_out_len < step3_size) {
// 		ig_AdminConnectionStep2_deinit(&step2);
// 		ig_AdminConnectionStep3_deinit(&step3);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}
// 	IgSerializerError encode_err = ig_AdminConnectionStep3_encode(&step3, step3_bytes, step3_size, &step3_bytes_written);
// 	if (encode_err != IgSerializerNoError) {
// 		ig_AdminConnectionStep2_deinit(&step2);
// 		ig_AdminConnectionStep3_deinit(&step3);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}

// 	// encrypt
// 	IgErrorCode encrypt_err = ig_encrypt_data(conn_id, step3_bytes, step3_bytes_written, data_out, data_out_len, bytes_written);
// 	if (encrypt_err != IgSerializerNoError) {
// 		ig_AdminConnectionStep2_deinit(&step2);
// 		ig_AdminConnectionStep3_deinit(&step3);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}

// 	ig_AdminConnectionStep2_deinit(&step2);
// 	ig_AdminConnectionStep3_deinit(&step3);
// 	return IG_ERROR_NONE;
// }

// bool ig_admin_end_connection(uint32_t conn_id) {
// 	return ig_end_connection(IG_CONNECTION_TYPE_ADMIN, conn_id);
// }

// uint32_t ig_guest_begin_connection(IgConnectionScratchpad *scratchpad) {
// 	return ig_begin_connection(IG_CONNECTION_TYPE_GUEST, scratchpad);
// }

// uint32_t ig_guest_connect_step2_size(void) {
// 	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// 	IgGuestConnectionStep2 step2;
// 	ig_GuestConnectionStep2_init(&step2);
// 	ig_GuestConnectionStep2_set_nonce(&step2, nonce, kNonceLength);
// 	size_t step2_size = ig_GuestConnectionStep2_get_max_payload_in_bytes(&step2);
// 	ig_GuestConnectionStep2_deinit(&step2);
// 	return step2_size;
// }

// IgErrorCode ig_guest_connect_step2(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written, IgGuestKeyParams *guest_key_params) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (!guest_key_params)
// 		return IG_ERROR_INVALID_PARAMS;

// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (connection_entry->type != IG_CONNECTION_TYPE_GUEST)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	IgGuestConnectionStep1 step1;
// 	ig_GuestConnectionStep1_init(&step1);
// 	ig_GuestConnectionStep1_decode(data_in, data_in_len, &step1, 0);
// 	if (!ig_GuestConnectionStep1_is_valid(&step1)) {
// 		ig_GuestConnectionStep1_deinit(&step1);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}

// 	// decrypt token
// 	uint32_t token_ciphertext_size = step1.token_size - kNonceLength;
// 	uint32_t decrypted_token_size = ig_decrypt_data_size(token_ciphertext_size);
// 	uint8_t decrypted_token[decrypted_token_size];
// 	uint32_t decrypted_token_bytes_written = 0;
// 	int token_decrypt_result = decrypt_data(admin_key, step1.token + kNonceLength, token_ciphertext_size, decrypted_token, decrypted_token_size, step1.token, &decrypted_token_bytes_written);
// 	if (token_decrypt_result) {
// 		ig_GuestConnectionStep1_deinit(&step1);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}

// 	IgGuestToken guest_token;
// 	ig_GuestToken_init(&guest_token);
// 	ig_GuestToken_decode(decrypted_token, decrypted_token_bytes_written, &guest_token, 0);
// 	if (!ig_GuestToken_is_valid(&guest_token) || 
// 			!guest_token.has_aes_key ||  guest_token.aes_key_size != IG_KEY_LENGTH || 
// 			!guest_token.has_access_rights || guest_token.access_rights_size != IG_ACCESS_RIGHTS_SIZE ||
// 			!guest_token.has_start_date || !guest_token.has_end_date) {
// 		ig_GuestConnectionStep1_deinit(&step1);
// 		ig_GuestToken_deinit(&guest_token);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	memcpy(guest_key_params->access_rights, guest_token.access_rights, sizeof(guest_key_params->access_rights));
// 	guest_key_params->start_time = guest_token.start_date;
// 	guest_key_params->end_time = guest_token.end_date;
// 	guest_key_params->key_id = guest_token.key_id;
// 	memcpy(connection_entry->aes_key, guest_token.aes_key, IG_KEY_LENGTH);

// 	// random nonce
// 	nrf_crypto_rng_vector_generate(connection_entry->rx_nonce, kNonceLength);
// 	// TEST: hardcoded nonce
// //	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// //	memcpy(connection_entry->rx_nonce, nonce, kNonceLength);

// 	// create response message
// 	IgGuestConnectionStep2 step2;
// 	ig_GuestConnectionStep2_init(&step2);
// 	ig_GuestConnectionStep2_set_nonce(&step2, connection_entry->rx_nonce, kNonceLength);
// 	size_t step2_size = ig_GuestConnectionStep2_get_max_payload_in_bytes(&step2);
// 	if (data_out_len < step2_size) {
// 		ig_GuestConnectionStep1_deinit(&step1);
// 		ig_GuestToken_deinit(&guest_token);
// 		ig_GuestConnectionStep2_deinit(&step2);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}
// 	size_t step2_bytes_written = 0;
// 	IgSerializerError err = ig_GuestConnectionStep2_encode(&step2, data_out, data_out_len, &step2_bytes_written);
// 	if (err != IgSerializerNoError) {
// 		ig_GuestConnectionStep1_deinit(&step1);
// 		ig_GuestToken_deinit(&guest_token);
// 		ig_GuestConnectionStep2_deinit(&step2);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}
// 	*bytes_written = step2_bytes_written;

// 	ig_GuestConnectionStep1_deinit(&step1);
// 	ig_GuestToken_deinit(&guest_token);
// 	ig_GuestConnectionStep2_deinit(&step2);
// 	return IG_ERROR_NONE;
// }

// uint32_t ig_guest_connect_step4_size(void) {
// 	IgGuestConnectionStep4 step4;
// 	ig_GuestConnectionStep4_init(&step4);
// 	ig_GuestConnectionStep4_set_success(&step4, true);
// 	size_t step4_size = ig_GuestConnectionStep4_get_max_payload_in_bytes(&step4);
// 	ig_GuestConnectionStep4_deinit(&step4);
// 	return step4_size;
// }

// IgErrorCode ig_guest_connect_step4(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;

// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (connection_entry->type != IG_CONNECTION_TYPE_GUEST)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	// decrypt
// 	uint32_t decrypted_data_len = ig_decrypt_data_size(data_in_len);
// 	uint8_t decrypted_data_in[decrypted_data_len];
// 	uint32_t decrypted_bytes_written = 0;
// 	IgErrorCode decrypt_err = ig_decrypt_data(conn_id, data_in, data_in_len, decrypted_data_in, decrypted_data_len, &decrypted_bytes_written);
// 	if (decrypt_err != IG_ERROR_NONE) {
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}

// 	// decode step 3
// 	IgGuestConnectionStep3 step3;
// 	ig_GuestConnectionStep3_init(&step3);
// 	ig_GuestConnectionStep3_decode(decrypted_data_in, decrypted_data_len, &step3, 0);
// 	if (!ig_GuestConnectionStep3_is_valid(&step3) || step3.nonce_size != kNonceLength) {
// 		ig_GuestConnectionStep3_deinit(&step3);
// 		return IG_ERROR_INVALID_MESSAGE;
// 	}
// 	memcpy(connection_entry->tx_nonce, step3.nonce, step3.nonce_size);

// 	// create step 4
// 	IgGuestConnectionStep4 step4;
// 	ig_GuestConnectionStep4_init(&step4);
// 	ig_GuestConnectionStep4_set_success(&step4, true);
// 	size_t step4_size = ig_GuestConnectionStep4_get_max_payload_in_bytes(&step4);
// 	uint8_t step4_bytes[step4_size];
// 	size_t step4_bytes_written = 0;
// 	if (data_out_len < step4_size) {
// 		ig_GuestConnectionStep3_deinit(&step3);
// 		ig_GuestConnectionStep4_deinit(&step4);
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	}
// 	IgSerializerError err = ig_GuestConnectionStep4_encode(&step4, step4_bytes, step4_size, &step4_bytes_written);
// 	if (err != IgSerializerNoError) {
// 		ig_GuestConnectionStep3_deinit(&step3);
// 		ig_GuestConnectionStep4_deinit(&step4);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}

// 	// encrypt
// 	IgErrorCode encrypt_err = ig_encrypt_data(conn_id, step4_bytes, step4_bytes_written, data_out, data_out_len, bytes_written);
// 	if (encrypt_err != IgSerializerNoError) {
// 		ig_GuestConnectionStep3_deinit(&step3);
// 		ig_GuestConnectionStep4_deinit(&step4);
// 		return IG_ERROR_GENERIC_FAIL;
// 	}

// 	return IG_ERROR_NONE;
// }

// bool ig_guest_end_connection(uint32_t conn_id) {
// 	return ig_end_connection(IG_CONNECTION_TYPE_GUEST, conn_id);
// }

// static void increment_nonce(uint8_t *nonce) {
// 	// nonce interpreted as big endian (most significant byte has lowest address/index)
// 	for (int i = kNonceLength-1; i >= 0; i--) {
// 		if (nonce[i] == 255) {
// 			nonce[i] = 0;
// 		}
// 		else {
// 			nonce[i]++;
// 			break;
// 		}
// 	}
// }

// uint32_t ig_encrypt_data_size(uint32_t data_len) {
// 	// TEST: non-encrypted version
// 	//return data_len;

// 	return data_len + kCcmTagLength;
// }

// static void encrypt_data(uint8_t *aes_key, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written)  {
// 	uint8_t tag[kCcmTagLength];
// 	uint8_t ciphertext[data_in_len];
// 	cf_aes_context aes_ctx;
// 	cf_aes_init(&aes_ctx, aes_key, IG_KEY_LENGTH); 
// 	cf_ccm_encrypt(&cf_aes, &aes_ctx, data_in, data_in_len, kCcmInternalCounterLength, NULL, 0, nonce, kNonceLength, ciphertext, tag, kCcmTagLength); 
// 	memcpy(data_out, ciphertext, data_in_len);
// 	memcpy(data_out+data_in_len, tag, kCcmTagLength);
// 	*bytes_written = ig_encrypt_data_size(data_in_len);
// 	cf_aes_finish(&aes_ctx);
// }

// IgErrorCode ig_encrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (data_in_len > 1024)
// 		return IG_ERROR_INVALID_PARAMS;
// 	if (data_out_len < ig_encrypt_data_size(data_in_len))
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	encrypt_data(connection_entry->aes_key, data_in, data_in_len, data_out, data_out_len, connections_[conn_id].tx_nonce, bytes_written);
// 	increment_nonce(connections_[conn_id].tx_nonce);

// 	//NRF_LOG_HEXDUMP_INFO(data_out, *bytes_written);
// 	//NRF_LOG_HEXDUMP_INFO(ciphertext, data_in_len);
// 	//NRF_LOG_HEXDUMP_INFO(tag, kCcmTagLength);

// 	// TEST: non-encrypted version
// //	memcpy(data_out, data_in, data_in_len);
// //	*bytes_written = data_in_len;

// 	return IG_ERROR_NONE;
// }

// uint32_t ig_decrypt_data_size(uint32_t data_len) {
// 	// TEST: non-encrypted version
// //	return data_len;

// 	return data_len - kCcmTagLength;
// }

//void decrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written)  {
//	// TEST function
//	ConnectionEntry *connection_entry = &(connections_[conn_id]);
//	uint8_t *ciphertext = data_in;
//	size_t ciphertext_len = data_in_len - kCcmTagLength;
//	//NRF_LOG_HEXDUMP_INFO(ciphertext, ciphertext_len);
//	uint8_t *tag = data_in + ciphertext_len;
//	size_t tag_len = kCcmTagLength;
//	//NRF_LOG_HEXDUMP_INFO(tag, tag_len);
//	cf_aes_context aes_ctx;
//	cf_aes_init(&aes_ctx, connection_entry->aes_key, sizeof(connection_entry->aes_key)); 
//	int result = cf_ccm_decrypt(&cf_aes, &aes_ctx, ciphertext, ciphertext_len, kCcmInternalCounterLength, NULL, 0, nonce, kNonceLength, tag, tag_len, data_out);
//	//NRF_LOG_INFO("result = %i\n", result);
//	*bytes_written = ciphertext_len;
//	cf_aes_finish(&aes_ctx);
//}

// static int decrypt_data(uint8_t *aes_key, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written)  {
// 	uint8_t *ciphertext = data_in;
// 	size_t ciphertext_len = data_in_len - kCcmTagLength;
// 	//NRF_LOG_HEXDUMP_INFO(ciphertext, ciphertext_len);
// 	uint8_t *tag = data_in + ciphertext_len;
// 	size_t tag_len = kCcmTagLength;
// 	//NRF_LOG_HEXDUMP_INFO(tag, tag_len);
// 	cf_aes_context aes_ctx;
// 	cf_aes_init(&aes_ctx, aes_key, IG_KEY_LENGTH); 
// 	int result = cf_ccm_decrypt(&cf_aes, &aes_ctx, ciphertext, ciphertext_len, kCcmInternalCounterLength, NULL, 0, nonce, kNonceLength, tag, tag_len, data_out);
// 	//NRF_LOG_INFO("result = %i\n", result);
// 	*bytes_written = ciphertext_len;
// 	cf_aes_finish(&aes_ctx);
// 	return result;
// }

// IgErrorCode ig_decrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	if (conn_id >= kMaxConnections) 
// 		return IG_ERROR_INVALID_CONN_ID;
// 	if (data_in_len > 1024)
// 		return IG_ERROR_INVALID_PARAMS;
// 	if (data_out_len < ig_decrypt_data_size(data_in_len))
// 		return IG_ERROR_DATA_TOO_SHORT;
// 	ConnectionEntry *connection_entry = &(connections_[conn_id]);
// 	if (connection_entry->active == false)
// 		return IG_ERROR_INVALID_CONN_ID;

// 	int result = decrypt_data(connection_entry->aes_key, data_in, data_in_len, data_out, data_out_len, connections_[conn_id].rx_nonce, bytes_written);
// 	if (result) {
// 		return IG_ERROR_GENERIC_FAIL;
// 	}
// 	increment_nonce(connections_[conn_id].rx_nonce);

// 	// TEST: non-encrypted version
// //	memcpy(data_out, data_in, data_in_len);
// //	*bytes_written = data_in_len;

// 	return IG_ERROR_NONE;
// }

// uint32_t ig_encrypt_log_data_size(uint32_t data_len) {
// 	// TEST: unencrypted version
// //	return data_len;
	
// 	return data_len + kNonceLength + kCcmTagLength;
// }

// IgErrorCode ig_encrypt_log_data(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written) {
// 	// validate inputs
// 	if (data_in_len == 0)
// 		return IG_ERROR_INVALID_PARAMS;
// 	if (data_out_len < ig_encrypt_log_data_size(data_in_len))
// 		return IG_ERROR_INVALID_PARAMS;

// 	// TEST: unencrypted version
// //	memcpy(data_out, data_in, data_in_len);
// //	*bytes_written = data_in_len;
// //	return IG_ERROR_NONE;

// 	// generate random nonce
// 	nrf_crypto_rng_vector_generate(data_out, kNonceLength);
	
// 	// encrypt
// 	encrypt_data(admin_key, data_in, data_in_len, data_out + kNonceLength, ig_encrypt_data_size(data_in_len), data_out, bytes_written);
// 	*bytes_written = *bytes_written + kNonceLength;
// 	return IG_ERROR_NONE;
// }



