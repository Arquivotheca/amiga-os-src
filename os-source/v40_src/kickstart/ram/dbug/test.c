#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#include "stdio.h"

void
main (argc,argv)
	int argc;
	char **argv;
{
	APTR devproc;
	BPTR lock;

	devproc = DeviceProc("myram:");
	lock = IoErr();
	printf("Process at 0x%lx, lock 0x%lx\n",devproc,lock);

}
