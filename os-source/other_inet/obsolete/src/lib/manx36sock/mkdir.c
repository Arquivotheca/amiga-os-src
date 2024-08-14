/*
** mkdir(path, mode)	- make a directory
*/

#include <errno.h>

mkdir(path, mode)
	char	*path;
	int	mode;
{
	long	lock, CreateDir();

	lock = CreateDir(path);
	if(!lock){
		errno = _IoErr();
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
