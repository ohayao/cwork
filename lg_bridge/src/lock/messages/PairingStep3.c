#include "PairingStep3.h"
#include "external/cbor/cbor.h"

void ig_PairingStep3_init(IgPairingStep3 *obj)
{
  memset(obj, 0, sizeof(IgPairingStep3));
}

IgSerializerError ig_PairingStep3_encode(IgPairingStep3 *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_PairingStep3_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_master_pin) fields_size++;
  if(obj->has_pin_key) fields_size++;
  if(obj->has_gmt_offset) fields_size++;
  if(obj->has_dst_times) fields_size++;
  if(obj->has_password) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 3);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->nonce, obj->nonce_size);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_master_pin){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->master_pin, obj->master_pin_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_pin_key){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->pin_key, obj->pin_key_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_gmt_offset){
      err = cbor_encode_uint(&map, 15);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_int(&map, obj->gmt_offset);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_dst_times){
      err = cbor_encode_uint(&map, 16);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->dst_times, obj->dst_times_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_password){
      err = cbor_encode_uint(&map, 17);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->password, obj->password_size);
      if(err) return (IgSerializerError) err;
  }
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_PairingStep3_decode(uint8_t *buf,size_t buf_size,IgPairingStep3 *retval,size_t index)
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
    printf("cbor_value_get_type\n");
    if(tag_id_type != CborIntegerType) return IgSerializerErrorInvalidTypeTagId;
  
    cbor_value_get_int64(&content, &tag_id);
    err = cbor_value_advance_fixed(&content);
    printf("cbor_value_advance_fixed\n");
    if(err) return (IgSerializerError) err;
  
    if(!err){
      //handle value
      CborType value_type = cbor_value_get_type(&content);
      printf("value_type %x\n", value_type);
      switch (tag_id) {
        case 0:
          printf("case 0\n");
          if(value_type == CborIntegerType){
            printf("case 0 end\n");
            int64_t val;
            cbor_value_get_int64(&content, &val);
            //msgId value
            if(val != 3) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
          break;
        
        case 11:
            printf("case 11\n");
            if(value_type == CborByteStringType){
                printf("case 11 end\n");
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_PairingStep3_set_nonce_nocopy(retval, data_arr, size);
                break;
            }
            break;
        
        
        case 13:
            printf("case 13\n");
            if(value_type == CborByteStringType){
                printf("case 13 end\n");
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
                for (int i = 0; i < size; i++)
                {
                  printf("%x ", data_arr[i]);
                }
                printf("\n");
                ig_PairingStep3_set_master_pin_nocopy(retval, data_arr, size);
                break;
            }
            break;
             
        case 14:
            printf("case 14\n");
            if(value_type == CborByteStringType){
                printf("case 14 end\n");
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                printf("case 14 cbor_value_get_string_length %d\n", size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                printf("case 14 cbor_value_copy_byte_string\n");
                if(err) return (IgSerializerError) err;
                printf("case 14 ig_PairingStep3_set_pin_key_nocopy\n");
                ig_PairingStep3_set_pin_key_nocopy(retval, data_arr, size);
                break;
            }
            break;
        
        case 15:
            printf("case 15\n");
            if(value_type == CborIntegerType){
                printf("case 15 end\n");
                int64_t val;
                cbor_value_get_int64(&content, &val);
                ig_PairingStep3_set_gmt_offset(retval, (int32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
            break;
        
        case 16:
            printf("case 16\n");
            if(value_type == CborByteStringType){
                printf("case 16 end\n");
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_PairingStep3_set_dst_times_nocopy(retval, data_arr, size);
                break;
            }
            break;
        
        case 17:
            printf("case 17\n");
            if(value_type == CborByteStringType){
                printf("case 17 end\n");
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return (IgSerializerError) err;
        
                uint8_t *data_arr = malloc(size);
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return (IgSerializerError) err;
        
                ig_PairingStep3_set_password_nocopy(retval, data_arr, size);
                break;
            }
            break;
          default:
            printf("default\n");
            return IgSerializerErrorUnknownTagId;
        }
  
      }
  
    }
  
  return (IgSerializerError) err;
}

uint32_t ig_PairingStep3_get_max_payload_in_bytes(IgPairingStep3 *obj)
{
  return 54 + obj->nonce_size + obj->master_pin_size + obj->pin_key_size + obj->dst_times_size + obj->password_size; //13 + 41;
}
bool ig_PairingStep3_is_valid(IgPairingStep3 *obj)
{
  return obj->has_nonce;
}
void ig_PairingStep3_deinit(IgPairingStep3 *obj)
{
  if(obj->has_nonce){
      free(obj->nonce);
      obj->nonce = NULL;
  }
  
  
  if(obj->has_master_pin){
      free(obj->master_pin);
      obj->master_pin = NULL;
  }
  
  
  if(obj->has_pin_key){
      free(obj->pin_key);
      obj->pin_key = NULL;
  }
  
  
  
  if(obj->has_dst_times){
      free(obj->dst_times);
      obj->dst_times = NULL;
  }
  
  
  if(obj->has_password){
      free(obj->password);
      obj->password = NULL;
  }
}
size_t ig_PairingStep3_get_nonce_size(IgPairingStep3 *obj)
{
  return obj->nonce_size;
}
void ig_PairingStep3_set_nonce_nocopy(IgPairingStep3 *obj,uint8_t* nonce,size_t size)
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
void ig_PairingStep3_set_nonce(IgPairingStep3 *obj,uint8_t* nonce,size_t size)
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
size_t ig_PairingStep3_get_master_pin_size(IgPairingStep3 *obj)
{
  return obj->master_pin_size;
}
void ig_PairingStep3_set_master_pin_nocopy(IgPairingStep3 *obj,uint8_t* master_pin,size_t size)
{
  if(obj->master_pin == master_pin)
      return;
  if(obj->has_master_pin){
      free(obj->master_pin);
      obj->master_pin = NULL;
  }
  obj->master_pin = master_pin;
  obj->master_pin_size = size;
  obj->has_master_pin = size > 0;
}
void ig_PairingStep3_set_master_pin(IgPairingStep3 *obj,uint8_t* master_pin,size_t size)
{
  if(obj->master_pin == master_pin)
      return;
  if(obj->has_master_pin){
      free(obj->master_pin);
      obj->master_pin = NULL;
  }
  obj->master_pin = malloc(size);
  memcpy(obj->master_pin, master_pin, size);
  obj->master_pin_size = size;
  obj->has_master_pin = size > 0;
}
size_t ig_PairingStep3_get_pin_key_size(IgPairingStep3 *obj)
{
  return obj->pin_key_size;
}
void ig_PairingStep3_set_pin_key_nocopy(IgPairingStep3 *obj,uint8_t* pin_key,size_t size)
{
  if(obj->pin_key == pin_key)
      return;
  if(obj->has_pin_key){
      free(obj->pin_key);
      obj->pin_key = NULL;
  }
  obj->pin_key = pin_key;
  obj->pin_key_size = size;
  obj->has_pin_key = size > 0;
}
void ig_PairingStep3_set_pin_key(IgPairingStep3 *obj,uint8_t* pin_key,size_t size)
{
  if(obj->pin_key == pin_key)
      return;
  if(obj->has_pin_key){
      free(obj->pin_key);
      obj->pin_key = NULL;
  }
  obj->pin_key = malloc(size);
  memcpy(obj->pin_key, pin_key, size);
  obj->pin_key_size = size;
  obj->has_pin_key = size > 0;
}
void ig_PairingStep3_set_gmt_offset(IgPairingStep3 *obj,int32_t gmt_offset)
{
  obj->gmt_offset = gmt_offset;
  obj->has_gmt_offset = true;
}
size_t ig_PairingStep3_get_dst_times_size(IgPairingStep3 *obj)
{
  return obj->dst_times_size;
}
void ig_PairingStep3_set_dst_times_nocopy(IgPairingStep3 *obj,uint8_t* dst_times,size_t size)
{
  if(obj->dst_times == dst_times)
      return;
  if(obj->has_dst_times){
      free(obj->dst_times);
      obj->dst_times = NULL;
  }
  obj->dst_times = dst_times;
  obj->dst_times_size = size;
  obj->has_dst_times = size > 0;
}
void ig_PairingStep3_set_dst_times(IgPairingStep3 *obj,uint8_t* dst_times,size_t size)
{
  if(obj->dst_times == dst_times)
      return;
  if(obj->has_dst_times){
      free(obj->dst_times);
      obj->dst_times = NULL;
  }
  obj->dst_times = malloc(size);
  memcpy(obj->dst_times, dst_times, size);
  obj->dst_times_size = size;
  obj->has_dst_times = size > 0;
}
size_t ig_PairingStep3_get_password_size(IgPairingStep3 *obj)
{
  return obj->password_size;
}
void ig_PairingStep3_set_password_nocopy(IgPairingStep3 *obj,uint8_t* password,size_t size)
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
void ig_PairingStep3_set_password(IgPairingStep3 *obj,uint8_t* password,size_t size)
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
