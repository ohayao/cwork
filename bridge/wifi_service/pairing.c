#include "bridge/wifi_service/pairing.h"
#include "bridge/bridge_main/log.h"

static PAIRING_STATUS flag_pairing_status;

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

int freeRecvAllocData(RecvData *recv_pairing_data)
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

int allocRecvData(RecvData *recv_pairing_data, int data_len)
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

int setRecvDataStatus(RecvData *recv_pairing_data, RCEV_STATUS status)
{
  if (!recv_pairing_data) return 1;
  recv_pairing_data->recv_status = status;
  return 0;
}

// 1: 已经分配
// 0: 没有分配
int isRecvDataAlloc(RecvData *recv_pairing_data)
{
  return recv_pairing_data->data == NULL;
}

int isRecvFullPkg(RecvData *recv_pairing_data)
{
  return recv_pairing_data->recv_len == recv_pairing_data->recv_len;
}

int copyData(RecvData *recv_pairing_data, uint8_t *data, uint16_t data_length)
{
  if (!recv_pairing_data) return 1;
  if (!recv_pairing_data->data) return 2;
  if (recv_pairing_data->data_len < recv_pairing_data->recv_len+data_length)
    return 3;
  memcpy(
    recv_pairing_data->data+recv_pairing_data->recv_len, 
    data, data_length
  );
  recv_pairing_data->recv_len += data_length;
  return 0;
}

// 
void recvData(RecvData *recv_pairing_data, uint8_t * data, uint16_t data_length)
{
  serverLog(LL_NOTICE, "int recvData -------------------");
  int err = 0;
  uint16_t pkg_len = 0;
  switch (recv_pairing_data->recv_status)
  {
    case NONE:
    {
      // 创建新的存储空间, 准备接收
      serverLog(LL_NOTICE, "recvData NONE-------------------");
      // 所获得的, 是 整个蓝牙包 (长度+加密报文)
      pkg_len = getDataLength(data, &recv_pairing_data->n_size_byte);
      if (allocRecvData(recv_pairing_data, pkg_len))
      {
        serverLog(LL_ERROR, "recvData alloc data err");
        // TODO: some fix method?
        return;
      }
      // 把当前的数据复制上去
      copyData(recv_pairing_data, data, data_length);
      // 设置为已经开始接收数据
      setRecvDataStatus(recv_pairing_data, GOT_PREV_DATA);
    }
    case GOT_PREV_DATA:
      serverLog(LL_NOTICE, "recvData GOT_PREV_DATA-------------------");
      /* code */
      if (copyData(recv_pairing_data, data, data_length))
      {
        serverLog(LL_ERROR, "copyData err");
      }
      if (recv_pairing_data && isRecvFullPkg(recv_pairing_data))
      {
        setRecvDataStatus(recv_pairing_data, FINISHED_SUCCESS);
      }
      break;
    case FINISHED_SUCCESS:
      serverLog(LL_NOTICE, "recvData FINISHED_SUCCESS-------------------");
      /* code */
      break;
    case FINISHED_ERROR:
      break;
    default:
      break;
  }

}