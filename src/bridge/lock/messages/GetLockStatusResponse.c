#include "GetLockStatusResponse.h"
#include "external/cbor/cbor.h"

void ig_GetLockStatusResponse_init(IgGetLockStatusResponse *obj)
{
  memset(obj, 0, sizeof(IgGetLockStatusResponse));
}
IgSerializerError ig_GetLockStatusResponse_encode(IgGetLockStatusResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_GetLockStatusResponse_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 1;
  if(obj->has_lock_open) fields_size++;
  if(obj->has_cell_network_status) fields_size++;
  if(obj->has_door_open) fields_size++;
  if(obj->has_operation_id) fields_size++;
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 32);
  if(err) return (IgSerializerError) err;
  
  
      err = cbor_encode_uint(&map, 11);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->result);
      if(err) return (IgSerializerError) err;
  
  
  
  if(obj->has_lock_open){
      err = cbor_encode_uint(&map, 12);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_boolean(&map, obj->lock_open);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_cell_network_status){
      err = cbor_encode_uint(&map, 13);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->cell_network_status);
      if(err) return (IgSerializerError) err;
  }
  
  
  if(obj->has_door_open){
      err = cbor_encode_uint(&map, 14);
      if(err) return (IgSerializerError) err;
      err = cbor_encode_uint(&map, obj->door_open);
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
IgSerializerError ig_GetLockStatusResponse_decode(uint8_t *buf,size_t buf_size,IgGetLockStatusResponse *retval,size_t index)
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
            if(val != 32) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLockStatusResponse_set_result(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborBooleanType){
                bool val;
                cbor_value_get_boolean(&content, &val);
                ig_GetLockStatusResponse_set_lock_open(retval, val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLockStatusResponse_set_cell_network_status(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 14:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLockStatusResponse_set_door_open(retval, (uint8_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 100:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_GetLockStatusResponse_set_operation_id(retval, (uint32_t) val);
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
uint32_t ig_GetLockStatusResponse_get_max_payload_in_bytes(IgGetLockStatusResponse *obj)
{
  return 30; //13 + 17;
}
bool ig_GetLockStatusResponse_is_valid(IgGetLockStatusResponse *obj)
{
  return obj->has_result;
}
void ig_GetLockStatusResponse_deinit(IgGetLockStatusResponse *obj)
{
  
  
  
}
void ig_GetLockStatusResponse_set_result(IgGetLockStatusResponse *obj,uint8_t result)
{
  obj->result = result;
  obj->has_result = true;
}
void ig_GetLockStatusResponse_set_lock_open(IgGetLockStatusResponse *obj,bool lock_open)
{
  obj->lock_open = lock_open;
  obj->has_lock_open = true;
}
void ig_GetLockStatusResponse_set_cell_network_status(IgGetLockStatusResponse *obj,uint8_t cell_network_status)
{
  obj->cell_network_status = cell_network_status;
  obj->has_cell_network_status = true;
}
void ig_GetLockStatusResponse_set_door_open(IgGetLockStatusResponse *obj,uint8_t door_open)
{
  obj->door_open = door_open;
  obj->has_door_open = true;
}
void ig_GetLockStatusResponse_set_operation_id(IgGetLockStatusResponse *obj,uint32_t operation_id)
{
  obj->operation_id = operation_id;
  obj->has_operation_id = true;
}
