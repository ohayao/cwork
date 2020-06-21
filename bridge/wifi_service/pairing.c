#include "bridge/wifi_service/pairing.h"
#include "bridge/bridge_main/log.h"

//传递数据, 获得这段数据的长度
uint16_t getDataLength(uint8_t data[], uint16_t *n_size_byte)
{
	uint16_t temp_result = 0;
	if(data[2] == 0xff)
	{
    *n_size_byte = 3;
    temp_result = data[0] * (0xfe) + data[1] + *n_size_byte;
		return temp_result;
	}
	else
	{
    *n_size_byte = 1;
    temp_result = data[0] + *n_size_byte;;
		return data[0];
	}
}

int freeRecvAllocData(recv_data *recv_pairing_data)
{
  if (!recv_pairing_data) 
  {
    serverLog(LL_ERROR, "freeRecvAllocData recv_pairing_data NULL");
    return 1;
  }
  if (!recv_pairing_data->data) 
  {
    serverLog(LL_ERROR, "freeRecvAllocData recv_pairing_data->data NULL");
    return 1;
  }
  free(recv_pairing_data->data);
  recv_pairing_data->data_len = 0;
  recv_pairing_data->recv_len = 0;
  recv_pairing_data->n_size_byte = 0;
  return 0;
}

int allocRecvData(recv_data *recv_pairing_data, int data_len)
{
  if (!recv_pairing_data) {
    serverLog(LL_ERROR, "allocRecvData recv_pairing_data NULL");
    return 1;
  }
  if (recv_pairing_data->data) {
    serverLog(LL_ERROR, "recv_pairing_data data not be free");
    return 1;
  }
    
  recv_pairing_data->data_len = data_len;
  recv_pairing_data->recv_len = 0;
  recv_pairing_data->n_size_byte = 0;
  recv_pairing_data->data = malloc(data_len);
  return 0;
}

// 1: 已经分配
// 0: 没有分配
int isRecvDataAlloc(recv_data *recv_pairing_data)
{
  return recv_pairing_data->data == NULL;
}

void recvData(recv_data *recv_pairing_data, uint8_t * data)
{
  serverLog(LL_NOTICE, "int recvData -------------------");
  int err = 0;
  uint16_t data_length = 0;
  switch (recv_pairing_data->recv_status)
  {
    case NONE:
    {
      serverLog(LL_NOTICE, "recvData NONE-------------------");
      uint16_t data_length = getDataLength(data, &recv_pairing_data->n_size_byte);
      if (allocRecvData(recv_pairing_data, data_length))
      {
        serverLog(LL_ERROR, "recvData alloc data err");
        // TODO: some fix method?
        return;
      }
      memcpy(recv_pairing_data->data, data, recv_pairing_data->data_len);
    }
    case GOT_PREV_DATA:
      /* code */
      break;
    case FINISHED_SUCCESS:
      /* code */
      break;
    case FINISHED_ERROR:
      break;
    default:
      break;
  }

}