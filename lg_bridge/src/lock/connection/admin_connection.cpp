#include "admin_connection.h"

#include "cifra/modes.h"
#include "cifra/aes.h"
#include "encryption.h"
#include "connection_common.h"
#include "messages/AdminConnectionStep1.h"
#include "messages/AdminConnectionStep2.h"
#include "messages/AdminConnectionStep3.h"
#include <stdio.h>
#include <iostream>
using namespace std;

int igloohome_ble_lock_crypto_AdminConnection_beginConnection(uint8_t *jKey, int keyLen) 
{
  return beginConnection(kConnectionTypeAdmin, jKey, keyLen);
}

int igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native(int connectionId, 
  uint8_t *jConnectionStep1, int step1Len, uint8_t **retBytes) {
  Connection *connection = getConnection(connectionId);
  if (!connection)
  {
    cerr << "!connection" << endl;
    return 0;
  }
    

  if (!jConnectionStep1)
  {
    cerr << "!jConnectionStep1" << endl;
    return 0;
  }
    

  uint8_t step1Bytes[step1Len];
  memcpy(step1Bytes, jConnectionStep1, step1Len);

  IgAdminConnectionStep1 step1;
  ig_AdminConnectionStep1_init(&step1);
  IgSerializerError step1_err = ig_AdminConnectionStep1_decode(
    step1Bytes, (size_t)step1Len, &step1, 0);
  if (step1_err || !ig_AdminConnectionStep1_is_valid(&step1) 
                                        || step1.nonce_size != kNonceLength) {
      ig_AdminConnectionStep1_deinit(&step1);
      cerr << "step1_err" << endl;
      return 0;
  }

  memcpy(connection->txNonce, step1.nonce, step1.nonce_size);
  for (int j = 0; j < step1.nonce_size; j++)
  {
    printf("%02x ", step1.nonce[j]);
  }
  printf("\n");
  for (int j = 0; j < step1.nonce_size; j++)
  {
    printf("%02x ", (connection->txNonce)[j]);
  }
  printf("\n");

  IgAdminConnectionStep2 step2;
  ig_AdminConnectionStep2_init(&step2);
  ig_AdminConnectionStep2_set_nonce(&step2, connection->rxNonce, kNonceLength);
  size_t step2MaxLen = ig_AdminConnectionStep2_get_max_payload_in_bytes(&step2);
  uint8_t plaintextBytes[step2MaxLen];
  size_t plaintextLen = 0;
  IgSerializerError err = ig_AdminConnectionStep2_encode(
                          &step2, plaintextBytes, step2MaxLen, &plaintextLen);
  if (err != IgSerializerNoError) {
    ig_AdminConnectionStep1_deinit(&step1);
    ig_AdminConnectionStep2_deinit(&step2);
    cerr << "ig_AdminConnectionStep2_encode error" << endl;
    return 0;
  }

  uint32_t retvalMaxLen = encryptDataSize(plaintextLen);
  uint8_t *retvalBytes = (uint8_t *)calloc(retvalMaxLen,1);
  int32_t retvalLen = encryptData(
    plaintextBytes, plaintextLen,
    retvalBytes, retvalMaxLen,
    connection->key, kConnectionKeyLength,
    connection->txNonce, kNonceLength
  );
  incrementNonce(connection->txNonce);
  if (retvalLen < 0) {
    ig_AdminConnectionStep1_deinit(&step1);
    ig_AdminConnectionStep2_deinit(&step2);
    free(retvalBytes);
    cerr << "incrementNonce error" << endl;
    return 0;
  }
  ig_AdminConnectionStep1_deinit(&step1);
  ig_AdminConnectionStep2_deinit(&step2);
  *retBytes = retvalBytes;
  return retvalLen;
}

bool igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(
  int connectionId, uint8_t *jConnectionStep3, int messageLen)
{
  Connection *connection = getConnection(connectionId);
  if (!connection)
    return false;
  if (!jConnectionStep3)
    return false;
  uint8_t *messageBytes = jConnectionStep3;

  uint32_t step3MaxLen = decryptDataSize(messageLen);
  uint8_t step3Bytes[step3MaxLen];
  int32_t step3Len = decryptData(
    messageBytes, messageLen,
    step3Bytes, step3MaxLen,
    connection->key, kConnectionKeyLength,
    connection->rxNonce, kNonceLength
  );
  incrementNonce(connection->rxNonce);
  if (step3Len < 0) {
    return false;
  }

  IgAdminConnectionStep3 step3;
  ig_AdminConnectionStep3_init(&step3);
  ig_AdminConnectionStep3_decode(step3Bytes, (size_t)step3Len, &step3, 0);
  if (!ig_AdminConnectionStep3_is_valid(&step3) || !step3.success) {
    ig_AdminConnectionStep3_deinit(&step3);
    return false;
  }

  ig_AdminConnectionStep3_deinit(&step3);
  return true;
}

int igloohome_ble_lock_crypto_AdminConnection_encryptNative(
  int connectionId, uint8_t *jPlaintext, int plaintextLen, uint8_t **retBytes) {
    return encryptNative(connectionId, jPlaintext, plaintextLen, retBytes);
}

int igloohome_ble_lock_crypto_AdminConnection_decryptNative(
  int connectionId, uint8_t *jMessage, int messageLen, uint8_t **retBytes) {
   decryptNative(
     connectionId, jMessage, messageLen, retBytes);
}



