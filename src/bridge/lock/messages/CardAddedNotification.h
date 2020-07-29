#ifndef H_CARDADDEDNOTIFICATION_
#define H_CARDADDEDNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgCardAddedNotification {
  bool has_card_uid;
  //optional
  uint8_t* card_uid;
  size_t card_uid_size;
} IgCardAddedNotification;

#define IG_CardAddedNotification_MSG_ID 101

void ig_CardAddedNotification_init(IgCardAddedNotification *obj);
IgSerializerError ig_CardAddedNotification_encode(IgCardAddedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_CardAddedNotification_decode(uint8_t *buf,size_t buf_size,IgCardAddedNotification *retval,size_t index);
uint32_t ig_CardAddedNotification_get_max_payload_in_bytes(IgCardAddedNotification *obj);
bool ig_CardAddedNotification_is_valid(IgCardAddedNotification *obj);
void ig_CardAddedNotification_deinit(IgCardAddedNotification *obj);

size_t ig_CardAddedNotification_get_card_uid_size(IgCardAddedNotification *obj);
void ig_CardAddedNotification_set_card_uid_nocopy(IgCardAddedNotification *obj,uint8_t* card_uid,size_t size);
void ig_CardAddedNotification_set_card_uid(IgCardAddedNotification *obj,uint8_t* card_uid,size_t size);




#ifdef __cplusplus
}
#endif

#endif
