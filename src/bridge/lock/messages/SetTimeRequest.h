#ifndef H_SETTIMEREQUEST_
#define H_SETTIMEREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetTimeRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_timestamp;
  //required
  uint32_t timestamp;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetTimeRequest;

#define IG_SetTimeRequest_MSG_ID 25

void ig_SetTimeRequest_init(IgSetTimeRequest *obj);
IgSerializerError ig_SetTimeRequest_encode(IgSetTimeRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetTimeRequest_decode(uint8_t *buf,size_t buf_size,IgSetTimeRequest *retval,size_t index);
uint32_t ig_SetTimeRequest_get_max_payload_in_bytes(IgSetTimeRequest *obj);
bool ig_SetTimeRequest_is_valid(IgSetTimeRequest *obj);
void ig_SetTimeRequest_deinit(IgSetTimeRequest *obj);

size_t ig_SetTimeRequest_get_password_size(IgSetTimeRequest *obj);
void ig_SetTimeRequest_set_password_nocopy(IgSetTimeRequest *obj,uint8_t* password,size_t size);
void ig_SetTimeRequest_set_password(IgSetTimeRequest *obj,uint8_t* password,size_t size);

void ig_SetTimeRequest_set_timestamp(IgSetTimeRequest *obj,uint32_t timestamp);

void ig_SetTimeRequest_set_operation_id(IgSetTimeRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
