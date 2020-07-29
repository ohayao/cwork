#ifndef H_SETVOLUMEREQUEST_
#define H_SETVOLUMEREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetVolumeRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_volume;
  //required
  uint32_t volume;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetVolumeRequest;

#define IG_SetVolumeRequest_MSG_ID 33

void ig_SetVolumeRequest_init(IgSetVolumeRequest *obj);
IgSerializerError ig_SetVolumeRequest_encode(IgSetVolumeRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetVolumeRequest_decode(uint8_t *buf,size_t buf_size,IgSetVolumeRequest *retval,size_t index);
uint32_t ig_SetVolumeRequest_get_max_payload_in_bytes(IgSetVolumeRequest *obj);
bool ig_SetVolumeRequest_is_valid(IgSetVolumeRequest *obj);
void ig_SetVolumeRequest_deinit(IgSetVolumeRequest *obj);

size_t ig_SetVolumeRequest_get_password_size(IgSetVolumeRequest *obj);
void ig_SetVolumeRequest_set_password_nocopy(IgSetVolumeRequest *obj,uint8_t* password,size_t size);
void ig_SetVolumeRequest_set_password(IgSetVolumeRequest *obj,uint8_t* password,size_t size);

void ig_SetVolumeRequest_set_volume(IgSetVolumeRequest *obj,uint32_t volume);

void ig_SetVolumeRequest_set_operation_id(IgSetVolumeRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
