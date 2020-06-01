#ifndef H_UNPAIRRESPONSE_
#define H_UNPAIRRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnpairResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgUnpairResponse;

#define IG_UnpairResponse_MSG_ID 16

void ig_UnpairResponse_init(IgUnpairResponse *obj);
IgSerializerError ig_UnpairResponse_encode(IgUnpairResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnpairResponse_decode(uint8_t *buf,size_t buf_size,IgUnpairResponse *retval,size_t index);
uint32_t ig_UnpairResponse_get_max_payload_in_bytes(IgUnpairResponse *obj);
bool ig_UnpairResponse_is_valid(IgUnpairResponse *obj);
void ig_UnpairResponse_deinit(IgUnpairResponse *obj);

void ig_UnpairResponse_set_result(IgUnpairResponse *obj,uint8_t result);

void ig_UnpairResponse_set_operation_id(IgUnpairResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
