#ifndef H_PAIRINGSTEP3_
#define H_PAIRINGSTEP3_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgPairingStep3 {
  bool has_nonce;
  //required
  uint8_t* nonce;
  size_t nonce_size;
  bool has_master_pin;
  //optional
  uint8_t* master_pin;
  size_t master_pin_size;
  bool has_pin_key;
  //optional
  uint8_t* pin_key;
  size_t pin_key_size;
  bool has_gmt_offset;
  //optional
  int32_t gmt_offset;
  bool has_dst_times;
  //optional
  uint8_t* dst_times;
  size_t dst_times_size;
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
} IgPairingStep3;

#define IG_PairingStep3_MSG_ID 3

void ig_PairingStep3_init(IgPairingStep3 *obj);
IgSerializerError ig_PairingStep3_encode(IgPairingStep3 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_PairingStep3_decode(uint8_t *buf,size_t buf_size,IgPairingStep3 *retval,size_t index);
uint32_t ig_PairingStep3_get_max_payload_in_bytes(IgPairingStep3 *obj);
bool ig_PairingStep3_is_valid(IgPairingStep3 *obj);
void ig_PairingStep3_deinit(IgPairingStep3 *obj);

size_t ig_PairingStep3_get_nonce_size(IgPairingStep3 *obj);
void ig_PairingStep3_set_nonce_nocopy(IgPairingStep3 *obj,uint8_t* nonce,size_t size);
void ig_PairingStep3_set_nonce(IgPairingStep3 *obj,uint8_t* nonce,size_t size);

size_t ig_PairingStep3_get_master_pin_size(IgPairingStep3 *obj);
void ig_PairingStep3_set_master_pin_nocopy(IgPairingStep3 *obj,uint8_t* master_pin,size_t size);
void ig_PairingStep3_set_master_pin(IgPairingStep3 *obj,uint8_t* master_pin,size_t size);

size_t ig_PairingStep3_get_pin_key_size(IgPairingStep3 *obj);
void ig_PairingStep3_set_pin_key_nocopy(IgPairingStep3 *obj,uint8_t* pin_key,size_t size);
void ig_PairingStep3_set_pin_key(IgPairingStep3 *obj,uint8_t* pin_key,size_t size);

void ig_PairingStep3_set_gmt_offset(IgPairingStep3 *obj,int32_t gmt_offset);

size_t ig_PairingStep3_get_dst_times_size(IgPairingStep3 *obj);
void ig_PairingStep3_set_dst_times_nocopy(IgPairingStep3 *obj,uint8_t* dst_times,size_t size);
void ig_PairingStep3_set_dst_times(IgPairingStep3 *obj,uint8_t* dst_times,size_t size);

size_t ig_PairingStep3_get_password_size(IgPairingStep3 *obj);
void ig_PairingStep3_set_password_nocopy(IgPairingStep3 *obj,uint8_t* password,size_t size);
void ig_PairingStep3_set_password(IgPairingStep3 *obj,uint8_t* password,size_t size);




#ifdef __cplusplus
}
#endif

#endif
