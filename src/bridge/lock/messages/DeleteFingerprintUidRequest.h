#ifndef H_DELETEFINGERPRINTUIDREQUEST_
#define H_DELETEFINGERPRINTUIDREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteFingerprintUidRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_uid;
  //optional
  uint16_t uid;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteFingerprintUidRequest;

#define IG_DeleteFingerprintUidRequest_MSG_ID 79

void ig_DeleteFingerprintUidRequest_init(IgDeleteFingerprintUidRequest *obj);
IgSerializerError ig_DeleteFingerprintUidRequest_encode(IgDeleteFingerprintUidRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteFingerprintUidRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteFingerprintUidRequest *retval,size_t index);
uint32_t ig_DeleteFingerprintUidRequest_get_max_payload_in_bytes(IgDeleteFingerprintUidRequest *obj);
bool ig_DeleteFingerprintUidRequest_is_valid(IgDeleteFingerprintUidRequest *obj);
void ig_DeleteFingerprintUidRequest_deinit(IgDeleteFingerprintUidRequest *obj);

size_t ig_DeleteFingerprintUidRequest_get_password_size(IgDeleteFingerprintUidRequest *obj);
void ig_DeleteFingerprintUidRequest_set_password_nocopy(IgDeleteFingerprintUidRequest *obj,uint8_t* password,size_t size);
void ig_DeleteFingerprintUidRequest_set_password(IgDeleteFingerprintUidRequest *obj,uint8_t* password,size_t size);

void ig_DeleteFingerprintUidRequest_set_uid(IgDeleteFingerprintUidRequest *obj,uint16_t uid);

void ig_DeleteFingerprintUidRequest_set_operation_id(IgDeleteFingerprintUidRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
