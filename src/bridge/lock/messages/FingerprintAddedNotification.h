#ifndef H_FINGERPRINTADDEDNOTIFICATION_
#define H_FINGERPRINTADDEDNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgFingerprintAddedNotification {
  bool has_uid;
  //optional
  uint16_t uid;
  bool has_result;
  //optional
  uint8_t result;
} IgFingerprintAddedNotification;

#define IG_FingerprintAddedNotification_MSG_ID 103

void ig_FingerprintAddedNotification_init(IgFingerprintAddedNotification *obj);
IgSerializerError ig_FingerprintAddedNotification_encode(IgFingerprintAddedNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_FingerprintAddedNotification_decode(uint8_t *buf,size_t buf_size,IgFingerprintAddedNotification *retval,size_t index);
uint32_t ig_FingerprintAddedNotification_get_max_payload_in_bytes(IgFingerprintAddedNotification *obj);
bool ig_FingerprintAddedNotification_is_valid(IgFingerprintAddedNotification *obj);
void ig_FingerprintAddedNotification_deinit(IgFingerprintAddedNotification *obj);

void ig_FingerprintAddedNotification_set_uid(IgFingerprintAddedNotification *obj,uint16_t uid);

void ig_FingerprintAddedNotification_set_result(IgFingerprintAddedNotification *obj,uint8_t result);




#ifdef __cplusplus
}
#endif

#endif
