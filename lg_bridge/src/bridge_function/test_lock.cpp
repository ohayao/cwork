#include <lock/connection/pairing_connection.h>

#include <cstdint>
#include <iostream>
#include <cstring>
#include "lock/external/cbor/cbor.h"
#include "lock/messages/PairingStep3.h"
#include "lock/messages/iglooconstants.h"
#include "lock/connection/connection_common.h"
using namespace std;

// xxxxxxxxxxxxxx Notification Handler: 
// 54 a3 00 02 0b 58 40 da 3c 73 25 7e 96 5c 8b 6b 27 f2 06 95 
// xxxxxxxxxxxxxxx on_handle_characteristic_property_change
// xxxxxxxxxxxxxx Notification Handler: 
// b5 51 44 46 38 12 f0 82 16 24 c0 09 bb 5b 9f d1 d0 6e ce fb 
// xxxxxxxxxxxxxxx on_handle_characteristic_property_change
// xxxxxxxxxxxxxx Notification Handler: 
// 70 55 26 e9 71 47 37 9e 13 7a dd d9 13 00 dd df 8a 35 8a 2b 
// xxxxxxxxxxxxxxx on_handle_characteristic_property_change
// xxxxxxxxxxxxxx Notification Handler: 
// ca 36 42 ef 8d 39 4a 71 f0 87 ee 0c 4c 19 9b d3 0e 2c 63 17 
// xxxxxxxxxxxxxxx on_handle_characteristic_property_change
// xxxxxxxxxxxxxx Notification Handler: 
// 70 d0 7f ae e

// bool igloohome_ble_lock_crypto_PairingConnection_beginConnection();
void test_beginConnection()
{
  cout << "starting test_beginConnection() -----------------------" << endl;
  bool begin_res = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (begin_res)
  {
    cout << "beginConnection success" << endl;;
  }
  else
  {
    cout << "beginConnection fail" << endl;
    return;
  }

  cout << "ending test_beginConnection()" << endl;
}

static size_t resolve_payload_len(size_t step_len)
{
	return step_len>254 ? (step_len+3) : (step_len+1);
}

static bool build_msg_payload(uint8_t **p_payloadBytes, 
  size_t &payload_len, uint8_t *stepBytes, size_t step_len)
{
	payload_len = resolve_payload_len(step_len);
	size_t n_byte_for_len = payload_len - step_len;
	(*p_payloadBytes) = (uint8_t *)malloc(payload_len);
	if (n_byte_for_len == 1)
	{
		(*p_payloadBytes)[0] = step_len;
	}
	else if (n_byte_for_len == 3)
	{
		// 大端存储
		uint8_t fst, sec;
		sec = 254;
		fst = step_len - sec;
		(*p_payloadBytes)[0] = fst;
		(*p_payloadBytes)[1] = sec;
		(*p_payloadBytes)[3] = 0xff;  
	}
	else
	{
		cerr << "wrong n_byte_for_len, error" << endl;
		return false;
	}
	memcpy((*p_payloadBytes)+ n_byte_for_len, stepBytes, step_len);
	return true;
}

// int igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(uint8_t **ret);
// 参数 ret 返回生成字符串
// 函数返回 int 生成字符串的长度
void test_genPairingStep1Native()
{
  cout << "starting test_genPairingStep1Native() -----------------------" << endl;
  bool begin_res = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (begin_res)
  {
    cout << "beginConnection success" << endl;;
  }
  else
  {
    cout << "beginConnection fail, return" << endl;
    return;
  }
  uint8_t *step1Bytes = NULL;
  size_t step1Bytes_len = 0;
  step1Bytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			&step1Bytes);
  cout << "step1Bytes_len : " << step1Bytes_len << endl;
  cout << "step1Bytes: " ;
  for (int i = 0; i < step1Bytes_len; i++)
  {
    printf("0x%02x ", step1Bytes[i]);
  }
  cout << endl;
  cout << "ending test_genPairingStep1Native()" << endl;
}


void test_genPairingStep1Native_payload()
{
  cout << "starting test_genPairingStep1Native_payload() -----------------------" << endl;
  bool begin_res = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (begin_res)
  {
    cout << "beginConnection success" << endl;;
  }
  else
  {
    cout << "beginConnection fail, return" << endl;
    return;
  }
  uint8_t *step1Bytes = NULL;
  size_t step1Bytes_len = 0;
  step1Bytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			&step1Bytes);
  cout << "step1Bytes_len : " << step1Bytes_len << endl;
  cout << "step1Bytes: " ;
  for (int i = 0; i < step1Bytes_len; i++)
  {
    printf("0x%02x ", step1Bytes[i]);
  }
  cout << endl;
  size_t payload_len = 0;
  uint8_t *msg_payload = NULL;
  if (build_msg_payload(&msg_payload, payload_len, step1Bytes, step1Bytes_len))
  {
    cout << "build_msg_payload success" << endl;
  }
  else
  {
    cout << "build_msg_payload fail" << endl;
  }
  cout << "msg_payload: " ;
  for (int i = 0; i < step1Bytes_len; i++)
  {
    printf("0x%02x ", msg_payload[i]);
  }
  cout << endl;

  cout << "ending test_genPairingStep1Native_payload()" << endl;
}

// addr : D9:78:2F:E3:1A:5C
// name : IGM303e31a5c
// 4b 9d f ed 0 3 59 39 c0 c4 86 c5 fa 88 49 1b

void test_adminbeginConnection()
{
  uint8_t key[] = {0x4b, 0x9d, 0x0f, 0xed, 0x0, 0x3, 0x59, 0x39, 0xc0, 0xc4, 
    0x86, 0xc5, 0xfa, 0x88, 0x49, 0x1b};
  int keyLen = 16;
  int connection_id = beginConnection(kConnectionTypeAdmin, key, keyLen);
  if (connection_id == -1)
  {
    cerr << "testbeginConnection beginConnection err" << endl;
  }
  return;
}

int main(int argc, char *argv[])
{
  // test_beginConnection();
  // test_genPairingStep1Native();
  // test_genPairingStep1Native_payload();
  test_adminbeginConnection();
  return 0;
}