#include <bridge/ble/lock.h>
#include <string.h>
#include <bridge/bridge_main/log.h>

int lockInit(igm_lock_t *lock)
{
  memset(lock, 0, sizeof(igm_lock_t));
  return 0;
}
  // struct list_head list;
  // char addr[MAX_DEVICE_ADDR];
  // int addr_len;
  // char name[MAX_DEVICE_NAME];
  // int name_len;
  // int paired;
  // int admin_connection;
  // int guest_connection;
  // int has_admin_key;
  // uint8_t *admin_key;
  // size_t admin_key_len;
  // int has_password;
  // uint8_t *password;
  // size_t password_size;
  // 
  // int connectionID;
int lockCopy(igm_lock_t *to, igm_lock_t *from)
{
  // 不要copy list header
  lockInit(to);
  to->addr_len = from->addr_len;
  to->name_len = from->name_len;
  memcpy(to->addr, from->addr, to->addr_len);
  memcpy(to->name, from->name, to->name_len);
  to->paired = from->paired;
  to->admin_connection = from->admin_connection;
  to->guest_connection = from->guest_connection;
  to->connectionID = from->connectionID;
  if (from->has_admin_key && from->admin_key && from->admin_key_len)
  {
    to->has_admin_key = from->has_admin_key;
    to->admin_key_len = from->admin_key_len;
    to->admin_key = calloc(from->admin_key_len, 1);
    memcpy(to->admin_key, from->admin_key, from->admin_key_len);
  }

  if (from->has_password && from->password && from->password_size)
  {
    to->has_password = from->has_password;
    to->password_size = from->password_size;
    to->password = calloc(from->password_size, 1);
    memcpy(to->password, from->password, from->password_size);
  }
  // if ()
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


// -------------------------------------
  // int has_admin_key;
  // uint8_t *admin_key;
  // size_t admin_key_len;
  // int has_password;
  // uint8_t *password;
  // size_t password_size;
  // int connectionID;
void printLock (igm_lock_t *lock)
{
  serverLog(LL_NOTICE, "--------------print Lock begin----------------");
  printf("addr_len %d, addr: %s\n", lock->addr_len, lock->addr);
  printf("name_len %d, name: %s\n", lock->name_len, lock->name);
  printf("paired %d\n", lock->paired);
  printf("admin_connection %d\n", lock->admin_connection);
  printf("guest_connection %d\n", lock->guest_connection);
  printf("connectionID %d\n", lock->connectionID);
  if (lock->has_password)
  {
    printf("password size %d password: ", lock->password_size);
    for (int j = 0; j < lock->password_size; j++)
    {
      printf("%x", lock->password[j]);
    }
    printf("\n");
  }
  if (lock->has_admin_key)
  {
    printf("admin_key size %d admin_key: ", lock->admin_key_len);
    for (int j = 0; j < lock->admin_key_len; j++)
    {
      printf("%x", lock->admin_key[j]);
    }
    printf("\n");
  }
  serverLog(LL_NOTICE, "--------------print Lock end------------------");
}

int isLockPaired(igm_lock_t *lock)
{
  return lock->paired;
}

int isLockAdmin(igm_lock_t *lock)
{
  return lock->admin_connection;
}

int setLockPaired(igm_lock_t *lock)
{
  lock->paired = 1;
}

int setLockConnectionID(igm_lock_t *lock, int ID)
{
  lock->connectionID = ID;
}

int releaseLockAminKey(igm_lock_t *lock)
{
  if (lock && lock->has_admin_key && lock->admin_key && lock->admin_key_len)
  {
    lock->has_admin_key = 0;
    lock->admin_key_len = 0;
    free(lock->admin_key);
    lock->admin_key = NULL;
  }
  return 0;
}

int releaseLockPassword(igm_lock_t *lock)
{
  if (lock && lock->has_password && lock->password_size && lock->password)
  {
    lock->has_password = 0;
    lock->password_size = 0;
    free(lock->password);
    lock->password = NULL;
  }
  return 0;
}

int setLockAdminKey(igm_lock_t *lock, 
  uint8_t *admin_key, int admin_key_len)
{
  releaseLockAminKey(lock);
  lock->has_admin_key = 1;
  lock->admin_key_len = admin_key_len;
  lock->admin_key = calloc(admin_key_len, 1);
  memcpy(lock->admin_key, admin_key, admin_key_len);
  return 0;
}

int setLockPassword(igm_lock_t *lock, 
  uint8_t *password, int password_size)
{
  releaseLockPassword(lock);
  lock->has_password = 1;
  lock->password_size = password_size;
  lock->password = calloc(password_size, 1);
  memcpy(lock->password, password, password_size);
  return 0;
}

int getLockAdminKey(igm_lock_t *lock, 
  uint8_t *admin_key, int *p_admin_key_len)
{
  if (lock->has_admin_key)
  {
    *p_admin_key_len = lock->admin_key_len;
    memcpy(admin_key, lock->admin_key, lock->admin_key_len);
  }
  return 0;
}

int getLockPassword(igm_lock_t *lock, 
  uint8_t *password, int *p_password_size)
{
  if (lock->has_password)
  {
    *p_password_size = lock->password_size;
    memcpy(password, lock->password, lock->password_size);
  }
  return 0;
}

int setLockAdminConnection(igm_lock_t *lock, int admin_connection)
{
  lock->admin_connection = admin_connection;
  return 0;
}

