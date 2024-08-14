/*
** mkdir(path, mode)	- make a directory
*/

#include <errno.h>

int
mkdir(path, mode)
	char	*path;
	int	mode;
{
	long	lock, CreateDir();

	lock = CreateDir(path);
	if(!lock){
		errno = IoErr();
		return -1;
	}

	UnLock(lock);
	return 0;
}

rmdir(path)
	char 	*path;
{
	return unlink(path);
}
