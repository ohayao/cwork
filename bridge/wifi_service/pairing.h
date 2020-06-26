#ifndef _PAIRING_H_
#define _PAIRING_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>



typedef enum {
	PAIRING_BEGIN = 0,
	PAIRING_STEP2,
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
}recv_data;


void recvData(recv_data *recv_pairing_data, uint8_t * data, uint16_t data_length);


#endif