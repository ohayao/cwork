#include "SetMaxIncorrectPinRequest.h"
#include "external/cbor/cbor.h"

void ig_SetMaxIncorrectPinRequest_init(IgSetMaxIncorrectPinRequest *obj)
{
  memset(obj, 0, sizeof(IgSetMaxIncorrectPinRequest));
}
IgSerializerError ig_SetMaxIncorrectPinRequest_encode(IgSetMaxIncorrectPinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_SetMaxIncorrectPinRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_max_incorrect_pins) fields_size++;
  if(obj->has_incorrect_pin_lockout_on) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 39);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_max_incorrect_pins){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->max_incorrect_pins);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_incorrect_pin_lockout_on){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_boolean(&map, obj->incorrect_pin_lockout_on);
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
IgSerializerError ig_SetMaxIncorrectPinRequest_decode(uint8_t *buf,size_t buf_size,IgSetMaxIncorrectPinRequest *retval,size_t index)
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
            if(val != 39) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_SetMaxIncorrectPinRequest_set_password_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetMaxIncorrectPinRequest_set_max_incorrect_pins(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborBooleanType){
                bool val;
                cbor_value_get_boolean(&content, &val);
                ig_SetMaxIncorrectPinRequest_set_incorrect_pin_lockout_on(retval, val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetMaxIncorrectPinRequest_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_SetMaxIncorrectPinRequest_get_max_payload_in_bytes(IgSetMaxIncorrectPinRequest *obj)
{
  return 31 + obj->password_size; //13 + 18;
}
bool ig_SetMaxIncorrectPinRequest_is_valid(IgSetMaxIncorrectPinRequest *obj)
{
  return obj->has_password;
}
void ig_SetMaxIncorrectPinRequest_deinit(IgSetMaxIncorrectPinRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  
}
size_t ig_SetMaxIncorrectPinRequest_get_password_size(IgSetMaxIncorrectPinRequest *obj)
{
  return obj->password_size;
}
void ig_SetMaxIncorrectPinRequest_set_password_nocopy(IgSetMaxIncorrectPinRequest *obj,uint8_t* password,size_t size)
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
void ig_SetMaxIncorrectPinRequest_set_password(IgSetMaxIncorrectPinRequest *obj,uint8_t* password,size_t size)
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
void ig_SetMaxIncorrectPinRequest_set_max_incorrect_pins(IgSetMaxIncorrectPinRequest *obj,uint8_t max_incorrect_pins)
{
  obj->max_incorrect_pins = max_incorrect_pins;
  obj->has_max_incorrect_pins = true;
}
void ig_SetMaxIncorrectPinRequest_set_incorrect_pin_lockout_on(IgSetMaxIncorrectPinRequest *obj,bool incorrect_pin_lockout_on)
{
  obj->incorrect_pin_lockout_on = incorrect_pin_lockout_on;
  obj->has_incorrect_pin_lockout_on = true;
}
void ig_SetMaxIncorrectPinRequest_set_operation_id(IgSetMaxIncorrectPinRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
