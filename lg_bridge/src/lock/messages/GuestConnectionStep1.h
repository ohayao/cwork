#ifndef H_GUESTCONNECTIONSTEP1_
#define H_GUESTCONNECTIONSTEP1_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestConnectionStep1 {
  bool has_token;
  //required
  uint8_t* token;
  size_t token_size;
} IgGuestConnectionStep1;

#define IG_GuestConnectionStep1_MSG_ID 9

void ig_GuestConnectionStep1_init(IgGuestConnectionStep1 *obj);
IgSerializerError ig_GuestConnectionStep1_encode(IgGuestConnectionStep1 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestConnectionStep1_decode(uint8_t *buf,size_t buf_size,IgGuestConnectionStep1 *retval,size_t index);
uint32_t ig_GuestConnectionStep1_get_max_payload_in_bytes(IgGuestConnectionStep1 *obj);
bool ig_GuestConnectionStep1_is_valid(IgGuestConnectionStep1 *obj);
void ig_GuestConnectionStep1_deinit(IgGuestConnectionStep1 *obj);

size_t ig_GuestConnectionStep1_get_token_size(IgGuestConnectionStep1 *obj);
void ig_GuestConnectionStep1_set_token_nocopy(IgGuestConnectionStep1 *obj,uint8_t* token,size_t size);
void ig_GuestConnectionStep1_set_token(IgGuestConnectionStep1 *obj,uint8_t* token,size_t size);




#ifdef __cplusplus
}
#endif

#endif
