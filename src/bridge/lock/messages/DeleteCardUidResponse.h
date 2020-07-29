#ifndef H_DELETECARDUIDRESPONSE_
#define H_DELETECARDUIDRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteCardUidResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteCardUidResponse;

#define IG_DeleteCardUidResponse_MSG_ID 70

void ig_DeleteCardUidResponse_init(IgDeleteCardUidResponse *obj);
IgSerializerError ig_DeleteCardUidResponse_encode(IgDeleteCardUidResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteCardUidResponse_decode(uint8_t *buf,size_t buf_size,IgDeleteCardUidResponse *retval,size_t index);
uint32_t ig_DeleteCardUidResponse_get_max_payload_in_bytes(IgDeleteCardUidResponse *obj);
bool ig_DeleteCardUidResponse_is_valid(IgDeleteCardUidResponse *obj);
void ig_DeleteCardUidResponse_deinit(IgDeleteCardUidResponse *obj);

void ig_DeleteCardUidResponse_set_result(IgDeleteCardUidResponse *obj,uint8_t result);

void ig_DeleteCardUidResponse_set_operation_id(IgDeleteCardUidResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
