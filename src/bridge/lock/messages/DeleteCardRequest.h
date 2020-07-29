#ifndef H_DELETECARDREQUEST_
#define H_DELETECARDREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteCardRequest {
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
} IgDeleteCardRequest;

#define IG_DeleteCardRequest_MSG_ID 67

void ig_DeleteCardRequest_init(IgDeleteCardRequest *obj);
IgSerializerError ig_DeleteCardRequest_encode(IgDeleteCardRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteCardRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteCardRequest *retval,size_t index);
uint32_t ig_DeleteCardRequest_get_max_payload_in_bytes(IgDeleteCardRequest *obj);
bool ig_DeleteCardRequest_is_valid(IgDeleteCardRequest *obj);
void ig_DeleteCardRequest_deinit(IgDeleteCardRequest *obj);

size_t ig_DeleteCardRequest_get_password_size(IgDeleteCardRequest *obj);
void ig_DeleteCardRequest_set_password_nocopy(IgDeleteCardRequest *obj,uint8_t* password,size_t size);
void ig_DeleteCardRequest_set_password(IgDeleteCardRequest *obj,uint8_t* password,size_t size);

void ig_DeleteCardRequest_set_duration(IgDeleteCardRequest *obj,uint32_t duration);

void ig_DeleteCardRequest_set_operation_id(IgDeleteCardRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
