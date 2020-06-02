#ifndef H_PAIRINGSTEP1_
#define H_PAIRINGSTEP1_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgPairingStep1 {
  bool has_public_key;
  //required
  uint8_t* public_key;
  size_t public_key_size;
} IgPairingStep1;

#define IG_PairingStep1_MSG_ID 1

void ig_PairingStep1_init(IgPairingStep1 *obj);
IgSerializerError ig_PairingStep1_encode(IgPairingStep1 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_PairingStep1_decode(uint8_t *buf,size_t buf_size,IgPairingStep1 *retval,size_t index);
uint32_t ig_PairingStep1_get_max_payload_in_bytes(IgPairingStep1 *obj);
bool ig_PairingStep1_is_valid(IgPairingStep1 *obj);
void ig_PairingStep1_deinit(IgPairingStep1 *obj);

size_t ig_PairingStep1_get_public_key_size(IgPairingStep1 *obj);
void ig_PairingStep1_set_public_key_nocopy(IgPairingStep1 *obj,uint8_t* public_key,size_t size);
void ig_PairingStep1_set_public_key(IgPairingStep1 *obj,uint8_t* public_key,size_t size);




#ifdef __cplusplus
}
#endif

#endif
