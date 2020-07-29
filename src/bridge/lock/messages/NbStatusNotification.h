#ifndef H_NBSTATUSNOTIFICATION_
#define H_NBSTATUSNOTIFICATION_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgNbStatusNotification {
  bool has_nb_network_status;
  //optional
  uint8_t nb_network_status;
} IgNbStatusNotification;

#define IG_NbStatusNotification_MSG_ID 105

void ig_NbStatusNotification_init(IgNbStatusNotification *obj);
IgSerializerError ig_NbStatusNotification_encode(IgNbStatusNotification *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_NbStatusNotification_decode(uint8_t *buf,size_t buf_size,IgNbStatusNotification *retval,size_t index);
uint32_t ig_NbStatusNotification_get_max_payload_in_bytes(IgNbStatusNotification *obj);
bool ig_NbStatusNotification_is_valid(IgNbStatusNotification *obj);
void ig_NbStatusNotification_deinit(IgNbStatusNotification *obj);

void ig_NbStatusNotification_set_nb_network_status(IgNbStatusNotification *obj,uint8_t nb_network_status);




#ifdef __cplusplus
}
#endif

#endif
