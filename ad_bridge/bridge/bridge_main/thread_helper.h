#ifndef _THREAD_HELPER_H_
#define _THREAD_HELPER_H_
#include <pthread.h>

pthread_t Thread_start(void* fn, void* parameter);

#endif