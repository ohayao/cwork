#ifndef H_DAYLIGHTSAVINGRANGE_
#define H_DAYLIGHTSAVINGRANGE_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgDaylightSavingRange {
  bool has_year;
  //required
  uint16_t year;
  bool has_start_date;
  //required
  uint32_t start_date;
  bool has_end_date;
  //required
  uint32_t end_date;
  bool has_offset;
  //required
  int32_t offset;
} IgDaylightSavingRange;

#define IG_DaylightSavingRange_MSG_ID 205

void ig_DaylightSavingRange_init(IgDaylightSavingRange *obj);
IgSerializerError ig_DaylightSavingRange_encode(IgDaylightSavingRange *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_DaylightSavingRange_decode(uint8_t *buf,size_t buf_size,IgDaylightSavingRange *retval,size_t index);
uint32_t ig_DaylightSavingRange_get_max_payload_in_bytes(IgDaylightSavingRange *obj);
bool ig_DaylightSavingRange_is_valid(IgDaylightSavingRange *obj);
void ig_DaylightSavingRange_deinit(IgDaylightSavingRange *obj);

void ig_DaylightSavingRange_set_year(IgDaylightSavingRange *obj,uint16_t year);

void ig_DaylightSavingRange_set_start_date(IgDaylightSavingRange *obj,uint32_t start_date);

void ig_DaylightSavingRange_set_end_date(IgDaylightSavingRange *obj,uint32_t end_date);

void ig_DaylightSavingRange_set_offset(IgDaylightSavingRange *obj,int32_t offset);




#ifdef __cplusplus
}
#endif

#endif
