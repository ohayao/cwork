#ifndef H_BLACKLISTGUESTKEYREQUEST_
#define H_BLACKLISTGUESTKEYREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgBlacklistGuestKeyRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_key_id;
  //required
  uint32_t key_id;
  bool has_start_date;
  //optional
  uint32_t start_date;
  bool has_end_date;
  //optional
  uint32_t end_date;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgBlacklistGuestKeyRequest;

#define IG_BlacklistGuestKeyRequest_MSG_ID 57

void ig_BlacklistGuestKeyRequest_init(IgBlacklistGuestKeyRequest *obj);
IgSerializerError ig_BlacklistGuestKeyRequest_encode(IgBlacklistGuestKeyRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_BlacklistGuestKeyRequest_decode(uint8_t *buf,size_t buf_size,IgBlacklistGuestKeyRequest *retval,size_t index);
uint32_t ig_BlacklistGuestKeyRequest_get_max_payload_in_bytes(IgBlacklistGuestKeyRequest *obj);
bool ig_BlacklistGuestKeyRequest_is_valid(IgBlacklistGuestKeyRequest *obj);
void ig_BlacklistGuestKeyRequest_deinit(IgBlacklistGuestKeyRequest *obj);

size_t ig_BlacklistGuestKeyRequest_get_password_size(IgBlacklistGuestKeyRequest *obj);
void ig_BlacklistGuestKeyRequest_set_password_nocopy(IgBlacklistGuestKeyRequest *obj,uint8_t* password,size_t size);
void ig_BlacklistGuestKeyRequest_set_password(IgBlacklistGuestKeyRequest *obj,uint8_t* password,size_t size);

void ig_BlacklistGuestKeyRequest_set_key_id(IgBlacklistGuestKeyRequest *obj,uint32_t key_id);

void ig_BlacklistGuestKeyRequest_set_start_date(IgBlacklistGuestKeyRequest *obj,uint32_t start_date);

void ig_BlacklistGuestKeyRequest_set_end_date(IgBlacklistGuestKeyRequest *obj,uint32_t end_date);

void ig_BlacklistGuestKeyRequest_set_operation_id(IgBlacklistGuestKeyRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
