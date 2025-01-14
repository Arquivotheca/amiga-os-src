/*
** 4.x Unix time struct compatibility.
*/

#ifndef SYS_TIME_H
#define SYS_TIME_H

#include <exec/types.h>
#include <devices/timer.h>

#define tv_sec tv_secs
#define tv_usec tv_micro

#define timerclear(tvp) (tvp)->tv_secs = (tvp)->tv_usec = 0L
#define timerisset(tvp) ((tvp)->tv_secs || (tvp)->tv_usec)

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct itimerval {
	struct timeval	it_value, it_interval;
};

#define ITIMER_REAL	1

#define timercmp(t1, t2, op)\
	( ((t1)->tv_sec op (t2)->tv_sec) || \
	  ((t1)->tv_sec==(t2)->tv_sec && (t1)->tv_usec op (t2)->tv_usec))

#include <time.h>		/* 4.2 compatibility */
#endif SYS_TIME_H

