#ifndef H_SETRELOCKALARMREQUEST_
#define H_SETRELOCKALARMREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetRelockAlarmRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_relock_alarm_seconds;
  //optional
  uint32_t relock_alarm_seconds;
  bool has_enable;
  //optional
  bool enable;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetRelockAlarmRequest;

#define IG_SetRelockAlarmRequest_MSG_ID 73

void ig_SetRelockAlarmRequest_init(IgSetRelockAlarmRequest *obj);
IgSerializerError ig_SetRelockAlarmRequest_encode(IgSetRelockAlarmRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetRelockAlarmRequest_decode(uint8_t *buf,size_t buf_size,IgSetRelockAlarmRequest *retval,size_t index);
uint32_t ig_SetRelockAlarmRequest_get_max_payload_in_bytes(IgSetRelockAlarmRequest *obj);
bool ig_SetRelockAlarmRequest_is_valid(IgSetRelockAlarmRequest *obj);
void ig_SetRelockAlarmRequest_deinit(IgSetRelockAlarmRequest *obj);

size_t ig_SetRelockAlarmRequest_get_password_size(IgSetRelockAlarmRequest *obj);
void ig_SetRelockAlarmRequest_set_password_nocopy(IgSetRelockAlarmRequest *obj,uint8_t* password,size_t size);
void ig_SetRelockAlarmRequest_set_password(IgSetRelockAlarmRequest *obj,uint8_t* password,size_t size);

void ig_SetRelockAlarmRequest_set_relock_alarm_seconds(IgSetRelockAlarmRequest *obj,uint32_t relock_alarm_seconds);

void ig_SetRelockAlarmRequest_set_enable(IgSetRelockAlarmRequest *obj,bool enable);

void ig_SetRelockAlarmRequest_set_operation_id(IgSetRelockAlarmRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
