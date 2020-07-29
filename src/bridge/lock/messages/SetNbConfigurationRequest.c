#include "SetNbConfigurationRequest.h"
#include "external/cbor/cbor.h"

void ig_SetNbConfigurationRequest_init(IgSetNbConfigurationRequest *obj)
{
  memset(obj, 0, sizeof(IgSetNbConfigurationRequest));
}
IgSerializerError ig_SetNbConfigurationRequest_encode(IgSetNbConfigurationRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_SetNbConfigurationRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_password) fields_size++;
  if(obj->has_cell_apn) fields_size++;
  if(obj->has_cell_dns) fields_size++;
  if(obj->has_cell_network_type) fields_size++;
  if(obj->has_cell_network_band) fields_size++;
  if(obj->has_nb_power_saving_mode_enabled) fields_size++;
  if(obj->has_nb_power_saving_mode_active_timer) fields_size++;
  if(obj->has_nb_power_saving_mode_tau) fields_size++;
  if(obj->has_nb_mqtt_topic) fields_size++;
  if(obj->has_nb_mqtt_broker_url) fields_size++;
  if(obj->has_nb_mqtt_broker_port) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 81);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_password){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_cell_apn){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->cell_apn, obj->cell_apn_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_cell_dns){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->cell_dns, obj->cell_dns_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_cell_network_type){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->cell_network_type);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_cell_network_band){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->cell_network_band);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_power_saving_mode_enabled){
      err = cbor_encode_uint(&map, 16);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_boolean(&map, obj->nb_power_saving_mode_enabled);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_power_saving_mode_active_timer){
      err = cbor_encode_uint(&map, 17);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->nb_power_saving_mode_active_timer);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_power_saving_mode_tau){
      err = cbor_encode_uint(&map, 18);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->nb_power_saving_mode_tau);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_mqtt_topic){
      err = cbor_encode_uint(&map, 19);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->nb_mqtt_topic, obj->nb_mqtt_topic_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_mqtt_broker_url){
      err = cbor_encode_uint(&map, 20);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->nb_mqtt_broker_url, obj->nb_mqtt_broker_url_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_nb_mqtt_broker_port){
      err = cbor_encode_uint(&map, 21);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->nb_mqtt_broker_port, obj->nb_mqtt_broker_port_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_operation_id){
      err = cbor_encode_uint(&map, 100);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->operation_id);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_SetNbConfigurationRequest_decode(uint8_t *buf,size_t buf_size,IgSetNbConfigurationRequest *retval,size_t index)
{
  CborParser parser;
  CborValue it;
  CborValue content;
  CborError err;
  
  err = cbor_parser_init(buf, buf_size, 0, &parser, &it);
  if(err) return (IgSerializerError) err;
  
  CborType type = cbor_value_get_type(&it);
  
  if(type == CborMapType){
    err = cbor_value_enter_container(&it, &content);
    if(err) return (IgSerializerError) err;
    //TODO: index has to be 0
  }else if(type == CborArrayType){
    err = cbor_value_enter_container(&it, &it);
    if(err) return (IgSerializerError) err;
  
    for(int i=0; i<index+1; i++){
      err = cbor_value_enter_container(&it, &content);
      if(err) return (IgSerializerError) err;
      err = cbor_value_advance(&it);
      if(err) return (IgSerializerError) err;
    }
  }else{
    //error
  }
  
  int64_t tag_id;
  while (!cbor_value_at_end(&content)) {
  
    CborType tag_id_type = cbor_value_get_type(&content);
    if(tag_id_type != CborIntegerType) return IgSerializerErrorInvalidTypeTagId;
  
    cbor_value_get_int64(&content, &tag_id);
    err = cbor_value_advance_fixed(&content);
    if(err) return (IgSerializerError) err;
  
    if(!err){
      //handle value
      CborType value_type = cbor_value_get_type(&content);
      switch (tag_id) {
        case 0:
          if(value_type == CborIntegerType){
            int64_t val;
            cbor_value_get_int64(&content, &val);
            //msgId value
            if(val != 81) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_password_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 12:
            if(value_type == CborTextStringType){
                size_t size;
                err = cbor_value_calculate_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                char* data_arr = malloc(size);
                err = cbor_value_copy_text_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_cell_apn_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 13:
            if(value_type == CborTextStringType){
                size_t size;
                err = cbor_value_calculate_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                char* data_arr = malloc(size);
                err = cbor_value_copy_text_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_cell_dns_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 14:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetNbConfigurationRequest_set_cell_network_type(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 15:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetNbConfigurationRequest_set_cell_network_band(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 16:
            if(value_type == CborBooleanType){
                bool val;
                cbor_value_get_boolean(&content, &val);
                ig_SetNbConfigurationRequest_set_nb_power_saving_mode_enabled(retval, val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 17:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetNbConfigurationRequest_set_nb_power_saving_mode_active_timer(retval, (uint16_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 18:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetNbConfigurationRequest_set_nb_power_saving_mode_tau(retval, (uint16_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 19:
            if(value_type == CborTextStringType){
                size_t size;
                err = cbor_value_calculate_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                char* data_arr = malloc(size);
                err = cbor_value_copy_text_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_nb_mqtt_topic_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 20:
            if(value_type == CborTextStringType){
                size_t size;
                err = cbor_value_calculate_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                char* data_arr = malloc(size);
                err = cbor_value_copy_text_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_nb_mqtt_broker_url_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 21:
            if(value_type == CborTextStringType){
                size_t size;
                err = cbor_value_calculate_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                char* data_arr = malloc(size);
                err = cbor_value_copy_text_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetNbConfigurationRequest_set_nb_mqtt_broker_port_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetNbConfigurationRequest_set_operation_id(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
          default:
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}
uint32_t ig_SetNbConfigurationRequest_get_max_payload_in_bytes(IgSetNbConfigurationRequest *obj)
{
  return 77 + obj->password_size + obj->cell_apn_size + obj->cell_dns_size + obj->nb_mqtt_topic_size + obj->nb_mqtt_broker_url_size + obj->nb_mqtt_broker_port_size; //13 + 64;
}
bool ig_SetNbConfigurationRequest_is_valid(IgSetNbConfigurationRequest *obj)
{
  return true;
}
void ig_SetNbConfigurationRequest_deinit(IgSetNbConfigurationRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  if(obj->has_cell_apn){
      free(obj->cell_apn);
      obj->cell_apn = NULL;
  }
  
  
  if(obj->has_cell_dns){
      free(obj->cell_dns);
      obj->cell_dns = NULL;
  }
  
  
  
  
  
  
  
  if(obj->has_nb_mqtt_topic){
      free(obj->nb_mqtt_topic);
      obj->nb_mqtt_topic = NULL;
  }
  
  
  if(obj->has_nb_mqtt_broker_url){
      free(obj->nb_mqtt_broker_url);
      obj->nb_mqtt_broker_url = NULL;
  }
  
  
  if(obj->has_nb_mqtt_broker_port){
      free(obj->nb_mqtt_broker_port);
      obj->nb_mqtt_broker_port = NULL;
  }
  
}
size_t ig_SetNbConfigurationRequest_get_password_size(IgSetNbConfigurationRequest *obj)
{
  return obj->password_size;
}
void ig_SetNbConfigurationRequest_set_password_nocopy(IgSetNbConfigurationRequest *obj,uint8_t* password,size_t size)
{
  if(obj->password == password)
      return;
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  obj->password = password;
  obj->password_size = size;
  obj->has_password = size > 0;
}
void ig_SetNbConfigurationRequest_set_password(IgSetNbConfigurationRequest *obj,uint8_t* password,size_t size)
{
  if(obj->password == password)
      return;
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  obj->password = malloc(size);
  memcpy(obj->password, password, size);
  obj->password_size = size;
  obj->has_password = size > 0;
}
size_t ig_SetNbConfigurationRequest_get_cell_apn_size(IgSetNbConfigurationRequest *obj)
{
  return obj->cell_apn_size;
}
void ig_SetNbConfigurationRequest_set_cell_apn_nocopy(IgSetNbConfigurationRequest *obj,char* cell_apn,size_t size)
{
  if(obj->cell_apn == cell_apn)
      return;
  if(obj->has_cell_apn){
      free(obj->cell_apn);
      obj->cell_apn = NULL;
  }
  obj->cell_apn = cell_apn;
  obj->cell_apn_size = size;
  obj->has_cell_apn = size > 0;
}
void ig_SetNbConfigurationRequest_set_cell_apn(IgSetNbConfigurationRequest *obj,char* cell_apn,size_t size)
{
  if(obj->cell_apn == cell_apn)
      return;
  if(obj->has_cell_apn){
      free(obj->cell_apn);
      obj->cell_apn = NULL;
  }
  obj->cell_apn = malloc(size);
  memcpy(obj->cell_apn, cell_apn, size);
  obj->cell_apn_size = size;
  obj->has_cell_apn = size > 0;
}
size_t ig_SetNbConfigurationRequest_get_cell_dns_size(IgSetNbConfigurationRequest *obj)
{
  return obj->cell_dns_size;
}
void ig_SetNbConfigurationRequest_set_cell_dns_nocopy(IgSetNbConfigurationRequest *obj,char* cell_dns,size_t size)
{
  if(obj->cell_dns == cell_dns)
      return;
  if(obj->has_cell_dns){
      free(obj->cell_dns);
      obj->cell_dns = NULL;
  }
  obj->cell_dns = cell_dns;
  obj->cell_dns_size = size;
  obj->has_cell_dns = size > 0;
}
void ig_SetNbConfigurationRequest_set_cell_dns(IgSetNbConfigurationRequest *obj,char* cell_dns,size_t size)
{
  if(obj->cell_dns == cell_dns)
      return;
  if(obj->has_cell_dns){
      free(obj->cell_dns);
      obj->cell_dns = NULL;
  }
  obj->cell_dns = malloc(size);
  memcpy(obj->cell_dns, cell_dns, size);
  obj->cell_dns_size = size;
  obj->has_cell_dns = size > 0;
}
void ig_SetNbConfigurationRequest_set_cell_network_type(IgSetNbConfigurationRequest *obj,uint8_t cell_network_type)
{
  obj->cell_network_type = cell_network_type;
  obj->has_cell_network_type = true;
}
void ig_SetNbConfigurationRequest_set_cell_network_band(IgSetNbConfigurationRequest *obj,uint8_t cell_network_band)
{
  obj->cell_network_band = cell_network_band;
  obj->has_cell_network_band = true;
}
void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_enabled(IgSetNbConfigurationRequest *obj,bool nb_power_saving_mode_enabled)
{
  obj->nb_power_saving_mode_enabled = nb_power_saving_mode_enabled;
  obj->has_nb_power_saving_mode_enabled = true;
}
void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_active_timer(IgSetNbConfigurationRequest *obj,uint16_t nb_power_saving_mode_active_timer)
{
  obj->nb_power_saving_mode_active_timer = nb_power_saving_mode_active_timer;
  obj->has_nb_power_saving_mode_active_timer = true;
}
void ig_SetNbConfigurationRequest_set_nb_power_saving_mode_tau(IgSetNbConfigurationRequest *obj,uint16_t nb_power_saving_mode_tau)
{
  obj->nb_power_saving_mode_tau = nb_power_saving_mode_tau;
  obj->has_nb_power_saving_mode_tau = true;
}
size_t ig_SetNbConfigurationRequest_get_nb_mqtt_topic_size(IgSetNbConfigurationRequest *obj)
{
  return obj->nb_mqtt_topic_size;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_topic_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_topic,size_t size)
{
  if(obj->nb_mqtt_topic == nb_mqtt_topic)
      return;
  if(obj->has_nb_mqtt_topic){
      free(obj->nb_mqtt_topic);
      obj->nb_mqtt_topic = NULL;
  }
  obj->nb_mqtt_topic = nb_mqtt_topic;
  obj->nb_mqtt_topic_size = size;
  obj->has_nb_mqtt_topic = size > 0;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_topic(IgSetNbConfigurationRequest *obj,char* nb_mqtt_topic,size_t size)
{
  if(obj->nb_mqtt_topic == nb_mqtt_topic)
      return;
  if(obj->has_nb_mqtt_topic){
      free(obj->nb_mqtt_topic);
      obj->nb_mqtt_topic = NULL;
  }
  obj->nb_mqtt_topic = malloc(size);
  memcpy(obj->nb_mqtt_topic, nb_mqtt_topic, size);
  obj->nb_mqtt_topic_size = size;
  obj->has_nb_mqtt_topic = size > 0;
}
size_t ig_SetNbConfigurationRequest_get_nb_mqtt_broker_url_size(IgSetNbConfigurationRequest *obj)
{
  return obj->nb_mqtt_broker_url_size;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_url_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_url,size_t size)
{
  if(obj->nb_mqtt_broker_url == nb_mqtt_broker_url)
      return;
  if(obj->has_nb_mqtt_broker_url){
      free(obj->nb_mqtt_broker_url);
      obj->nb_mqtt_broker_url = NULL;
  }
  obj->nb_mqtt_broker_url = nb_mqtt_broker_url;
  obj->nb_mqtt_broker_url_size = size;
  obj->has_nb_mqtt_broker_url = size > 0;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_url(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_url,size_t size)
{
  if(obj->nb_mqtt_broker_url == nb_mqtt_broker_url)
      return;
  if(obj->has_nb_mqtt_broker_url){
      free(obj->nb_mqtt_broker_url);
      obj->nb_mqtt_broker_url = NULL;
  }
  obj->nb_mqtt_broker_url = malloc(size);
  memcpy(obj->nb_mqtt_broker_url, nb_mqtt_broker_url, size);
  obj->nb_mqtt_broker_url_size = size;
  obj->has_nb_mqtt_broker_url = size > 0;
}
size_t ig_SetNbConfigurationRequest_get_nb_mqtt_broker_port_size(IgSetNbConfigurationRequest *obj)
{
  return obj->nb_mqtt_broker_port_size;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_port_nocopy(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_port,size_t size)
{
  if(obj->nb_mqtt_broker_port == nb_mqtt_broker_port)
      return;
  if(obj->has_nb_mqtt_broker_port){
      free(obj->nb_mqtt_broker_port);
      obj->nb_mqtt_broker_port = NULL;
  }
  obj->nb_mqtt_broker_port = nb_mqtt_broker_port;
  obj->nb_mqtt_broker_port_size = size;
  obj->has_nb_mqtt_broker_port = size > 0;
}
void ig_SetNbConfigurationRequest_set_nb_mqtt_broker_port(IgSetNbConfigurationRequest *obj,char* nb_mqtt_broker_port,size_t size)
{
  if(obj->nb_mqtt_broker_port == nb_mqtt_broker_port)
      return;
  if(obj->has_nb_mqtt_broker_port){
      free(obj->nb_mqtt_broker_port);
      obj->nb_mqtt_broker_port = NULL;
  }
  obj->nb_mqtt_broker_port = malloc(size);
  memcpy(obj->nb_mqtt_broker_port, nb_mqtt_broker_port, size);
  obj->nb_mqtt_broker_port_size = size;
  obj->has_nb_mqtt_broker_port = size > 0;
}
void ig_SetNbConfigurationRequest_set_operation_id(IgSetNbConfigurationRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
