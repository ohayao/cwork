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

int lockInit(igm_lock_t *lock);
int lockCopy(igm_lock_t *to, igm_lock_t *from);
int lockSetName(igm_lock_t *lock, const char *name_, int name_len_);
int lockSetAddr(igm_lock_t *lock, const char *addr_, int addr_len_);

#endif