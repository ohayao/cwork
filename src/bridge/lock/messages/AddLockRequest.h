#ifndef H_ADDLOCKREQUEST_
#define H_ADDLOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddLockRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_lock_id;
  //optional
  uint8_t* lock_id;
  size_t lock_id_size;
  bool has_guest_aes_key;
  //optional
  uint8_t* guest_aes_key;
  size_t guest_aes_key_size;
  bool has_guest_token;
  //optional
  uint8_t* guest_token;
  size_t guest_token_size;
  bool has_lock_password;
  //optional
  uint8_t* lock_password;
  size_t lock_password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddLockRequest;

#define IG_AddLockRequest_MSG_ID 83

void ig_AddLockRequest_init(IgAddLockRequest *obj);
IgSerializerError ig_AddLockRequest_encode(IgAddLockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddLockRequest_decode(uint8_t *buf,size_t buf_size,IgAddLockRequest *retval,size_t index);
uint32_t ig_AddLockRequest_get_max_payload_in_bytes(IgAddLockRequest *obj);
bool ig_AddLockRequest_is_valid(IgAddLockRequest *obj);
void ig_AddLockRequest_deinit(IgAddLockRequest *obj);

size_t ig_AddLockRequest_get_password_size(IgAddLockRequest *obj);
void ig_AddLockRequest_set_password_nocopy(IgAddLockRequest *obj,uint8_t* password,size_t size);
void ig_AddLockRequest_set_password(IgAddLockRequest *obj,uint8_t* password,size_t size);

size_t ig_AddLockRequest_get_lock_id_size(IgAddLockRequest *obj);
void ig_AddLockRequest_set_lock_id_nocopy(IgAddLockRequest *obj,uint8_t* lock_id,size_t size);
void ig_AddLockRequest_set_lock_id(IgAddLockRequest *obj,uint8_t* lock_id,size_t size);

size_t ig_AddLockRequest_get_guest_aes_key_size(IgAddLockRequest *obj);
void ig_AddLockRequest_set_guest_aes_key_nocopy(IgAddLockRequest *obj,uint8_t* guest_aes_key,size_t size);
void ig_AddLockRequest_set_guest_aes_key(IgAddLockRequest *obj,uint8_t* guest_aes_key,size_t size);

size_t ig_AddLockRequest_get_guest_token_size(IgAddLockRequest *obj);
void ig_AddLockRequest_set_guest_token_nocopy(IgAddLockRequest *obj,uint8_t* guest_token,size_t size);
void ig_AddLockRequest_set_guest_token(IgAddLockRequest *obj,uint8_t* guest_token,size_t size);

size_t ig_AddLockRequest_get_lock_password_size(IgAddLockRequest *obj);
void ig_AddLockRequest_set_lock_password_nocopy(IgAddLockRequest *obj,uint8_t* lock_password,size_t size);
void ig_AddLockRequest_set_lock_password(IgAddLockRequest *obj,uint8_t* lock_password,size_t size);

void ig_AddLockRequest_set_operation_id(IgAddLockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
