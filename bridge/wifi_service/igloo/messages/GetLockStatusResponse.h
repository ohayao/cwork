#ifndef H_GETLOCKSTATUSRESPONSE_
#define H_GETLOCKSTATUSRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetLockStatusResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_lock_open;
  //optional
  bool lock_open;
  bool has_cell_network_status;
  //optional
  uint8_t cell_network_status;
  bool has_door_open;
  //optional
  uint8_t door_open;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetLockStatusResponse;

#define IG_GetLockStatusResponse_MSG_ID 32

void ig_GetLockStatusResponse_init(IgGetLockStatusResponse *obj);
IgSerializerError ig_GetLockStatusResponse_encode(IgGetLockStatusResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetLockStatusResponse_decode(uint8_t *buf,size_t buf_size,IgGetLockStatusResponse *retval,size_t index);
uint32_t ig_GetLockStatusResponse_get_max_payload_in_bytes(IgGetLockStatusResponse *obj);
bool ig_GetLockStatusResponse_is_valid(IgGetLockStatusResponse *obj);
void ig_GetLockStatusResponse_deinit(IgGetLockStatusResponse *obj);

void ig_GetLockStatusResponse_set_result(IgGetLockStatusResponse *obj,uint8_t result);

void ig_GetLockStatusResponse_set_lock_open(IgGetLockStatusResponse *obj,bool lock_open);

void ig_GetLockStatusResponse_set_cell_network_status(IgGetLockStatusResponse *obj,uint8_t cell_network_status);

void ig_GetLockStatusResponse_set_door_open(IgGetLockStatusResponse *obj,uint8_t door_open);

void ig_GetLockStatusResponse_set_operation_id(IgGetLockStatusResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
