#ifndef H_GETTIMEREQUEST_
#define H_GETTIMEREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetTimeRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetTimeRequest;

#define IG_GetTimeRequest_MSG_ID 27

void ig_GetTimeRequest_init(IgGetTimeRequest *obj);
IgSerializerError ig_GetTimeRequest_encode(IgGetTimeRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetTimeRequest_decode(uint8_t *buf,size_t buf_size,IgGetTimeRequest *retval,size_t index);
uint32_t ig_GetTimeRequest_get_max_payload_in_bytes(IgGetTimeRequest *obj);
bool ig_GetTimeRequest_is_valid(IgGetTimeRequest *obj);
void ig_GetTimeRequest_deinit(IgGetTimeRequest *obj);

size_t ig_GetTimeRequest_get_password_size(IgGetTimeRequest *obj);
void ig_GetTimeRequest_set_password_nocopy(IgGetTimeRequest *obj,uint8_t* password,size_t size);
void ig_GetTimeRequest_set_password(IgGetTimeRequest *obj,uint8_t* password,size_t size);

void ig_GetTimeRequest_set_operation_id(IgGetTimeRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
