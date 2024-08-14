#include "ed.h"

#if	AMIGA

/* trying a real simple allocate/free scheme for awhile */

UBYTE *malloc(size)
LONG size; {
    LONG *p = NULL;
    LONG asize = size+4;

    if (size > 0) p = (LONG *)AllocMem(asize,MEMF_CLEAR|MEMF_PUBLIC);
    if (p != NULL) *p++ = asize; /* post-bump p to point at start of memory */
    return((UBYTE *)p);
}

void free(p)
UBYTE *p; 
{
    if (p != NULL) {
	p -= 4;
	FreeMem(p, *((LONG *)p));
    }
}

#endif
