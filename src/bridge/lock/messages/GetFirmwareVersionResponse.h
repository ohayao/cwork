#ifndef H_GETFIRMWAREVERSIONRESPONSE_
#define H_GETFIRMWAREVERSIONRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetFirmwareVersionResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_version;
  //optional
  uint8_t* version;
  size_t version_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetFirmwareVersionResponse;

#define IG_GetFirmwareVersionResponse_MSG_ID 88

void ig_GetFirmwareVersionResponse_init(IgGetFirmwareVersionResponse *obj);
IgSerializerError ig_GetFirmwareVersionResponse_encode(IgGetFirmwareVersionResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetFirmwareVersionResponse_decode(uint8_t *buf,size_t buf_size,IgGetFirmwareVersionResponse *retval,size_t index);
uint32_t ig_GetFirmwareVersionResponse_get_max_payload_in_bytes(IgGetFirmwareVersionResponse *obj);
bool ig_GetFirmwareVersionResponse_is_valid(IgGetFirmwareVersionResponse *obj);
void ig_GetFirmwareVersionResponse_deinit(IgGetFirmwareVersionResponse *obj);

void ig_GetFirmwareVersionResponse_set_result(IgGetFirmwareVersionResponse *obj,uint8_t result);

size_t ig_GetFirmwareVersionResponse_get_version_size(IgGetFirmwareVersionResponse *obj);
void ig_GetFirmwareVersionResponse_set_version_nocopy(IgGetFirmwareVersionResponse *obj,uint8_t* version,size_t size);
void ig_GetFirmwareVersionResponse_set_version(IgGetFirmwareVersionResponse *obj,uint8_t* version,size_t size);

void ig_GetFirmwareVersionResponse_set_operation_id(IgGetFirmwareVersionResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
