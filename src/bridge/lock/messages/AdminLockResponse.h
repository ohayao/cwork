#ifndef H_ADMINLOCKRESPONSE_
#define H_ADMINLOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminLockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAdminLockResponse;

#define IG_AdminLockResponse_MSG_ID 52

void ig_AdminLockResponse_init(IgAdminLockResponse *obj);
IgSerializerError ig_AdminLockResponse_encode(IgAdminLockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminLockResponse_decode(uint8_t *buf,size_t buf_size,IgAdminLockResponse *retval,size_t index);
uint32_t ig_AdminLockResponse_get_max_payload_in_bytes(IgAdminLockResponse *obj);
bool ig_AdminLockResponse_is_valid(IgAdminLockResponse *obj);
void ig_AdminLockResponse_deinit(IgAdminLockResponse *obj);

void ig_AdminLockResponse_set_result(IgAdminLockResponse *obj,uint8_t result);

void ig_AdminLockResponse_set_operation_id(IgAdminLockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
