#ifndef H_SETBRIGHTNESSREQUEST_
#define H_SETBRIGHTNESSREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetBrightnessRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_brightness;
  //optional
  uint8_t brightness;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetBrightnessRequest;

#define IG_SetBrightnessRequest_MSG_ID 71

void ig_SetBrightnessRequest_init(IgSetBrightnessRequest *obj);
IgSerializerError ig_SetBrightnessRequest_encode(IgSetBrightnessRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetBrightnessRequest_decode(uint8_t *buf,size_t buf_size,IgSetBrightnessRequest *retval,size_t index);
uint32_t ig_SetBrightnessRequest_get_max_payload_in_bytes(IgSetBrightnessRequest *obj);
bool ig_SetBrightnessRequest_is_valid(IgSetBrightnessRequest *obj);
void ig_SetBrightnessRequest_deinit(IgSetBrightnessRequest *obj);

size_t ig_SetBrightnessRequest_get_password_size(IgSetBrightnessRequest *obj);
void ig_SetBrightnessRequest_set_password_nocopy(IgSetBrightnessRequest *obj,uint8_t* password,size_t size);
void ig_SetBrightnessRequest_set_password(IgSetBrightnessRequest *obj,uint8_t* password,size_t size);

void ig_SetBrightnessRequest_set_brightness(IgSetBrightnessRequest *obj,uint8_t brightness);

void ig_SetBrightnessRequest_set_operation_id(IgSetBrightnessRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
