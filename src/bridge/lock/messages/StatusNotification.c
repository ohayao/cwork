#include "StatusNotification.h"
#include "external/cbor/cbor.h"

void ig_StatusNotification_init(IgStatusNotification *obj)
{
  memset(obj, 0, sizeof(IgStatusNotification));
}
IgSerializerError ig_StatusNotification_encode(IgStatusNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_StatusNotification_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_woken) fields_size++;
  if(obj->has_lock_open) fields_size++;
  if(obj->has_has_logs) fields_size++;
  if(obj->has_door_open) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 100);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_woken){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->woken);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_lock_open){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->lock_open);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_has_logs){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->has_logs);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_door_open){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->door_open);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_StatusNotification_decode(uint8_t *buf,size_t buf_size,IgStatusNotification *retval,size_t index)
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
            if(val != 100) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_StatusNotification_set_woken(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_StatusNotification_set_lock_open(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_StatusNotification_set_has_logs(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 15:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_StatusNotification_set_door_open(retval, (uint8_t) val);
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
uint32_t ig_StatusNotification_get_max_payload_in_bytes(IgStatusNotification *obj)
{
  return 25; //13 + 12;
}
bool ig_StatusNotification_is_valid(IgStatusNotification *obj)
{
  return true;
}
void ig_StatusNotification_deinit(IgStatusNotification *obj)
{
  
  
}
void ig_StatusNotification_set_woken(IgStatusNotification *obj,uint8_t woken)
{
  obj->woken = woken;
  obj->has_woken = true;
}
void ig_StatusNotification_set_lock_open(IgStatusNotification *obj,uint8_t lock_open)
{
  obj->lock_open = lock_open;
  obj->has_lock_open = true;
}
void ig_StatusNotification_set_has_logs(IgStatusNotification *obj,uint8_t has_logs)
{
  obj->has_logs = has_logs;
  obj->has_has_logs = true;
}
void ig_StatusNotification_set_door_open(IgStatusNotification *obj,uint8_t door_open)
{
  obj->door_open = door_open;
  obj->has_door_open = true;
}
