#ifndef _LOCK_TYPES_H_
#define _LOCK_TYPES_H_

#define MAX_DEVICE_NAME (12+1)
#define MAX_DEVICE_ADDR (17+1)
#define DEFAULT_SCAN_TIMEOUT  (4)
typedef struct IGMLock{
  char addr[MAX_DEVICE_ADDR];
  int addr_len;
  char name[MAX_DEVICE_NAME];
  int name_len;
}igm_lock_t;

#endif