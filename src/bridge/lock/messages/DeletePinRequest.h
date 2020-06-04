#ifndef H_DELETEPINREQUEST_
#define H_DELETEPINREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeletePinRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_old_pin;
  //required
  uint8_t* old_pin;
  size_t old_pin_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeletePinRequest;

#define IG_DeletePinRequest_MSG_ID 47

void ig_DeletePinRequest_init(IgDeletePinRequest *obj);
IgSerializerError ig_DeletePinRequest_encode(IgDeletePinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeletePinRequest_decode(uint8_t *buf,size_t buf_size,IgDeletePinRequest *retval,size_t index);
uint32_t ig_DeletePinRequest_get_max_payload_in_bytes(IgDeletePinRequest *obj);
bool ig_DeletePinRequest_is_valid(IgDeletePinRequest *obj);
void ig_DeletePinRequest_deinit(IgDeletePinRequest *obj);

size_t ig_DeletePinRequest_get_password_size(IgDeletePinRequest *obj);
void ig_DeletePinRequest_set_password_nocopy(IgDeletePinRequest *obj,uint8_t* password,size_t size);
void ig_DeletePinRequest_set_password(IgDeletePinRequest *obj,uint8_t* password,size_t size);

size_t ig_DeletePinRequest_get_old_pin_size(IgDeletePinRequest *obj);
void ig_DeletePinRequest_set_old_pin_nocopy(IgDeletePinRequest *obj,uint8_t* old_pin,size_t size);
void ig_DeletePinRequest_set_old_pin(IgDeletePinRequest *obj,uint8_t* old_pin,size_t size);

void ig_DeletePinRequest_set_operation_id(IgDeletePinRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
