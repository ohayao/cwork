#include "bridge/wifi_service/pairing.h"
#include "bridge/bridge_main/log.h"
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/connection/connection_common.h"
#include "bridge/lock/cifra/drbg.h"
#include "bridge/lock/cifra/sha1.h"

#define kMaxConnections 64
#define kNonceLength 12
#define kCcmTagLength 8
#define kCcmInternalCounterLength 3
#define kDstArrayLen 256
#define kSha256Len 32

enum {
  kPrivateKeyLength=32,
  kPublicKeyLength=64
};

static uint8_t server_pairing_private_key[kPrivateKeyLength];
static uint8_t server_pairing_public_key[kPublicKeyLength];
static uint8_t server_nonce[kNonceLength];
static uint8_t client_nonce[kNonceLength];
static uint8_t client_pairing_public_key[kPublicKeyLength];

// static uint8_t pairing_admin_key[IG_KEY_LENGTH];
// static uint8_t pairing_rx_nonce[kNonceLength];
// static uint8_t pairing_tx_nonce[kNonceLength];
// static uint8_t pairing_pin_key[IG_PIN_KEY_LENGTH];
// static uint8_t pairing_password[IG_PASSWORD_LENGTH];
// static uint32_t pairing_master_pin_length;
// static uint8_t pairing_master_pin[IG_MASTER_PIN_MAX_LENGTH];
// static int32_t pairing_gmt_offset;
// static uint32_t pairing_dst_len;
// static uint8_t pairing_dst[kDstArrayLen];


static PAIRING_STATUS flag_pairing_status;

//传递数据, 获得这段数据的长度
uint16_t getDataLength(uint8_t data[], uint16_t *n_size_byte, uint16_t *pkg_len)
{
	uint16_t temp_result = 0;
	if(data[2] == 0xff)
	{
    *n_size_byte = 3;
    temp_result = data[0] * (0xfe) + data[1] + *n_size_byte;
    *pkg_len = temp_result - *n_size_byte;
		return temp_result;
	}
	else
	{
    *n_size_byte = 1;
    temp_result = data[0] + *n_size_byte;
    *pkg_len = data[0];
		return temp_result;
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
  recv_pairing_data->pkg_len = 0;
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
  return recv_pairing_data->recv_len == recv_pairing_data->data_len;
}

int copyData(RecvData *recv_pairing_data, uint8_t *data, uint16_t data_length)
{
  if (!recv_pairing_data) 
  {
    serverLog(LL_ERROR, "recv_pairing_data is null");
    return 1;
  }
  if (!recv_pairing_data->data) 
  {
    serverLog(LL_ERROR, "recv_pairing_data->data is null");
    return 2;
  }
  if (recv_pairing_data->data_len < recv_pairing_data->recv_len+data_length)
  {
    
    serverLog(LL_ERROR, "recv_pairing_data->data_len too samll");
    return 3;
  
  }
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
      pkg_len = getDataLength(data, &(recv_pairing_data->n_size_byte), &recv_pairing_data->pkg_len);

      if (allocRecvData(recv_pairing_data, pkg_len))
      {
        serverLog(LL_ERROR, "recvData alloc data err");
        // TODO: some fix method?
        return;
      }
      // 把当前的数据复制上去
      if (copyData(recv_pairing_data, data, data_length))
      {
        serverLog(LL_ERROR, "copyData err");
        return;
      }
      // 设置为已经开始接收数据
      setRecvDataStatus(recv_pairing_data, GOT_PREV_DATA);
      break;
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

int getRecvData(RecvData **p_recv_data)
{
  if (!p_recv_data)
  {
    serverLog(LL_ERROR, "getRecvData p_recv_data is null");
    return 1;
  }
  if (*p_recv_data)
  {
    serverLog(LL_ERROR, "getRecvData *p_recv_data is not null");
    return 1;
  }
  *p_recv_data = malloc(sizeof(RecvData));
  return 0;
}

int initRecvData(RecvData *recv_data)
{
  if (!recv_data)
  {
    serverLog(LL_ERROR, "recv_data is null");
    return 1;
  }
  memset(recv_data, 0, sizeof(RecvData));
  return 0;
}

int freeRecvData(RecvData **p_recv_data)
{
  if (!p_recv_data)
  {
    serverLog(LL_ERROR, "freeRecvData p_recv_data is null");
    return 1;
  }
  if (!*p_recv_data)
  {
    serverLog(LL_ERROR, "getRecvData *p_recv_data is null");
    return 1;
  }
  free(*p_recv_data);
  *p_recv_data = NULL;
  return 0;
}

int getRecvPkgLen(RecvData *recv_pairing_data, size_t *return_size)
{
  if (!recv_pairing_data)
  {
    serverLog(LL_ERROR, "recv_pairing_data is null");
    return 1;
  }
  if (!recv_pairing_data->data)
  {
    serverLog(LL_ERROR, "recv_pairing_data->data is null");
    return 1;
  }
  if (!return_size)
  {
    serverLog(LL_ERROR, "return_size is null");
    return 1;
  }
  *return_size = recv_pairing_data->pkg_len;
  return 0;
}

int getPkgFromRecvData(RecvData *recv_pairing_data, uint8_t *step_pkg_data)
{
  if (!recv_pairing_data)
  {
    serverLog(LL_ERROR, "getPkgFromRecvData recv_pairing_data is null");
    return 1;
  }

  if (!recv_pairing_data->data)
  {
    serverLog(LL_ERROR, "getPkgFromRecvData recv_pairing_data data is null");
    return 1;
  }

  if (!step_pkg_data)
  {
    serverLog(LL_ERROR, "getPkgFromRecvData step_pkg_data is null");
    return 1;
  }
  printf("n byte: %u\n", recv_pairing_data->n_size_byte);
  memcpy(step_pkg_data,
    recv_pairing_data->data + recv_pairing_data->n_size_byte,
    recv_pairing_data->data_len
  );
  return 0;
}

// return val: 返回是否出错
uint32_t ig_pairing_step2_size() {
	IgPairingStep2 step2;
	ig_PairingStep2_init(&step2);
	uint8_t nonce[kNonceLength] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
	ig_PairingStep2_set_nonce(&step2, nonce, kNonceLength);
	uint8_t public_key[IG_KEY_EXCHANGE_PUBLIC_LENGTH] = {0x0};
	ig_PairingStep2_set_public_key(&step2, public_key, IG_KEY_EXCHANGE_PUBLIC_LENGTH);
	size_t step2_size = ig_PairingStep2_get_max_payload_in_bytes(&step2);

	ig_PairingStep2_deinit(&step2);
	return step2_size;
}

// return val: 返回是否出错
// 
int server_gen_pairing_step2(
  uint8_t *step1_bytes, uint32_t step1_len, 
  uint8_t *step2_bytes, uint32_t step2_len, 
  uint32_t *bytes_written) {
  serverLog(LL_NOTICE, "server_gen_pairing_step2");

  int ret = 0;
  IgPairingStep1 step1;
  ig_PairingStep1_init(&step1);
  IgSerializerError step1_err = ig_PairingStep1_decode(step1_bytes, step1_len, &step1, 0);
  if (step1_err || !ig_PairingStep1_is_valid(&step1)) {
    serverLog(LL_ERROR, "server_gen_pairing_step2 ig_PairingStep1_decode error");
		ig_PairingStep1_deinit(&step1);
		return IG_ERROR_INVALID_MESSAGE;
	}
  serverLog(LL_ERROR, "server_gen_pairing_step2 ig_PairingStep1_decode success");

  if (ig_PairingStep1_get_public_key_size(&step1) != IG_KEY_EXCHANGE_PUBLIC_LENGTH) {
		ig_PairingStep1_deinit(&step1);
		return IG_ERROR_INVALID_MESSAGE;
	}

  if(step1.public_key_size != kPublicKeyLength)
  {
    serverLog(LL_ERROR, "step1.public_key_size %u != IG_KEY_LENGTH %u", step1.public_key_size, kPublicKeyLength);
    return IG_ERROR_INVALID_MESSAGE;
  }
  memcpy(client_pairing_public_key, step1.public_key, step1.public_key_size);

  IgPairingStep2 step2;
	ig_PairingStep2_init(&step2);
  generateRandomNonce(kNonceLength, server_nonce);
  serverLog(LL_NOTICE, "server_nonce: ");
  for (int i = 0; i < kNonceLength; i++)
  {
    printf(" %x", server_nonce[i]);
  }
  printf("\n");

  ig_PairingStep2_set_nonce(&step2, server_nonce, kNonceLength);

  const struct uECC_Curve_t *p_curve = uECC_secp256r1();
  ret = uECC_make_key(
    server_pairing_public_key, server_pairing_private_key, p_curve);
  if (ret == 0)
  {
    serverLog(LL_ERROR, "uECC_make_key for client error");
    return 1;
  }

  // 显示输出 client
  serverLog(LL_NOTICE, "client_publicKey_ :");
  for (int i = 0; i < kPublicKeyLength; i++)
  {
    printf("%x ", server_pairing_public_key[i]);
  }
  printf("\n\n");

  serverLog(LL_NOTICE, "client_privateKey_ :");
  for (int i = 0; i < kPrivateKeyLength; i++)
  {
    printf("%x ", server_pairing_private_key[i]);
  }
  printf("\n\n");

  ig_PairingStep2_set_public_key(
    &step2, server_pairing_public_key, kPublicKeyLength);
  
  
  return 0;
}
