#ifndef _PAIRING_H_
#define _PAIRING_H_
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/wifi_service/crypt.h"


#define IG_KEY_EXCHANGE_PUBLIC_LENGTH 64
#define IG_KEY_EXCHANGE_PRIVATE_LENGTH 32
#define IG_KEY_LENGTH 16
#define IG_PIN_KEY_LENGTH 16
#define IG_PASSWORD_LENGTH 8
#define IG_MASTER_PIN_MIN_LENGTH 4
#define IG_MASTER_PIN_DEFAULT_LENGTH 8
#define IG_MASTER_PIN_MAX_LENGTH 10
#define IG_PIN_INPUT_MAX_LENGTH 16
#define IG_PIN_MAX_LENGTH 9
#define IG_PIN_MIN_LENGTH 4
#define IG_ACCESS_RIGHTS_SIZE 8
#define IG_ERROR_MAX_CONNECTIONS 1001

typedef enum IgErrorCode {
	IG_ERROR_NONE = 0,
	IG_ERROR_GENERIC_FAIL = 1,
	IG_ERROR_INVALID_CONN_ID = 2,
	IG_ERROR_DATA_TOO_SHORT = 3,
	IG_ERROR_INVALID_MESSAGE = 4,
	IG_ERROR_INVALID_PARAMS = 5,
	IG_ERROR_INTERNAL_ERROR = 6,
} IgErrorCode;

typedef enum {
	C_SUBSCRIBE = 0,
  C_WRITE_STEP1,
  S_REPLY_STEP2,
  C_WRITE_STEP3,
  S_REPLY_STEP4,
  C_WRITE_COMMIT,
	S_PAIRING_COMPLETE,
  ERR_EVENT=7,
} PAIRING_EVENT;

typedef enum {
	PAIRING_BEGIN = 0,
  PAIRING_STEP1,
	PAIRING_STEP2,
  PAIRING_STEP3,
	PAIRING_STEP4,
	PAIRING_COMMIT,
	PAIRING_COMPLETE = 6
} PAIRING_STATUS;

typedef enum {
  C_WRITE_WIFI_REQUEST=8,
  S_REPLY_WIFI_RESPONSE=9
} SET_WIFI_EVENT;

typedef enum {
	SET_WIFI_BEGIN = 7,
  SET_WIFI_REQUEST,
	SET_WIFI_COMPLETE=9,
} SET_WIFI_STATUS;

typedef enum{
	NONE = 0,					// 没有任何接收数据
	GOT_PREV_DATA,		// 已经开始接收数据
	FINISHED_SUCCESS,	// 完成接收
	FINISHED_ERROR,		// 接收出错
} RCEV_STATUS;

typedef struct 
{
	uint8_t *data;
	uint32_t data_len;
	uint32_t recv_len;
	// 
	uint32_t n_size_byte;
	uint32_t pkg_len;
	RCEV_STATUS recv_status;
}RecvData;

typedef struct IgPairingResult {
	uint8_t admin_key[IG_KEY_LENGTH];
	uint8_t pin_key[IG_PIN_KEY_LENGTH];
	uint8_t password[IG_PASSWORD_LENGTH];
	uint32_t master_pin_length;
	uint8_t master_pin[IG_MASTER_PIN_MAX_LENGTH];
	uint32_t current_time;
	int32_t gmt_offset;
	uint32_t dst_length;
	uint8_t *dst;	// do not free
} IgPairingResult;

int makeCrypt(Crypt **p_reval, size_t *written_len);
int getRecvData(RecvData **p_recv_data);
int freeRecvData(RecvData **p_recv_data);
int initRecvData(RecvData *recv_data);
void recvData(RecvData *recv_pairing_data, uint8_t * data, uint16_t data_length);
int getPkgFromRecvData(RecvData *recv_pairing_data, uint8_t *step_pkg_data);
int getRecvPkgLen(
	RecvData *recv_pairing_data, size_t *return_size);
uint32_t getDataLength(
	uint8_t data[], uint32_t *n_size_byte, uint32_t *pkg_len);
int isRecvFullPkg(RecvData *recv_pairing_data);
uint32_t ig_pairing_step2_size();
int server_gen_pairing_step2(
  uint8_t *step1_bytes, uint32_t step1_len, 
  uint8_t *step2_bytes, uint32_t step2_len, 
  uint32_t *bytes_written);
uint32_t ig_decrypt_data_size(uint32_t data_len);

int decryptClientData(
  uint8_t *data_in, uint32_t data_in_len, 
  uint8_t *data_out, uint32_t data_out_len,
  uint32_t *bytes_written);
void incrementClientNonce();
void incrementServerNonce();
void printRecvData(RecvData *recv_data);
void resetRecvData(RecvData *recv_pairing_data);
int setPairingPinKey(IgPairingStep3 *step3);
int setPairingPassword(IgPairingStep3 *step3);
int setMasterPin(IgPairingStep3 *step3);
int setClientNonce(IgPairingStep3 *step3);
int setGmtOffset(IgPairingStep3 *step3);
int setDstTimes(IgPairingStep3 *step3);
int setIgStep4(IgPairingStep4 *step4);
uint32_t ig_pairing_step4_size();
IgErrorCode ig_pairing_step4(
  uint8_t *encrypt_step3_bytes, uint32_t encrypt_step3_bytes_len, 
  uint8_t *encrypt_step4_bytes, uint32_t encrypt_step4_bytes_len, 
  uint32_t *bytes_written);
IgErrorCode ig_commit_pairing(
  uint8_t *encrypt_commit_Bytes, uint32_t encrypt_commit_Bytes_len, IgPairingResult *pairing_result_out);

// debug
void showAllKeys();
#endif