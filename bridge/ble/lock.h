#ifndef _LOCK_TYPES_H_
#define _LOCK_TYPES_H_
#include <stdint.h>
#include <stdlib.h>
#include <bridge/bridge_main/list.h>

#define MAX_DEVICE_NAME (12+1)
#define MAX_DEVICE_ADDR (17+1)
#define DEFAULT_SCAN_TIMEOUT  (4)

typedef struct IGMLock{
  struct list_head list;
  char addr[MAX_DEVICE_ADDR];
  int addr_len;
  char name[MAX_DEVICE_NAME];
  int name_len;
  int paired;
  int admin_connection;
  int guest_connection;
  int has_admin_key;
  uint8_t *admin_key;
  size_t admin_key_len;
  int has_password;
  uint8_t *password;
  size_t password_size;
  int connectionID;
}igm_lock_t;

int lockInit(igm_lock_t *lock);
int lockCopy(igm_lock_t *to, igm_lock_t *from);
int lockSetName(igm_lock_t *lock, const char *name_, int name_len_);
int lockSetAddr(igm_lock_t *lock, const char *addr_, int addr_len_);
void printLock (igm_lock_t *lock);
int isLockPaired(igm_lock_t *lock);
int isLockAdmin(igm_lock_t *lock);
int setLockPaired(igm_lock_t *lock);
// isLockGuest()
#endif