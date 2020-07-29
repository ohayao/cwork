#include "CardDeletedNotification.h"
#include "external/cbor/cbor.h"

void ig_CardDeletedNotification_init(IgCardDeletedNotification *obj)
{
  memset(obj, 0, sizeof(IgCardDeletedNotification));
}
IgSerializerError ig_CardDeletedNotification_encode(IgCardDeletedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_CardDeletedNotification_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_card_uid) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 102);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_card_uid){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->card_uid, obj->card_uid_size);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_CardDeletedNotification_decode(uint8_t *buf,size_t buf_size,IgCardDeletedNotification *retval,size_t index)
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
            if(val != 102) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_CardDeletedNotification_set_card_uid_nocopy(retval, data_arr, size);
                break;
            }
          default:
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}
uint32_t ig_CardDeletedNotification_get_max_payload_in_bytes(IgCardDeletedNotification *obj)
{
  return 20 + obj->card_uid_size; //13 + 7;
}
bool ig_CardDeletedNotification_is_valid(IgCardDeletedNotification *obj)
{
  return true;
}
void ig_CardDeletedNotification_deinit(IgCardDeletedNotification *obj)
{
  if(obj->has_card_uid){
      free(obj->card_uid);
      obj->card_uid = NULL;
  }
}
size_t ig_CardDeletedNotification_get_card_uid_size(IgCardDeletedNotification *obj)
{
  return obj->card_uid_size;
}
void ig_CardDeletedNotification_set_card_uid_nocopy(IgCardDeletedNotification *obj,uint8_t* card_uid,size_t size)
{
  if(obj->card_uid == card_uid)
      return;
  if(obj->has_card_uid){
      free(obj->card_uid);
      obj->card_uid = NULL;
  }
  obj->card_uid = card_uid;
  obj->card_uid_size = size;
  obj->has_card_uid = size > 0;
}
void ig_CardDeletedNotification_set_card_uid(IgCardDeletedNotification *obj,uint8_t* card_uid,size_t size)
{
  if(obj->card_uid == card_uid)
      return;
  if(obj->has_card_uid){
      free(obj->card_uid);
      obj->card_uid = NULL;
  }
  obj->card_uid = malloc(size);
  memcpy(obj->card_uid, card_uid, size);
  obj->card_uid_size = size;
  obj->has_card_uid = size > 0;
}
