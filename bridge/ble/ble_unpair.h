#ifndef _BLE_UNPAIRED_H_
#define _BLE_UNPAIRED_H_
#include <bridge/ble/lock.h>


typedef struct BLEAdminUnpairParam {
  igm_lock_t *lock;
}ble_unpair_param_t;

int bleInitUnpairParam(ble_unpair_param_t *unpair_param);
int bleReleaseUnpairParam(ble_unpair_param_t *unpair_param);
int bleSetUnpairParam(ble_unpair_param_t *unpair_param, igm_lock_t *lock);


#endif 