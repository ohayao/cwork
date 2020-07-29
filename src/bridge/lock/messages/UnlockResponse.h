#ifndef H_UNLOCKRESPONSE_
#define H_UNLOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnlockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgUnlockResponse;

#define IG_UnlockResponse_MSG_ID 22

void ig_UnlockResponse_init(IgUnlockResponse *obj);
IgSerializerError ig_UnlockResponse_encode(IgUnlockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnlockResponse_decode(uint8_t *buf,size_t buf_size,IgUnlockResponse *retval,size_t index);
uint32_t ig_UnlockResponse_get_max_payload_in_bytes(IgUnlockResponse *obj);
bool ig_UnlockResponse_is_valid(IgUnlockResponse *obj);
void ig_UnlockResponse_deinit(IgUnlockResponse *obj);

void ig_UnlockResponse_set_result(IgUnlockResponse *obj,uint8_t result);

void ig_UnlockResponse_set_operation_id(IgUnlockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
