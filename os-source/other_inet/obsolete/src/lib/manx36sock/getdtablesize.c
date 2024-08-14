/*
 * getdtablsize - return "file" table size.  In Manx, this is fixed
 *		  at MAXSTREAM (from <stdio.h>).
 */

#ifdef LATTICE
extern int _nufbs;
#define NUMFD _nufbs
#else
#include <fcntl.h>
#define NUMFD _numdev
#endif

getdtablesize()
{
	return NUMFD;
}

