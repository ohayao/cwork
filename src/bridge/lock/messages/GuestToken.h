#ifndef H_GUESTTOKEN_
#define H_GUESTTOKEN_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestToken {
  bool has_key_id;
  //required
  uint32_t key_id;
  bool has_start_date;
  //optional
  uint32_t start_date;
  bool has_end_date;
  //optional
  uint32_t end_date;
  bool has_aes_key;
  //optional
  uint8_t* aes_key;
  size_t aes_key_size;
  bool has_access_rights;
  //optional
  uint8_t* access_rights;
  size_t access_rights_size;
} IgGuestToken;

#define IG_GuestToken_MSG_ID 202

void ig_GuestToken_init(IgGuestToken *obj);
IgSerializerError ig_GuestToken_encode(IgGuestToken *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestToken_decode(uint8_t *buf,size_t buf_size,IgGuestToken *retval,size_t index);
uint32_t ig_GuestToken_get_max_payload_in_bytes(IgGuestToken *obj);
bool ig_GuestToken_is_valid(IgGuestToken *obj);
void ig_GuestToken_deinit(IgGuestToken *obj);

void ig_GuestToken_set_key_id(IgGuestToken *obj,uint32_t key_id);

void ig_GuestToken_set_start_date(IgGuestToken *obj,uint32_t start_date);

void ig_GuestToken_set_end_date(IgGuestToken *obj,uint32_t end_date);

size_t ig_GuestToken_get_aes_key_size(IgGuestToken *obj);
void ig_GuestToken_set_aes_key_nocopy(IgGuestToken *obj,uint8_t* aes_key,size_t size);
void ig_GuestToken_set_aes_key(IgGuestToken *obj,uint8_t* aes_key,size_t size);

size_t ig_GuestToken_get_access_rights_size(IgGuestToken *obj);
void ig_GuestToken_set_access_rights_nocopy(IgGuestToken *obj,uint8_t* access_rights,size_t size);
void ig_GuestToken_set_access_rights(IgGuestToken *obj,uint8_t* access_rights,size_t size);




#ifdef __cplusplus
}
#endif

#endif
