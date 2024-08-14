/*
** chdir(dirname) - change to directory. 
*/

#include <libraries/dos.h>

chdir(name)
	char	*name;
{
	long	current, new, Lock();

	if((new = Lock(name, ACCESS_READ)) == 0){
		return -1;
	}
	current = CurrentDir(new);
	if(current){
		UnLock(current);
	}
	return 0;
}
