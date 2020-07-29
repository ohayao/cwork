#ifndef H_GUESTKEYBLACKLIST_
#define H_GUESTKEYBLACKLIST_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgGuestKeyBlacklist {
  bool has_key_id;
  //required
  uint32_t key_id;
  bool has_start_date;
  //optional
  uint32_t start_date;
  bool has_end_date;
  //optional
  uint32_t end_date;
} IgGuestKeyBlacklist;

#define IG_GuestKeyBlacklist_MSG_ID 204

void ig_GuestKeyBlacklist_init(IgGuestKeyBlacklist *obj);
IgSerializerError ig_GuestKeyBlacklist_encode(IgGuestKeyBlacklist *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_GuestKeyBlacklist_decode(uint8_t *buf,size_t buf_size,IgGuestKeyBlacklist *retval,size_t index);
uint32_t ig_GuestKeyBlacklist_get_max_payload_in_bytes(IgGuestKeyBlacklist *obj);
bool ig_GuestKeyBlacklist_is_valid(IgGuestKeyBlacklist *obj);
void ig_GuestKeyBlacklist_deinit(IgGuestKeyBlacklist *obj);

void ig_GuestKeyBlacklist_set_key_id(IgGuestKeyBlacklist *obj,uint32_t key_id);

void ig_GuestKeyBlacklist_set_start_date(IgGuestKeyBlacklist *obj,uint32_t start_date);

void ig_GuestKeyBlacklist_set_end_date(IgGuestKeyBlacklist *obj,uint32_t end_date);




#ifdef __cplusplus
}
#endif

#endif
