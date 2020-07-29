#ifndef H_SETNBCONFIGURATIONREQUEST_
#define H_SETNBCONFIGURATIONREQUEST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgSetNbConfigurationRequest {
  bool has_password;
  //optional
  uint8_t* password;
  size_t password_size;
  bool has_cell_apn;
  //optional
  char* cell_apn;
  size_t cell_apn_size;
  bool has_cell_dns;
  //optional
  char* cell_dns;
  size_t cell_dns_size;
  bool has_cell_network_type;
  //optional
  uint8_t cell_network_type;
  bool has_cell_network_band;
  //optional
  uint8_t cell_network_band;
  bool has_nb_power_saving_mode_enabled;
  //optional
  bool nb_power_saving_mode_enabled;
  bool has_nb_power_saving_mode_active_timer;
  //optional
  uint16_t nb_power_saving_mode_active_timer;
  bool has_nb_power_saving_mode_tau;
  //optional
  uint16_t nb_power_saving_mode_tau;
  bool has_nb_mqtt_topic;
  //optional
  char* nb_mqtt_topic;
  size_t nb_mqtt_topic_size;
  bool has_nb_mqtt_broker_url;
  //optional
  char* nb_mqtt_broker_url;
  size_t nb_mqtt_broker_url_size;
  bool has_nb_mqtt_broker_port;
  //optional
  char* nb_mqtt_broker_port;
  size_t nb_mqtt_broker_port_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgSetNbConfigurationRequest;

#define IG_SetNbConfigurationRequest_MSG_ID 81

void ig_SetNbConfigurationRequest_init(IgSetNbConfigurationRequest *obj);
IgSerializerError ig_SetNbConfigurationRequest_encode(IgSetNbConfigurationRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_SetNbConfigurationRequest_decode(uint8_t *buf,size_t buf_size,IgSetNbConfigurationRequest *retval,size_t index);
uint32_t ig_SetNbConfigurationRequest_get_max_payload_in_bytes(IgSetNbConfigurationRequest *obj);
bool ig_SetNbConfigurationRequest_is_valid(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_deinit(IgSetNbConfigurationRequest *obj);

size_t ig_SetNbConfigurationRequest_get_password_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_password_nocopy(IgSetNbConfigurationRequest *obj,uint8_t* password,size_t size);
void ig_SetNbConfigurationRequest_set_password(IgSetNbConfigurationRequest *obj,uint8_t* password,size_t size);

size_t ig_SetNbConfigurationRequest_get_cell_apn_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_cell_apn_nocopy(IgSetNbConfigurationRequest *obj,char* cell_apn,size_t size);
void ig_SetNbConfigurationRequest_set_cell_apn(IgSetNbConfigurationRequest *obj,char* cell_apn,size_t size);

size_t ig_SetNbConfigurationRequest_get_cell_dns_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_cell_dns_nocopy(IgSetNbConfigurationRequest *obj,char* cell_dns,size_t size);
void ig_SetNbConfigurationRequest_set_cell_dns(IgSetNbConfigurationRequest *obj,char* cell_dns,size_t size);

void ig_SetNbConfigurationRequest_set_cell_network_type(IgSetNbConfigurationRequest *obj,uint8_t cell_network_type);

void ig_SetNbConfigurationRequest_set_cell_network_band(IgSetNbConfigurationRequest *obj,uint8_t cell_network_band);

void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_enabled(IgSetNbConfigurationRequest *obj,bool nb_power_saving_mode_enabled);

void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_active_timer(IgSetNbConfigurationRequest *obj,uint16_t nb_power_saving_mode_active_timer);

void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_tau(IgSetNbConfigurationRequest *obj,uint16_t nb_power_saving_mode_tau);

size_t ig_SetNbConfigurationRequest_get_nb_mqtt_topic_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_nb_mqtt_topic_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_topic,size_t size);
void ig_SetNbConfigurationRequest_set_nb_mqtt_topic(IgSetNbConfigurationRequest *obj,char* nb_mqtt_topic,size_t size);

size_t ig_SetNbConfigurationRequest_get_nb_mqtt_broker_url_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_url_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_url,size_t size);
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_url(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_url,size_t size);

size_t ig_SetNbConfigurationRequest_get_nb_mqtt_broker_port_size(IgSetNbConfigurationRequest *obj);
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_port_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_port,size_t size);
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_port(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_port,size_t size);

void ig_SetNbConfigurationRequest_set_operation_id(IgSetNbConfigurationRequest *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
