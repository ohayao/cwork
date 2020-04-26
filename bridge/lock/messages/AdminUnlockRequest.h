#ifndef H_ADMINUNLOCKREQUEST_
#define H_ADMINUNLOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminUnlockRequest {
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
} IgAdminUnlockRequest;

#define IG_AdminUnlockRequest_MSG_ID 21

void ig_AdminUnlockRequest_init(IgAdminUnlockRequest *obj);
IgSerializerError ig_AdminUnlockRequest_encode(IgAdminUnlockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminUnlockRequest_decode(uint8_t *buf,size_t buf_size,IgAdminUnlockRequest *retval,size_t index);
uint32_t ig_AdminUnlockRequest_get_max_payload_in_bytes(IgAdminUnlockRequest *obj);
bool ig_AdminUnlockRequest_is_valid(IgAdminUnlockRequest *obj);
void ig_AdminUnlockRequest_deinit(IgAdminUnlockRequest *obj);

size_t ig_AdminUnlockRequest_get_password_size(IgAdminUnlockRequest *obj);
void ig_AdminUnlockRequest_set_password_nocopy(IgAdminUnlockRequest *obj,uint8_t* password,size_t size);
void ig_AdminUnlockRequest_set_password(IgAdminUnlockRequest *obj,uint8_t* password,size_t size);

void ig_AdminUnlockRequest_set_timestamp(IgAdminUnlockRequest *obj,uint32_t timestamp);

void ig_AdminUnlockRequest_set_operation_id(IgAdminUnlockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
