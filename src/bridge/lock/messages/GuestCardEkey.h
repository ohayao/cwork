#ifndef H_GUESTCARDEKEY_
#define H_GUESTCARDEKEY_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestCardEkey {
  bool has_encrypted_ekey;
  //required
  uint8_t* encrypted_ekey;
  size_t encrypted_ekey_size;
  bool has_mac_address;
  //required
  uint8_t* mac_address;
  size_t mac_address_size;
} IgGuestCardEkey;

#define IG_GuestCardEkey_MSG_ID 201

void ig_GuestCardEkey_init(IgGuestCardEkey *obj);
IgSerializerError ig_GuestCardEkey_encode(IgGuestCardEkey *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestCardEkey_decode(uint8_t *buf,size_t buf_size,IgGuestCardEkey *retval,size_t index);
uint32_t ig_GuestCardEkey_get_max_payload_in_bytes(IgGuestCardEkey *obj);
bool ig_GuestCardEkey_is_valid(IgGuestCardEkey *obj);
void ig_GuestCardEkey_deinit(IgGuestCardEkey *obj);

size_t ig_GuestCardEkey_get_encrypted_ekey_size(IgGuestCardEkey *obj);
void ig_GuestCardEkey_set_encrypted_ekey_nocopy(IgGuestCardEkey *obj,uint8_t* encrypted_ekey,size_t size);
void ig_GuestCardEkey_set_encrypted_ekey(IgGuestCardEkey *obj,uint8_t* encrypted_ekey,size_t size);

size_t ig_GuestCardEkey_get_mac_address_size(IgGuestCardEkey *obj);
void ig_GuestCardEkey_set_mac_address_nocopy(IgGuestCardEkey *obj,uint8_t* mac_address,size_t size);
void ig_GuestCardEkey_set_mac_address(IgGuestCardEkey *obj,uint8_t* mac_address,size_t size);




#ifdef __cplusplus
}
#endif

#endif
