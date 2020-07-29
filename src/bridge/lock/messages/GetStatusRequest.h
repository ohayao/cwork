#ifndef H_GETSTATUSREQUEST_
#define H_GETSTATUSREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetStatusRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetStatusRequest;

#define IG_GetStatusRequest_MSG_ID 31

void ig_GetStatusRequest_init(IgGetStatusRequest *obj);
IgSerializerError ig_GetStatusRequest_encode(IgGetStatusRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetStatusRequest_decode(uint8_t *buf,size_t buf_size,IgGetStatusRequest *retval,size_t index);
uint32_t ig_GetStatusRequest_get_max_payload_in_bytes(IgGetStatusRequest *obj);
bool ig_GetStatusRequest_is_valid(IgGetStatusRequest *obj);
void ig_GetStatusRequest_deinit(IgGetStatusRequest *obj);

size_t ig_GetStatusRequest_get_password_size(IgGetStatusRequest *obj);
void ig_GetStatusRequest_set_password_nocopy(IgGetStatusRequest *obj,uint8_t* password,size_t size);
void ig_GetStatusRequest_set_password(IgGetStatusRequest *obj,uint8_t* password,size_t size);

void ig_GetStatusRequest_set_operation_id(IgGetStatusRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
