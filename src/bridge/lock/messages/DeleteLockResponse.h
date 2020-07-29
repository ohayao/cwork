#ifndef H_DELETELOCKRESPONSE_
#define H_DELETELOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteLockResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteLockResponse;

#define IG_DeleteLockResponse_MSG_ID 86

void ig_DeleteLockResponse_init(IgDeleteLockResponse *obj);
IgSerializerError ig_DeleteLockResponse_encode(IgDeleteLockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteLockResponse_decode(uint8_t *buf,size_t buf_size,IgDeleteLockResponse *retval,size_t index);
uint32_t ig_DeleteLockResponse_get_max_payload_in_bytes(IgDeleteLockResponse *obj);
bool ig_DeleteLockResponse_is_valid(IgDeleteLockResponse *obj);
void ig_DeleteLockResponse_deinit(IgDeleteLockResponse *obj);

void ig_DeleteLockResponse_set_result(IgDeleteLockResponse *obj,uint8_t result);

void ig_DeleteLockResponse_set_operation_id(IgDeleteLockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
