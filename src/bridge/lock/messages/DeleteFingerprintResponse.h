#ifndef H_DELETEFINGERPRINTRESPONSE_
#define H_DELETEFINGERPRINTRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteFingerprintResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteFingerprintResponse;

#define IG_DeleteFingerprintResponse_MSG_ID 78

void ig_DeleteFingerprintResponse_init(IgDeleteFingerprintResponse *obj);
IgSerializerError ig_DeleteFingerprintResponse_encode(IgDeleteFingerprintResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteFingerprintResponse_decode(uint8_t *buf,size_t buf_size,IgDeleteFingerprintResponse *retval,size_t index);
uint32_t ig_DeleteFingerprintResponse_get_max_payload_in_bytes(IgDeleteFingerprintResponse *obj);
bool ig_DeleteFingerprintResponse_is_valid(IgDeleteFingerprintResponse *obj);
void ig_DeleteFingerprintResponse_deinit(IgDeleteFingerprintResponse *obj);

void ig_DeleteFingerprintResponse_set_result(IgDeleteFingerprintResponse *obj,uint8_t result);

void ig_DeleteFingerprintResponse_set_operation_id(IgDeleteFingerprintResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
