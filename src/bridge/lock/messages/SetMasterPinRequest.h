#ifndef H_SETMASTERPINREQUEST_
#define H_SETMASTERPINREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetMasterPinRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_pin;
  //required
  uint8_t* pin;
  size_t pin_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetMasterPinRequest;

#define IG_SetMasterPinRequest_MSG_ID 49

void ig_SetMasterPinRequest_init(IgSetMasterPinRequest *obj);
IgSerializerError ig_SetMasterPinRequest_encode(IgSetMasterPinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetMasterPinRequest_decode(uint8_t *buf,size_t buf_size,IgSetMasterPinRequest *retval,size_t index);
uint32_t ig_SetMasterPinRequest_get_max_payload_in_bytes(IgSetMasterPinRequest *obj);
bool ig_SetMasterPinRequest_is_valid(IgSetMasterPinRequest *obj);
void ig_SetMasterPinRequest_deinit(IgSetMasterPinRequest *obj);

size_t ig_SetMasterPinRequest_get_password_size(IgSetMasterPinRequest *obj);
void ig_SetMasterPinRequest_set_password_nocopy(IgSetMasterPinRequest *obj,uint8_t* password,size_t size);
void ig_SetMasterPinRequest_set_password(IgSetMasterPinRequest *obj,uint8_t* password,size_t size);

size_t ig_SetMasterPinRequest_get_pin_size(IgSetMasterPinRequest *obj);
void ig_SetMasterPinRequest_set_pin_nocopy(IgSetMasterPinRequest *obj,uint8_t* pin,size_t size);
void ig_SetMasterPinRequest_set_pin(IgSetMasterPinRequest *obj,uint8_t* pin,size_t size);

void ig_SetMasterPinRequest_set_operation_id(IgSetMasterPinRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
