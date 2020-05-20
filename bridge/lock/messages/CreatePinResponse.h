#ifndef H_CREATEPINRESPONSE_
#define H_CREATEPINRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgCreatePinResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgCreatePinResponse;

#define IG_CreatePinResponse_MSG_ID 44

void ig_CreatePinResponse_init(IgCreatePinResponse *obj);
IgSerializerError ig_CreatePinResponse_encode(IgCreatePinResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_CreatePinResponse_decode(uint8_t *buf,size_t buf_size,IgCreatePinResponse *retval,size_t index);
uint32_t ig_CreatePinResponse_get_max_payload_in_bytes(IgCreatePinResponse *obj);
bool ig_CreatePinResponse_is_valid(IgCreatePinResponse *obj);
void ig_CreatePinResponse_deinit(IgCreatePinResponse *obj);

void ig_CreatePinResponse_set_result(IgCreatePinResponse *obj,uint8_t result);

void ig_CreatePinResponse_set_operation_id(IgCreatePinResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
