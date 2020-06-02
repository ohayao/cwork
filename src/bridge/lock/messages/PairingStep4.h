#ifndef H_PAIRINGSTEP4_
#define H_PAIRINGSTEP4_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgPairingStep4 {
  bool has_success;
  //required
  bool success;
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
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
} IgPairingStep4;

#define IG_PairingStep4_MSG_ID 4

void ig_PairingStep4_init(IgPairingStep4 *obj);
IgSerializerError ig_PairingStep4_encode(IgPairingStep4 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_PairingStep4_decode(uint8_t *buf,size_t buf_size,IgPairingStep4 *retval,size_t index);
uint32_t ig_PairingStep4_get_max_payload_in_bytes(IgPairingStep4 *obj);
bool ig_PairingStep4_is_valid(IgPairingStep4 *obj);
void ig_PairingStep4_deinit(IgPairingStep4 *obj);

void ig_PairingStep4_set_success(IgPairingStep4 *obj,bool success);

size_t ig_PairingStep4_get_master_pin_size(IgPairingStep4 *obj);
void ig_PairingStep4_set_master_pin_nocopy(IgPairingStep4 *obj,uint8_t* master_pin,size_t size);
void ig_PairingStep4_set_master_pin(IgPairingStep4 *obj,uint8_t* master_pin,size_t size);

size_t ig_PairingStep4_get_pin_key_size(IgPairingStep4 *obj);
void ig_PairingStep4_set_pin_key_nocopy(IgPairingStep4 *obj,uint8_t* pin_key,size_t size);
void ig_PairingStep4_set_pin_key(IgPairingStep4 *obj,uint8_t* pin_key,size_t size);

void ig_PairingStep4_set_gmt_offset(IgPairingStep4 *obj,int32_t gmt_offset);

size_t ig_PairingStep4_get_password_size(IgPairingStep4 *obj);
void ig_PairingStep4_set_password_nocopy(IgPairingStep4 *obj,uint8_t* password,size_t size);
void ig_PairingStep4_set_password(IgPairingStep4 *obj,uint8_t* password,size_t size);




#ifdef __cplusplus
}
#endif

#endif
