#ifndef H_CARDDELETEDNOTIFICATION_
#define H_CARDDELETEDNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgCardDeletedNotification {
  bool has_card_uid;
  //optional
  uint8_t* card_uid;
  size_t card_uid_size;
} IgCardDeletedNotification;

#define IG_CardDeletedNotification_MSG_ID 102

void ig_CardDeletedNotification_init(IgCardDeletedNotification *obj);
IgSerializerError ig_CardDeletedNotification_encode(IgCardDeletedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_CardDeletedNotification_decode(uint8_t *buf,size_t buf_size,IgCardDeletedNotification *retval,size_t index);
uint32_t ig_CardDeletedNotification_get_max_payload_in_bytes(IgCardDeletedNotification *obj);
bool ig_CardDeletedNotification_is_valid(IgCardDeletedNotification *obj);
void ig_CardDeletedNotification_deinit(IgCardDeletedNotification *obj);

size_t ig_CardDeletedNotification_get_card_uid_size(IgCardDeletedNotification *obj);
void ig_CardDeletedNotification_set_card_uid_nocopy(IgCardDeletedNotification *obj,uint8_t* card_uid,size_t size);
void ig_CardDeletedNotification_set_card_uid(IgCardDeletedNotification *obj,uint8_t* card_uid,size_t size);




#ifdef __cplusplus
}
#endif

#endif
