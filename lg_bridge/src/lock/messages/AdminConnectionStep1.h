#ifndef H_ADMINCONNECTIONSTEP1_
#define H_ADMINCONNECTIONSTEP1_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminConnectionStep1 {
  bool has_nonce;
  //required
  uint8_t* nonce;
  size_t nonce_size;
} IgAdminConnectionStep1;

#define IG_AdminConnectionStep1_MSG_ID 6

void ig_AdminConnectionStep1_init(IgAdminConnectionStep1 *obj);
IgSerializerError ig_AdminConnectionStep1_encode(IgAdminConnectionStep1 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminConnectionStep1_decode(uint8_t *buf,size_t buf_size,IgAdminConnectionStep1 *retval,size_t index);
uint32_t ig_AdminConnectionStep1_get_max_payload_in_bytes(IgAdminConnectionStep1 *obj);
bool ig_AdminConnectionStep1_is_valid(IgAdminConnectionStep1 *obj);
void ig_AdminConnectionStep1_deinit(IgAdminConnectionStep1 *obj);

size_t ig_AdminConnectionStep1_get_nonce_size(IgAdminConnectionStep1 *obj);
void ig_AdminConnectionStep1_set_nonce_nocopy(IgAdminConnectionStep1 *obj,uint8_t* nonce,size_t size);
void ig_AdminConnectionStep1_set_nonce(IgAdminConnectionStep1 *obj,uint8_t* nonce,size_t size);




#ifdef __cplusplus
}
#endif

#endif
