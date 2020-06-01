#ifndef H_GETBATTERYLEVELRESPONSE_
#define H_GETBATTERYLEVELRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetBatteryLevelResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_battery_level;
  //optional
  uint32_t battery_level;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetBatteryLevelResponse;

#define IG_GetBatteryLevelResponse_MSG_ID 30

void ig_GetBatteryLevelResponse_init(IgGetBatteryLevelResponse *obj);
IgSerializerError ig_GetBatteryLevelResponse_encode(IgGetBatteryLevelResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetBatteryLevelResponse_decode(uint8_t *buf,size_t buf_size,IgGetBatteryLevelResponse *retval,size_t index);
uint32_t ig_GetBatteryLevelResponse_get_max_payload_in_bytes(IgGetBatteryLevelResponse *obj);
bool ig_GetBatteryLevelResponse_is_valid(IgGetBatteryLevelResponse *obj);
void ig_GetBatteryLevelResponse_deinit(IgGetBatteryLevelResponse *obj);

void ig_GetBatteryLevelResponse_set_result(IgGetBatteryLevelResponse *obj,uint8_t result);

void ig_GetBatteryLevelResponse_set_battery_level(IgGetBatteryLevelResponse *obj,uint32_t battery_level);

void ig_GetBatteryLevelResponse_set_operation_id(IgGetBatteryLevelResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
