#include "external/cbor/cbor.h"
#include "serializer.h"

IgSerializerError ig_get_payload_length(uint8_t *buf,size_t buf_size,size_t *retval)
{
  
  CborParser parser;
  CborValue it;
  CborError err;
  size_t size;
  
  err = cbor_parser_init(buf, buf_size, 0, &parser, &it);
  if(err) return err;
  
  CborType type = cbor_value_get_type(&it);
  if(type == CborMapType){
      size = 1;
  }else if(type == CborArrayType){
    err = cbor_value_get_array_length(&it, &size);
    if(err) return err;
  }else{
    return IgSerializerErrorInvalidPayloadType;
  }
  
  *retval = size;
  return IgSerializerNoError;
  
}
IgSerializerError ig_decode_msg_ids(uint8_t *buf,size_t buf_size,ig_object_id_t *retval)
{
  CborParser parser;
  CborValue it;
  CborValue content;
  CborError err;
  size_t size;
  int64_t value;
  
  err = cbor_parser_init(buf, buf_size, 0, &parser, &it);
  if(err) return (IgSerializerError) err;
  
  CborType type = cbor_value_get_type(&it);
  if(type == CborMapType){
  
    err = cbor_value_enter_container(&it, &content);
    if(err) return (IgSerializerError) err;
  
    if(cbor_value_get_type(&content) != CborIntegerType) return IgSerializerErrorInvalidTypeMsgId;
    err = cbor_value_get_int64(&content, &value);
    if(err) return (IgSerializerError) err;
    if(value != 0) return IgSerializerErrorInvalidMsgIdValue;
  
    err = cbor_value_advance_fixed(&content);
    if(err) return (IgSerializerError) err;
  
    if(cbor_value_get_type(&content) != CborIntegerType) return IgSerializerErrorInvalidTypeMsgId;
    err = cbor_value_get_int64(&content, &value);
    if(err) return (IgSerializerError) err;
    if(value <= 0) return IgSerializerErrorInvalidMsgIdValue;
    retval[0] = value;
  
  }else if(type == CborArrayType){
    CborValue payload_content;
  
    err = cbor_value_get_array_length(&it, &size);
    if(err) return (IgSerializerError) err;
  
    err = cbor_value_enter_container(&it, &content);
    if(err) return (IgSerializerError) err;
  
    //loop starts here
    for(int i=0;i<size;i++){
      //check if type == A0/map
      if(cbor_value_get_type(&content) != CborMapType) return IgSerializerErrorInvalidTypeMsgId;
  
      err = cbor_value_enter_container(&content, &payload_content);
      if(err) return (IgSerializerError) err;
  
      if(cbor_value_get_type(&payload_content) != CborIntegerType) return IgSerializerErrorInvalidTypeMsgId;
      err = cbor_value_get_int64(&payload_content, &value);
      if(err) return (IgSerializerError) err;
      if(value != 0) return IgSerializerErrorInvalidMsgIdValue;
  
      err = cbor_value_advance_fixed(&payload_content);
      if(err) return (IgSerializerError) err;
  
      if(cbor_value_get_type(&payload_content) != CborIntegerType) return IgSerializerErrorInvalidTypeMsgId;
      err = cbor_value_get_int64(&payload_content, &value);
      if(err) return (IgSerializerError) err;
      if(value <= 0) return IgSerializerErrorInvalidMsgIdValue;
      retval[i] = value;
  
      err = cbor_value_advance(&content);
      if(err) return (IgSerializerError) err;
    }
  
  }else{
    return IgSerializerErrorInvalidPayloadType;
  }
  
  return IgSerializerNoError;
}
IgSerializerError ig_encode_calculate_length(size_t *objs_lens,size_t array_size,size_t *retval)
{
  size_t sum;
  if(array_size < 1 || array_size > 23){
    return IgSerializerErrorInvalidPayload;
  }else if(array_size == 1){
    sum = objs_lens[0];
  }else{
    sum = 1;
    for(int i=0;i<array_size;i++){
      sum += objs_lens[i];
    }
  }
  *retval = sum;
  return IgSerializerNoError;
}
IgSerializerError ig_encode_objects(uint8_t **objs_in,size_t *objs_lens,size_t array_size,uint8_t *retval,size_t total_size)
{
  if(array_size < 1){
    return IgSerializerErrorInvalidPayload;
  }else if(array_size == 1){
    for(int i=0;i<objs_lens[0];i++){
      retval[i] = objs_in[0][i];
    }
  }else{
    size_t calculated_total_size;
    IgSerializerError err = ig_encode_calculate_length(objs_lens, array_size, &calculated_total_size);
    if(err) return (IgSerializerError) err;
    if(total_size != calculated_total_size) return IgSerializerErrorInvalidPayload;
  
    retval[0] = 0x80 + array_size;
    size_t current_index = 1;
    for(int i=0;i<array_size;i++){
      for(int j=0;j<objs_lens[i];j++){
        retval[current_index] = objs_in[i][j];
        current_index++;
      }
    }
  }
  return IgSerializerNoError;
}
