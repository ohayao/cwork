#ifndef __IGN__
#define __IGN__


#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include "list.h"
#include "MQTTClient.h"

atomic_int g_msg_id = 0;

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID"
#define CA_PATH "/tmp/igkey/"
#define TRUST_STORE "/tmp/igkey/ca.pem"
#define PRIVATE_KEY "/tmp/igkey/key.pem"
#define KEY_STORE "/tmp/igkey/cert.pem"
static char LOG_FILE[] = "./log_ign";

#define thread_type pthread_t
#define mutex_type pthread_mutex_t*

#define LOG_MAX_LEN    1024 /* Default maximum length of syslog messages.*/
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_ERROR 4
#define LL_RAW (1<<10) /* Modifier to log without timestamp */


typedef struct st_sysinfo {
    int inited;
    MQTTClient mqtt_c;
    mutex_type mutex;
    char wifi_ssid[128];
    char wifi_pswd[32];
    char user_id[64];
    char user_pswd[32];
    void* userinfo;
    char lock1_id[32];
    char lock2_id[32];
    char lock3_id[32];
    char lock4_id[32];
    char lock5_id[32];
    void* lock1info;
    void* lock2info;
    void* lock3info;
    void* lock4info;
    void* lock5info;
}sysinfo_t;

typedef struct st_fsm_table {
	unsigned char cur_state;
	int (*eventActFun)(void*);
	unsigned char next_state;
}fsm_table_t;

typedef struct task_node {
    struct list_head list;
    sysinfo_t *sysif;
    char lock_id[32];
    void* lockinfo;
	//struct task_node *next;
	unsigned int msg_id;
    void * dataBLE;
    void * dataMQTT;
    //void * dataBtn;
    unsigned int start_time;
    fsm_table_t* p_sm_table;
    unsigned char cur_state;
}task_node_t;

//LIST_HEAD(task_head);
//INIT_LIST_HEAD(task_head);

enum
{
    BEGIN,
    DONE,
	CMD_INIT,
    INITED,
    GET_WIFI_USER,
	CMD_REQ_USERINFO,
    CMD_UPDATE_USERINFO,
    CMD_CONNECT_LOCK,
    CMD_UPDATE_LOCKSTATUS,
    CMD_UNLOCK,
};

/* We use a private localtime implementation which is fork-safe. The logging
 * function of Redis may be called from other threads. */
static int is_leap_year(time_t year) {
    if (year % 4) return 0;         /* A year not divisible by 4 is not leap. */
    else if (year % 100) return 1;  /* If div by 4 and not 100 is surely leap. */
    else if (year % 400) return 0;  /* If div by 100 *and* not by 400 is not leap. */
    else return 1;                  /* If div by 100 and 400 is leap. */
}

void nolocks_localtime(struct tm *tmp, time_t t, time_t tz, int dst) {
    const time_t secs_min = 60;
    const time_t secs_hour = 3600;
    const time_t secs_day = 3600*24;

    t -= tz;                            /* Adjust for timezone. */
    t += 3600*dst;                      /* Adjust for daylight time. */
    time_t days = t / secs_day;         /* Days passed since epoch. */
    time_t seconds = t % secs_day;      /* Remaining seconds. */

    tmp->tm_isdst = dst;
    tmp->tm_hour = seconds / secs_hour;
    tmp->tm_min = (seconds % secs_hour) / secs_min;
    tmp->tm_sec = (seconds % secs_hour) % secs_min;

    /* 1/1/1970 was a Thursday, that is, day 4 from the POV of the tm structure
     * where sunday = 0, so to calculate the day of the week we have to add 4
     * and take the modulo by 7. */
    tmp->tm_wday = (days+4)%7;

    /* Calculate the current year. */
    tmp->tm_year = 1970;
    while(1) {
        /* Leap years have one day more. */
        time_t days_this_year = 365 + is_leap_year(tmp->tm_year);
        if (days_this_year > days) break;
        days -= days_this_year;
        tmp->tm_year++;
    }
    tmp->tm_yday = days;  /* Number of day of the current year. */

    /* We need to calculate in which month and day of the month we are. To do
     * so we need to skip days according to how many days there are in each
     * month, and adjust for the leap year that has one more day in February. */
    int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    mdays[1] += is_leap_year(tmp->tm_year);

    tmp->tm_mon = 0;
    while(days >= mdays[tmp->tm_mon]) {
        days -= mdays[tmp->tm_mon];
        tmp->tm_mon++;
    }

    tmp->tm_mday = days+1;  /* Add 1 since our 'days' is zero-based. */
    tmp->tm_year -= 1900;   /* Surprisingly tm_year is year-1900. */
}


unsigned long getTimeZone(void) {
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return tz.tz_minuteswest * 60UL;
}

long long get_ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

int updateCachedTime() {
    long long ustime = get_ustime();
    long long mstime = ustime / 1000;
    _Atomic time_t unixtime = mstime / 1000;    /* Unix time sampled every cron cycle. */

    /* To get information about daylight saving time, we need to call
     * localtime_r and cache the result. However calling localtime_r in this
     * context is safe since we will never fork() while here, in the main
     * thread. The logging function will call a thread safe version of
     * localtime that has no locks. */
    struct tm tm;
    time_t ut = unixtime;
    localtime_r(&ut,&tm);
    return tm.tm_isdst;
}

/* Low level logging. To use only for very big messages, otherwise
 * serverLog() is to prefer. */
void serverLogRaw(int level, const char *msg) {
    //const int syslogLevelMap[] = { LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING };
    const int syslogLevelMap[] = { 0, 1, 2, 3 };
    const char *c = ".-*#";
    FILE *fp;
    char buf[64];
    int rawmode = (level & LL_RAW);
    int log_to_stdout = LOG_FILE == '\0';

    level &= 0xff; /* clear flags */
    //if (level < server.verbosity) return;

    fp = log_to_stdout ? stdout : fopen(LOG_FILE, "a");
    if (!fp) return;

    if (rawmode) {
        fprintf(fp,"%s",msg);
    } else {
        int off;
        struct timeval tv;
        pid_t pid = getpid();

        gettimeofday(&tv,NULL);
        struct tm tm;
        unsigned long timezone = getTimeZone();
        int daylight_active = updateCachedTime();
        nolocks_localtime(&tm, tv.tv_sec, timezone, daylight_active);
        off = strftime(buf,sizeof(buf),"%d %b %Y %H:%M:%S.",&tm);
        snprintf(buf+off,sizeof(buf)-off,"%03d",(int)tv.tv_usec/1000);
        fprintf(fp,"%d: %s %c %s\n",
            pid, buf,c[level],msg);
    }
    fflush(fp);

    if (!log_to_stdout) fclose(fp);
    //if (server.syslog_enabled) 
    syslog(syslogLevelMap[level], "%s", msg);
}

/* Like serverLogRaw() but with printf-alike support. This is the function that
 * is used across the code. The raw version is only used in order to dump
 * the INFO output on crash. */
void serverLog(int level, const char *fmt, ...) {
    va_list ap;
    char msg[LOG_MAX_LEN];

    //if ((level&0xff) < server.verbosity) return;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    serverLogRaw(level,msg);
}

unsigned int GetMsgID() {
    atomic_fetch_add_explicit(&g_msg_id, 1, memory_order_relaxed);
    return g_msg_id;
}

thread_type Thread_start(void* fn, void* parameter) {
    thread_type thread = 0;
    pthread_attr_t attr;

    //FUNC_ENTRY
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, fn, parameter) != 0)
        thread = 0;
    pthread_attr_destroy(&attr);
    //FUNC_EXIT;
    return thread;
}

mutex_type Thread_create_mutex(void)
{
    mutex_type mutex = NULL;

    //FUNC_ENTRY;
    mutex = malloc(sizeof(pthread_mutex_t));
    int rc = pthread_mutex_init(mutex, NULL);
    if (0>rc)
        serverLog(LL_ERROR,"create thread error, rc[%d].", rc);
    else
        serverLog(LL_NOTICE,"create thread rc[%d].", rc);
    //FUNC_EXIT_RC(rc);
    return mutex;
}

void Thread_destroy_mutex(mutex_type mutex)
{
    //int rc = 0;
    //FUNC_ENTRY;
    pthread_mutex_destroy(mutex);
    free(mutex);
    //FUNC_EXIT_RC(rc);
}

//thread_type thread = Thread_start(thread_fn fn, void* parameter);
//mutex_type mutex = Thread_create_mutex();
//rc = pthread_mutex_lock(mutex);
//rc = pthread_mutex_unlock(mutex);
//pthread_self();
//pthread_detach(p);??????


#endif
