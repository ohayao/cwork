#include "SetVolumeResponse.h"
#include "external/cbor/cbor.h"

void ig_SetVolumeResponse_init(IgSetVolumeResponse *obj)
{
  memset(obj, 0, sizeof(IgSetVolumeResponse));
}
IgSerializerError ig_SetVolumeResponse_encode(IgSetVolumeResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_SetVolumeResponse_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_old_volume) fields_size++;
  if(obj->has_new_volume) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 34);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->result);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_old_volume){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->old_volume);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_new_volume){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->new_volume);
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
IgSerializerError ig_SetVolumeResponse_decode(uint8_t *buf,size_t buf_size,IgSetVolumeResponse *retval,size_t index)
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
            if(val != 34) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetVolumeResponse_set_result(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetVolumeResponse_set_old_volume(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetVolumeResponse_set_new_volume(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_SetVolumeResponse_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_SetVolumeResponse_get_max_payload_in_bytes(IgSetVolumeResponse *obj)
{
  return 34; //13 + 21;
}
bool ig_SetVolumeResponse_is_valid(IgSetVolumeResponse *obj)
{
  return obj->has_result;
}
void ig_SetVolumeResponse_deinit(IgSetVolumeResponse *obj)
{
  
  
}
void ig_SetVolumeResponse_set_result(IgSetVolumeResponse *obj,uint8_t result)
{
  obj->result = result;
  obj->has_result = true;
}
void ig_SetVolumeResponse_set_old_volume(IgSetVolumeResponse *obj,uint32_t old_volume)
{
  obj->old_volume = old_volume;
  obj->has_old_volume = true;
}
void ig_SetVolumeResponse_set_new_volume(IgSetVolumeResponse *obj,uint32_t new_volume)
{
  obj->new_volume = new_volume;
  obj->has_new_volume = true;
}
void ig_SetVolumeResponse_set_operation_id(IgSetVolumeResponse *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
