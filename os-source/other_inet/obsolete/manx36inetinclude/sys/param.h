
#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef SYS_SIGNAL_H
#include <sys/signal.h>
#endif

#define CLSIZE	4
#define NBPG	1024
#define CLBYTES	(CLSIZE*NBPG)
#define CLSHIFT 10
#define CLOFSET (CLBYTES-1)
#define PZERO	25
#define MAXHOSTNAMELEN	32
#define MAXPATHLEN	256
#define MAXDOMAIN	32
#define MAXVOL		32
#define NGROUP		8
#define NCARGS		32		/* for ftp client, primarily */
