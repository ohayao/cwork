#ifndef H_PAIRINGCOMMIT_
#define H_PAIRINGCOMMIT_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgPairingCommit {
  bool has_current_time;
  //required
  uint32_t current_time;
} IgPairingCommit;

#define IG_PairingCommit_MSG_ID 5

void ig_PairingCommit_init(IgPairingCommit *obj);
IgSerializerError ig_PairingCommit_encode(IgPairingCommit *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_PairingCommit_decode(uint8_t *buf,size_t buf_size,IgPairingCommit *retval,size_t index);
uint32_t ig_PairingCommit_get_max_payload_in_bytes(IgPairingCommit *obj);
bool ig_PairingCommit_is_valid(IgPairingCommit *obj);
void ig_PairingCommit_deinit(IgPairingCommit *obj);

void ig_PairingCommit_set_current_time(IgPairingCommit *obj,uint32_t current_time);




#ifdef __cplusplus
}
#endif

#endif
