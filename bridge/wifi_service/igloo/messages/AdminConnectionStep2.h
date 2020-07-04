#ifndef H_ADMINCONNECTIONSTEP2_
#define H_ADMINCONNECTIONSTEP2_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminConnectionStep2 {
  bool has_nonce;
  //required
  uint8_t* nonce;
  size_t nonce_size;
} IgAdminConnectionStep2;

#define IG_AdminConnectionStep2_MSG_ID 7

void ig_AdminConnectionStep2_init(IgAdminConnectionStep2 *obj);
IgSerializerError ig_AdminConnectionStep2_encode(IgAdminConnectionStep2 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminConnectionStep2_decode(uint8_t *buf,size_t buf_size,IgAdminConnectionStep2 *retval,size_t index);
uint32_t ig_AdminConnectionStep2_get_max_payload_in_bytes(IgAdminConnectionStep2 *obj);
bool ig_AdminConnectionStep2_is_valid(IgAdminConnectionStep2 *obj);
void ig_AdminConnectionStep2_deinit(IgAdminConnectionStep2 *obj);

size_t ig_AdminConnectionStep2_get_nonce_size(IgAdminConnectionStep2 *obj);
void ig_AdminConnectionStep2_set_nonce_nocopy(IgAdminConnectionStep2 *obj,uint8_t* nonce,size_t size);
void ig_AdminConnectionStep2_set_nonce(IgAdminConnectionStep2 *obj,uint8_t* nonce,size_t size);




#ifdef __cplusplus
}
#endif

#endif
