/****** socket/access ******************************************
*
*   NAME
*		access -- Check the accessibility of a file
*
*   SYNOPSIS
*		return = access( filename, mode)
*
*		int access (char *, int); 
*
*   FUNCTION
*		Checks to see if a file can be accessed with a certain mode.
*		Returns true (0) if mode is write and file is not found.
*
*	INPUTS
*		filename	name of the file
*
*		mode		file mode
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


/*
** Modified Manx access() - returns affirmative when asking if file is
**			    writeable and doesn't exist.
*/

#include <errno.h>
#include <libraries/dos.h>

int
access(filename, mode)
	char *filename;
	int mode;
{
	long l, Lock();
	int ret = -1;

	if ((l = Lock(filename, ACCESS_READ)) == 0) {
		if(mode==2 && (IoErr() == ERROR_OBJECT_NOT_FOUND)){
			return 0;
		} else {
			errno = ENOENT;
			return(-1);
		}
	}
	switch(mode) {
	case 2:			/* write */
		UnLock(l);
		if ((l = _Lock(filename, ACCESS_WRITE)) == 0) {
			
			errno = EACCES;
			return(-1);
		}
		/* FALL THRU */
	case 0:			/* existence */
	case 1:			/* execute (if file) or search (if directory) */
	case 4:			/* read */
		ret = 0;
		break;
	}
	UnLock(l);
	return(ret);
}

