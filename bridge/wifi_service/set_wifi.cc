#include "bridge/wifi_service/set_wifi.h"
#include "bridge/bridge_main/log.h"
#include "bridge/lock/cifra/drbg.h"
#include "bridge/lock/cifra/sha1.h"
#include "bridge/lock/micro-ecc/uECC.h"
#include "bridge/lock/connection/connection_common.h"


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

// 接收/发送加/解密用
static uint8_t server_pairing_admin_key[IG_KEY_LENGTH];

// 发送加密用
static uint8_t client_nonce[kNonceLength];

// 接收解密用
static uint8_t server_nonce[kNonceLength];



