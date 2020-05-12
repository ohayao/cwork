#ifndef H_GETLOGSRESPONSE_
#define H_GETLOGSRESPONSE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGetLogsResponse {
  bool has_result;
  //required
  uint8_t result;
  bool has_data;
  //optional
  uint8_t* data;
  size_t data_size;
  bool has_has_more;
  //optional
  uint8_t has_more;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgGetLogsResponse;

#define IG_GetLogsResponse_MSG_ID 42

void ig_GetLogsResponse_init(IgGetLogsResponse *obj);
IgSerializerError ig_GetLogsResponse_encode(IgGetLogsResponse *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GetLogsResponse_decode(uint8_t *buf,size_t buf_size,IgGetLogsResponse *retval,size_t index);
uint32_t ig_GetLogsResponse_get_max_payload_in_bytes(IgGetLogsResponse *obj);
bool ig_GetLogsResponse_is_valid(IgGetLogsResponse *obj);
void ig_GetLogsResponse_deinit(IgGetLogsResponse *obj);

void ig_GetLogsResponse_set_result(IgGetLogsResponse *obj,uint8_t result);

size_t ig_GetLogsResponse_get_data_size(IgGetLogsResponse *obj);
void ig_GetLogsResponse_set_data_nocopy(IgGetLogsResponse *obj,uint8_t* data,size_t size);
void ig_GetLogsResponse_set_data(IgGetLogsResponse *obj,uint8_t* data,size_t size);

void ig_GetLogsResponse_set_has_more(IgGetLogsResponse *obj,uint8_t has_more);

void ig_GetLogsResponse_set_operation_id(IgGetLogsResponse *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
