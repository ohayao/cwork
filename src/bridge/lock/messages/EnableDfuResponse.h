#ifndef H_ENABLEDFURESPONSE_
#define H_ENABLEDFURESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEnableDfuResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgEnableDfuResponse;

#define IG_EnableDfuResponse_MSG_ID 62

void ig_EnableDfuResponse_init(IgEnableDfuResponse *obj);
IgSerializerError ig_EnableDfuResponse_encode(IgEnableDfuResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EnableDfuResponse_decode(uint8_t *buf,size_t buf_size,IgEnableDfuResponse *retval,size_t index);
uint32_t ig_EnableDfuResponse_get_max_payload_in_bytes(IgEnableDfuResponse *obj);
bool ig_EnableDfuResponse_is_valid(IgEnableDfuResponse *obj);
void ig_EnableDfuResponse_deinit(IgEnableDfuResponse *obj);

void ig_EnableDfuResponse_set_result(IgEnableDfuResponse *obj,uint8_t result);

void ig_EnableDfuResponse_set_operation_id(IgEnableDfuResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
