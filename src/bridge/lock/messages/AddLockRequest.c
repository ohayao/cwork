#include "AddLockRequest.h"
#include "external/cbor/cbor.h"

void ig_AddLockRequest_init(IgAddLockRequest *obj)
{
  memset(obj, 0, sizeof(IgAddLockRequest));
}
IgSerializerError ig_AddLockRequest_encode(IgAddLockRequest *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_AddLockRequest_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_password) fields_size++;
  if(obj->has_lock_id) fields_size++;
  if(obj->has_guest_aes_key) fields_size++;
  if(obj->has_guest_token) fields_size++;
  if(obj->has_lock_password) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 83);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_password){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_lock_id){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->lock_id, obj->lock_id_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_guest_aes_key){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->guest_aes_key, obj->guest_aes_key_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_guest_token){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->guest_token, obj->guest_token_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_lock_password){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->lock_password, obj->lock_password_size);
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
IgSerializerError ig_AddLockRequest_decode(uint8_t *buf,size_t buf_size,IgAddLockRequest *retval,size_t index)
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
            if(val != 83) return IgSerializerErrorInvalidMsgIdValue;
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
        
                ig_AddLockRequest_set_password_nocopy(retval, data_arr, size);
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
        
                ig_AddLockRequest_set_lock_id_nocopy(retval, data_arr, size);
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
        
                ig_AddLockRequest_set_guest_aes_key_nocopy(retval, data_arr, size);
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
        
                ig_AddLockRequest_set_guest_token_nocopy(retval, data_arr, size);
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
        
                ig_AddLockRequest_set_lock_password_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_AddLockRequest_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_AddLockRequest_get_max_payload_in_bytes(IgAddLockRequest *obj)
{
  return 54 + obj->password_size + obj->lock_id_size + obj->guest_aes_key_size + obj->guest_token_size + obj->lock_password_size; //13 + 41;
}
bool ig_AddLockRequest_is_valid(IgAddLockRequest *obj)
{
  return true;
}
void ig_AddLockRequest_deinit(IgAddLockRequest *obj)
{
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
  
  
  if(obj->has_lock_id){
      free(obj->lock_id);
      obj->lock_id = NULL;
  }
  
  
  if(obj->has_guest_aes_key){
      free(obj->guest_aes_key);
      obj->guest_aes_key = NULL;
  }
  
  
  if(obj->has_guest_token){
      free(obj->guest_token);
      obj->guest_token = NULL;
  }
  
  
  if(obj->has_lock_password){
      free(obj->lock_password);
      obj->lock_password = NULL;
  }
  
}
size_t ig_AddLockRequest_get_password_size(IgAddLockRequest *obj)
{
  return obj->password_size;
}
void ig_AddLockRequest_set_password_nocopy(IgAddLockRequest *obj,uint8_t* password,size_t size)
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
void ig_AddLockRequest_set_password(IgAddLockRequest *obj,uint8_t* password,size_t size)
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
size_t ig_AddLockRequest_get_lock_id_size(IgAddLockRequest *obj)
{
  return obj->lock_id_size;
}
void ig_AddLockRequest_set_lock_id_nocopy(IgAddLockRequest *obj,uint8_t* lock_id,size_t size)
{
  if(obj->lock_id == lock_id)
      return;
  if(obj->has_lock_id){
      free(obj->lock_id);
      obj->lock_id = NULL;
  }
  obj->lock_id = lock_id;
  obj->lock_id_size = size;
  obj->has_lock_id = size > 0;
}
void ig_AddLockRequest_set_lock_id(IgAddLockRequest *obj,uint8_t* lock_id,size_t size)
{
  if(obj->lock_id == lock_id)
      return;
  if(obj->has_lock_id){
      free(obj->lock_id);
      obj->lock_id = NULL;
  }
  obj->lock_id = malloc(size);
  memcpy(obj->lock_id, lock_id, size);
  obj->lock_id_size = size;
  obj->has_lock_id = size > 0;
}
size_t ig_AddLockRequest_get_guest_aes_key_size(IgAddLockRequest *obj)
{
  return obj->guest_aes_key_size;
}
void ig_AddLockRequest_set_guest_aes_key_nocopy(IgAddLockRequest *obj,uint8_t* guest_aes_key,size_t size)
{
  if(obj->guest_aes_key == guest_aes_key)
      return;
  if(obj->has_guest_aes_key){
      free(obj->guest_aes_key);
      obj->guest_aes_key = NULL;
  }
  obj->guest_aes_key = guest_aes_key;
  obj->guest_aes_key_size = size;
  obj->has_guest_aes_key = size > 0;
}
void ig_AddLockRequest_set_guest_aes_key(IgAddLockRequest *obj,uint8_t* guest_aes_key,size_t size)
{
  if(obj->guest_aes_key == guest_aes_key)
      return;
  if(obj->has_guest_aes_key){
      free(obj->guest_aes_key);
      obj->guest_aes_key = NULL;
  }
  obj->guest_aes_key = malloc(size);
  memcpy(obj->guest_aes_key, guest_aes_key, size);
  obj->guest_aes_key_size = size;
  obj->has_guest_aes_key = size > 0;
}
size_t ig_AddLockRequest_get_guest_token_size(IgAddLockRequest *obj)
{
  return obj->guest_token_size;
}
void ig_AddLockRequest_set_guest_token_nocopy(IgAddLockRequest *obj,uint8_t* guest_token,size_t size)
{
  if(obj->guest_token == guest_token)
      return;
  if(obj->has_guest_token){
      free(obj->guest_token);
      obj->guest_token = NULL;
  }
  obj->guest_token = guest_token;
  obj->guest_token_size = size;
  obj->has_guest_token = size > 0;
}
void ig_AddLockRequest_set_guest_token(IgAddLockRequest *obj,uint8_t* guest_token,size_t size)
{
  if(obj->guest_token == guest_token)
      return;
  if(obj->has_guest_token){
      free(obj->guest_token);
      obj->guest_token = NULL;
  }
  obj->guest_token = malloc(size);
  memcpy(obj->guest_token, guest_token, size);
  obj->guest_token_size = size;
  obj->has_guest_token = size > 0;
}
size_t ig_AddLockRequest_get_lock_password_size(IgAddLockRequest *obj)
{
  return obj->lock_password_size;
}
void ig_AddLockRequest_set_lock_password_nocopy(IgAddLockRequest *obj,uint8_t* lock_password,size_t size)
{
  if(obj->lock_password == lock_password)
      return;
  if(obj->has_lock_password){
      free(obj->lock_password);
      obj->lock_password = NULL;
  }
  obj->lock_password = lock_password;
  obj->lock_password_size = size;
  obj->has_lock_password = size > 0;
}
void ig_AddLockRequest_set_lock_password(IgAddLockRequest *obj,uint8_t* lock_password,size_t size)
{
  if(obj->lock_password == lock_password)
      return;
  if(obj->has_lock_password){
      free(obj->lock_password);
      obj->lock_password = NULL;
  }
  obj->lock_password = malloc(size);
  memcpy(obj->lock_password, lock_password, size);
  obj->lock_password_size = size;
  obj->has_lock_password = size > 0;
}
void ig_AddLockRequest_set_operation_id(IgAddLockRequest *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
