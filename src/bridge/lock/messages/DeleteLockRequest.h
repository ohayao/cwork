#ifndef H_DELETELOCKREQUEST_
#define H_DELETELOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteLockRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_lock_id;
  //optional
  uint8_t* lock_id;
  size_t lock_id_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteLockRequest;

#define IG_DeleteLockRequest_MSG_ID 85

void ig_DeleteLockRequest_init(IgDeleteLockRequest *obj);
IgSerializerError ig_DeleteLockRequest_encode(IgDeleteLockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteLockRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteLockRequest *retval,size_t index);
uint32_t ig_DeleteLockRequest_get_max_payload_in_bytes(IgDeleteLockRequest *obj);
bool ig_DeleteLockRequest_is_valid(IgDeleteLockRequest *obj);
void ig_DeleteLockRequest_deinit(IgDeleteLockRequest *obj);

size_t ig_DeleteLockRequest_get_password_size(IgDeleteLockRequest *obj);
void ig_DeleteLockRequest_set_password_nocopy(IgDeleteLockRequest *obj,uint8_t* password,size_t size);
void ig_DeleteLockRequest_set_password(IgDeleteLockRequest *obj,uint8_t* password,size_t size);

size_t ig_DeleteLockRequest_get_lock_id_size(IgDeleteLockRequest *obj);
void ig_DeleteLockRequest_set_lock_id_nocopy(IgDeleteLockRequest *obj,uint8_t* lock_id,size_t size);
void ig_DeleteLockRequest_set_lock_id(IgDeleteLockRequest *obj,uint8_t* lock_id,size_t size);

void ig_DeleteLockRequest_set_operation_id(IgDeleteLockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
