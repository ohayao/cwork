#ifndef H_CREATEPINREQUEST_
#define H_CREATEPINREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgCreatePinRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_new_pin;
  //required
  uint8_t* new_pin;
  size_t new_pin_size;
  bool has_start_date;
  //optional
  uint32_t start_date;
  bool has_end_date;
  //optional
  uint32_t end_date;
  bool has_pin_type;
  //optional
  uint8_t pin_type;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgCreatePinRequest;

#define IG_CreatePinRequest_MSG_ID 43

void ig_CreatePinRequest_init(IgCreatePinRequest *obj);
IgSerializerError ig_CreatePinRequest_encode(IgCreatePinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_CreatePinRequest_decode(uint8_t *buf,size_t buf_size,IgCreatePinRequest *retval,size_t index);
uint32_t ig_CreatePinRequest_get_max_payload_in_bytes(IgCreatePinRequest *obj);
bool ig_CreatePinRequest_is_valid(IgCreatePinRequest *obj);
void ig_CreatePinRequest_deinit(IgCreatePinRequest *obj);

size_t ig_CreatePinRequest_get_password_size(IgCreatePinRequest *obj);
void ig_CreatePinRequest_set_password_nocopy(IgCreatePinRequest *obj,uint8_t* password,size_t size);
void ig_CreatePinRequest_set_password(IgCreatePinRequest *obj,uint8_t* password,size_t size);

size_t ig_CreatePinRequest_get_new_pin_size(IgCreatePinRequest *obj);
void ig_CreatePinRequest_set_new_pin_nocopy(IgCreatePinRequest *obj,uint8_t* new_pin,size_t size);
void ig_CreatePinRequest_set_new_pin(IgCreatePinRequest *obj,uint8_t* new_pin,size_t size);

void ig_CreatePinRequest_set_start_date(IgCreatePinRequest *obj,uint32_t start_date);

void ig_CreatePinRequest_set_end_date(IgCreatePinRequest *obj,uint32_t end_date);

void ig_CreatePinRequest_set_pin_type(IgCreatePinRequest *obj,uint8_t pin_type);

void ig_CreatePinRequest_set_operation_id(IgCreatePinRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
