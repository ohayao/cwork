#ifndef H_ADDFINGERPRINTRESPONSE_
#define H_ADDFINGERPRINTRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAddFingerprintResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgAddFingerprintResponse;

#define IG_AddFingerprintResponse_MSG_ID 76

void ig_AddFingerprintResponse_init(IgAddFingerprintResponse *obj);
IgSerializerError ig_AddFingerprintResponse_encode(IgAddFingerprintResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AddFingerprintResponse_decode(uint8_t *buf,size_t buf_size,IgAddFingerprintResponse *retval,size_t index);
uint32_t ig_AddFingerprintResponse_get_max_payload_in_bytes(IgAddFingerprintResponse *obj);
bool ig_AddFingerprintResponse_is_valid(IgAddFingerprintResponse *obj);
void ig_AddFingerprintResponse_deinit(IgAddFingerprintResponse *obj);

void ig_AddFingerprintResponse_set_result(IgAddFingerprintResponse *obj,uint8_t result);

void ig_AddFingerprintResponse_set_operation_id(IgAddFingerprintResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
