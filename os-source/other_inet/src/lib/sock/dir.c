
/*
** Unix compatible directory routines for the Amiga.
*/

/****** socket/opendir ******************************************
*
*   NAME
*		opendir -- opens a directory
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/dir.h>
*
*		dirp = opendir( dirname )
*
*		DIR *opendir (char *); 
*
*   FUNCTION
*		Opens the named directory and returns a pointer to the
*		directory stream.  
*
*	INPUTS
*		dirname		name of the directory
*
*   RESULT
*		Returns a pointer to the directory stream if successful.
*		Returns NULL if dirname cannot be accessed.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		readdir(), rewinddir(), closedir()
*
******************************************************************************
*
*/

#include <sys/types.h>
#include <sys/dir.h>

#include <libraries/dos.h>

/*
** fib must come first as it is supposed to be longword aligned.
*/
struct directory {
	struct direct d;		/* what Unix application sees	*/
	struct FileInfoBlock fib;	/* FIB we'll use		*/
	long lock;			/* lock associated with dir	*/
};

#define MAKENT(dp)\
	dp->d.d_name = dp->fib.fib_FileName;\
	dp->d.d_namelen = strlen(dp->d.d_name);\
	dp->d.d_ino = dp->fib.fib_DiskKey;


DIR *opendir(path)
	char	*path;
{
	register struct directory *dp;
	extern void *malloc();
	long Lock(), lk;

	if(!(lk = Lock(path, ACCESS_READ))){
		return NULL;
	}
	if(dp = (struct directory *)malloc(sizeof(*dp))){
		if(!Examine(lk, &dp->fib)){
			free(dp);
			return NULL;
		}
		dp->lock = lk;
		MAKENT(dp);
	}

	return (DIR *)dp;
}


/****** socket/readdir ******************************************
*
*   NAME
*		readdir -- reads a directory
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/dir.h>
*
*		next_dir = readdir( dirp )
*
*		struct direct *readdir (DIR *); 
*
*   FUNCTION
*		returns a pointer to the next directory entry.
*
*	INPUTS
*		dirp	directory stream from opendir().
*
*   RESULT
*		Returns a pointer to the next directory entry if successful.
*		Returns NULL on end of directory.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		opendir(), rewinddir(), closedir()
*
******************************************************************************
*
*/

struct direct *readdir(dp)
	DIR *dp;
{
	if(!ExNext(((struct directory *)dp)->lock, &((struct directory *)dp)->fib)){
		return 0;
	}

	MAKENT(((struct directory *)dp));
	return dp;
}


/****** socket/rewinddir ******************************************
*
*   NAME
*		rewinddir -- set position in directory back to beginning.
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/dir.h>
*
*		result = rewinddir( dirp )
*
*		int rewinddir (DIR *); 
*
*   FUNCTION
*		Sets the position in the directory stream back to the beginning.
*
*	INPUTS
*		dirp	directory stream from opendir().
*
*   RESULT
*		Returns -1 if unsuccessful.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		readdir(), opendir(), closedir()
*
******************************************************************************
*
*/


int
rewinddir(dp)
	DIR *dp;
{
	return Examine(((struct directory *)dp)->lock, &((struct directory *)dp)->fib)==0 ? -1:0;
}


/****** socket/closedir ******************************************
*
*   NAME
*		closedir -- closes a directory stream
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/dir.h>
*
*		result = closedir( dirp )
*
*		int closedir (DIR *); 
*
*   FUNCTION
*		Closes the named directory stream.
*
*	INPUTS
*		dirp	directory stream from opendir().
*
*   RESULT
*		Returns 0.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		readdir(), rewinddir(), opendir()
*
******************************************************************************
*
*/


int
closedir(dp)
	DIR *dp;
{
	UnLock(((struct directory *)dp)->lock);
	free(dp);
	return 0;
}
