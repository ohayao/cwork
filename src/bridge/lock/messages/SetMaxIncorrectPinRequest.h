#ifndef H_SETMAXINCORRECTPINREQUEST_
#define H_SETMAXINCORRECTPINREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetMaxIncorrectPinRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_max_incorrect_pins;
  //optional
  uint8_t max_incorrect_pins;
  bool has_incorrect_pin_lockout_on;
  //optional
  bool incorrect_pin_lockout_on;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetMaxIncorrectPinRequest;

#define IG_SetMaxIncorrectPinRequest_MSG_ID 39

void ig_SetMaxIncorrectPinRequest_init(IgSetMaxIncorrectPinRequest *obj);
IgSerializerError ig_SetMaxIncorrectPinRequest_encode(IgSetMaxIncorrectPinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetMaxIncorrectPinRequest_decode(uint8_t *buf,size_t buf_size,IgSetMaxIncorrectPinRequest *retval,size_t index);
uint32_t ig_SetMaxIncorrectPinRequest_get_max_payload_in_bytes(IgSetMaxIncorrectPinRequest *obj);
bool ig_SetMaxIncorrectPinRequest_is_valid(IgSetMaxIncorrectPinRequest *obj);
void ig_SetMaxIncorrectPinRequest_deinit(IgSetMaxIncorrectPinRequest *obj);

size_t ig_SetMaxIncorrectPinRequest_get_password_size(IgSetMaxIncorrectPinRequest *obj);
void ig_SetMaxIncorrectPinRequest_set_password_nocopy(IgSetMaxIncorrectPinRequest *obj,uint8_t* password,size_t size);
void ig_SetMaxIncorrectPinRequest_set_password(IgSetMaxIncorrectPinRequest *obj,uint8_t* password,size_t size);

void ig_SetMaxIncorrectPinRequest_set_max_incorrect_pins(IgSetMaxIncorrectPinRequest *obj,uint8_t max_incorrect_pins);

void ig_SetMaxIncorrectPinRequest_set_incorrect_pin_lockout_on(IgSetMaxIncorrectPinRequest *obj,bool incorrect_pin_lockout_on);

void ig_SetMaxIncorrectPinRequest_set_operation_id(IgSetMaxIncorrectPinRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
