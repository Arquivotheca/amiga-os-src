/*
** fstat() for amiga.  We can't do everything "right", but all attempts to
** remain faitful to the original Unix call have been made.  Maybe some day
** we can get this totally right.
**
** NB.  We don't use Manx lseek() here because it doesn't return old
**      position.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <bstr.h>

fstat(fd, stbp)
	int	fd;
	struct stat *stbp;
{
	register struct _dev *refp;
	long	old;

	bzero(stbp, sizeof(*stbp));
	stbp->st_uid = getuid();
	stbp->st_gid = getgid();
	stbp->st_mode = S_IFREG;
	stbp->st_blksize = 8192;

	refp = _devtab + fd;
	if(fd < 0 || fd > _numdev || refp->fd == 0){
		errno = EBADF;
		return -1;
	}

	old = Seek(refp->fd, 0, 1);
	stbp->st_size = Seek(refp->fd, old, -1);
	if(old < 0 || stbp->st_size < 0){
		errno = IoErr();
		return -1;
	}

	return 0;
}
