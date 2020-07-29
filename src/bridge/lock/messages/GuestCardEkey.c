#include "GuestCardEkey.h"
#include "external/cbor/cbor.h"

void ig_GuestCardEkey_init(IgGuestCardEkey *obj)
{
  memset(obj, 0, sizeof(IgGuestCardEkey));
}
IgSerializerError ig_GuestCardEkey_encode(IgGuestCardEkey *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GuestCardEkey_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 2;
  
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 201);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encode_uint(&map, 11);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_byte_string(&map, obj->encrypted_ekey, obj->encrypted_ekey_size);
  if(err) return (IgSerializerError) err;
  
  
  
  
  err = cbor_encode_uint(&map, 12);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_byte_string(&map, obj->mac_address, obj->mac_address_size);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_GuestCardEkey_decode(uint8_t *buf,size_t buf_size,IgGuestCardEkey *retval,size_t index)
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
            if(val != 201) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_GuestCardEkey_set_encrypted_ekey_nocopy(retval, data_arr, size);
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
        
                ig_GuestCardEkey_set_mac_address_nocopy(retval, data_arr, size);
                break;
            }
          default:
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}
uint32_t ig_GuestCardEkey_get_max_payload_in_bytes(IgGuestCardEkey *obj)
{
  return 27 + obj->encrypted_ekey_size + obj->mac_address_size; //13 + 14;
}
bool ig_GuestCardEkey_is_valid(IgGuestCardEkey *obj)
{
  return obj->has_encrypted_ekey && obj->has_mac_address;
}
void ig_GuestCardEkey_deinit(IgGuestCardEkey *obj)
{
  if(obj->has_encrypted_ekey){
      free(obj->encrypted_ekey);
      obj->encrypted_ekey = NULL;
  }
  
  
  if(obj->has_mac_address){
      free(obj->mac_address);
      obj->mac_address = NULL;
  }
}
size_t ig_GuestCardEkey_get_encrypted_ekey_size(IgGuestCardEkey *obj)
{
  return obj->encrypted_ekey_size;
}
void ig_GuestCardEkey_set_encrypted_ekey_nocopy(IgGuestCardEkey *obj,uint8_t* encrypted_ekey,size_t size)
{
  if(obj->encrypted_ekey == encrypted_ekey)
      return;
  if(obj->has_encrypted_ekey){
      free(obj->encrypted_ekey);
      obj->encrypted_ekey = NULL;
  }
  obj->encrypted_ekey = encrypted_ekey;
  obj->encrypted_ekey_size = size;
  obj->has_encrypted_ekey = size > 0;
}
void ig_GuestCardEkey_set_encrypted_ekey(IgGuestCardEkey *obj,uint8_t* encrypted_ekey,size_t size)
{
  if(obj->encrypted_ekey == encrypted_ekey)
      return;
  if(obj->has_encrypted_ekey){
      free(obj->encrypted_ekey);
      obj->encrypted_ekey = NULL;
  }
  obj->encrypted_ekey = malloc(size);
  memcpy(obj->encrypted_ekey, encrypted_ekey, size);
  obj->encrypted_ekey_size = size;
  obj->has_encrypted_ekey = size > 0;
}
size_t ig_GuestCardEkey_get_mac_address_size(IgGuestCardEkey *obj)
{
  return obj->mac_address_size;
}
void ig_GuestCardEkey_set_mac_address_nocopy(IgGuestCardEkey *obj,uint8_t* mac_address,size_t size)
{
  if(obj->mac_address == mac_address)
      return;
  if(obj->has_mac_address){
      free(obj->mac_address);
      obj->mac_address = NULL;
  }
  obj->mac_address = mac_address;
  obj->mac_address_size = size;
  obj->has_mac_address = size > 0;
}
void ig_GuestCardEkey_set_mac_address(IgGuestCardEkey *obj,uint8_t* mac_address,size_t size)
{
  if(obj->mac_address == mac_address)
      return;
  if(obj->has_mac_address){
      free(obj->mac_address);
      obj->mac_address = NULL;
  }
  obj->mac_address = malloc(size);
  memcpy(obj->mac_address, mac_address, size);
  obj->mac_address_size = size;
  obj->has_mac_address = size > 0;
}
