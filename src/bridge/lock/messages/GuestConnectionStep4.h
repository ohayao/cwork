#ifndef H_GUESTCONNECTIONSTEP4_
#define H_GUESTCONNECTIONSTEP4_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestConnectionStep4 {
  bool has_success;
  //required
  bool success;
} IgGuestConnectionStep4;

#define IG_GuestConnectionStep4_MSG_ID 12

void ig_GuestConnectionStep4_init(IgGuestConnectionStep4 *obj);
IgSerializerError ig_GuestConnectionStep4_encode(IgGuestConnectionStep4 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestConnectionStep4_decode(uint8_t *buf,size_t buf_size,IgGuestConnectionStep4 *retval,size_t index);
uint32_t ig_GuestConnectionStep4_get_max_payload_in_bytes(IgGuestConnectionStep4 *obj);
bool ig_GuestConnectionStep4_is_valid(IgGuestConnectionStep4 *obj);
void ig_GuestConnectionStep4_deinit(IgGuestConnectionStep4 *obj);

void ig_GuestConnectionStep4_set_success(IgGuestConnectionStep4 *obj,bool success);




#ifdef __cplusplus
}
#endif

#endif
