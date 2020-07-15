#include "bridge/wifi_service/pairing.h"

#include "bridge/bridge_main/log.h"
#include "bridge/lock/cifra/drbg.h"
#include "bridge/lock/cifra/sha1.h"
#include "bridge/lock/micro-ecc/uECC.h"
#include "bridge/lock/connection/connection_common.h"


#define kMaxConnections 64
#define kNonceLength 12
#define kCcmTagLength 8
#define kCcmInternalCounterLength 3
#define kDstArrayLen 256
#define kSha256Len 32
// #define UINT32_MAX 4294967295

enum {
  kPrivateKeyLength=32,
  kPublicKeyLength=64
};

static uint8_t server_pairing_admin_key[IG_KEY_LENGTH];
static uint8_t server_pairing_private_key[kPrivateKeyLength];
static uint8_t server_pairing_public_key[kPublicKeyLength];

static uint8_t server_nonce[kNonceLength];
static uint8_t client_nonce[kNonceLength];
static uint8_t client_pairing_public_key[kPublicKeyLength];

// static uint8_t pairing_admin_key[IG_KEY_LENGTH];
// static uint8_t pairing_rx_nonce[kNonceLength];
// static uint8_t pairing_tx_nonce[kNonceLength];

// 锁的相关设置
static uint8_t pairing_pin_key[IG_PIN_KEY_LENGTH];
static uint8_t pairing_password[IG_PASSWORD_LENGTH];
static uint32_t pairing_master_pin_length;
static uint8_t pairing_master_pin[IG_MASTER_PIN_MAX_LENGTH];
static int32_t pairing_gmt_offset;
static uint32_t pairing_dst_len;
static uint8_t pairing_dst[kDstArrayLen];


static PAIRING_STATUS flag_pairing_status;

void printRecvData(RecvData *recv_data)
{
  printf("RecvData: \n");
  printf("data_len: %u\n", recv_data->data_len);
  printf("recv_len: %u\n", recv_data->recv_len);
  printf("pkg_len: %u\n", recv_data->pkg_len);
  return;
}

//传递数据, 获得这段数据的长度
uint16_t getDataLength(uint8_t data[], uint16_t *n_size_byte, uint16_t *pkg_len)
{
  printf(" getDataLength \n");
	uint16_t temp_result = 0;
	if(data[2] == 0xff)
	{
    printf(" getDataLength 3\n");
    *n_size_byte = 3;
    temp_result = data[0] * (0xfe) + data[1] + *n_size_byte;
    *pkg_len = temp_result - *n_size_byte;
		return temp_result;
	}
	else
	{
    printf(" getDataLength 1\n");
    printf(" getDataLength data[0] %u \n", data[0]);
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
  recv_pairing_data->data = NULL;
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
    
    serverLog(LL_ERROR, "recv_pairing_data->data_len too samll, data len %d, recv len %d, data length %d", recv_pairing_data->data_len, recv_pairing_data->recv_len, data_length);
    return 3;
  }
  memcpy(
    recv_pairing_data->data+recv_pairing_data->recv_len, 
    data, data_length
  );
  recv_pairing_data->recv_len += data_length;
  return 0;
}

void resetRecvData(RecvData *recv_pairing_data)
{
  if (!recv_pairing_data)
    return;
  if (recv_pairing_data->data)
  {
    freeRecvAllocData(recv_pairing_data);
  }
  setRecvDataStatus(recv_pairing_data, NONE);
}

// 
void recvData(RecvData *recv_pairing_data, uint8_t * data, uint16_t data_length)
{
  // serverLog(LL_NOTICE, "int recvData -------------------");
  if (!recv_pairing_data) return;
  // if (recv_pairing_data->recv_status == FINISHED_SUCCESS)
  // {
  //   resetRecvData(recv_pairing_data);
  // }
  int err = 0;
  uint16_t pkg_len = 0;
  switch (recv_pairing_data->recv_status)
  {
    case NONE:
    {
      // 创建新的存储空间, 准备接收
      // serverLog(LL_NOTICE, "recvData NONE-------------------");
      // 所获得的, 是 整个蓝牙包 (长度+加密报文)
      // 首先释放数据
      pkg_len = getDataLength(data, &(recv_pairing_data->n_size_byte), &recv_pairing_data->pkg_len);
      // serverLog(LL_NOTICE, "pkg_len: %d", pkg_len);
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
      // serverLog(LL_NOTICE, "recvData GOT_PREV_DATA-------------------");
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
      // serverLog(LL_NOTICE, "recvData FINISHED_SUCCESS-------------------");
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
  // serverLog(LL_NOTICE, "server_gen_pairing_step2");

  int ret = 0;
  IgPairingStep1 step1;
  ig_PairingStep1_init(&step1);
  IgSerializerError step1_err = ig_PairingStep1_decode(step1_bytes, step1_len, &step1, 0);
  if (step1_err || !ig_PairingStep1_is_valid(&step1)) {
    serverLog(LL_ERROR, "server_gen_pairing_step2 ig_PairingStep1_decode error");
		ig_PairingStep1_deinit(&step1);
		return IG_ERROR_INVALID_MESSAGE;
	}
  // serverLog(LL_ERROR, "server_gen_pairing_step2 ig_PairingStep1_decode success");

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
  // serverLog(LL_NOTICE, "server_nonce: ");
  // for (int i = 0; i < kNonceLength; i++)
  // {
  //   printf(" %x", server_nonce[i]);
  // }
  // printf("\n");

  ig_PairingStep2_set_nonce(&step2, server_nonce, kNonceLength);

  const struct uECC_Curve_t *p_curve = uECC_secp256r1();
  ret = uECC_make_key(
    server_pairing_public_key, server_pairing_private_key, p_curve);
  if (ret == 0)
  {
    serverLog(LL_ERROR, "uECC_make_key for client error");
    return 1;
  }
  printf("------------- server_pairing_public_key: ");
  for (int i = 0; i< 64; i++)
  {
    printf(" %x", server_pairing_public_key[i]);
  }
  printf("\n");

  // 显示输出 client
  // serverLog(LL_NOTICE, "client_publicKey_ :");
  // for (int i = 0; i < kPublicKeyLength; i++)
  // {
  //   printf("%x ", server_pairing_public_key[i]);
  // }
  // printf("\n\n");

  // serverLog(LL_NOTICE, "client_privateKey_ :");
  // for (int i = 0; i < kPrivateKeyLength; i++)
  // {
  //   printf("%x ", server_pairing_private_key[i]);
  // }
  // printf("\n\n");

  ig_PairingStep2_set_public_key(
    &step2, server_pairing_public_key, kPublicKeyLength);
  
  uint32_t step2_size =ig_PairingStep2_get_max_payload_in_bytes(&step2);
  if (step2_len < step2_size) {
		ig_PairingStep1_deinit(&step1);
		ig_PairingStep2_deinit(&step2);
		return IG_ERROR_DATA_TOO_SHORT;
	}

  size_t step2_written_bytes = 0;
  IgSerializerError err = ig_PairingStep2_encode(
    &step2, step2_bytes, step2_len, &step2_written_bytes);

  if (err != IgSerializerNoError) {
		ig_PairingStep1_deinit(&step1);
		ig_PairingStep2_deinit(&step2);
		return IG_ERROR_GENERIC_FAIL;
	}
	*bytes_written = (uint32_t)step2_written_bytes;

  uint8_t shared_secret[IG_KEY_EXCHANGE_PRIVATE_LENGTH];
  ret = uECC_shared_secret(
    client_pairing_public_key, server_pairing_private_key, shared_secret, p_curve);
  if (ret != 1) {
    serverLog(LL_ERROR, "uECC_shared_secret err");
    return 1;
  }
  printf("------------- client_pairing_public_key: ");
  for (int i = 0; i< 64; i++)
  {
    printf(" %x", client_pairing_public_key[i]);
  }
  printf("\n");
  printf("------------- server_pairing_private_key: ");
  for (int i = 0; i< 64; i++)
  {
    printf(" %x", server_pairing_private_key[i]);
  }
  printf("\n");
  printf("------------- shared_secret: ");
  for (int i = 0; i< 32; i++)
  {
    printf(" %x", shared_secret[i]);
  }
  printf("\n");

  // truncate to get admin key
	memcpy(server_pairing_admin_key, shared_secret, IG_KEY_LENGTH);
  printf("------------- shared_secret: ");
  for (int i = 0; i< 16; i++)
  {
    printf(" %x", server_pairing_admin_key[i]);
  }
  printf("\n");
  ig_PairingStep1_deinit(&step1);
	ig_PairingStep2_deinit(&step2);

  return IG_ERROR_NONE;
}

void incrementServerNonce() 
{
  uint8_t *nonce = server_nonce;
  for (int i = kNonceLength-1; i >= 0; i--) {
		if (nonce[i] == 255) {
			nonce[i] = 0;
		}
		else {
			nonce[i]++;
			break;
		}
	}
}

void incrementClientNonce() {
  uint8_t *nonce = client_nonce;
	// nonce interpreted as big endian (most significant byte has lowest address/index)
	for (int i = kNonceLength-1; i >= 0; i--) {
		if (nonce[i] == 255) {
			nonce[i] = 0;
		}
		else {
			nonce[i]++;
			break;
		}
	}
}

uint32_t ig_decrypt_data_size(uint32_t data_len) {
	return data_len - kCcmTagLength;
}

int decryptClientData(
  uint8_t *data_in, uint32_t data_in_len, 
  uint8_t *data_out, uint32_t data_out_len,
  uint32_t *bytes_written)
{ 
  *bytes_written = decryptData
    (data_in, data_in_len, 
    data_out, data_out_len, 
    server_pairing_admin_key, kConnectionKeyLength,
    server_nonce, kNonceLength
  );

  if (*bytes_written == UINT32_MAX) return 1;
  return 0;
}

void genRandomVector(int vector_len, uint8_t resultNonceArrar[]) {
    // time_t t;
    // srand((unsigned) time(&t));
    // for (int i = 0; i < vector_len; i++)
    // {
    //     resultNonceArrar[i] = rand() % 256; // [0 - 255]
    // }
    time_t t;
    srand((unsigned) time(&t));
    uint8_t entropy_size=16, nonce_size=8, reseed_size=16;
    uint8_t entropy[entropy_size], nonce[nonce_size], reseed[reseed_size];
    
    for (int i = 0; i < entropy_size; i++)
    {
        entropy[i] = rand() % 256;
    }
    for (int i = 0; i < nonce_size; i++)
    {
        nonce[i] = rand() % 256;
    }
    for (int i = 0; i < reseed_size; i++)
    {
        reseed[i] = rand() % 256;
    }

    cf_hmac_drbg ctx;
    cf_hmac_drbg_init(
        &ctx, &cf_sha1, 
        entropy, sizeof entropy, 
        nonce, sizeof nonce, NULL, 0);
    cf_hmac_drbg_reseed(
        &ctx, reseed, sizeof reseed, NULL, 0);
    cf_hmac_drbg_gen(
        &ctx, resultNonceArrar, vector_len);
    return;
}

int setPairingPinKey(IgPairingStep3 *step3)
{
  if (step3->has_pin_key) {
		memcpy(pairing_pin_key, step3->pin_key, ig_PairingStep3_get_pin_key_size(step3));
	}
	else {
    genRandomVector(sizeof(pairing_pin_key), pairing_pin_key);
		// nrf_crypto_rng_vector_generate(pairing_pin_key, sizeof(pairing_pin_key));
		// TEST: hardcoded value
  //		memset(pairing_pin_key, 0x2, sizeof(pairing_pin_key));
	}
  return 0;
}

int setPairingPassword(IgPairingStep3 *step3)
{
  if (step3->has_password) {
		memcpy(pairing_password, step3->password, ig_PairingStep3_get_password_size(step3));
	}
	else {
		genRandomVector(sizeof(pairing_password), pairing_password);
		// TEST: hardcoded value
//		memset(pairing_password, 0x3, sizeof(pairing_password));
	}
  return 0;
}

int setMasterPin(IgPairingStep3 *step3)
{
  if (step3->has_master_pin && 
			ig_PairingStep3_get_master_pin_size(step3) >= IG_MASTER_PIN_MIN_LENGTH && 
			ig_PairingStep3_get_master_pin_size(step3) <= IG_MASTER_PIN_MAX_LENGTH) {
		pairing_master_pin_length = ig_PairingStep3_get_master_pin_size(step3);
		memcpy(pairing_master_pin, step3->master_pin, pairing_master_pin_length);
	}
	else {
		pairing_master_pin_length = IG_MASTER_PIN_DEFAULT_LENGTH;
		genRandomVector(IG_MASTER_PIN_DEFAULT_LENGTH, pairing_master_pin);
		for (int i = 0; i < IG_MASTER_PIN_DEFAULT_LENGTH; ++i) {
			pairing_master_pin[i] = pairing_master_pin[i] % 10;
			pairing_master_pin[i] = pairing_master_pin[i] + 0x30;
		}
	}
  return 0;
}

int setClientNonce(IgPairingStep3 *step3)
{
  memcpy(client_nonce, step3->nonce, step3->nonce_size);
  return 0;
}

int setGmtOffset(IgPairingStep3 *step3)
{
  pairing_gmt_offset = step3->has_gmt_offset ? step3->gmt_offset : 0;
  return 0;
}

int setDstTimes(IgPairingStep3 *step3)
{
  if (step3->has_dst_times) {
	   	if (ig_PairingStep3_get_dst_times_size(step3) <= kDstArrayLen) {
			pairing_dst_len = ig_PairingStep3_get_dst_times_size(step3);
			memcpy(pairing_dst, step3->dst_times, pairing_dst_len);
		}
		else {
			return IG_ERROR_INTERNAL_ERROR;
		}
	}
	else {
		pairing_dst_len = 0;
	}
  return 0;
}

int setIgStep4(IgPairingStep4 *step4)
{
  ig_PairingStep4_init(step4);
  ig_PairingStep4_set_success(step4, true);
  ig_PairingStep4_set_pin_key(step4, pairing_pin_key, sizeof(pairing_pin_key));
  ig_PairingStep4_set_password(step4, pairing_password, sizeof(pairing_password));
  ig_PairingStep4_set_master_pin(step4, pairing_master_pin, pairing_master_pin_length);
  ig_PairingStep4_set_gmt_offset(step4, pairing_gmt_offset);
  return 0;
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

IgErrorCode ig_pairing_step4(
  uint8_t *encrypt_step3_bytes, uint32_t encrypt_step3_bytes_len, 
  uint8_t *encrypt_step4_bytes, uint32_t encrypt_step4_bytes_len, 
  uint32_t *bytes_written)
{
  uint32_t decrypted_data_in_len = ig_decrypt_data_size(encrypt_step3_bytes_len);
  uint8_t decrypted_data_in[decrypted_data_in_len];
  uint32_t decrypted_bytes_written = 0;

  int decrypt_result = decryptClientData(
    encrypt_step3_bytes, encrypt_step3_bytes_len, 
    decrypted_data_in, decrypted_data_in_len, 
    &decrypted_bytes_written
  );

  if (decrypt_result)
  {
    serverLog(LL_ERROR, "decryptClientData err");
    return 1;
  }

  incrementServerNonce();

  IgPairingStep3 step3;
  ig_PairingStep3_init(&step3);
  ig_PairingStep3_decode(decrypted_data_in, decrypted_bytes_written, &step3, 0);
  if (!ig_PairingStep3_is_valid(&step3)  ||
    (step3.has_nonce && ig_PairingStep3_get_nonce_size(&step3) != kNonceLength) ||
    (step3.has_pin_key && ig_PairingStep3_get_pin_key_size(&step3) != IG_PIN_KEY_LENGTH) ||
    (step3.has_password && ig_PairingStep3_get_password_size(&step3) != IG_PASSWORD_LENGTH) ||
    (step3.has_master_pin && ig_PairingStep3_get_master_pin_size(&step3) > IG_MASTER_PIN_MAX_LENGTH) ||
    (step3.has_master_pin && ig_PairingStep3_get_master_pin_size(&step3) < IG_MASTER_PIN_MIN_LENGTH)) 
  {
    serverLog(LL_ERROR, "ig_PairingStep3_is_valid error");
    ig_PairingStep3_deinit(&step3);
    return IG_ERROR_INVALID_MESSAGE;
  }
  serverLog(LL_NOTICE, "ig_PairingStep3_is_valid");
  
  setPairingPinKey(&step3);
  setPairingPassword(&step3);
  setMasterPin(&step3);
  setClientNonce(&step3);
  setGmtOffset(&step3);
  setDstTimes(&step3);

  IgPairingStep4 step4;
  setIgStep4(&step4);

  size_t step4_size = ig_PairingStep4_get_max_payload_in_bytes(&step4);
  if (encrypt_step4_bytes_len < step4_size) {
    ig_PairingStep3_deinit(&step3);
    ig_PairingStep4_deinit(&step4);
    return IG_ERROR_DATA_TOO_SHORT;
  }

  uint8_t step4_serialized[step4_size];
	size_t step4_written_bytes = 0;
  IgSerializerError err = ig_PairingStep4_encode(&step4, step4_serialized, step4_size, &step4_written_bytes);
	if (err != IgSerializerNoError) {
		ig_PairingStep3_deinit(&step3);
		ig_PairingStep4_deinit(&step4);
		return IG_ERROR_GENERIC_FAIL;
	}

  uint32_t encrypted_bytes_written = 0;
  // printf ("-----------------------step4_written_bytes  %u\n", step4_written_bytes);
  //  printf ("-----------------------step4_size  %u\n", step4_size);
  encrypted_bytes_written = encryptData(
    step4_serialized, step4_written_bytes, encrypt_step4_bytes, encrypt_step4_bytes_len,
    server_pairing_admin_key, kConnectionKeyLength, client_nonce, kNonceLength);
	*bytes_written = encrypted_bytes_written;
  ig_PairingStep3_deinit(&step3);
	ig_PairingStep4_deinit(&step4);
  return IG_ERROR_NONE;
}

