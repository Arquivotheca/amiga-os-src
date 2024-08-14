/* mygetvec.c */

#include "ram_includes.h"
#include "ram.h"

void *
getvec (ULONG size)
{
	void *foo;

	foo = AllocVec(size,0);
	if (!foo)
		fileerr = ERROR_NO_FREE_STORE;

	return foo;
}
