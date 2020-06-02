#ifndef _MUTEX_HELPER_H_
#define _MUTEX_HELPER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <bridge/bridge_main/log.h>

pthread_mutex_t *Thread_create_mutex();

void Thread_destroy_mutex(pthread_mutex_t* mutex);

#endif