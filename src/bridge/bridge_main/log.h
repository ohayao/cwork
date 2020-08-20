#ifndef _LOG_H_
#define _LOG_H_
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_MAX_LEN    2048 /* Default maximum length of syslog messages.*/
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_ERROR 4
#define LL_RAW (1<<10) /* Modifier to log without timestamp */

static char LOG_FILE[] = "./log_ign";
int updateCachedTime();
long long get_ustime(void);
int updateCachedTime();
static int is_leap_year(time_t year);
void serverLogRaw(int level, const char *msg);
void serverLog(int level, const char *fmt, ...);

#endif
