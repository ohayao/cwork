#include "DaylightSavingRange.h"
#include "external/cbor/cbor.h"

void ig_DaylightSavingRange_init(IgDaylightSavingRange *obj)
{
  memset(obj, 0, sizeof(IgDaylightSavingRange));
}
IgSerializerError ig_DaylightSavingRange_encode(IgDaylightSavingRange *obj,uint8_t *retval,uint32_t length,size_t *written_length)
{
  
  if(!ig_DaylightSavingRange_is_valid(obj)) return IgSerializerErrorInvalidPayload;
  
  CborEncoder encoder;
  CborEncoder map;
  CborError err;
  
  //msg_id property + required properties
  size_t fields_size = 1 + 4;
  
  
  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return (IgSerializerError) err;
  
  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, 205);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encode_uint(&map, 11);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, obj->year);
  if(err) return (IgSerializerError) err;
  
  
  
  
  err = cbor_encode_uint(&map, 12);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, obj->start_date);
  if(err) return (IgSerializerError) err;
  
  
  
  
  err = cbor_encode_uint(&map, 13);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_uint(&map, obj->end_date);
  if(err) return (IgSerializerError) err;
  
  
  
  
  err = cbor_encode_uint(&map, 14);
  if(err) return (IgSerializerError) err;
  err = cbor_encode_int(&map, obj->offset);
  if(err) return (IgSerializerError) err;
  
  
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return (IgSerializerError) err;
  
  *written_length = cbor_encoder_get_buffer_size(&map, retval);
  
  return IgSerializerNoError;
}
IgSerializerError ig_DaylightSavingRange_decode(uint8_t *buf,size_t buf_size,IgDaylightSavingRange *retval,size_t index)
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
            if(val != 205) return IgSerializerErrorInvalidMsgIdValue;
            err = cbor_value_advance_fixed(&content);
            break;
          }
  
        case 11:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_DaylightSavingRange_set_year(retval, (uint16_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 12:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_DaylightSavingRange_set_start_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 13:
            if(value_type == CborIntegerType){
                uint64_t val;
                cbor_value_get_uint64(&content, &val);
                ig_DaylightSavingRange_set_end_date(retval, (uint32_t) val);
                err = cbor_value_advance_fixed(&content);
                if(err) return (IgSerializerError) err;
                break;
            }
        
        
        case 14:
            if(value_type == CborIntegerType){
                int64_t val;
                cbor_value_get_int64(&content, &val);
                ig_DaylightSavingRange_set_offset(retval, (int32_t) val);
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
uint32_t ig_DaylightSavingRange_get_max_payload_in_bytes(IgDaylightSavingRange *obj)
{
  return 35; //13 + 22;
}
bool ig_DaylightSavingRange_is_valid(IgDaylightSavingRange *obj)
{
  return obj->has_year && obj->has_start_date && obj->has_end_date && obj->has_offset;
}
void ig_DaylightSavingRange_deinit(IgDaylightSavingRange *obj)
{
  
  
}
void ig_DaylightSavingRange_set_year(IgDaylightSavingRange *obj,uint16_t year)
{
  obj->year = year;
  obj->has_year = true;
}
void ig_DaylightSavingRange_set_start_date(IgDaylightSavingRange *obj,uint32_t start_date)
{
  obj->start_date = start_date;
  obj->has_start_date = true;
}
void ig_DaylightSavingRange_set_end_date(IgDaylightSavingRange *obj,uint32_t end_date)
{
  obj->end_date = end_date;
  obj->has_end_date = true;
}
void ig_DaylightSavingRange_set_offset(IgDaylightSavingRange *obj,int32_t offset)
{
  obj->offset = offset;
  obj->has_offset = true;
}
