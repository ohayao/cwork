#ifndef H_LOCKREQUEST_
#define H_LOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgLockRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_timestamp;
  //optional
  uint32_t timestamp;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgLockRequest;

#define IG_LockRequest_MSG_ID 51

void ig_LockRequest_init(IgLockRequest *obj);
IgSerializerError ig_LockRequest_encode(IgLockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_LockRequest_decode(uint8_t *buf,size_t buf_size,IgLockRequest *retval,size_t index);
uint32_t ig_LockRequest_get_max_payload_in_bytes(IgLockRequest *obj);
bool ig_LockRequest_is_valid(IgLockRequest *obj);
void ig_LockRequest_deinit(IgLockRequest *obj);

size_t ig_LockRequest_get_password_size(IgLockRequest *obj);
void ig_LockRequest_set_password_nocopy(IgLockRequest *obj,uint8_t* password,size_t size);
void ig_LockRequest_set_password(IgLockRequest *obj,uint8_t* password,size_t size);

void ig_LockRequest_set_timestamp(IgLockRequest *obj,uint32_t timestamp);

void ig_LockRequest_set_operation_id(IgLockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
