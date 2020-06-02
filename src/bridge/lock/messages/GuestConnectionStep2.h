#ifndef H_GUESTCONNECTIONSTEP2_
#define H_GUESTCONNECTIONSTEP2_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestConnectionStep2 {
  bool has_nonce;
  //required
  uint8_t* nonce;
  size_t nonce_size;
} IgGuestConnectionStep2;

#define IG_GuestConnectionStep2_MSG_ID 10

void ig_GuestConnectionStep2_init(IgGuestConnectionStep2 *obj);
IgSerializerError ig_GuestConnectionStep2_encode(IgGuestConnectionStep2 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestConnectionStep2_decode(uint8_t *buf,size_t buf_size,IgGuestConnectionStep2 *retval,size_t index);
uint32_t ig_GuestConnectionStep2_get_max_payload_in_bytes(IgGuestConnectionStep2 *obj);
bool ig_GuestConnectionStep2_is_valid(IgGuestConnectionStep2 *obj);
void ig_GuestConnectionStep2_deinit(IgGuestConnectionStep2 *obj);

size_t ig_GuestConnectionStep2_get_nonce_size(IgGuestConnectionStep2 *obj);
void ig_GuestConnectionStep2_set_nonce_nocopy(IgGuestConnectionStep2 *obj,uint8_t* nonce,size_t size);
void ig_GuestConnectionStep2_set_nonce(IgGuestConnectionStep2 *obj,uint8_t* nonce,size_t size);




#ifdef __cplusplus
}
#endif

#endif
