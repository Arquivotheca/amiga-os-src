/****** unix/chdir ******************************************
*
*   NAME
*		chdir -- change directory
*
*   SYNOPSIS
*		return = chdir( path )
*
*		int chdir (char *); 
*
*   FUNCTION
*		chdir() changes the current working directory to the
*		specified path.
*
*	INPUTS
*		path	pathname
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
extern struct DosLibrary *DOSBase;

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
