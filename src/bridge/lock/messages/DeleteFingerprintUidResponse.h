#ifndef H_DELETEFINGERPRINTUIDRESPONSE_
#define H_DELETEFINGERPRINTUIDRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDeleteFingerprintUidResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgDeleteFingerprintUidResponse;

#define IG_DeleteFingerprintUidResponse_MSG_ID 80

void ig_DeleteFingerprintUidResponse_init(IgDeleteFingerprintUidResponse *obj);
IgSerializerError ig_DeleteFingerprintUidResponse_encode(IgDeleteFingerprintUidResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DeleteFingerprintUidResponse_decode(uint8_t *buf,size_t buf_size,IgDeleteFingerprintUidResponse *retval,size_t index);
uint32_t ig_DeleteFingerprintUidResponse_get_max_payload_in_bytes(IgDeleteFingerprintUidResponse *obj);
bool ig_DeleteFingerprintUidResponse_is_valid(IgDeleteFingerprintUidResponse *obj);
void ig_DeleteFingerprintUidResponse_deinit(IgDeleteFingerprintUidResponse *obj);

void ig_DeleteFingerprintUidResponse_set_result(IgDeleteFingerprintUidResponse *obj,uint8_t result);

void ig_DeleteFingerprintUidResponse_set_operation_id(IgDeleteFingerprintUidResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
