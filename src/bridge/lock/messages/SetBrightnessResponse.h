#ifndef H_SETBRIGHTNESSRESPONSE_
#define H_SETBRIGHTNESSRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetBrightnessResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetBrightnessResponse;

#define IG_SetBrightnessResponse_MSG_ID 72

void ig_SetBrightnessResponse_init(IgSetBrightnessResponse *obj);
IgSerializerError ig_SetBrightnessResponse_encode(IgSetBrightnessResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetBrightnessResponse_decode(uint8_t *buf,size_t buf_size,IgSetBrightnessResponse *retval,size_t index);
uint32_t ig_SetBrightnessResponse_get_max_payload_in_bytes(IgSetBrightnessResponse *obj);
bool ig_SetBrightnessResponse_is_valid(IgSetBrightnessResponse *obj);
void ig_SetBrightnessResponse_deinit(IgSetBrightnessResponse *obj);

void ig_SetBrightnessResponse_set_result(IgSetBrightnessResponse *obj,uint8_t result);

void ig_SetBrightnessResponse_set_operation_id(IgSetBrightnessResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
