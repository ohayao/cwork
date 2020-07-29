#include "EditPinRequest.h"
#include "external/cbor/cbor.h"

void ig_EditPinRequest_init(IgEditPinRequest *obj)
{
  memset(obj, 0, sizeof(IgEditPinRequest));
}
IgSerializerError ig_EditPinRequest_encode(IgEditPinRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_EditPinRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 2;
  if(obj->has_new_pin) fields_size++;
  if(obj->has_start_date) fields_size++;
  if(obj->has_end_date) fields_size++;
  if(obj->has_pin_type) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 45);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  
  
  
  
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->old_pin, obj->old_pin_size);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_new_pin){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->new_pin, obj->new_pin_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_start_date){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->start_date);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_end_date){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->end_date);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_pin_type){
      err = cbor_encode_uint(&map, 16);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->pin_type);
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
IgSerializerError ig_EditPinRequest_decode(uint8_t *buf,size_t buf_size,IgEditPinRequest *retval,size_t index)
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
            if(val != 45) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_EditPinRequest_set_password_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 12:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_EditPinRequest_set_old_pin_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 13:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_EditPinRequest_set_new_pin_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 14:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_EditPinRequest_set_start_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 15:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_EditPinRequest_set_end_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 16:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_EditPinRequest_set_pin_type(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_EditPinRequest_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_EditPinRequest_get_max_payload_in_bytes(IgEditPinRequest *obj)
{
  return 55 + obj->password_size + obj->old_pin_size + obj->new_pin_size; //13 + 42;
}
bool ig_EditPinRequest_is_valid(IgEditPinRequest *obj)
{
  return obj->has_password && obj->has_old_pin;
}
void ig_EditPinRequest_deinit(IgEditPinRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  if(obj->has_old_pin){
      free(obj->old_pin);
      obj->old_pin = NULL;
  }
  
  
  if(obj->has_new_pin){
      free(obj->new_pin);
      obj->new_pin = NULL;
  }
  
  
  
  
}
size_t ig_EditPinRequest_get_password_size(IgEditPinRequest *obj)
{
  return obj->password_size;
}
void ig_EditPinRequest_set_password_nocopy(IgEditPinRequest *obj,uint8_t* password,size_t size)
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
void ig_EditPinRequest_set_password(IgEditPinRequest *obj,uint8_t* password,size_t size)
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
size_t ig_EditPinRequest_get_old_pin_size(IgEditPinRequest *obj)
{
  return obj->old_pin_size;
}
void ig_EditPinRequest_set_old_pin_nocopy(IgEditPinRequest *obj,uint8_t* old_pin,size_t size)
{
  if(obj->old_pin == old_pin)
      return;
  if(obj->has_old_pin){
      free(obj->old_pin);
      obj->old_pin = NULL;
  }
  obj->old_pin = old_pin;
  obj->old_pin_size = size;
  obj->has_old_pin = size > 0;
}
void ig_EditPinRequest_set_old_pin(IgEditPinRequest *obj,uint8_t* old_pin,size_t size)
{
  if(obj->old_pin == old_pin)
      return;
  if(obj->has_old_pin){
      free(obj->old_pin);
      obj->old_pin = NULL;
  }
  obj->old_pin = malloc(size);
  memcpy(obj->old_pin, old_pin, size);
  obj->old_pin_size = size;
  obj->has_old_pin = size > 0;
}
size_t ig_EditPinRequest_get_new_pin_size(IgEditPinRequest *obj)
{
  return obj->new_pin_size;
}
void ig_EditPinRequest_set_new_pin_nocopy(IgEditPinRequest *obj,uint8_t* new_pin,size_t size)
{
  if(obj->new_pin == new_pin)
      return;
  if(obj->has_new_pin){
      free(obj->new_pin);
      obj->new_pin = NULL;
  }
  obj->new_pin = new_pin;
  obj->new_pin_size = size;
  obj->has_new_pin = size > 0;
}
void ig_EditPinRequest_set_new_pin(IgEditPinRequest *obj,uint8_t* new_pin,size_t size)
{
  if(obj->new_pin == new_pin)
      return;
  if(obj->has_new_pin){
      free(obj->new_pin);
      obj->new_pin = NULL;
  }
  obj->new_pin = malloc(size);
  memcpy(obj->new_pin, new_pin, size);
  obj->new_pin_size = size;
  obj->has_new_pin = size > 0;
}
void ig_EditPinRequest_set_start_date(IgEditPinRequest *obj,uint32_t start_date)
{
  obj->start_date = start_date;
  obj->has_start_date = true;
}
void ig_EditPinRequest_set_end_date(IgEditPinRequest *obj,uint32_t end_date)
{
  obj->end_date = end_date;
  obj->has_end_date = true;
}
void ig_EditPinRequest_set_pin_type(IgEditPinRequest *obj,uint8_t pin_type)
{
  obj->pin_type = pin_type;
  obj->has_pin_type = true;
}
void ig_EditPinRequest_set_operation_id(IgEditPinRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
