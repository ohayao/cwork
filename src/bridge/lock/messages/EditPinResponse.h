#ifndef H_EDITPINRESPONSE_
#define H_EDITPINRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEditPinResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgEditPinResponse;

#define IG_EditPinResponse_MSG_ID 46

void ig_EditPinResponse_init(IgEditPinResponse *obj);
IgSerializerError ig_EditPinResponse_encode(IgEditPinResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EditPinResponse_decode(uint8_t *buf,size_t buf_size,IgEditPinResponse *retval,size_t index);
uint32_t ig_EditPinResponse_get_max_payload_in_bytes(IgEditPinResponse *obj);
bool ig_EditPinResponse_is_valid(IgEditPinResponse *obj);
void ig_EditPinResponse_deinit(IgEditPinResponse *obj);

void ig_EditPinResponse_set_result(IgEditPinResponse *obj,uint8_t result);

void ig_EditPinResponse_set_operation_id(IgEditPinResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
