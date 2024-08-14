/*
** Unix compatible directory routines for the Amiga.
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
		return 0;
	}
	if(dp = (struct directory *)malloc(sizeof(*dp))){
		if(!Examine(lk, &dp->fib)){
			free(dp);
			return 0;
		}
		dp->lock = lk;
		MAKENT(dp);
	}

	return (DIR *)dp;
}

DIR *readdir(dp)
	struct directory *dp;
{
	if(!ExNext(dp->lock, &dp->fib)){
		return 0;
	}

	MAKENT(dp);
	return (DIR *)dp;
}

int
rewinddir(dp)
	struct directory *dp;
{
	return Examine(dp->lock, &dp->fib)==0 ? -1:0;
}

int
closedir(dp)
	struct directory *dp;
{
	UnLock(dp->lock);
	free(dp);
	return 0;
}
