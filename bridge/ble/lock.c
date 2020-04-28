#include <bridge/ble/lock.h>
#include <string.h>
#include <bridge/bridge_main/log.h>

int lockInit(igm_lock_t *lock)
{
  memset(lock, 0, sizeof(igm_lock_t));
  return 0;
}

int lockCopy(igm_lock_t *to, igm_lock_t *from)
{
  lockInit(to);
  to->addr_len = from->addr_len;
  to->name_len = from->name_len;
  memcpy(to->addr, from->addr, to->addr_len);
  memcpy(to->name, from->name, to->name_len);
  return 0;
}

int lockSetName(igm_lock_t *lock, const char *name_, int name_len_)
{
  memset(lock->name, 0, MAX_DEVICE_NAME);
  lock->name_len = name_len_;
  memcpy(lock->name, name_, lock->name_len);
  return 0;
}

int lockSetAddr(igm_lock_t *lock, const char *addr_, int addr_len_)
{
  memset(lock->addr, 0, MAX_DEVICE_ADDR);
  lock->addr_len = addr_len_;
  memcpy(lock->addr, addr_, lock->addr_len);
  return 0;
}



