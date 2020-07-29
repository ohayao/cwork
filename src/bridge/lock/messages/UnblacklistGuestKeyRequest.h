#ifndef H_UNBLACKLISTGUESTKEYREQUEST_
#define H_UNBLACKLISTGUESTKEYREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnblacklistGuestKeyRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_key_id;
  //required
  uint32_t key_id;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgUnblacklistGuestKeyRequest;

#define IG_UnblacklistGuestKeyRequest_MSG_ID 59

void ig_UnblacklistGuestKeyRequest_init(IgUnblacklistGuestKeyRequest *obj);
IgSerializerError ig_UnblacklistGuestKeyRequest_encode(IgUnblacklistGuestKeyRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnblacklistGuestKeyRequest_decode(uint8_t *buf,size_t buf_size,IgUnblacklistGuestKeyRequest *retval,size_t index);
uint32_t ig_UnblacklistGuestKeyRequest_get_max_payload_in_bytes(IgUnblacklistGuestKeyRequest *obj);
bool ig_UnblacklistGuestKeyRequest_is_valid(IgUnblacklistGuestKeyRequest *obj);
void ig_UnblacklistGuestKeyRequest_deinit(IgUnblacklistGuestKeyRequest *obj);

size_t ig_UnblacklistGuestKeyRequest_get_password_size(IgUnblacklistGuestKeyRequest *obj);
void ig_UnblacklistGuestKeyRequest_set_password_nocopy(IgUnblacklistGuestKeyRequest *obj,uint8_t* password,size_t size);
void ig_UnblacklistGuestKeyRequest_set_password(IgUnblacklistGuestKeyRequest *obj,uint8_t* password,size_t size);

void ig_UnblacklistGuestKeyRequest_set_key_id(IgUnblacklistGuestKeyRequest *obj,uint32_t key_id);

void ig_UnblacklistGuestKeyRequest_set_operation_id(IgUnblacklistGuestKeyRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
