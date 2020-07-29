#include "LogEntry.h"
#include "external/cbor/cbor.h"

void ig_LogEntry_init(IgLogEntry *obj)
{
  memset(obj, 0, sizeof(IgLogEntry));
}
IgSerializerError ig_LogEntry_encode(IgLogEntry *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_LogEntry_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 2;
  if(obj->has_key_id) fields_size++;
  if(obj->has_pin) fields_size++;
  if(obj->has_new_pin) fields_size++;
  if(obj->has_time1) fields_size++;
  if(obj->has_time2) fields_size++;
  if(obj->has_pin_type) fields_size++;
  if(obj->has_value1) fields_size++;
  if(obj->has_value2) fields_size++;
  if(obj->has_value3) fields_size++;
  if(obj->has_value4) fields_size++;
  if(obj->has_value5) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 203);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->entry_date);
      if(err) return (IgSerializerError) err;
  
  
  
  
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->log_type);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_key_id){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->key_id);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_pin){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->pin, obj->pin_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_new_pin){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->new_pin, obj->new_pin_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_time1){
      err = cbor_encode_uint(&map, 16);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->time1);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_time2){
      err = cbor_encode_uint(&map, 17);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->time2);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_pin_type){
      err = cbor_encode_uint(&map, 18);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->pin_type);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_value1){
      err = cbor_encode_uint(&map, 19);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->value1);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_value2){
      err = cbor_encode_uint(&map, 20);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->value2);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_value3){
      err = cbor_encode_uint(&map, 21);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->value3);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_value4){
      err = cbor_encode_uint(&map, 22);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->value4);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_value5){
      err = cbor_encode_uint(&map, 23);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->value5, obj->value5_size);
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
IgSerializerError ig_LogEntry_decode(uint8_t *buf,size_t buf_size,IgLogEntry *retval,size_t index)
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
            if(val != 203) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_entry_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_log_type(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_key_id(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
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
        
                ig_LogEntry_set_pin_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 15:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_LogEntry_set_new_pin_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 16:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_time1(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 17:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_time2(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 18:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_pin_type(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 19:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_value1(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 20:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_value2(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 21:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_value3(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 22:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_value4(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 23:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_LogEntry_set_value5_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_LogEntry_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_LogEntry_get_max_payload_in_bytes(IgLogEntry *obj)
{
  return 100 + obj->pin_size + obj->new_pin_size + obj->value5_size; //13 + 87;
}
bool ig_LogEntry_is_valid(IgLogEntry *obj)
{
  return obj->has_entry_date && obj->has_log_type;
}
void ig_LogEntry_deinit(IgLogEntry *obj)
{
  
  
  
  if(obj->has_pin){
      free(obj->pin);
      obj->pin = NULL;
  }
  
  
  if(obj->has_new_pin){
      free(obj->new_pin);
      obj->new_pin = NULL;
  }
  
  
  
  
  
  
  
  
  
  if(obj->has_value5){
      free(obj->value5);
      obj->value5 = NULL;
  }
  
}
void ig_LogEntry_set_entry_date(IgLogEntry *obj,uint32_t entry_date)
{
  obj->entry_date = entry_date;
  obj->has_entry_date = true;
}
void ig_LogEntry_set_log_type(IgLogEntry *obj,uint32_t log_type)
{
  obj->log_type = log_type;
  obj->has_log_type = true;
}
void ig_LogEntry_set_key_id(IgLogEntry *obj,uint32_t key_id)
{
  obj->key_id = key_id;
  obj->has_key_id = true;
}
size_t ig_LogEntry_get_pin_size(IgLogEntry *obj)
{
  return obj->pin_size;
}
void ig_LogEntry_set_pin_nocopy(IgLogEntry *obj,uint8_t* pin,size_t size)
{
  if(obj->pin == pin)
      return;
  if(obj->has_pin){
      free(obj->pin);
      obj->pin = NULL;
  }
  obj->pin = pin;
  obj->pin_size = size;
  obj->has_pin = size > 0;
}
void ig_LogEntry_set_pin(IgLogEntry *obj,uint8_t* pin,size_t size)
{
  if(obj->pin == pin)
      return;
  if(obj->has_pin){
      free(obj->pin);
      obj->pin = NULL;
  }
  obj->pin = malloc(size);
  memcpy(obj->pin, pin, size);
  obj->pin_size = size;
  obj->has_pin = size > 0;
}
size_t ig_LogEntry_get_new_pin_size(IgLogEntry *obj)
{
  return obj->new_pin_size;
}
void ig_LogEntry_set_new_pin_nocopy(IgLogEntry *obj,uint8_t* new_pin,size_t size)
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
void ig_LogEntry_set_new_pin(IgLogEntry *obj,uint8_t* new_pin,size_t size)
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
void ig_LogEntry_set_time1(IgLogEntry *obj,uint32_t time1)
{
  obj->time1 = time1;
  obj->has_time1 = true;
}
void ig_LogEntry_set_time2(IgLogEntry *obj,uint32_t time2)
{
  obj->time2 = time2;
  obj->has_time2 = true;
}
void ig_LogEntry_set_pin_type(IgLogEntry *obj,uint32_t pin_type)
{
  obj->pin_type = pin_type;
  obj->has_pin_type = true;
}
void ig_LogEntry_set_value1(IgLogEntry *obj,uint32_t value1)
{
  obj->value1 = value1;
  obj->has_value1 = true;
}
void ig_LogEntry_set_value2(IgLogEntry *obj,uint32_t value2)
{
  obj->value2 = value2;
  obj->has_value2 = true;
}
void ig_LogEntry_set_value3(IgLogEntry *obj,uint32_t value3)
{
  obj->value3 = value3;
  obj->has_value3 = true;
}
void ig_LogEntry_set_value4(IgLogEntry *obj,uint32_t value4)
{
  obj->value4 = value4;
  obj->has_value4 = true;
}
size_t ig_LogEntry_get_value5_size(IgLogEntry *obj)
{
  return obj->value5_size;
}
void ig_LogEntry_set_value5_nocopy(IgLogEntry *obj,uint8_t* value5,size_t size)
{
  if(obj->value5 == value5)
      return;
  if(obj->has_value5){
      free(obj->value5);
      obj->value5 = NULL;
  }
  obj->value5 = value5;
  obj->value5_size = size;
  obj->has_value5 = size > 0;
}
void ig_LogEntry_set_value5(IgLogEntry *obj,uint8_t* value5,size_t size)
{
  if(obj->value5 == value5)
      return;
  if(obj->has_value5){
      free(obj->value5);
      obj->value5 = NULL;
  }
  obj->value5 = malloc(size);
  memcpy(obj->value5, value5, size);
  obj->value5_size = size;
  obj->has_value5 = size > 0;
}
void ig_LogEntry_set_operation_id(IgLogEntry *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
