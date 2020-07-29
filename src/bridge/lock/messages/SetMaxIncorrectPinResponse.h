#ifndef H_SETMAXINCORRECTPINRESPONSE_
#define H_SETMAXINCORRECTPINRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetMaxIncorrectPinResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetMaxIncorrectPinResponse;

#define IG_SetMaxIncorrectPinResponse_MSG_ID 40

void ig_SetMaxIncorrectPinResponse_init(IgSetMaxIncorrectPinResponse *obj);
IgSerializerError ig_SetMaxIncorrectPinResponse_encode(IgSetMaxIncorrectPinResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetMaxIncorrectPinResponse_decode(uint8_t *buf,size_t buf_size,IgSetMaxIncorrectPinResponse *retval,size_t index);
uint32_t ig_SetMaxIncorrectPinResponse_get_max_payload_in_bytes(IgSetMaxIncorrectPinResponse *obj);
bool ig_SetMaxIncorrectPinResponse_is_valid(IgSetMaxIncorrectPinResponse *obj);
void ig_SetMaxIncorrectPinResponse_deinit(IgSetMaxIncorrectPinResponse *obj);

void ig_SetMaxIncorrectPinResponse_set_result(IgSetMaxIncorrectPinResponse *obj,uint8_t result);

void ig_SetMaxIncorrectPinResponse_set_operation_id(IgSetMaxIncorrectPinResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
