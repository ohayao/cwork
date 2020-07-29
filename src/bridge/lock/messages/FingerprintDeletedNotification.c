#include "FingerprintDeletedNotification.h"
#include "external/cbor/cbor.h"

void ig_FingerprintDeletedNotification_init(IgFingerprintDeletedNotification *obj)
{
  memset(obj, 0, sizeof(IgFingerprintDeletedNotification));
}
IgSerializerError ig_FingerprintDeletedNotification_encode(IgFingerprintDeletedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_FingerprintDeletedNotification_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_uid) fields_size++;
  if(obj->has_result) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 104);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_uid){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->uid);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_result){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->result);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_FingerprintDeletedNotification_decode(uint8_t *buf,size_t buf_size,IgFingerprintDeletedNotification *retval,size_t index)
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
            if(val != 104) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_FingerprintDeletedNotification_set_uid(retval, (uint16_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_FingerprintDeletedNotification_set_result(retval, (uint8_t) val);
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
uint32_t ig_FingerprintDeletedNotification_get_max_payload_in_bytes(IgFingerprintDeletedNotification *obj)
{
  return 20; //13 + 7;
}
bool ig_FingerprintDeletedNotification_is_valid(IgFingerprintDeletedNotification *obj)
{
  return true;
}
void ig_FingerprintDeletedNotification_deinit(IgFingerprintDeletedNotification *obj)
{
  
}
void ig_FingerprintDeletedNotification_set_uid(IgFingerprintDeletedNotification *obj,uint16_t uid)
{
  obj->uid = uid;
  obj->has_uid = true;
}
void ig_FingerprintDeletedNotification_set_result(IgFingerprintDeletedNotification *obj,uint8_t result)
{
  obj->result = result;
  obj->has_result = true;
}
