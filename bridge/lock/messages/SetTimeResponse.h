#ifndef H_SETTIMERESPONSE_
#define H_SETTIMERESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetTimeResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetTimeResponse;

#define IG_SetTimeResponse_MSG_ID 26

void ig_SetTimeResponse_init(IgSetTimeResponse *obj);
IgSerializerError ig_SetTimeResponse_encode(IgSetTimeResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetTimeResponse_decode(uint8_t *buf,size_t buf_size,IgSetTimeResponse *retval,size_t index);
uint32_t ig_SetTimeResponse_get_max_payload_in_bytes(IgSetTimeResponse *obj);
bool ig_SetTimeResponse_is_valid(IgSetTimeResponse *obj);
void ig_SetTimeResponse_deinit(IgSetTimeResponse *obj);

void ig_SetTimeResponse_set_result(IgSetTimeResponse *obj,uint8_t result);

void ig_SetTimeResponse_set_operation_id(IgSetTimeResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
