#include <bridge/ble/lock.h>
#include <string.h>
#include <bridge/bridge_main/log.h>

int getLock(igm_lock_t **pp_lock) {
	if (!pp_lock) return 1;
	if (*pp_lock) releaseLock(pp_lock);
	*pp_lock = malloc(sizeof(igm_lock_t));
	return 0;
}

int initLock(igm_lock_t *lock) {
	if (!lock) return 1;
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
// int has_key;
// uint8_t *key;
// size_t key_len;
// int has_password;
// uint8_t *password;
// size_t password_size;
// 
// int connectionID;
int copyLock(igm_lock_t *to, igm_lock_t *from) {
	if (!to && !from) return 1;
	// 不要copy list header
	initLock(to);
	to->addr_len = from->addr_len;
	to->name_len = from->name_len;
	memcpy(to->addr, from->addr, to->addr_len);
	memcpy(to->name, from->name, to->name_len);
	to->paired = from->paired;
	to->admin_connection = from->admin_connection;
	to->guest_connection = from->guest_connection;
	to->connectionID = from->connectionID;
	if (from->has_key && from->key && from->key_len) {
		releaseLockKey(to);
		to->has_key = from->has_key;
		to->key_len = from->key_len;
		//to->key = calloc(from->key_len, 1);
		//memcpy(to->key, from->key, from->key_len);
		to->key = from->key;
	}

	if (from->has_password && from->password && from->password_size) {
		releaseLockPassword(to);
		to->has_password = from->has_password;
		to->password_size = from->password_size;
		//to->password = calloc(from->password_size, 1);
		//memcpy(to->password, from->password, from->password_size);
		to->password = from->password;
	}
	if (from->has_lock_cmd && from->lock_cmd && from->lock_cmd_size) {
		releaseLockCmd(to);
		to->has_lock_cmd = from->has_lock_cmd;
		to->lock_cmd_size = from->lock_cmd_size;
		to->lock_cmd = from->lock_cmd;
	}
	if (from->has_token && from->token && from->token_len) {
		releaseLockToken(to);
		to->has_token = from->has_token;
		to->token_len = from->token_len;
		to->token = from->token;
	}

	return 0;
}

int setLockName(igm_lock_t *lock, const char *name_, int name_len_) {
	memset(lock->name, 0, MAX_DEVICE_NAME);
	lock->name_len = name_len_;
	memcpy(lock->name, name_, lock->name_len);
	return 0;
}

int setLockAddr(igm_lock_t *lock, const char *addr_, int addr_len_) {
	memset(lock->addr, 0, MAX_DEVICE_ADDR);
	lock->addr_len = addr_len_;
	memcpy(lock->addr, addr_, lock->addr_len);
	return 0;
}


// -------------------------------------
// int has_key;
// uint8_t *key;
// size_t key_len;
// int has_password;
// uint8_t *password;
// size_t password_size;
// int connectionID;
void printLock (igm_lock_t *lock) {
	serverLog(LL_NOTICE, "--------------print Lock begin----------------");
	printf("addr_len %d, addr: %s\n", lock->addr_len, lock->addr);
	printf("name_len %d, name: %s\n", lock->name_len, lock->name);
	printf("paired %d\n", lock->paired);
	printf("admin_connection %d\n", lock->admin_connection);
	printf("guest_connection %d\n", lock->guest_connection);
	printf("connectionID %d\n", lock->connectionID);
	if (lock->has_password) {
		printf("password size %zu password:[", lock->password_size);
		for (int j = 0; j < lock->password_size; j++) {
			printf("%x", lock->password[j]);
		}
		printf("]\n");
	}
	if (lock->has_key) {
		printf("key size %zu key:[ ", lock->key_len);
		for (int j = 0; j < lock->key_len; j++) {
			printf("%x", lock->key[j]);
		}
		printf("]\n");
	}
	if (lock->has_lock_cmd) {
		printf("lock_cmd size %zu lock_cmd:[ ", lock->lock_cmd_size);
		for (int j = 0; j < lock->lock_cmd_size; j++) {
			printf("%x", lock->lock_cmd[j]);
		}
		printf("]\n");
	}
	serverLog(LL_NOTICE, "--------------print Lock end------------------");
}

int isLockPaired(igm_lock_t *lock) {
	return lock->paired;
}

int isLockAdmin(igm_lock_t *lock) {
	return lock->admin_connection;
}

int setLockPaired(igm_lock_t *lock) {
	lock->paired = 1;
}

int setLockConnectionID(igm_lock_t *lock, int ID) {
	lock->connectionID = ID;
}

int releaseLockKey(igm_lock_t *lock) {
	serverLog(LL_NOTICE, "releaseLockKey");
	if (lock && lock->has_key && lock->key && lock->key_len) {
		lock->has_key = 0;
		lock->key_len = 0;
		free(lock->key);
		lock->key = NULL;
	}
	return 0;
}

int releaseLockPassword(igm_lock_t *lock) {
	serverLog(LL_NOTICE, "releaseLockPassword");
	if (lock && lock->has_password && lock->password_size && lock->password) {
		lock->has_password = 0;
		lock->password_size = 0;
		free(lock->password);
		lock->password = NULL;
	}
	return 0;
}

int releaseLockCmd(igm_lock_t *lock) {
	serverLog(LL_NOTICE, "releaseLockCmd");
	if (lock && lock->has_lock_cmd && lock->lock_cmd_size && lock->lock_cmd) {
		lock->has_lock_cmd = 0;
		lock->lock_cmd_size = 0;
		free(lock->lock_cmd);
		lock->lock_cmd = NULL;
	}
	return 0;
}

int releaseLockToken(igm_lock_t *lock) {
	serverLog(LL_NOTICE, "releaseLockToken");
	if (lock && lock->has_token && lock->token_len && lock->token) {
		lock->has_token = 0;
		lock->token_len = 0;
		free(lock->token );
		lock->token = NULL;
	}
	return 0;
}


int setLockToken(igm_lock_t *lock, uint8_t *token, int token_len) {
	releaseLockToken(lock);
	lock->has_token = 1;
	lock->token_len = token_len;
	lock->token = calloc(token_len, 1);
	memcpy(lock->token, token, token_len);
	return 0;
}

int setLockKey(igm_lock_t *lock, uint8_t *key, int key_len) {
	releaseLockKey(lock);
	lock->has_key = 1;
	lock->key_len = key_len;
	lock->key = calloc(key_len, 1);
	memcpy(lock->key, key, key_len);
	return 0;
}

int setLockPassword(igm_lock_t *lock, uint8_t *password, int password_size) {
	releaseLockPassword(lock);
	serverLog(LL_NOTICE, "releaseLockPassword");
	lock->has_password = 1;
	lock->password_size = password_size;
	lock->password = calloc(password_size, 1);
	memcpy(lock->password, password, password_size);
	return 0;
}

int setLockCmd(igm_lock_t *lock, uint8_t *lock_cmd, unsigned int lock_cmd_size) {
	//releaseLockCmd(lock);
	serverLog(LL_NOTICE, "releaseLockCmd");
	lock->has_lock_cmd = 1;
	lock->lock_cmd_size = lock_cmd_size;
	lock->lock_cmd = calloc(lock_cmd_size, 1);
	memcpy(lock->lock_cmd, lock_cmd, lock_cmd_size);
	return 0;
}

int getLockKey(igm_lock_t *lock, uint8_t *key, int *p_key_len) {
	if (lock->has_key) {
		*p_key_len = lock->key_len;
		memcpy(key, lock->key, lock->key_len);
	}
	return 0;
}

int getLockPassword(igm_lock_t *lock, uint8_t *password, int *p_password_size) {
	if (lock->has_password) {
		*p_password_size = lock->password_size;
		memcpy(password, lock->password, lock->password_size);
	}
	return 0;
}

int setLockAdminConnection(igm_lock_t *lock, int admin_connection) {
	lock->admin_connection = admin_connection;
	return 0;
}

void releaseLock(igm_lock_t ** pp_lock) {
	if (!pp_lock) return;
	if (!(*pp_lock)) return;
	releaseLockKey(*pp_lock);
	releaseLockPassword(*pp_lock);
	releaseLockCmd(*pp_lock);
	free(*pp_lock);
	*pp_lock = NULL;
}

