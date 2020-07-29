#ifndef H_UNPAIRREQUEST_
#define H_UNPAIRREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgUnpairRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_force;
  //optional
  bool force;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgUnpairRequest;

#define IG_UnpairRequest_MSG_ID 15

void ig_UnpairRequest_init(IgUnpairRequest *obj);
IgSerializerError ig_UnpairRequest_encode(IgUnpairRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_UnpairRequest_decode(uint8_t *buf,size_t buf_size,IgUnpairRequest *retval,size_t index);
uint32_t ig_UnpairRequest_get_max_payload_in_bytes(IgUnpairRequest *obj);
bool ig_UnpairRequest_is_valid(IgUnpairRequest *obj);
void ig_UnpairRequest_deinit(IgUnpairRequest *obj);

size_t ig_UnpairRequest_get_password_size(IgUnpairRequest *obj);
void ig_UnpairRequest_set_password_nocopy(IgUnpairRequest *obj,uint8_t* password,size_t size);
void ig_UnpairRequest_set_password(IgUnpairRequest *obj,uint8_t* password,size_t size);

void ig_UnpairRequest_set_force(IgUnpairRequest *obj,bool force);

void ig_UnpairRequest_set_operation_id(IgUnpairRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
