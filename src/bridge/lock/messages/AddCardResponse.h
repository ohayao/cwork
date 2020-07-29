#ifndef H_ADDCARDRESPONSE_
#define H_ADDCARDRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddCardResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddCardResponse;

#define IG_AddCardResponse_MSG_ID 66

void ig_AddCardResponse_init(IgAddCardResponse *obj);
IgSerializerError ig_AddCardResponse_encode(IgAddCardResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddCardResponse_decode(uint8_t *buf,size_t buf_size,IgAddCardResponse *retval,size_t index);
uint32_t ig_AddCardResponse_get_max_payload_in_bytes(IgAddCardResponse *obj);
bool ig_AddCardResponse_is_valid(IgAddCardResponse *obj);
void ig_AddCardResponse_deinit(IgAddCardResponse *obj);

void ig_AddCardResponse_set_result(IgAddCardResponse *obj,uint8_t result);

void ig_AddCardResponse_set_operation_id(IgAddCardResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
