/*
** sleep(time)	- sleep for <time> seconds.
*/

#include <libraries/dos.h>
#ifdef LATTICE
#include <proto/dos.h>
#endif

void
sleep(seconds)
	int	seconds;
{
	Delay(seconds*TICKS_PER_SECOND);
}

