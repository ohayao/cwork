#ifndef H_ADDLOCKRESPONSE_
#define H_ADDLOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddLockResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddLockResponse;

#define IG_AddLockResponse_MSG_ID 84

void ig_AddLockResponse_init(IgAddLockResponse *obj);
IgSerializerError ig_AddLockResponse_encode(IgAddLockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddLockResponse_decode(uint8_t *buf,size_t buf_size,IgAddLockResponse *retval,size_t index);
uint32_t ig_AddLockResponse_get_max_payload_in_bytes(IgAddLockResponse *obj);
bool ig_AddLockResponse_is_valid(IgAddLockResponse *obj);
void ig_AddLockResponse_deinit(IgAddLockResponse *obj);

void ig_AddLockResponse_set_result(IgAddLockResponse *obj,uint8_t result);

void ig_AddLockResponse_set_operation_id(IgAddLockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
