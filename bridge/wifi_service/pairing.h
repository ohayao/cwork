#ifndef _PAIRING_H_
#define _PAIRING_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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
  ERR_EVENT,
} PAIRING_EVENT;

typedef enum {
	PAIRING_BEGIN = 0,
  PAIRING_STEP1,
	PAIRING_STEP2,
  PAIRING_STEP3,
	PAIRING_STEP4,
	PAIRING_COMMIT,
	PAIRING_COMPLETE,
	PAIRING_IDLE,
} PAIRING_STATUS;

typedef enum{
	NONE = 0,					// 没有任何接收数据
	GOT_PREV_DATA,		// 已经开始接收数据
	FINISHED_SUCCESS,	// 完成接收
	FINISHED_ERROR,		// 接收出错
} RCEV_STATUS;

typedef struct 
{
	uint8_t *data;
	uint16_t data_len;
	uint16_t recv_len;
	// 
	uint16_t n_size_byte;
	uint16_t pkg_len;
	RCEV_STATUS recv_status;
}RecvData;

int getRecvData(RecvData **p_recv_data);
int freeRecvData(RecvData **p_recv_data);
int initRecvData(RecvData *recv_data);
void recvData(RecvData *recv_pairing_data, uint8_t * data, uint16_t data_length);
int getPkgFromRecvData(RecvData *recv_pairing_data, uint8_t *step_pkg_data);
int getRecvPkgLen(RecvData *recv_pairing_data, size_t *return_size);
uint16_t getDataLength(uint8_t data[], uint16_t *n_size_byte, uint16_t *pkg_len);
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
#endif