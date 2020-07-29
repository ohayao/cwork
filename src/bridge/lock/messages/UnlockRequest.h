#ifndef H_UNLOCKREQUEST_
#define H_UNLOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnlockRequest {
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
} IgUnlockRequest;

#define IG_UnlockRequest_MSG_ID 21

void ig_UnlockRequest_init(IgUnlockRequest *obj);
IgSerializerError ig_UnlockRequest_encode(IgUnlockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnlockRequest_decode(uint8_t *buf,size_t buf_size,IgUnlockRequest *retval,size_t index);
uint32_t ig_UnlockRequest_get_max_payload_in_bytes(IgUnlockRequest *obj);
bool ig_UnlockRequest_is_valid(IgUnlockRequest *obj);
void ig_UnlockRequest_deinit(IgUnlockRequest *obj);

size_t ig_UnlockRequest_get_password_size(IgUnlockRequest *obj);
void ig_UnlockRequest_set_password_nocopy(IgUnlockRequest *obj,uint8_t* password,size_t size);
void ig_UnlockRequest_set_password(IgUnlockRequest *obj,uint8_t* password,size_t size);

void ig_UnlockRequest_set_timestamp(IgUnlockRequest *obj,uint32_t timestamp);

void ig_UnlockRequest_set_operation_id(IgUnlockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
