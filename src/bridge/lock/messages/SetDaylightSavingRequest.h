#ifndef H_SETDAYLIGHTSAVINGREQUEST_
#define H_SETDAYLIGHTSAVINGREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetDaylightSavingRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_dst_times;
  //optional
  uint8_t* dst_times;
  size_t dst_times_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetDaylightSavingRequest;

#define IG_SetDaylightSavingRequest_MSG_ID 63

void ig_SetDaylightSavingRequest_init(IgSetDaylightSavingRequest *obj);
IgSerializerError ig_SetDaylightSavingRequest_encode(IgSetDaylightSavingRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetDaylightSavingRequest_decode(uint8_t *buf,size_t buf_size,IgSetDaylightSavingRequest *retval,size_t index);
uint32_t ig_SetDaylightSavingRequest_get_max_payload_in_bytes(IgSetDaylightSavingRequest *obj);
bool ig_SetDaylightSavingRequest_is_valid(IgSetDaylightSavingRequest *obj);
void ig_SetDaylightSavingRequest_deinit(IgSetDaylightSavingRequest *obj);

size_t ig_SetDaylightSavingRequest_get_password_size(IgSetDaylightSavingRequest *obj);
void ig_SetDaylightSavingRequest_set_password_nocopy(IgSetDaylightSavingRequest *obj,uint8_t* password,size_t size);
void ig_SetDaylightSavingRequest_set_password(IgSetDaylightSavingRequest *obj,uint8_t* password,size_t size);

size_t ig_SetDaylightSavingRequest_get_dst_times_size(IgSetDaylightSavingRequest *obj);
void ig_SetDaylightSavingRequest_set_dst_times_nocopy(IgSetDaylightSavingRequest *obj,uint8_t* dst_times,size_t size);
void ig_SetDaylightSavingRequest_set_dst_times(IgSetDaylightSavingRequest *obj,uint8_t* dst_times,size_t size);

void ig_SetDaylightSavingRequest_set_operation_id(IgSetDaylightSavingRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
