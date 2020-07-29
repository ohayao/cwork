#ifndef H_SETNBCONFIGURATIONRESPONSE_
#define H_SETNBCONFIGURATIONRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetNbConfigurationResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetNbConfigurationResponse;

#define IG_SetNbConfigurationResponse_MSG_ID 82

void ig_SetNbConfigurationResponse_init(IgSetNbConfigurationResponse *obj);
IgSerializerError ig_SetNbConfigurationResponse_encode(IgSetNbConfigurationResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetNbConfigurationResponse_decode(uint8_t *buf,size_t buf_size,IgSetNbConfigurationResponse *retval,size_t index);
uint32_t ig_SetNbConfigurationResponse_get_max_payload_in_bytes(IgSetNbConfigurationResponse *obj);
bool ig_SetNbConfigurationResponse_is_valid(IgSetNbConfigurationResponse *obj);
void ig_SetNbConfigurationResponse_deinit(IgSetNbConfigurationResponse *obj);

void ig_SetNbConfigurationResponse_set_result(IgSetNbConfigurationResponse *obj,uint8_t result);

void ig_SetNbConfigurationResponse_set_operation_id(IgSetNbConfigurationResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
