#ifndef H_SETRELOCKALARMRESPONSE_
#define H_SETRELOCKALARMRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetRelockAlarmResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetRelockAlarmResponse;

#define IG_SetRelockAlarmResponse_MSG_ID 74

void ig_SetRelockAlarmResponse_init(IgSetRelockAlarmResponse *obj);
IgSerializerError ig_SetRelockAlarmResponse_encode(IgSetRelockAlarmResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetRelockAlarmResponse_decode(uint8_t *buf,size_t buf_size,IgSetRelockAlarmResponse *retval,size_t index);
uint32_t ig_SetRelockAlarmResponse_get_max_payload_in_bytes(IgSetRelockAlarmResponse *obj);
bool ig_SetRelockAlarmResponse_is_valid(IgSetRelockAlarmResponse *obj);
void ig_SetRelockAlarmResponse_deinit(IgSetRelockAlarmResponse *obj);

void ig_SetRelockAlarmResponse_set_result(IgSetRelockAlarmResponse *obj,uint8_t result);

void ig_SetRelockAlarmResponse_set_operation_id(IgSetRelockAlarmResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
