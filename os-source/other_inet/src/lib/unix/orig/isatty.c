/*
** isatty(fd) returns 1 if fd is a tty, 0 otherwise
*/
#include <ios1.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
extern struct DosLibrary *DOSBase;

extern struct UFB _ufbs[];

int isatty(int fd)
{
	if(IsInteractive(_ufbs[fd].ufbfh)==DOSTRUE)
		return 1;
	else
		return 0;
}
