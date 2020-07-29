#ifndef H_DELETEFINGERPRINTREQUEST_
#define H_DELETEFINGERPRINTREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteFingerprintRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_timeout_seconds;
  //optional
  uint32_t timeout_seconds;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteFingerprintRequest;

#define IG_DeleteFingerprintRequest_MSG_ID 77

void ig_DeleteFingerprintRequest_init(IgDeleteFingerprintRequest *obj);
IgSerializerError ig_DeleteFingerprintRequest_encode(IgDeleteFingerprintRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteFingerprintRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteFingerprintRequest *retval,size_t index);
uint32_t ig_DeleteFingerprintRequest_get_max_payload_in_bytes(IgDeleteFingerprintRequest *obj);
bool ig_DeleteFingerprintRequest_is_valid(IgDeleteFingerprintRequest *obj);
void ig_DeleteFingerprintRequest_deinit(IgDeleteFingerprintRequest *obj);

size_t ig_DeleteFingerprintRequest_get_password_size(IgDeleteFingerprintRequest *obj);
void ig_DeleteFingerprintRequest_set_password_nocopy(IgDeleteFingerprintRequest *obj,uint8_t* password,size_t size);
void ig_DeleteFingerprintRequest_set_password(IgDeleteFingerprintRequest *obj,uint8_t* password,size_t size);

void ig_DeleteFingerprintRequest_set_timeout_seconds(IgDeleteFingerprintRequest *obj,uint32_t timeout_seconds);

void ig_DeleteFingerprintRequest_set_operation_id(IgDeleteFingerprintRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
