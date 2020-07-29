#ifndef H_DELETECARDUIDREQUEST_
#define H_DELETECARDUIDREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteCardUidRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_card_uid;
  //optional
  uint8_t* card_uid;
  size_t card_uid_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteCardUidRequest;

#define IG_DeleteCardUidRequest_MSG_ID 69

void ig_DeleteCardUidRequest_init(IgDeleteCardUidRequest *obj);
IgSerializerError ig_DeleteCardUidRequest_encode(IgDeleteCardUidRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteCardUidRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteCardUidRequest *retval,size_t index);
uint32_t ig_DeleteCardUidRequest_get_max_payload_in_bytes(IgDeleteCardUidRequest *obj);
bool ig_DeleteCardUidRequest_is_valid(IgDeleteCardUidRequest *obj);
void ig_DeleteCardUidRequest_deinit(IgDeleteCardUidRequest *obj);

size_t ig_DeleteCardUidRequest_get_password_size(IgDeleteCardUidRequest *obj);
void ig_DeleteCardUidRequest_set_password_nocopy(IgDeleteCardUidRequest *obj,uint8_t* password,size_t size);
void ig_DeleteCardUidRequest_set_password(IgDeleteCardUidRequest *obj,uint8_t* password,size_t size);

size_t ig_DeleteCardUidRequest_get_card_uid_size(IgDeleteCardUidRequest *obj);
void ig_DeleteCardUidRequest_set_card_uid_nocopy(IgDeleteCardUidRequest *obj,uint8_t* card_uid,size_t size);
void ig_DeleteCardUidRequest_set_card_uid(IgDeleteCardUidRequest *obj,uint8_t* card_uid,size_t size);

void ig_DeleteCardUidRequest_set_operation_id(IgDeleteCardUidRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
