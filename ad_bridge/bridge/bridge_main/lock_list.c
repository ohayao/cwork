#include <bridge/bridge_main/lock_list.h>
#include <bridge/bridge_main/list.h>
#include <bridge/ble/lock.h>
#include <string.h>
#include <stdlib.h>
#include <bridge/bridge_main/log.h>
#include <pthread.h>


list_head_t locks_head = {&locks_head, &locks_head};
pthread_mutex_t lock_list_mutex = PTHREAD_MUTEX_INITIALIZER;
size_t num_of_locks = 0;

#define _lock() pthread_mutex_lock(&lock_list_mutex)

#define _unlock() pthread_mutex_unlock(&lock_list_mutex)

// -------------------------------------

int isLocksListEmpty()
{
	int ret = 0;
	_lock();
	ret = list_empty(&locks_head);
	_unlock();
	return ret;
}

// inserLock()
int insertLock(igm_lock_t *lock)
{
	igm_lock_t *new_lock = (igm_lock_t *)malloc(sizeof(igm_lock_t));
	lockCopy(new_lock, lock);
	_lock();
	list_add(&(new_lock->list), &locks_head);
	_unlock();
	return 0;
}

// findLockByAddr()
igm_lock_t *findLockByAddr(const char addr[MAX_DEVICE_ADDR])
{
	list_head_t *pos = NULL;
	igm_lock_t *pnode = NULL;
	_lock();
	list_for_each(pos, &locks_head) {
		pnode = list_entry(pos, igm_lock_t, list);
		if (!strncmp(pnode->addr, addr, pnode->addr_len))
		{
			_unlock();
			return pnode;
		}
	}
	_unlock();
	return NULL;
}

igm_lock_t *findLockByName(const char name[MAX_DEVICE_NAME])
{
	list_head_t *pos = NULL;
	igm_lock_t *pnode = NULL;
	_lock();
	list_for_each(pos, &locks_head) {
		pnode = list_entry(pos, igm_lock_t, list);
		if (!strncmp(pnode->name, name, pnode->name_len))
		{
			_unlock();
			return pnode;
		}
	}
	_unlock();
	return NULL;
}

void DeleteTask(igm_lock_t** pp_lock) {
	igm_lock_t* lock = *pp_lock;
	_lock();
	list_del(&lock->list);
	free(lock);
	_unlock();
	*pp_lock = NULL;
}

void printLockList()
{
	if (isLocksListEmpty()) return;
	list_head_t *pos = NULL;
	igm_lock_t *pnode = NULL;
	_lock();
	list_for_each(pos, &locks_head) {
		pnode = list_entry(pos, igm_lock_t, list);
		printLock(pnode);
	}
	_unlock();
	return;
}




