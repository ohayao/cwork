#ifndef H_SETAUTORELOCKRESPONSE_
#define H_SETAUTORELOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetAutorelockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetAutorelockResponse;

#define IG_SetAutorelockResponse_MSG_ID 38

void ig_SetAutorelockResponse_init(IgSetAutorelockResponse *obj);
IgSerializerError ig_SetAutorelockResponse_encode(IgSetAutorelockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetAutorelockResponse_decode(uint8_t *buf,size_t buf_size,IgSetAutorelockResponse *retval,size_t index);
uint32_t ig_SetAutorelockResponse_get_max_payload_in_bytes(IgSetAutorelockResponse *obj);
bool ig_SetAutorelockResponse_is_valid(IgSetAutorelockResponse *obj);
void ig_SetAutorelockResponse_deinit(IgSetAutorelockResponse *obj);

void ig_SetAutorelockResponse_set_result(IgSetAutorelockResponse *obj,uint8_t result);

void ig_SetAutorelockResponse_set_operation_id(IgSetAutorelockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
