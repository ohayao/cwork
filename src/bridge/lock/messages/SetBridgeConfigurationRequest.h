#ifndef H_SETBRIDGECONFIGURATIONREQUEST_
#define H_SETBRIDGECONFIGURATIONREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetBridgeConfigurationRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_ssid;
  //optional
  char* ssid;
  size_t ssid_size;
  bool has_network_password;
  //optional
  char* network_password;
  size_t network_password_size;
  bool has_mqtt_jwt_token;
  //optional
  uint8_t* mqtt_jwt_token;
  size_t mqtt_jwt_token_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetBridgeConfigurationRequest;

#define IG_SetBridgeConfigurationRequest_MSG_ID 89

void ig_SetBridgeConfigurationRequest_init(IgSetBridgeConfigurationRequest *obj);
IgSerializerError ig_SetBridgeConfigurationRequest_encode(IgSetBridgeConfigurationRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetBridgeConfigurationRequest_decode(uint8_t *buf,size_t buf_size,IgSetBridgeConfigurationRequest *retval,size_t index);
uint32_t ig_SetBridgeConfigurationRequest_get_max_payload_in_bytes(IgSetBridgeConfigurationRequest *obj);
bool ig_SetBridgeConfigurationRequest_is_valid(IgSetBridgeConfigurationRequest *obj);
void ig_SetBridgeConfigurationRequest_deinit(IgSetBridgeConfigurationRequest *obj);

size_t ig_SetBridgeConfigurationRequest_get_password_size(IgSetBridgeConfigurationRequest *obj);
void ig_SetBridgeConfigurationRequest_set_password_nocopy(IgSetBridgeConfigurationRequest *obj,uint8_t* password,size_t size);
void ig_SetBridgeConfigurationRequest_set_password(IgSetBridgeConfigurationRequest *obj,uint8_t* password,size_t size);

size_t ig_SetBridgeConfigurationRequest_get_ssid_size(IgSetBridgeConfigurationRequest *obj);
void ig_SetBridgeConfigurationRequest_set_ssid_nocopy(IgSetBridgeConfigurationRequest *obj,char* ssid,size_t size);
void ig_SetBridgeConfigurationRequest_set_ssid(IgSetBridgeConfigurationRequest *obj,char* ssid,size_t size);

size_t ig_SetBridgeConfigurationRequest_get_network_password_size(IgSetBridgeConfigurationRequest *obj);
void ig_SetBridgeConfigurationRequest_set_network_password_nocopy(IgSetBridgeConfigurationRequest *obj,char* network_password,size_t size);
void ig_SetBridgeConfigurationRequest_set_network_password(IgSetBridgeConfigurationRequest *obj,char* network_password,size_t size);

size_t ig_SetBridgeConfigurationRequest_get_mqtt_jwt_token_size(IgSetBridgeConfigurationRequest *obj);
void ig_SetBridgeConfigurationRequest_set_mqtt_jwt_token_nocopy(IgSetBridgeConfigurationRequest *obj,uint8_t* mqtt_jwt_token,size_t size);
void ig_SetBridgeConfigurationRequest_set_mqtt_jwt_token(IgSetBridgeConfigurationRequest *obj,uint8_t* mqtt_jwt_token,size_t size);

void ig_SetBridgeConfigurationRequest_set_operation_id(IgSetBridgeConfigurationRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
