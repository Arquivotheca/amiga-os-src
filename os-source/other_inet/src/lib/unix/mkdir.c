/*
** mkdir(path, mode)	- make a directory
*/

#include <stdio.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <errno.h>

extern struct Library *DOSBase;

int mkdir(char *path, int mode)
{
	long	lock;

	lock = CreateDir(path);
	if(!lock){
		errno = IoErr();
		return -1;
	}

	UnLock(lock);
	return 0;
}

rmdir(char *path)
{
	return unlink(path);
}
