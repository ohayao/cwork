#include "GuestToken.h"
#include "external/cbor/cbor.h"

void ig_GuestToken_init(IgGuestToken *obj)
{
  memset(obj, 0, sizeof(IgGuestToken));
}
IgSerializerError ig_GuestToken_encode(IgGuestToken *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GuestToken_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_start_date) fields_size++;
  if(obj->has_end_date) fields_size++;
  if(obj->has_aes_key) fields_size++;
  if(obj->has_access_rights) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 202);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->key_id);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_start_date){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->start_date);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_end_date){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->end_date);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_aes_key){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->aes_key, obj->aes_key_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_access_rights){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->access_rights, obj->access_rights_size);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_GuestToken_decode(uint8_t *buf,size_t buf_size,IgGuestToken *retval,size_t index)
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
            if(val != 202) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GuestToken_set_key_id(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GuestToken_set_start_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GuestToken_set_end_date(retval, (uint32_t) val);
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
        
                ig_GuestToken_set_aes_key_nocopy(retval, data_arr, size);
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
        
                ig_GuestToken_set_access_rights_nocopy(retval, data_arr, size);
                break;
            }
          default:
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}
uint32_t ig_GuestToken_get_max_payload_in_bytes(IgGuestToken *obj)
{
  return 45 + obj->aes_key_size + obj->access_rights_size; //13 + 32;
}
bool ig_GuestToken_is_valid(IgGuestToken *obj)
{
  return obj->has_key_id;
}
void ig_GuestToken_deinit(IgGuestToken *obj)
{
  
  
  
  if(obj->has_aes_key){
      free(obj->aes_key);
      obj->aes_key = NULL;
  }
  
  
  if(obj->has_access_rights){
      free(obj->access_rights);
      obj->access_rights = NULL;
  }
}
void ig_GuestToken_set_key_id(IgGuestToken *obj,uint32_t key_id)
{
  obj->key_id = key_id;
  obj->has_key_id = true;
}
void ig_GuestToken_set_start_date(IgGuestToken *obj,uint32_t start_date)
{
  obj->start_date = start_date;
  obj->has_start_date = true;
}
void ig_GuestToken_set_end_date(IgGuestToken *obj,uint32_t end_date)
{
  obj->end_date = end_date;
  obj->has_end_date = true;
}
size_t ig_GuestToken_get_aes_key_size(IgGuestToken *obj)
{
  return obj->aes_key_size;
}
void ig_GuestToken_set_aes_key_nocopy(IgGuestToken *obj,uint8_t* aes_key,size_t size)
{
  if(obj->aes_key == aes_key)
      return;
  if(obj->has_aes_key){
      free(obj->aes_key);
      obj->aes_key = NULL;
  }
  obj->aes_key = aes_key;
  obj->aes_key_size = size;
  obj->has_aes_key = size > 0;
}
void ig_GuestToken_set_aes_key(IgGuestToken *obj,uint8_t* aes_key,size_t size)
{
  if(obj->aes_key == aes_key)
      return;
  if(obj->has_aes_key){
      free(obj->aes_key);
      obj->aes_key = NULL;
  }
  obj->aes_key = malloc(size);
  memcpy(obj->aes_key, aes_key, size);
  obj->aes_key_size = size;
  obj->has_aes_key = size > 0;
}
size_t ig_GuestToken_get_access_rights_size(IgGuestToken *obj)
{
  return obj->access_rights_size;
}
void ig_GuestToken_set_access_rights_nocopy(IgGuestToken *obj,uint8_t* access_rights,size_t size)
{
  if(obj->access_rights == access_rights)
      return;
  if(obj->has_access_rights){
      free(obj->access_rights);
      obj->access_rights = NULL;
  }
  obj->access_rights = access_rights;
  obj->access_rights_size = size;
  obj->has_access_rights = size > 0;
}
void ig_GuestToken_set_access_rights(IgGuestToken *obj,uint8_t* access_rights,size_t size)
{
  if(obj->access_rights == access_rights)
      return;
  if(obj->has_access_rights){
      free(obj->access_rights);
      obj->access_rights = NULL;
  }
  obj->access_rights = malloc(size);
  memcpy(obj->access_rights, access_rights, size);
  obj->access_rights_size = size;
  obj->has_access_rights = size > 0;
}
