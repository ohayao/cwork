#ifndef H_STATUSNOTIFICATION_
#define H_STATUSNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgStatusNotification {
  bool has_woken;
  //optional
  uint8_t woken;
  bool has_lock_open;
  //optional
  uint8_t lock_open;
  bool has_has_logs;
  //optional
  uint8_t has_logs;
  bool has_door_open;
  //optional
  uint8_t door_open;
} IgStatusNotification;

#define IG_StatusNotification_MSG_ID 100

void ig_StatusNotification_init(IgStatusNotification *obj);
IgSerializerError ig_StatusNotification_encode(IgStatusNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_StatusNotification_decode(uint8_t *buf,size_t buf_size,IgStatusNotification *retval,size_t index);
uint32_t ig_StatusNotification_get_max_payload_in_bytes(IgStatusNotification *obj);
bool ig_StatusNotification_is_valid(IgStatusNotification *obj);
void ig_StatusNotification_deinit(IgStatusNotification *obj);

void ig_StatusNotification_set_woken(IgStatusNotification *obj,uint8_t woken);

void ig_StatusNotification_set_lock_open(IgStatusNotification *obj,uint8_t lock_open);

void ig_StatusNotification_set_has_logs(IgStatusNotification *obj,uint8_t has_logs);

void ig_StatusNotification_set_door_open(IgStatusNotification *obj,uint8_t door_open);




#ifdef __cplusplus
}
#endif

#endif
