#ifndef H_ENABLEAUTOUNLOCKRESPONSE_
#define H_ENABLEAUTOUNLOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEnableAutoUnlockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgEnableAutoUnlockResponse;

#define IG_EnableAutoUnlockResponse_MSG_ID 56

void ig_EnableAutoUnlockResponse_init(IgEnableAutoUnlockResponse *obj);
IgSerializerError ig_EnableAutoUnlockResponse_encode(IgEnableAutoUnlockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EnableAutoUnlockResponse_decode(uint8_t *buf,size_t buf_size,IgEnableAutoUnlockResponse *retval,size_t index);
uint32_t ig_EnableAutoUnlockResponse_get_max_payload_in_bytes(IgEnableAutoUnlockResponse *obj);
bool ig_EnableAutoUnlockResponse_is_valid(IgEnableAutoUnlockResponse *obj);
void ig_EnableAutoUnlockResponse_deinit(IgEnableAutoUnlockResponse *obj);

void ig_EnableAutoUnlockResponse_set_result(IgEnableAutoUnlockResponse *obj,uint8_t result);

void ig_EnableAutoUnlockResponse_set_operation_id(IgEnableAutoUnlockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
