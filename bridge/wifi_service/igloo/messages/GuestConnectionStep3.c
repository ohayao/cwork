#include "GuestConnectionStep3.h"
#include "../cbor/cbor.h"

void ig_GuestConnectionStep3_init(IgGuestConnectionStep3 *obj)
{
  memset(obj, 0, sizeof(IgGuestConnectionStep3));
}
IgSerializerError ig_GuestConnectionStep3_encode(IgGuestConnectionStep3 *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GuestConnectionStep3_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 11);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encode_uint(&map, 11);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_byte_string(&map, obj->nonce, obj->nonce_size);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_GuestConnectionStep3_decode(uint8_t *buf,size_t buf_size,IgGuestConnectionStep3 *retval,size_t index)
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
            if(val != 11) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_GuestConnectionStep3_set_nonce_nocopy(retval, data_arr, size);
                break;
            }
          default:
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}
uint32_t ig_GuestConnectionStep3_get_max_payload_in_bytes(IgGuestConnectionStep3 *obj)
{
  return 20 + obj->nonce_size; //13 + 7;
}
bool ig_GuestConnectionStep3_is_valid(IgGuestConnectionStep3 *obj)
{
  return obj->has_nonce;
}
void ig_GuestConnectionStep3_deinit(IgGuestConnectionStep3 *obj)
{
  if(obj->has_nonce){
      free(obj->nonce);
      obj->nonce = NULL;
  }
}
size_t ig_GuestConnectionStep3_get_nonce_size(IgGuestConnectionStep3 *obj)
{
  return obj->nonce_size;
}
void ig_GuestConnectionStep3_set_nonce_nocopy(IgGuestConnectionStep3 *obj,uint8_t* nonce,size_t size)
{
  if(obj->nonce == nonce)
      return;
  if(obj->has_nonce){
      free(obj->nonce);
      obj->nonce = NULL;
  }
  obj->nonce = nonce;
  obj->nonce_size = size;
  obj->has_nonce = size > 0;
}
void ig_GuestConnectionStep3_set_nonce(IgGuestConnectionStep3 *obj,uint8_t* nonce,size_t size)
{
  if(obj->nonce == nonce)
      return;
  if(obj->has_nonce){
      free(obj->nonce);
      obj->nonce = NULL;
  }
  obj->nonce = malloc(size);
  memcpy(obj->nonce, nonce, size);
  obj->nonce_size = size;
  obj->has_nonce = size > 0;
}
