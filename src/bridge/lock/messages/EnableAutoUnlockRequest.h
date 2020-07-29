#ifndef H_ENABLEAUTOUNLOCKREQUEST_
#define H_ENABLEAUTOUNLOCKREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgEnableAutoUnlockRequest {
  bool has_password;
  //required
  uint8_t* password;
  size_t password_size;
  bool has_enable;
  //optional
  bool enable;
  bool has_rssi_min;
  //optional
  int32_t rssi_min;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgEnableAutoUnlockRequest;

#define IG_EnableAutoUnlockRequest_MSG_ID 55

void ig_EnableAutoUnlockRequest_init(IgEnableAutoUnlockRequest *obj);
IgSerializerError ig_EnableAutoUnlockRequest_encode(IgEnableAutoUnlockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_EnableAutoUnlockRequest_decode(uint8_t *buf,size_t buf_size,IgEnableAutoUnlockRequest *retval,size_t index);
uint32_t ig_EnableAutoUnlockRequest_get_max_payload_in_bytes(IgEnableAutoUnlockRequest *obj);
bool ig_EnableAutoUnlockRequest_is_valid(IgEnableAutoUnlockRequest *obj);
void ig_EnableAutoUnlockRequest_deinit(IgEnableAutoUnlockRequest *obj);

size_t ig_EnableAutoUnlockRequest_get_password_size(IgEnableAutoUnlockRequest *obj);
void ig_EnableAutoUnlockRequest_set_password_nocopy(IgEnableAutoUnlockRequest *obj,uint8_t* password,size_t size);
void ig_EnableAutoUnlockRequest_set_password(IgEnableAutoUnlockRequest *obj,uint8_t* password,size_t size);

void ig_EnableAutoUnlockRequest_set_enable(IgEnableAutoUnlockRequest *obj,bool enable);

void ig_EnableAutoUnlockRequest_set_rssi_min(IgEnableAutoUnlockRequest *obj,int32_t rssi_min);

void ig_EnableAutoUnlockRequest_set_operation_id(IgEnableAutoUnlockRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
