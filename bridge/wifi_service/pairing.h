#ifndef _PAIRING_H_
#define _PAIRING_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	C_SUBSCRIBE = 0,
  C_WRITE_STEP1,
  S_REPLY_STEP2,
  C_WRITE_STEP3,
  S_REPLY_STEP4,
  C_WRITE_COMMIT,
  ERR_EVENT,
} PAIRING_EVENT;

typedef enum {
	PAIRING_BEGIN = 0,
  PAIRING_STEP1,
	PAIRING_STEP2,
  PAIRING_STEP3,
	PAIRING_STEP4,
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
	uint16_t n_size_byte;
	uint16_t recv_len;
	RCEV_STATUS recv_status;
}RecvData;

int getRecvData(RecvData **p_recv_data);
int freeRecvData(RecvData **p_recv_data);
void recvData(RecvData *recv_pairing_data, uint8_t * data, uint16_t data_length);


#endif