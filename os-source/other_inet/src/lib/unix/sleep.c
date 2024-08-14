/*
** sleep(time)	- sleep for <time> seconds.
*/

#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
extern struct DosLibrary *DOSBase;

void
sleep(seconds)
	int	seconds;
{
	Delay(seconds*TICKS_PER_SECOND);
}

