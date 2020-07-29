#ifndef H_SETPINKEYRESPONSE_
#define H_SETPINKEYRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetPinKeyResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_pin_key;
  //optional
  uint8_t* pin_key;
  size_t pin_key_size;
  bool has_gmt_offset;
  //optional
  int32_t gmt_offset;
  bool has_dst_times;
  //optional
  uint8_t* dst_times;
  size_t dst_times_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetPinKeyResponse;

#define IG_SetPinKeyResponse_MSG_ID 36

void ig_SetPinKeyResponse_init(IgSetPinKeyResponse *obj);
IgSerializerError ig_SetPinKeyResponse_encode(IgSetPinKeyResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetPinKeyResponse_decode(uint8_t *buf,size_t buf_size,IgSetPinKeyResponse *retval,size_t index);
uint32_t ig_SetPinKeyResponse_get_max_payload_in_bytes(IgSetPinKeyResponse *obj);
bool ig_SetPinKeyResponse_is_valid(IgSetPinKeyResponse *obj);
void ig_SetPinKeyResponse_deinit(IgSetPinKeyResponse *obj);

void ig_SetPinKeyResponse_set_result(IgSetPinKeyResponse *obj,uint8_t result);

size_t ig_SetPinKeyResponse_get_pin_key_size(IgSetPinKeyResponse *obj);
void ig_SetPinKeyResponse_set_pin_key_nocopy(IgSetPinKeyResponse *obj,uint8_t* pin_key,size_t size);
void ig_SetPinKeyResponse_set_pin_key(IgSetPinKeyResponse *obj,uint8_t* pin_key,size_t size);

void ig_SetPinKeyResponse_set_gmt_offset(IgSetPinKeyResponse *obj,int32_t gmt_offset);

size_t ig_SetPinKeyResponse_get_dst_times_size(IgSetPinKeyResponse *obj);
void ig_SetPinKeyResponse_set_dst_times_nocopy(IgSetPinKeyResponse *obj,uint8_t* dst_times,size_t size);
void ig_SetPinKeyResponse_set_dst_times(IgSetPinKeyResponse *obj,uint8_t* dst_times,size_t size);

void ig_SetPinKeyResponse_set_operation_id(IgSetPinKeyResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
