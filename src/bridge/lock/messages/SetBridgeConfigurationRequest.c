#include "SetBridgeConfigurationRequest.h"
#include "external/cbor/cbor.h"

void ig_SetBridgeConfigurationRequest_init(IgSetBridgeConfigurationRequest *obj)
{
  memset(obj, 0, sizeof(IgSetBridgeConfigurationRequest));
}
IgSerializerError ig_SetBridgeConfigurationRequest_encode(IgSetBridgeConfigurationRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_SetBridgeConfigurationRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_password) fields_size++;
  if(obj->has_ssid) fields_size++;
  if(obj->has_network_password) fields_size++;
  if(obj->has_mqtt_jwt_token) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 89);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_password){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_ssid){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->ssid, obj->ssid_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_network_password){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_text_string(&map, obj->network_password, obj->network_password_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_mqtt_jwt_token){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->mqtt_jwt_token, obj->mqtt_jwt_token_size);
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
IgSerializerError ig_SetBridgeConfigurationRequest_decode(uint8_t *buf,size_t buf_size,IgSetBridgeConfigurationRequest *retval,size_t index)
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
            if(val != 89) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_SetBridgeConfigurationRequest_set_password_nocopy(retval, data_arr, size);
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
        
                ig_SetBridgeConfigurationRequest_set_ssid_nocopy(retval, data_arr, size);
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
        
                ig_SetBridgeConfigurationRequest_set_network_password_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 14:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_SetBridgeConfigurationRequest_set_mqtt_jwt_token_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetBridgeConfigurationRequest_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_SetBridgeConfigurationRequest_get_max_payload_in_bytes(IgSetBridgeConfigurationRequest *obj)
{
  return 47 + obj->password_size + obj->ssid_size + obj->network_password_size + obj->mqtt_jwt_token_size; //13 + 34;
}
bool ig_SetBridgeConfigurationRequest_is_valid(IgSetBridgeConfigurationRequest *obj)
{
  return true;
}
void ig_SetBridgeConfigurationRequest_deinit(IgSetBridgeConfigurationRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  if(obj->has_ssid){
      free(obj->ssid);
      obj->ssid = NULL;
  }
  
  
  if(obj->has_network_password){
      free(obj->network_password);
      obj->network_password = NULL;
  }
  
  
  if(obj->has_mqtt_jwt_token){
      free(obj->mqtt_jwt_token);
      obj->mqtt_jwt_token = NULL;
  }
  
}
size_t ig_SetBridgeConfigurationRequest_get_password_size(IgSetBridgeConfigurationRequest *obj)
{
  return obj->password_size;
}
void ig_SetBridgeConfigurationRequest_set_password_nocopy(IgSetBridgeConfigurationRequest *obj,uint8_t* password,size_t size)
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
void ig_SetBridgeConfigurationRequest_set_password(IgSetBridgeConfigurationRequest *obj,uint8_t* password,size_t size)
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
size_t ig_SetBridgeConfigurationRequest_get_ssid_size(IgSetBridgeConfigurationRequest *obj)
{
  return obj->ssid_size;
}
void ig_SetBridgeConfigurationRequest_set_ssid_nocopy(IgSetBridgeConfigurationRequest *obj,char* ssid,size_t size)
{
  if(obj->ssid == ssid)
      return;
  if(obj->has_ssid){
      free(obj->ssid);
      obj->ssid = NULL;
  }
  obj->ssid = ssid;
  obj->ssid_size = size;
  obj->has_ssid = size > 0;
}
void ig_SetBridgeConfigurationRequest_set_ssid(IgSetBridgeConfigurationRequest *obj,char* ssid,size_t size)
{
  if(obj->ssid == ssid)
      return;
  if(obj->has_ssid){
      free(obj->ssid);
      obj->ssid = NULL;
  }
  obj->ssid = malloc(size);
  memcpy(obj->ssid, ssid, size);
  obj->ssid_size = size;
  obj->has_ssid = size > 0;
}
size_t ig_SetBridgeConfigurationRequest_get_network_password_size(IgSetBridgeConfigurationRequest *obj)
{
  return obj->network_password_size;
}
void ig_SetBridgeConfigurationRequest_set_network_password_nocopy(IgSetBridgeConfigurationRequest *obj,char* network_password,size_t size)
{
  if(obj->network_password == network_password)
      return;
  if(obj->has_network_password){
      free(obj->network_password);
      obj->network_password = NULL;
  }
  obj->network_password = network_password;
  obj->network_password_size = size;
  obj->has_network_password = size > 0;
}
void ig_SetBridgeConfigurationRequest_set_network_password(IgSetBridgeConfigurationRequest *obj,char* network_password,size_t size)
{
  if(obj->network_password == network_password)
      return;
  if(obj->has_network_password){
      free(obj->network_password);
      obj->network_password = NULL;
  }
  obj->network_password = malloc(size);
  memcpy(obj->network_password, network_password, size);
  obj->network_password_size = size;
  obj->has_network_password = size > 0;
}
size_t ig_SetBridgeConfigurationRequest_get_mqtt_jwt_token_size(IgSetBridgeConfigurationRequest *obj)
{
  return obj->mqtt_jwt_token_size;
}
void ig_SetBridgeConfigurationRequest_set_mqtt_jwt_token_nocopy(IgSetBridgeConfigurationRequest *obj,uint8_t* mqtt_jwt_token,size_t size)
{
  if(obj->mqtt_jwt_token == mqtt_jwt_token)
      return;
  if(obj->has_mqtt_jwt_token){
      free(obj->mqtt_jwt_token);
      obj->mqtt_jwt_token = NULL;
  }
  obj->mqtt_jwt_token = mqtt_jwt_token;
  obj->mqtt_jwt_token_size = size;
  obj->has_mqtt_jwt_token = size > 0;
}
void ig_SetBridgeConfigurationRequest_set_mqtt_jwt_token(IgSetBridgeConfigurationRequest *obj,uint8_t* mqtt_jwt_token,size_t size)
{
  if(obj->mqtt_jwt_token == mqtt_jwt_token)
      return;
  if(obj->has_mqtt_jwt_token){
      free(obj->mqtt_jwt_token);
      obj->mqtt_jwt_token = NULL;
  }
  obj->mqtt_jwt_token = malloc(size);
  memcpy(obj->mqtt_jwt_token, mqtt_jwt_token, size);
  obj->mqtt_jwt_token_size = size;
  obj->has_mqtt_jwt_token = size > 0;
}
void ig_SetBridgeConfigurationRequest_set_operation_id(IgSetBridgeConfigurationRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
