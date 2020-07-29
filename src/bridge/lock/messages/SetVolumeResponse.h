#ifndef H_SETVOLUMERESPONSE_
#define H_SETVOLUMERESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetVolumeResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_old_volume;
  //optional
  uint32_t old_volume;
  bool has_new_volume;
  //optional
  uint32_t new_volume;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetVolumeResponse;

#define IG_SetVolumeResponse_MSG_ID 34

void ig_SetVolumeResponse_init(IgSetVolumeResponse *obj);
IgSerializerError ig_SetVolumeResponse_encode(IgSetVolumeResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetVolumeResponse_decode(uint8_t *buf,size_t buf_size,IgSetVolumeResponse *retval,size_t index);
uint32_t ig_SetVolumeResponse_get_max_payload_in_bytes(IgSetVolumeResponse *obj);
bool ig_SetVolumeResponse_is_valid(IgSetVolumeResponse *obj);
void ig_SetVolumeResponse_deinit(IgSetVolumeResponse *obj);

void ig_SetVolumeResponse_set_result(IgSetVolumeResponse *obj,uint8_t result);

void ig_SetVolumeResponse_set_old_volume(IgSetVolumeResponse *obj,uint32_t old_volume);

void ig_SetVolumeResponse_set_new_volume(IgSetVolumeResponse *obj,uint32_t new_volume);

void ig_SetVolumeResponse_set_operation_id(IgSetVolumeResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
