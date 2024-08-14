#ifndef UTMP_H
#define UTMP_H

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

/*
 * User login record file format.
 */

struct utmp {
	char	ut_line[8];
	char	ut_name[8];
	char	ut_host[16];
	u_long	ut_time;
};

#define UTMPFILE	"inet:log/utmp"
#define WTMPFILE	"inet:log/wtmp"

#endif
