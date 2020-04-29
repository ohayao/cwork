#ifndef H_ADMINUNLOCKRESPONSE_
#define H_ADMINUNLOCKRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminUnlockResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAdminUnlockResponse;

#define IG_AdminUnlockResponse_MSG_ID 22

void ig_AdminUnlockResponse_init(IgAdminUnlockResponse *obj);
IgSerializerError ig_AdminUnlockResponse_encode(IgAdminUnlockResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminUnlockResponse_decode(uint8_t *buf,size_t buf_size,IgAdminUnlockResponse *retval,size_t index);
uint32_t ig_AdminUnlockResponse_get_max_payload_in_bytes(IgAdminUnlockResponse *obj);
bool ig_AdminUnlockResponse_is_valid(IgAdminUnlockResponse *obj);
void ig_AdminUnlockResponse_deinit(IgAdminUnlockResponse *obj);

void ig_AdminUnlockResponse_set_result(IgAdminUnlockResponse *obj,uint8_t result);

void ig_AdminUnlockResponse_set_operation_id(IgAdminUnlockResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
