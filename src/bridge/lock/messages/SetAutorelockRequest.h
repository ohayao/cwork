#ifndef H_SETAUTORELOCKREQUEST_
#define H_SETAUTORELOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetAutorelockRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_sensor_autorelock_seconds;
  //optional
  uint32_t sensor_autorelock_seconds;
  bool has_enable;
  //optional
  bool enable;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetAutorelockRequest;

#define IG_SetAutorelockRequest_MSG_ID 37

void ig_SetAutorelockRequest_init(IgSetAutorelockRequest *obj);
IgSerializerError ig_SetAutorelockRequest_encode(IgSetAutorelockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetAutorelockRequest_decode(uint8_t *buf,size_t buf_size,IgSetAutorelockRequest *retval,size_t index);
uint32_t ig_SetAutorelockRequest_get_max_payload_in_bytes(IgSetAutorelockRequest *obj);
bool ig_SetAutorelockRequest_is_valid(IgSetAutorelockRequest *obj);
void ig_SetAutorelockRequest_deinit(IgSetAutorelockRequest *obj);

size_t ig_SetAutorelockRequest_get_password_size(IgSetAutorelockRequest *obj);
void ig_SetAutorelockRequest_set_password_nocopy(IgSetAutorelockRequest *obj,uint8_t* password,size_t size);
void ig_SetAutorelockRequest_set_password(IgSetAutorelockRequest *obj,uint8_t* password,size_t size);

void ig_SetAutorelockRequest_set_sensor_autorelock_seconds(IgSetAutorelockRequest *obj,uint32_t sensor_autorelock_seconds);

void ig_SetAutorelockRequest_set_enable(IgSetAutorelockRequest *obj,bool enable);

void ig_SetAutorelockRequest_set_operation_id(IgSetAutorelockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
