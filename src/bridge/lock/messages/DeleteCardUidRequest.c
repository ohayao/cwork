#include "DeleteCardUidRequest.h"
#include "external/cbor/cbor.h"

void ig_DeleteCardUidRequest_init(IgDeleteCardUidRequest *obj)
{
  memset(obj, 0, sizeof(IgDeleteCardUidRequest));
}
IgSerializerError ig_DeleteCardUidRequest_encode(IgDeleteCardUidRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_DeleteCardUidRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_card_uid) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 69);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_card_uid){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->card_uid, obj->card_uid_size);
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
IgSerializerError ig_DeleteCardUidRequest_decode(uint8_t *buf,size_t buf_size,IgDeleteCardUidRequest *retval,size_t index)
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
            if(val != 69) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_DeleteCardUidRequest_set_password_nocopy(retval, data_arr, size);
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
        
                ig_DeleteCardUidRequest_set_card_uid_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_DeleteCardUidRequest_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_DeleteCardUidRequest_get_max_payload_in_bytes(IgDeleteCardUidRequest *obj)
{
  return 33 + obj->password_size + obj->card_uid_size; //13 + 20;
}
bool ig_DeleteCardUidRequest_is_valid(IgDeleteCardUidRequest *obj)
{
  return obj->has_password;
}
void ig_DeleteCardUidRequest_deinit(IgDeleteCardUidRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  if(obj->has_card_uid){
      free(obj->card_uid);
      obj->card_uid = NULL;
  }
  
}
size_t ig_DeleteCardUidRequest_get_password_size(IgDeleteCardUidRequest *obj)
{
  return obj->password_size;
}
void ig_DeleteCardUidRequest_set_password_nocopy(IgDeleteCardUidRequest *obj,uint8_t* password,size_t size)
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
void ig_DeleteCardUidRequest_set_password(IgDeleteCardUidRequest *obj,uint8_t* password,size_t size)
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
size_t ig_DeleteCardUidRequest_get_card_uid_size(IgDeleteCardUidRequest *obj)
{
  return obj->card_uid_size;
}
void ig_DeleteCardUidRequest_set_card_uid_nocopy(IgDeleteCardUidRequest *obj,uint8_t* card_uid,size_t size)
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
void ig_DeleteCardUidRequest_set_card_uid(IgDeleteCardUidRequest *obj,uint8_t* card_uid,size_t size)
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
void ig_DeleteCardUidRequest_set_operation_id(IgDeleteCardUidRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
