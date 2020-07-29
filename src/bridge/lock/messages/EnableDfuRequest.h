#ifndef H_ENABLEDFUREQUEST_
#define H_ENABLEDFUREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEnableDfuRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_enable;
  //required
  bool enable;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgEnableDfuRequest;

#define IG_EnableDfuRequest_MSG_ID 61

void ig_EnableDfuRequest_init(IgEnableDfuRequest *obj);
IgSerializerError ig_EnableDfuRequest_encode(IgEnableDfuRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EnableDfuRequest_decode(uint8_t *buf,size_t buf_size,IgEnableDfuRequest *retval,size_t index);
uint32_t ig_EnableDfuRequest_get_max_payload_in_bytes(IgEnableDfuRequest *obj);
bool ig_EnableDfuRequest_is_valid(IgEnableDfuRequest *obj);
void ig_EnableDfuRequest_deinit(IgEnableDfuRequest *obj);

size_t ig_EnableDfuRequest_get_password_size(IgEnableDfuRequest *obj);
void ig_EnableDfuRequest_set_password_nocopy(IgEnableDfuRequest *obj,uint8_t* password,size_t size);
void ig_EnableDfuRequest_set_password(IgEnableDfuRequest *obj,uint8_t* password,size_t size);

void ig_EnableDfuRequest_set_enable(IgEnableDfuRequest *obj,bool enable);

void ig_EnableDfuRequest_set_operation_id(IgEnableDfuRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
