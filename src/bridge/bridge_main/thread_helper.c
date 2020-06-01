#include <bridge/bridge_main/thread_helper.h>
#include <pthread.h>

pthread_t Thread_start(void* fn, void* fn_parameter) {
    pthread_t thread = 0;
    pthread_attr_t attr;

    //FUNC_ENTRY
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, fn, fn_parameter) != 0)
        thread = 0;
    pthread_attr_destroy(&attr);
    //FUNC_EXIT;
    return thread;
}