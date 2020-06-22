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
	int has_lock_cmd;
	uint8_t *lock_cmd;
	size_t lock_cmd_size;
	int connectionID;
}igm_lock_t;

int getLock(igm_lock_t **pp_lock);
int initLock(igm_lock_t *lock);
int copyLock(igm_lock_t *to, igm_lock_t *from);
// 不会泄漏
int setLockName(igm_lock_t *lock, const char *name_, int name_len_);
int setLockAddr(igm_lock_t *lock, const char *addr_, int addr_len_);


void printLock(igm_lock_t *lock);

int isLockPaired(igm_lock_t *lock);
int isLockAdmin(igm_lock_t *lock);
int setLockPaired(igm_lock_t *lock);
int setLockConnectionID(igm_lock_t *lock, int ID);

// 可能泄漏操作
int releaseLockAminKey(igm_lock_t *lock);
int releaseLockPassword(igm_lock_t *lock);
int releaseLockCmd(igm_lock_t *lock);
int setLockAdminKey(igm_lock_t *lock, uint8_t *admin_key, int admin_key_len);
int setLockPassword(igm_lock_t *lock, uint8_t *password, int password_size);
int setLockCmd(igm_lock_t *lock, uint8_t *lock_cmd, unsigned int lock_cmd_size);

int getLockAdminKey(igm_lock_t *lock, 
		uint8_t *admin_key, int *p_admin_key_len);
int getLockPassword(igm_lock_t *lock, 
		uint8_t *password, int *p_password_size);

void releaseLock(igm_lock_t **pp_lock);
// isLockGuest()
#endif
