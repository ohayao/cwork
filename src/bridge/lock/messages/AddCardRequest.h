#ifndef H_ADDCARDREQUEST_
#define H_ADDCARDREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddCardRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_duration;
  //optional
  uint32_t duration;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddCardRequest;

#define IG_AddCardRequest_MSG_ID 65

void ig_AddCardRequest_init(IgAddCardRequest *obj);
IgSerializerError ig_AddCardRequest_encode(IgAddCardRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddCardRequest_decode(uint8_t *buf,size_t buf_size,IgAddCardRequest *retval,size_t index);
uint32_t ig_AddCardRequest_get_max_payload_in_bytes(IgAddCardRequest *obj);
bool ig_AddCardRequest_is_valid(IgAddCardRequest *obj);
void ig_AddCardRequest_deinit(IgAddCardRequest *obj);

size_t ig_AddCardRequest_get_password_size(IgAddCardRequest *obj);
void ig_AddCardRequest_set_password_nocopy(IgAddCardRequest *obj,uint8_t* password,size_t size);
void ig_AddCardRequest_set_password(IgAddCardRequest *obj,uint8_t* password,size_t size);

void ig_AddCardRequest_set_duration(IgAddCardRequest *obj,uint32_t duration);

void ig_AddCardRequest_set_operation_id(IgAddCardRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
