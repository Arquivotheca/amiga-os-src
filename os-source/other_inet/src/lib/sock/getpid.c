/*
** getpid for amiga.
*/

#ifdef LATTICE
#include <proto/exec.h>
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

struct Task *
getpid()
{
	return(FindTask((char *)0L));
}
