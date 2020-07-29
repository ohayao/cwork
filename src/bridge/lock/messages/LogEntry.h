#ifndef H_LOGENTRY_
#define H_LOGENTRY_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgLogEntry {
  bool has_entry_date;
  //required
  uint32_t entry_date;
  bool has_log_type;
  //required
  uint32_t log_type;
  bool has_key_id;
  //optional
  uint32_t key_id;
  bool has_pin;
  //optional
  uint8_t* pin;
  size_t pin_size;
  bool has_new_pin;
  //optional
  uint8_t* new_pin;
  size_t new_pin_size;
  bool has_time1;
  //optional
  uint32_t time1;
  bool has_time2;
  //optional
  uint32_t time2;
  bool has_pin_type;
  //optional
  uint32_t pin_type;
  bool has_value1;
  //optional
  uint32_t value1;
  bool has_value2;
  //optional
  uint32_t value2;
  bool has_value3;
  //optional
  uint32_t value3;
  bool has_value4;
  //optional
  uint32_t value4;
  bool has_value5;
  //optional
  uint8_t* value5;
  size_t value5_size;
  bool has_operation_id;
  //optional
  uint32_t operation_id;
} IgLogEntry;

#define IG_LogEntry_MSG_ID 203

void ig_LogEntry_init(IgLogEntry *obj);
IgSerializerError ig_LogEntry_encode(IgLogEntry *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_LogEntry_decode(uint8_t *buf,size_t buf_size,IgLogEntry *retval,size_t index);
uint32_t ig_LogEntry_get_max_payload_in_bytes(IgLogEntry *obj);
bool ig_LogEntry_is_valid(IgLogEntry *obj);
void ig_LogEntry_deinit(IgLogEntry *obj);

void ig_LogEntry_set_entry_date(IgLogEntry *obj,uint32_t entry_date);

void ig_LogEntry_set_log_type(IgLogEntry *obj,uint32_t log_type);

void ig_LogEntry_set_key_id(IgLogEntry *obj,uint32_t key_id);

size_t ig_LogEntry_get_pin_size(IgLogEntry *obj);
void ig_LogEntry_set_pin_nocopy(IgLogEntry *obj,uint8_t* pin,size_t size);
void ig_LogEntry_set_pin(IgLogEntry *obj,uint8_t* pin,size_t size);

size_t ig_LogEntry_get_new_pin_size(IgLogEntry *obj);
void ig_LogEntry_set_new_pin_nocopy(IgLogEntry *obj,uint8_t* new_pin,size_t size);
void ig_LogEntry_set_new_pin(IgLogEntry *obj,uint8_t* new_pin,size_t size);

void ig_LogEntry_set_time1(IgLogEntry *obj,uint32_t time1);

void ig_LogEntry_set_time2(IgLogEntry *obj,uint32_t time2);

void ig_LogEntry_set_pin_type(IgLogEntry *obj,uint32_t pin_type);

void ig_LogEntry_set_value1(IgLogEntry *obj,uint32_t value1);

void ig_LogEntry_set_value2(IgLogEntry *obj,uint32_t value2);

void ig_LogEntry_set_value3(IgLogEntry *obj,uint32_t value3);

void ig_LogEntry_set_value4(IgLogEntry *obj,uint32_t value4);

size_t ig_LogEntry_get_value5_size(IgLogEntry *obj);
void ig_LogEntry_set_value5_nocopy(IgLogEntry *obj,uint8_t* value5,size_t size);
void ig_LogEntry_set_value5(IgLogEntry *obj,uint8_t* value5,size_t size);

void ig_LogEntry_set_operation_id(IgLogEntry *obj,uint32_t operation_id);




#ifdef __cplusplus
}
#endif

#endif
