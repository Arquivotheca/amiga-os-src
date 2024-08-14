/*
** 4.x Unix time struct compatibility.
*/

#ifndef SYS_TIME_H
#define SYS_TIME_H

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

#define timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0L
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2

struct  itimerval {
        struct  timeval it_interval;    /* timer interval */
        struct  timeval it_value;       /* current value */
};

#include <time.h>               /* 4.2 compatibility */
#endif SYS_TIME_H

