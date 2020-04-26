#include <bridge/ble/lock.h>
#include <string.h>

int lockInit(igm_lock_t *lock)
{
  memset(lock, 0, sizeof(igm_lock_t));
  return 0;
}

int lockCopy(igm_lock_t *to, igm_lock_t *from)
{
  to->addr_len = from->addr_len;
  to->name_len = from->name_len;
}