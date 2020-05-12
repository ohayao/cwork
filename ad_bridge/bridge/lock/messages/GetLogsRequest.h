#ifndef H_GETLOGSREQUEST_
#define H_GETLOGSREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetLogsRequest {
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
} IgGetLogsRequest;

#define IG_GetLogsRequest_MSG_ID 41

void ig_GetLogsRequest_init(IgGetLogsRequest *obj);
IgSerializerError ig_GetLogsRequest_encode(IgGetLogsRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetLogsRequest_decode(uint8_t *buf,size_t buf_size,IgGetLogsRequest *retval,size_t index);
uint32_t ig_GetLogsRequest_get_max_payload_in_bytes(IgGetLogsRequest *obj);
bool ig_GetLogsRequest_is_valid(IgGetLogsRequest *obj);
void ig_GetLogsRequest_deinit(IgGetLogsRequest *obj);

size_t ig_GetLogsRequest_get_password_size(IgGetLogsRequest *obj);
void ig_GetLogsRequest_set_password_nocopy(IgGetLogsRequest *obj,uint8_t* password,size_t size);
void ig_GetLogsRequest_set_password(IgGetLogsRequest *obj,uint8_t* password,size_t size);

void ig_GetLogsRequest_set_timestamp(IgGetLogsRequest *obj,uint32_t timestamp);

void ig_GetLogsRequest_set_operation_id(IgGetLogsRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
