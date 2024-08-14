/****** socket/fstat ******************************************
*
*   NAME
*		fstat -- obtain information about an open file
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/stat.h>
*
*		return = fstat( fd, buf )
*
*		int fstat (int , struct stat *); 
*
*   FUNCTION
*		This function is only partially implemented.  A full
*		implementation will be available in the future.
*
*		Fills out the stat structure pointer to by buf.  The following
*		fields are filled in:
*		
*		u_short		st_mode;	always S_IFREG
*		short		st_uid;		 see notes
*		short		st_gid;		 see notes
*		long		st_size;
*		long		st_mtime;	 always 0
*		long		st_atime;	 always 0
*		short		st_nlink;	 always 1
*		short		st_blksize;	 always 8192 	
*
*	INPUTS
*		fd		file descriptor returned from open().
*
*		buf		pointer to stat structure.
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*
*   NOTES
* 		st_uid and st_gid will be set to that of the local user.  
*		The networking software must have been started or -1 will 
*		be returned for both.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


#ifdef LATTICE

/*
**
** NB.  This is the LATTICE ONLY version !!!
**
** bj - 8-2-90
**
**
*/


#define UNIX_BITS TRUE   /* delete this is protection bits */
                         /* used are AmigaDOS version      */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ios1.h>
#include <stdio.h>


extern struct UFB _ufbs[] ;

extern struct UFB *chkufb( int ) ;
extern int getuid( void ) ;
extern int getgid( void ) ;

fstat( int fd, struct stat *stbp)
{
	
	struct UFB *ufb = NULL ;
	long	old;

	bzero(stbp, sizeof(*stbp));
	stbp->st_uid = getuid();
	stbp->st_gid = getgid();
	stbp->st_mode = S_IFREG;
	stbp->st_blksize = 8192;

	if( fd < 0 || ((ufb = chkufb(fd)) == NULL) ) {
		errno = EBADF ;
		return -1 ; 
		}
	old = lseek( fd, 0L, 2 ) ;	
	stbp->st_size = lseek( fd, 0L, 2 ) ;	
	if( stbp->st_size < 0L ) {
		errno = IoErr() ;
		return( -1 ) ;
		}
	
	return 0;
}

#else

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

#endif /* LATTICE */