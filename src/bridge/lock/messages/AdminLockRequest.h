#ifndef H_ADMINLOCKREQUEST_
#define H_ADMINLOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminLockRequest {
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
} IgAdminLockRequest;

#define IG_AdminLockRequest_MSG_ID 51

void ig_AdminLockRequest_init(IgAdminLockRequest *obj);
IgSerializerError ig_AdminLockRequest_encode(IgAdminLockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminLockRequest_decode(uint8_t *buf,size_t buf_size,IgAdminLockRequest *retval,size_t index);
uint32_t ig_AdminLockRequest_get_max_payload_in_bytes(IgAdminLockRequest *obj);
bool ig_AdminLockRequest_is_valid(IgAdminLockRequest *obj);
void ig_AdminLockRequest_deinit(IgAdminLockRequest *obj);

size_t ig_AdminLockRequest_get_password_size(IgAdminLockRequest *obj);
void ig_AdminLockRequest_set_password_nocopy(IgAdminLockRequest *obj,uint8_t* password,size_t size);
void ig_AdminLockRequest_set_password(IgAdminLockRequest *obj,uint8_t* password,size_t size);

void ig_AdminLockRequest_set_timestamp(IgAdminLockRequest *obj,uint32_t timestamp);

void ig_AdminLockRequest_set_operation_id(IgAdminLockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
