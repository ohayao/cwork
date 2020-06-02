#ifndef H_GETBATTERYLEVELREQUEST_
#define H_GETBATTERYLEVELREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetBatteryLevelRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetBatteryLevelRequest;

#define IG_GetBatteryLevelRequest_MSG_ID 29

void ig_GetBatteryLevelRequest_init(IgGetBatteryLevelRequest *obj);
IgSerializerError ig_GetBatteryLevelRequest_encode(IgGetBatteryLevelRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetBatteryLevelRequest_decode(uint8_t *buf,size_t buf_size,IgGetBatteryLevelRequest *retval,size_t index);
uint32_t ig_GetBatteryLevelRequest_get_max_payload_in_bytes(IgGetBatteryLevelRequest *obj);
bool ig_GetBatteryLevelRequest_is_valid(IgGetBatteryLevelRequest *obj);
void ig_GetBatteryLevelRequest_deinit(IgGetBatteryLevelRequest *obj);

size_t ig_GetBatteryLevelRequest_get_password_size(IgGetBatteryLevelRequest *obj);
void ig_GetBatteryLevelRequest_set_password_nocopy(IgGetBatteryLevelRequest *obj,uint8_t* password,size_t size);
void ig_GetBatteryLevelRequest_set_password(IgGetBatteryLevelRequest *obj,uint8_t* password,size_t size);

void ig_GetBatteryLevelRequest_set_operation_id(IgGetBatteryLevelRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
