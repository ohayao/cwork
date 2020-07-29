#ifndef H_GETTIMERESPONSE_
#define H_GETTIMERESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetTimeResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_timestamp;
  //optional
  uint32_t timestamp;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetTimeResponse;

#define IG_GetTimeResponse_MSG_ID 28

void ig_GetTimeResponse_init(IgGetTimeResponse *obj);
IgSerializerError ig_GetTimeResponse_encode(IgGetTimeResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetTimeResponse_decode(uint8_t *buf,size_t buf_size,IgGetTimeResponse *retval,size_t index);
uint32_t ig_GetTimeResponse_get_max_payload_in_bytes(IgGetTimeResponse *obj);
bool ig_GetTimeResponse_is_valid(IgGetTimeResponse *obj);
void ig_GetTimeResponse_deinit(IgGetTimeResponse *obj);

void ig_GetTimeResponse_set_result(IgGetTimeResponse *obj,uint8_t result);

void ig_GetTimeResponse_set_timestamp(IgGetTimeResponse *obj,uint32_t timestamp);

void ig_GetTimeResponse_set_operation_id(IgGetTimeResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
