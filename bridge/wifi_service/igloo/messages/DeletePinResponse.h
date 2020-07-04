#ifndef H_DELETEPINRESPONSE_
#define H_DELETEPINRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeletePinResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeletePinResponse;

#define IG_DeletePinResponse_MSG_ID 48

void ig_DeletePinResponse_init(IgDeletePinResponse *obj);
IgSerializerError ig_DeletePinResponse_encode(IgDeletePinResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeletePinResponse_decode(uint8_t *buf,size_t buf_size,IgDeletePinResponse *retval,size_t index);
uint32_t ig_DeletePinResponse_get_max_payload_in_bytes(IgDeletePinResponse *obj);
bool ig_DeletePinResponse_is_valid(IgDeletePinResponse *obj);
void ig_DeletePinResponse_deinit(IgDeletePinResponse *obj);

void ig_DeletePinResponse_set_result(IgDeletePinResponse *obj,uint8_t result);

void ig_DeletePinResponse_set_operation_id(IgDeletePinResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
