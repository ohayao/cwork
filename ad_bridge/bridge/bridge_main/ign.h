#ifndef __IGN__
#define __IGN__


#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/time.h>
// #include "MQTTClient.h"


// atomic_int g_msg_id;

//LIST_HEAD(task_head);
//INIT_LIST_HEAD(task_head);











// unsigned int GetMsgID() {
//     atomic_fetch_add_explicit(&g_msg_id, 1, memory_order_relaxed);
//     return g_msg_id;
// // }

// thread_type Thread_start(void* fn, void* parameter) {
//     thread_type thread = 0;
//     pthread_attr_t attr;

//     //FUNC_ENTRY
//     pthread_attr_init(&attr);
//     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
//     if (pthread_create(&thread, &attr, fn, parameter) != 0)
//         thread = 0;
//     pthread_attr_destroy(&attr);
//     //FUNC_EXIT;
//     return thread;
// }

// mutex_type Thread_create_mutex(void)
// {
//     mutex_type mutex = NULL;

//     //FUNC_ENTRY;
//     mutex = malloc(sizeof(pthread_mutex_t));
//     int rc = pthread_mutex_init(mutex, NULL);
//     if (0>rc)
//         serverLog(LL_ERROR,"create thread error, rc[%d].", rc);
//     else
//         serverLog(LL_NOTICE,"create thread rc[%d].", rc);
//     //FUNC_EXIT_RC(rc);
//     return mutex;
// }

// void Thread_destroy_mutex(mutex_type mutex)
// {
//     //int rc = 0;
//     //FUNC_ENTRY;
//     pthread_mutex_destroy(mutex);
//     free(mutex);
//     //FUNC_EXIT_RC(rc);
// }

//thread_type thread = Thread_start(thread_fn fn, void* parameter);
//mutex_type mutex = Thread_create_mutex();
//rc = pthread_mutex_lock(mutex);
//rc = pthread_mutex_unlock(mutex);
//pthread_self();
//pthread_detach(p);??????


#endif
