#ifndef H_LOCKRESPONSE_
#define H_LOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgLockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgLockResponse;

#define IG_LockResponse_MSG_ID 52

void ig_LockResponse_init(IgLockResponse *obj);
IgSerializerError ig_LockResponse_encode(IgLockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_LockResponse_decode(uint8_t *buf,size_t buf_size,IgLockResponse *retval,size_t index);
uint32_t ig_LockResponse_get_max_payload_in_bytes(IgLockResponse *obj);
bool ig_LockResponse_is_valid(IgLockResponse *obj);
void ig_LockResponse_deinit(IgLockResponse *obj);

void ig_LockResponse_set_result(IgLockResponse *obj,uint8_t result);

void ig_LockResponse_set_operation_id(IgLockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
