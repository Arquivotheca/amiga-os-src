
#include <exec/ports.h>
/* local socket structure */

struct AmigaSocket
{
	struct MsgPort socketPort;
	long	readers;
	long	writers;
	ULONG	flags;
};

/* definations for flags */
#define AMIGAPORT_SHUTDOWN	1
#define AMIGAPORT_PUBLIC	2	/* on exec list */

