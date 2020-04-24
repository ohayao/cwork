#ifndef H_GUESTCONNECTIONSTEP3_
#define H_GUESTCONNECTIONSTEP3_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestConnectionStep3 {
  bool has_nonce;
  //required
  uint8_t* nonce;
  size_t nonce_size;
} IgGuestConnectionStep3;

#define IG_GuestConnectionStep3_MSG_ID 11

void ig_GuestConnectionStep3_init(IgGuestConnectionStep3 *obj);
IgSerializerError ig_GuestConnectionStep3_encode(IgGuestConnectionStep3 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestConnectionStep3_decode(uint8_t *buf,size_t buf_size,IgGuestConnectionStep3 *retval,size_t index);
uint32_t ig_GuestConnectionStep3_get_max_payload_in_bytes(IgGuestConnectionStep3 *obj);
bool ig_GuestConnectionStep3_is_valid(IgGuestConnectionStep3 *obj);
void ig_GuestConnectionStep3_deinit(IgGuestConnectionStep3 *obj);

size_t ig_GuestConnectionStep3_get_nonce_size(IgGuestConnectionStep3 *obj);
void ig_GuestConnectionStep3_set_nonce_nocopy(IgGuestConnectionStep3 *obj,uint8_t* nonce,size_t size);
void ig_GuestConnectionStep3_set_nonce(IgGuestConnectionStep3 *obj,uint8_t* nonce,size_t size);




#ifdef __cplusplus
}
#endif

#endif
