#include "GetLogsResponse.h"
#include "../cbor/cbor.h"

void ig_GetLogsResponse_init(IgGetLogsResponse *obj)
{
  memset(obj, 0, sizeof(IgGetLogsResponse));
}
IgSerializerError ig_GetLogsResponse_encode(IgGetLogsResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GetLogsResponse_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_data) fields_size++;
  if(obj->has_has_more) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 42);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->result);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_data){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->data, obj->data_size);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_has_more){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->has_more);
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
IgSerializerError ig_GetLogsResponse_decode(uint8_t *buf,size_t buf_size,IgGetLogsResponse *retval,size_t index)
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
            if(val != 42) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLogsResponse_set_result(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
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
        
                ig_GetLogsResponse_set_data_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLogsResponse_set_has_more(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLogsResponse_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_GetLogsResponse_get_max_payload_in_bytes(IgGetLogsResponse *obj)
{
  return 32 + obj->data_size; //13 + 19;
}
bool ig_GetLogsResponse_is_valid(IgGetLogsResponse *obj)
{
  return obj->has_result;
}
void ig_GetLogsResponse_deinit(IgGetLogsResponse *obj)
{
  
  if(obj->has_data){
      free(obj->data);
      obj->data = NULL;
  }
  
  
}
void ig_GetLogsResponse_set_result(IgGetLogsResponse *obj,uint8_t result)
{
  obj->result = result;
  obj->has_result = true;
}
size_t ig_GetLogsResponse_get_data_size(IgGetLogsResponse *obj)
{
  return obj->data_size;
}
void ig_GetLogsResponse_set_data_nocopy(IgGetLogsResponse *obj,uint8_t* data,size_t size)
{
  if(obj->data == data)
      return;
  if(obj->has_data){
      free(obj->data);
      obj->data = NULL;
  }
  obj->data = data;
  obj->data_size = size;
  obj->has_data = size > 0;
}
void ig_GetLogsResponse_set_data(IgGetLogsResponse *obj,uint8_t* data,size_t size)
{
  if(obj->data == data)
      return;
  if(obj->has_data){
      free(obj->data);
      obj->data = NULL;
  }
  obj->data = malloc(size);
  memcpy(obj->data, data, size);
  obj->data_size = size;
  obj->has_data = size > 0;
}
void ig_GetLogsResponse_set_has_more(IgGetLogsResponse *obj,uint8_t has_more)
{
  obj->has_more = has_more;
  obj->has_has_more = true;
}
void ig_GetLogsResponse_set_operation_id(IgGetLogsResponse *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
