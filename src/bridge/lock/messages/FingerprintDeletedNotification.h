#ifndef H_FINGERPRINTDELETEDNOTIFICATION_
#define H_FINGERPRINTDELETEDNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgFingerprintDeletedNotification {
  bool has_uid;
  //optional
  uint16_t uid;
  bool has_result;
  //optional
  uint8_t result;
} IgFingerprintDeletedNotification;

#define IG_FingerprintDeletedNotification_MSG_ID 104

void ig_FingerprintDeletedNotification_init(IgFingerprintDeletedNotification *obj);
IgSerializerError ig_FingerprintDeletedNotification_encode(IgFingerprintDeletedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_FingerprintDeletedNotification_decode(uint8_t *buf,size_t buf_size,IgFingerprintDeletedNotification *retval,size_t index);
uint32_t ig_FingerprintDeletedNotification_get_max_payload_in_bytes(IgFingerprintDeletedNotification *obj);
bool ig_FingerprintDeletedNotification_is_valid(IgFingerprintDeletedNotification *obj);
void ig_FingerprintDeletedNotification_deinit(IgFingerprintDeletedNotification *obj);

void ig_FingerprintDeletedNotification_set_uid(IgFingerprintDeletedNotification *obj,uint16_t uid);

void ig_FingerprintDeletedNotification_set_result(IgFingerprintDeletedNotification *obj,uint8_t result);




#ifdef __cplusplus
}
#endif

#endif
