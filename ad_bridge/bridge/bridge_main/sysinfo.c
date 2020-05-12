#include <bridge/bridge_main/sysinfo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t sysinfo_mutex = PTHREAD_MUTEX_INITIALIZER;

#define _lock()pthread_mutex_lock(&sysinfo_mutex)
#define _unlock() pthread_mutex_unlock(&sysinfo_mutex)

int tasksysinfoInit(sysinfo_t *sysinfo)
{
  _lock();
  memset(sysinfo, 0, sizeof(sysinfo_t));
  _unlock();
  return 0;
}



