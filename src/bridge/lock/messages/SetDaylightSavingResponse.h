#ifndef H_SETDAYLIGHTSAVINGRESPONSE_
#define H_SETDAYLIGHTSAVINGRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetDaylightSavingResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetDaylightSavingResponse;

#define IG_SetDaylightSavingResponse_MSG_ID 64

void ig_SetDaylightSavingResponse_init(IgSetDaylightSavingResponse *obj);
IgSerializerError ig_SetDaylightSavingResponse_encode(IgSetDaylightSavingResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetDaylightSavingResponse_decode(uint8_t *buf,size_t buf_size,IgSetDaylightSavingResponse *retval,size_t index);
uint32_t ig_SetDaylightSavingResponse_get_max_payload_in_bytes(IgSetDaylightSavingResponse *obj);
bool ig_SetDaylightSavingResponse_is_valid(IgSetDaylightSavingResponse *obj);
void ig_SetDaylightSavingResponse_deinit(IgSetDaylightSavingResponse *obj);

void ig_SetDaylightSavingResponse_set_result(IgSetDaylightSavingResponse *obj,uint8_t result);

void ig_SetDaylightSavingResponse_set_operation_id(IgSetDaylightSavingResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
