#ifndef H_ADDFINGERPRINTREQUEST_
#define H_ADDFINGERPRINTREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddFingerprintRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_tries;
  //optional
  uint8_t tries;
  bool has_timeout_seconds;
  //optional
  uint32_t timeout_seconds;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddFingerprintRequest;

#define IG_AddFingerprintRequest_MSG_ID 75

void ig_AddFingerprintRequest_init(IgAddFingerprintRequest *obj);
IgSerializerError ig_AddFingerprintRequest_encode(IgAddFingerprintRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddFingerprintRequest_decode(uint8_t *buf,size_t buf_size,IgAddFingerprintRequest *retval,size_t index);
uint32_t ig_AddFingerprintRequest_get_max_payload_in_bytes(IgAddFingerprintRequest *obj);
bool ig_AddFingerprintRequest_is_valid(IgAddFingerprintRequest *obj);
void ig_AddFingerprintRequest_deinit(IgAddFingerprintRequest *obj);

size_t ig_AddFingerprintRequest_get_password_size(IgAddFingerprintRequest *obj);
void ig_AddFingerprintRequest_set_password_nocopy(IgAddFingerprintRequest *obj,uint8_t* password,size_t size);
void ig_AddFingerprintRequest_set_password(IgAddFingerprintRequest *obj,uint8_t* password,size_t size);

void ig_AddFingerprintRequest_set_tries(IgAddFingerprintRequest *obj,uint8_t tries);

void ig_AddFingerprintRequest_set_timeout_seconds(IgAddFingerprintRequest *obj,uint32_t timeout_seconds);

void ig_AddFingerprintRequest_set_operation_id(IgAddFingerprintRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
