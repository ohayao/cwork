#ifndef H_SETMASTERPINRESPONSE_
#define H_SETMASTERPINRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetMasterPinResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetMasterPinResponse;

#define IG_SetMasterPinResponse_MSG_ID 50

void ig_SetMasterPinResponse_init(IgSetMasterPinResponse *obj);
IgSerializerError ig_SetMasterPinResponse_encode(IgSetMasterPinResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetMasterPinResponse_decode(uint8_t *buf,size_t buf_size,IgSetMasterPinResponse *retval,size_t index);
uint32_t ig_SetMasterPinResponse_get_max_payload_in_bytes(IgSetMasterPinResponse *obj);
bool ig_SetMasterPinResponse_is_valid(IgSetMasterPinResponse *obj);
void ig_SetMasterPinResponse_deinit(IgSetMasterPinResponse *obj);

void ig_SetMasterPinResponse_set_result(IgSetMasterPinResponse *obj,uint8_t result);

void ig_SetMasterPinResponse_set_operation_id(IgSetMasterPinResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
