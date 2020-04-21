#ifndef H_ADMINCONNECTIONSTEP3_
#define H_ADMINCONNECTIONSTEP3_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "iglooconstants.h"

typedef struct IgAdminConnectionStep3 {
  bool has_success;
  //required
  bool success;
} IgAdminConnectionStep3;

#define IG_AdminConnectionStep3_MSG_ID 8

void ig_AdminConnectionStep3_init(IgAdminConnectionStep3 *obj);
IgSerializerError ig_AdminConnectionStep3_encode(IgAdminConnectionStep3 *obj,uint8_t *retval,uint32_t length,size_t *written_length);
IgSerializerError ig_AdminConnectionStep3_decode(uint8_t *buf,size_t buf_size,IgAdminConnectionStep3 *retval,size_t index);
uint32_t ig_AdminConnectionStep3_get_max_payload_in_bytes(IgAdminConnectionStep3 *obj);
bool ig_AdminConnectionStep3_is_valid(IgAdminConnectionStep3 *obj);
void ig_AdminConnectionStep3_deinit(IgAdminConnectionStep3 *obj);

void ig_AdminConnectionStep3_set_success(IgAdminConnectionStep3 *obj,bool success);




#ifdef __cplusplus
}
#endif

#endif
