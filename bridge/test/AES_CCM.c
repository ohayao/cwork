#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bridge/lock/cifra/modes.h"
#include "bridge/lock/cifra/aes.h"
#include "bridge/lock/micro-ecc/uECC.h"
#include "bridge/lock/connection/encryption.h"
#include "bridge/bridge_main/log.h"
#include "bridge/lock/cifra/drbg.h"
#include "bridge/lock/cifra/sha1.h"
//简要介绍

enum {
  kConnectionKeyLength=16,
  kPrivateKeyLength=32,
  kPublicKeyLength=64
};

static uint8_t client_privateKey_[kPrivateKeyLength] = {0x00};
static uint8_t client_publicKey_[kPublicKeyLength] = {0x00};
static uint8_t client_Nonce_[kNonceLength] = {0x00};
static uint8_t client_shared_key[kPrivateKeyLength] = {0x00};

static uint8_t server_shared_key[kPrivateKeyLength] = {0x00};
static uint8_t server_Nonce_[kNonceLength] = {0x00};
static uint8_t server_privateKey_[kPrivateKeyLength] = {0x00};
static uint8_t server_publicKey_[kPublicKeyLength] = {0x00};

static inline uint8_t unhex_chr(char a)
{
  if (a >= '0' && a <= '9')
    return a - '0';
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  return 0;
}

static inline size_t unhex(uint8_t *buf, size_t len, const char *str)
{
  size_t used = 0;

  assert(strlen(str) % 2 == 0);
  assert(strlen(str) / 2 <= len);

  while (*str)
  {
    assert(len);
    *buf = unhex_chr(str[0]) << 4 | unhex_chr(str[1]);
    buf++;
    used++;
    str += 2;
    len--;
  }

  return used;
}


int main(int argc, char *argv[])
{
  int ret = 0;
  char msg[] = "Google 12345678";
  
  
  
  // 首先, 是要生成client, 部分
  const struct uECC_Curve_t * p_curve = uECC_secp256r1();
  ret = uECC_make_key(client_publicKey_, client_privateKey_, p_curve);

  if (ret == 0)
  {
    serverLog(LL_ERROR, "uECC_make_key for client error");
    return 1;
  }
  // 显示输出 client
  serverLog(LL_NOTICE, "client_publicKey_ :");
  for (int i = 0; i < kPublicKeyLength; i++)
  {
    printf("%x ", client_publicKey_[i]);
  }
  printf("\n\n");

  serverLog(LL_NOTICE, "client_privateKey_ :");
  for (int i = 0; i < kPrivateKeyLength; i++)
  {
    printf("%x ", client_privateKey_[i]);
  }
  printf("\n\n");

  // 基于杂凑函数的确定性随机比特生成器（Hash_DRBG）
  uint8_t entropy[16], nonce[8], reseed[16];
  
  unhex(entropy, sizeof entropy, "79349bbf7cdda5799557866621c91383");
  unhex(nonce, sizeof nonce, "1146733abf8c35c8");
  unhex(reseed, sizeof reseed, "c7215b5b96c48e9b338c74e3e99dfedf");
  // 一个确定性生成器
  cf_hmac_drbg ctx;
  cf_hmac_drbg_init(&ctx, &cf_sha1, entropy, sizeof entropy, nonce, sizeof nonce, NULL, 0);
  cf_hmac_drbg_reseed(&ctx, reseed, sizeof reseed, NULL, 0);
  cf_hmac_drbg_gen(&ctx, client_Nonce_, sizeof(client_Nonce_));

  serverLog(LL_NOTICE, "client_Nonce_ :");
  for (int i = 0; i < sizeof client_Nonce_; i++)
  {
    printf("%x ", client_Nonce_[i]);
  }
  printf("\n\n");

  // 然后生成 server 的
  p_curve = uECC_secp256r1();
  ret = uECC_make_key(server_publicKey_, server_privateKey_, p_curve);
  if (ret == 0)
  {
    serverLog(LL_ERROR, "uECC_make_key for client error");
    return 1;
  }

  // 显示输出 server
  serverLog(LL_NOTICE, "server_publicKey_ :");
  for (int i = 0; i < kPublicKeyLength; i++)
  {
    printf("%x ", server_publicKey_[i]);
  }
  printf("\n\n");

  serverLog(LL_NOTICE, "server_privateKey_ :");
  for (int i = 0; i < kPrivateKeyLength; i++)
  {
    printf("%x ", server_privateKey_[i]);
  }
  printf("\n\n");

  cf_hmac_drbg_gen(&ctx, server_Nonce_, sizeof(server_Nonce_));
  serverLog(LL_NOTICE, "server_Nonce_ :");
  for (int i = 0; i < sizeof server_Nonce_; i++)
  {
    printf("%x ", server_Nonce_[i]);
  }
  printf("\n\n");

  // client shared key
  ret = uECC_shared_secret(server_publicKey_, client_privateKey_, client_shared_key, p_curve);
  if (ret != 1) {
    serverLog(LL_ERROR, "uECC_shared_secret error");
    return 1;
  }

  serverLog(LL_NOTICE, "client_shared_key :");
  for (int i = 0; i < sizeof client_shared_key; i++)
  {
    printf("%x ", client_shared_key[i]);
  }
  printf("\n\n");

  // encryption
  // 加密是要用 shared_key, 然后用自己的nonce, 加密
  uint32_t client_encrypt_data_size = 0;
  client_encrypt_data_size = encryptDataSize(sizeof msg);
  uint8_t client_encrypt_data[client_encrypt_data_size];
  int32_t client_encrypt_data_len = encryptData(
            msg, sizeof msg,
            client_encrypt_data, client_encrypt_data_size,
            client_shared_key, kConnectionKeyLength,
            client_Nonce_, kNonceLength
  );

  if (client_encrypt_data_len <= 0)
  {
    serverLog(LL_ERROR, "client encryptDataSize error");
    return 1;
  }

  serverLog(LL_NOTICE, "encrypt_data :");
  for (int i = 0; i < client_encrypt_data_len; i++)
  {
    printf("%x ", client_shared_key[i]);
  }
  printf("\n\n");

  // 首先是要计算共享的key
  ret = uECC_shared_secret(client_publicKey_, server_privateKey_, server_shared_key, p_curve);
  if (ret != 1) {
    serverLog(LL_ERROR, "uECC_shared_secret");
    return 0;
  }

  serverLog(LL_NOTICE, "server_shared_key :");
  for (int i = 0; i < sizeof server_shared_key; i++)
  {
    printf("%x ", server_shared_key[i]);
  }
  printf("\n\n");

  // 检测 两个shared_key 是否一致
  if (memcmp(server_shared_key, client_shared_key, sizeof server_shared_key) != 0)
  {
    serverLog(LL_ERROR, "server_shared_key is not same as client_shared_key ");
    return 1;
  }
  serverLog(LL_ERROR, "server_shared_key is same as client_shared_key ");


  // server 接收到这个消息了, 然后需要进行解密, 
  uint32_t server_decrypt_data_size = decryptDataSize(client_encrypt_data_len);
  uint8_t server_decrypt_data[server_decrypt_data_size];
  int32_t server_decrypt_data_len = decryptData(
          client_encrypt_data, client_encrypt_data_len,
          server_decrypt_data, server_decrypt_data_size,
          server_shared_key, kConnectionKeyLength,
          client_Nonce_, kNonceLength
  );

  if (server_decrypt_data_len < 0) {
    serverLog(LL_ERROR, "decryptData error");
    return 1;
  }

  serverLog(LL_NOTICE, "server_decrypt_data :");
  for (int i = 0; i < server_decrypt_data_len; i++)
  {
    printf("%c", server_decrypt_data[i]);
  }
  printf("\n\n");
}