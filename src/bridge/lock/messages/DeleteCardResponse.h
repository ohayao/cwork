#ifndef H_DELETECARDRESPONSE_
#define H_DELETECARDRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteCardResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteCardResponse;

#define IG_DeleteCardResponse_MSG_ID 68

void ig_DeleteCardResponse_init(IgDeleteCardResponse *obj);
IgSerializerError ig_DeleteCardResponse_encode(IgDeleteCardResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteCardResponse_decode(uint8_t *buf,size_t buf_size,IgDeleteCardResponse *retval,size_t index);
uint32_t ig_DeleteCardResponse_get_max_payload_in_bytes(IgDeleteCardResponse *obj);
bool ig_DeleteCardResponse_is_valid(IgDeleteCardResponse *obj);
void ig_DeleteCardResponse_deinit(IgDeleteCardResponse *obj);

void ig_DeleteCardResponse_set_result(IgDeleteCardResponse *obj,uint8_t result);

void ig_DeleteCardResponse_set_operation_id(IgDeleteCardResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
