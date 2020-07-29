#ifndef H_GETFIRMWAREVERSIONREQUEST_
#define H_GETFIRMWAREVERSIONREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetFirmwareVersionRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetFirmwareVersionRequest;

#define IG_GetFirmwareVersionRequest_MSG_ID 87

void ig_GetFirmwareVersionRequest_init(IgGetFirmwareVersionRequest *obj);
IgSerializerError ig_GetFirmwareVersionRequest_encode(IgGetFirmwareVersionRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetFirmwareVersionRequest_decode(uint8_t *buf,size_t buf_size,IgGetFirmwareVersionRequest *retval,size_t index);
uint32_t ig_GetFirmwareVersionRequest_get_max_payload_in_bytes(IgGetFirmwareVersionRequest *obj);
bool ig_GetFirmwareVersionRequest_is_valid(IgGetFirmwareVersionRequest *obj);
void ig_GetFirmwareVersionRequest_deinit(IgGetFirmwareVersionRequest *obj);

size_t ig_GetFirmwareVersionRequest_get_password_size(IgGetFirmwareVersionRequest *obj);
void ig_GetFirmwareVersionRequest_set_password_nocopy(IgGetFirmwareVersionRequest *obj,uint8_t* password,size_t size);
void ig_GetFirmwareVersionRequest_set_password(IgGetFirmwareVersionRequest *obj,uint8_t* password,size_t size);

void ig_GetFirmwareVersionRequest_set_operation_id(IgGetFirmwareVersionRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
