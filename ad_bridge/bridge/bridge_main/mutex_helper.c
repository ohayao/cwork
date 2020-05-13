#include <bridge/bridge_main/mutex_helper.h>

void Thread_destroy_mutex(pthread_mutex_t* mutex)
{
    //int rc = 0;
    //FUNC_ENTRY;
    pthread_mutex_destroy(mutex);
    free(mutex);
    //FUNC_EXIT_RC(rc);
}

pthread_mutex_t* Thread_create_mutex()
{
    pthread_mutex_t *mutex = NULL;

    //FUNC_ENTRY;
    mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    int rc = pthread_mutex_init(mutex, NULL);
    if (0>rc)
        serverLog(LL_ERROR,"create thread error, rc[%d].", rc);
    else
        serverLog(LL_NOTICE,"create thread rc[%d].", rc);
    //FUNC_EXIT_RC(rc);
    return mutex;
}
