#ifndef H_GETSTATUSRESPONSE_
#define H_GETSTATUSRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetStatusResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_lock_open;
  //optional
  bool lock_open;
  bool has_nb_network_status;
  //optional
  uint8_t nb_network_status;
  bool has_door_open;
  //optional
  uint8_t door_open;
  bool has_wifi_network_status;
  //optional
  uint8_t wifi_network_status;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetStatusResponse;

#define IG_GetStatusResponse_MSG_ID 32

void ig_GetStatusResponse_init(IgGetStatusResponse *obj);
IgSerializerError ig_GetStatusResponse_encode(IgGetStatusResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetStatusResponse_decode(uint8_t *buf,size_t buf_size,IgGetStatusResponse *retval,size_t index);
uint32_t ig_GetStatusResponse_get_max_payload_in_bytes(IgGetStatusResponse *obj);
bool ig_GetStatusResponse_is_valid(IgGetStatusResponse *obj);
void ig_GetStatusResponse_deinit(IgGetStatusResponse *obj);

void ig_GetStatusResponse_set_result(IgGetStatusResponse *obj,uint8_t result);

void ig_GetStatusResponse_set_lock_open(IgGetStatusResponse *obj,bool lock_open);

void ig_GetStatusResponse_set_nb_network_status(IgGetStatusResponse *obj,uint8_t nb_network_status);

void ig_GetStatusResponse_set_door_open(IgGetStatusResponse *obj,uint8_t door_open);

void ig_GetStatusResponse_set_wifi_network_status(IgGetStatusResponse *obj,uint8_t wifi_network_status);

void ig_GetStatusResponse_set_operation_id(IgGetStatusResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
