#ifndef H_UNBLACKLISTGUESTKEYRESPONSE_
#define H_UNBLACKLISTGUESTKEYRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnblacklistGuestKeyResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgUnblacklistGuestKeyResponse;

#define IG_UnblacklistGuestKeyResponse_MSG_ID 60

void ig_UnblacklistGuestKeyResponse_init(IgUnblacklistGuestKeyResponse *obj);
IgSerializerError ig_UnblacklistGuestKeyResponse_encode(IgUnblacklistGuestKeyResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnblacklistGuestKeyResponse_decode(uint8_t *buf,size_t buf_size,IgUnblacklistGuestKeyResponse *retval,size_t index);
uint32_t ig_UnblacklistGuestKeyResponse_get_max_payload_in_bytes(IgUnblacklistGuestKeyResponse *obj);
bool ig_UnblacklistGuestKeyResponse_is_valid(IgUnblacklistGuestKeyResponse *obj);
void ig_UnblacklistGuestKeyResponse_deinit(IgUnblacklistGuestKeyResponse *obj);

void ig_UnblacklistGuestKeyResponse_set_result(IgUnblacklistGuestKeyResponse *obj,uint8_t result);

void ig_UnblacklistGuestKeyResponse_set_operation_id(IgUnblacklistGuestKeyResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
