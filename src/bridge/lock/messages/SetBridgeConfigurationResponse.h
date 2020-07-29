#ifndef H_SETBRIDGECONFIGURATIONRESPONSE_
#define H_SETBRIDGECONFIGURATIONRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetBridgeConfigurationResponse {
  bool has_result;
  //optional
  uint8_t result;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetBridgeConfigurationResponse;

#define IG_SetBridgeConfigurationResponse_MSG_ID 90

void ig_SetBridgeConfigurationResponse_init(IgSetBridgeConfigurationResponse *obj);
IgSerializerError ig_SetBridgeConfigurationResponse_encode(IgSetBridgeConfigurationResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetBridgeConfigurationResponse_decode(uint8_t *buf,size_t buf_size,IgSetBridgeConfigurationResponse *retval,size_t index);
uint32_t ig_SetBridgeConfigurationResponse_get_max_payload_in_bytes(IgSetBridgeConfigurationResponse *obj);
bool ig_SetBridgeConfigurationResponse_is_valid(IgSetBridgeConfigurationResponse *obj);
void ig_SetBridgeConfigurationResponse_deinit(IgSetBridgeConfigurationResponse *obj);

void ig_SetBridgeConfigurationResponse_set_result(IgSetBridgeConfigurationResponse *obj,uint8_t result);

void ig_SetBridgeConfigurationResponse_set_operation_id(IgSetBridgeConfigurationResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
