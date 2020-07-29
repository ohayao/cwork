#include "GetFirmwareVersionResponse.h"
#include "external/cbor/cbor.h"

void ig_GetFirmwareVersionResponse_init(IgGetFirmwareVersionResponse *obj)
{
  memset(obj, 0, sizeof(IgGetFirmwareVersionResponse));
}
IgSerializerError ig_GetFirmwareVersionResponse_encode(IgGetFirmwareVersionResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GetFirmwareVersionResponse_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 0;
  if(obj->has_result) fields_size++;
  if(obj->has_version) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 88);
  if(err) return (IgSerializerError) err;
  
  if(obj->has_result){
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->result);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_version){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_byte_string(&map, obj->version, obj->version_size);
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
IgSerializerError ig_GetFirmwareVersionResponse_decode(uint8_t *buf,size_t buf_size,IgGetFirmwareVersionResponse *retval,size_t index)
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
            if(val != 88) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetFirmwareVersionResponse_set_result(retval, (uint8_t) val);
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
        
                ig_GetFirmwareVersionResponse_set_version_nocopy(retval, data_arr, size);
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetFirmwareVersionResponse_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_GetFirmwareVersionResponse_get_max_payload_in_bytes(IgGetFirmwareVersionResponse *obj)
{
  return 29 + obj->version_size; //13 + 16;
}
bool ig_GetFirmwareVersionResponse_is_valid(IgGetFirmwareVersionResponse *obj)
{
  return true;
}
void ig_GetFirmwareVersionResponse_deinit(IgGetFirmwareVersionResponse *obj)
{
  
  if(obj->has_version){
      free(obj->version);
      obj->version = NULL;
  }
  
}
void ig_GetFirmwareVersionResponse_set_result(IgGetFirmwareVersionResponse *obj,uint8_t result)
{
  obj->result = result;
  obj->has_result = true;
}
size_t ig_GetFirmwareVersionResponse_get_version_size(IgGetFirmwareVersionResponse *obj)
{
  return obj->version_size;
}
void ig_GetFirmwareVersionResponse_set_version_nocopy(IgGetFirmwareVersionResponse *obj,uint8_t* version,size_t size)
{
  if(obj->version == version)
      return;
  if(obj->has_version){
      free(obj->version);
      obj->version = NULL;
  }
  obj->version = version;
  obj->version_size = size;
  obj->has_version = size > 0;
}
void ig_GetFirmwareVersionResponse_set_version(IgGetFirmwareVersionResponse *obj,uint8_t* version,size_t size)
{
  if(obj->version == version)
      return;
  if(obj->has_version){
      free(obj->version);
      obj->version = NULL;
  }
  obj->version = malloc(size);
  memcpy(obj->version, version, size);
  obj->version_size = size;
  obj->has_version = size > 0;
}
void ig_GetFirmwareVersionResponse_set_operation_id(IgGetFirmwareVersionResponse *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
