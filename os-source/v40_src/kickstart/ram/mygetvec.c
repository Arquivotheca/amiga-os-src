/* mygetvec.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* ptr := mygetvec(n)                                   */
/*                                                      */
/* Central memory management routine for RAM:           */
/********************************************************/

/* leave a 30K chunk */
#define RIP_PRIME
#define SAFETY_FACTOR 30000

void *
mygetvec (n)
	ULONG n;
{
	void *p;

#ifdef RIP
	if (ripcord == NULL)
	{
		ripcord = AllocVec(SAFETY_FACTOR*4,0);
		if (ripcord == NULL)
			return NULL;
	}
#endif
#ifdef RIP_PRIME
	/* try to leave a chunk of a certain size in the system */
	if ((n == DATA_BLOCK_SIZE*4) &&
	    (AvailMem(MEMF_LARGEST | MEMF_PUBLIC) < SAFETY_FACTOR))
	{
		fileerr = ERROR_DISK_FULL;
		return NULL;
	}
#endif
	p = AllocVec(n,0);
	if (p == NULL)
	{
		fileerr = ERROR_DISK_FULL;
#ifdef RIP
		freevec(ripcord);
		ripcord = NULL;
#endif
	}

	return p;
}

void *
getvec (ULONG size)
{
	void *foo;

	foo = AllocVec(size,0);
	if (!foo)
		fileerr = ERROR_NO_FREE_STORE;

	return foo;
}
