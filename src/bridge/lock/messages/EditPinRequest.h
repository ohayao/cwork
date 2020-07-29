#ifndef H_EDITPINREQUEST_
#define H_EDITPINREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEditPinRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_old_pin;
  //required
  uint8_t* old_pin;
  size_t old_pin_size;
  bool has_new_pin;
  //optional
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
} IgEditPinRequest;

#define IG_EditPinRequest_MSG_ID 45

void ig_EditPinRequest_init(IgEditPinRequest *obj);
IgSerializerError ig_EditPinRequest_encode(IgEditPinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EditPinRequest_decode(uint8_t *buf,size_t buf_size,IgEditPinRequest *retval,size_t index);
uint32_t ig_EditPinRequest_get_max_payload_in_bytes(IgEditPinRequest *obj);
bool ig_EditPinRequest_is_valid(IgEditPinRequest *obj);
void ig_EditPinRequest_deinit(IgEditPinRequest *obj);

size_t ig_EditPinRequest_get_password_size(IgEditPinRequest *obj);
void ig_EditPinRequest_set_password_nocopy(IgEditPinRequest *obj,uint8_t* password,size_t size);
void ig_EditPinRequest_set_password(IgEditPinRequest *obj,uint8_t* password,size_t size);

size_t ig_EditPinRequest_get_old_pin_size(IgEditPinRequest *obj);
void ig_EditPinRequest_set_old_pin_nocopy(IgEditPinRequest *obj,uint8_t* old_pin,size_t size);
void ig_EditPinRequest_set_old_pin(IgEditPinRequest *obj,uint8_t* old_pin,size_t size);

size_t ig_EditPinRequest_get_new_pin_size(IgEditPinRequest *obj);
void ig_EditPinRequest_set_new_pin_nocopy(IgEditPinRequest *obj,uint8_t* new_pin,size_t size);
void ig_EditPinRequest_set_new_pin(IgEditPinRequest *obj,uint8_t* new_pin,size_t size);

void ig_EditPinRequest_set_start_date(IgEditPinRequest *obj,uint32_t start_date);

void ig_EditPinRequest_set_end_date(IgEditPinRequest *obj,uint32_t end_date);

void ig_EditPinRequest_set_pin_type(IgEditPinRequest *obj,uint8_t pin_type);

void ig_EditPinRequest_set_operation_id(IgEditPinRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
