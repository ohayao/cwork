#ifndef H_SETPINKEYREQUEST_
#define H_SETPINKEYREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetPinKeyRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
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
} IgSetPinKeyRequest;

#define IG_SetPinKeyRequest_MSG_ID 35

void ig_SetPinKeyRequest_init(IgSetPinKeyRequest *obj);
IgSerializerError ig_SetPinKeyRequest_encode(IgSetPinKeyRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetPinKeyRequest_decode(uint8_t *buf,size_t buf_size,IgSetPinKeyRequest *retval,size_t index);
uint32_t ig_SetPinKeyRequest_get_max_payload_in_bytes(IgSetPinKeyRequest *obj);
bool ig_SetPinKeyRequest_is_valid(IgSetPinKeyRequest *obj);
void ig_SetPinKeyRequest_deinit(IgSetPinKeyRequest *obj);

size_t ig_SetPinKeyRequest_get_password_size(IgSetPinKeyRequest *obj);
void ig_SetPinKeyRequest_set_password_nocopy(IgSetPinKeyRequest *obj,uint8_t* password,size_t size);
void ig_SetPinKeyRequest_set_password(IgSetPinKeyRequest *obj,uint8_t* password,size_t size);

size_t ig_SetPinKeyRequest_get_pin_key_size(IgSetPinKeyRequest *obj);
void ig_SetPinKeyRequest_set_pin_key_nocopy(IgSetPinKeyRequest *obj,uint8_t* pin_key,size_t size);
void ig_SetPinKeyRequest_set_pin_key(IgSetPinKeyRequest *obj,uint8_t* pin_key,size_t size);

void ig_SetPinKeyRequest_set_gmt_offset(IgSetPinKeyRequest *obj,int32_t gmt_offset);

size_t ig_SetPinKeyRequest_get_dst_times_size(IgSetPinKeyRequest *obj);
void ig_SetPinKeyRequest_set_dst_times_nocopy(IgSetPinKeyRequest *obj,uint8_t* dst_times,size_t size);
void ig_SetPinKeyRequest_set_dst_times(IgSetPinKeyRequest *obj,uint8_t* dst_times,size_t size);

void ig_SetPinKeyRequest_set_operation_id(IgSetPinKeyRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
