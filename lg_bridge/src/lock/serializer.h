#ifndef H_SERIALIZER_
#define H_SERIALIZER_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "messages/iglooconstants.h"

typedef int32_t ig_object_id_t;

  IgSerializerError ig_get_payload_length(uint8_t *buf,size_t buf_size,size_t *retval);
  IgSerializerError ig_decode_msg_ids(uint8_t *buf,size_t buf_size,ig_object_id_t *retval);
  IgSerializerError ig_encode_calculate_length(size_t *objs_lens,size_t array_size,size_t *retval);
  IgSerializerError ig_encode_objects(uint8_t **objs_in,size_t *objs_lens,size_t array_size,uint8_t *retval,size_t total_size);




  #ifdef __cplusplus
  }
  #endif

  #endif
