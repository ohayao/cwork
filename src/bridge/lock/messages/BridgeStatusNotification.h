#ifndef H_BRIDGESTATUSNOTIFICATION_
#define H_BRIDGESTATUSNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgBridgeStatusNotification {
  bool has_wifi_network_status;
  //optional
  uint8_t wifi_network_status;
} IgBridgeStatusNotification;

#define IG_BridgeStatusNotification_MSG_ID 106

void ig_BridgeStatusNotification_init(IgBridgeStatusNotification *obj);
IgSerializerError ig_BridgeStatusNotification_encode(IgBridgeStatusNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_BridgeStatusNotification_decode(uint8_t *buf,size_t buf_size,IgBridgeStatusNotification *retval,size_t index);
uint32_t ig_BridgeStatusNotification_get_max_payload_in_bytes(IgBridgeStatusNotification *obj);
bool ig_BridgeStatusNotification_is_valid(IgBridgeStatusNotification *obj);
void ig_BridgeStatusNotification_deinit(IgBridgeStatusNotification *obj);

void ig_BridgeStatusNotification_set_wifi_network_status(IgBridgeStatusNotification *obj,uint8_t wifi_network_status);




#ifdef __cplusplus
}
#endif

#endif
