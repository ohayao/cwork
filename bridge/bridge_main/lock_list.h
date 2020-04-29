#ifndef _DISCOVER_LOCK_LIST_H_
#define _DISCOVER_LOCK_LIST_H_
#include <bridge/ble/lock.h>

// 用于管理扫描到的设备.

int isLocksListEmpty();
int insertLock(igm_lock_t *lock);
igm_lock_t *findLockByName(char name[MAX_DEVICE_NAME]);
igm_lock_t *findLockByAddr(char addr[MAX_DEVICE_ADDR]);
void DeleteTask(igm_lock_t** pp_lock);
void printLockList();

// clearLocks()
// DeleteLock()



#endif