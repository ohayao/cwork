#ifndef H_GETLOCKSTATUSREQUEST_
#define H_GETLOCKSTATUSREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetLockStatusRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetLockStatusRequest;

#define IG_GetLockStatusRequest_MSG_ID 31

void ig_GetLockStatusRequest_init(IgGetLockStatusRequest *obj);
IgSerializerError ig_GetLockStatusRequest_encode(IgGetLockStatusRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetLockStatusRequest_decode(uint8_t *buf,size_t buf_size,IgGetLockStatusRequest *retval,size_t index);
uint32_t ig_GetLockStatusRequest_get_max_payload_in_bytes(IgGetLockStatusRequest *obj);
bool ig_GetLockStatusRequest_is_valid(IgGetLockStatusRequest *obj);
void ig_GetLockStatusRequest_deinit(IgGetLockStatusRequest *obj);

size_t ig_GetLockStatusRequest_get_password_size(IgGetLockStatusRequest *obj);
void ig_GetLockStatusRequest_set_password_nocopy(IgGetLockStatusRequest *obj,uint8_t* password,size_t size);
void ig_GetLockStatusRequest_set_password(IgGetLockStatusRequest *obj,uint8_t* password,size_t size);

void ig_GetLockStatusRequest_set_operation_id(IgGetLockStatusRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
