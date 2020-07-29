#ifndef H_BLACKLISTGUESTKEYRESPONSE_
#define H_BLACKLISTGUESTKEYRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgBlacklistGuestKeyResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgBlacklistGuestKeyResponse;

#define IG_BlacklistGuestKeyResponse_MSG_ID 58

void ig_BlacklistGuestKeyResponse_init(IgBlacklistGuestKeyResponse *obj);
IgSerializerError ig_BlacklistGuestKeyResponse_encode(IgBlacklistGuestKeyResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_BlacklistGuestKeyResponse_decode(uint8_t *buf,size_t buf_size,IgBlacklistGuestKeyResponse *retval,size_t index);
uint32_t ig_BlacklistGuestKeyResponse_get_max_payload_in_bytes(IgBlacklistGuestKeyResponse *obj);
bool ig_BlacklistGuestKeyResponse_is_valid(IgBlacklistGuestKeyResponse *obj);
void ig_BlacklistGuestKeyResponse_deinit(IgBlacklistGuestKeyResponse *obj);

void ig_BlacklistGuestKeyResponse_set_result(IgBlacklistGuestKeyResponse *obj,uint8_t result);

void ig_BlacklistGuestKeyResponse_set_operation_id(IgBlacklistGuestKeyResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
