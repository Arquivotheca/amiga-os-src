/****** socket/stat.c ******************************************
*
*   NAME
*		stat -- get file status
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/stat.h>
*
*		return = stat( filename, buf)
*
*		int stat (char *, struct stat *); 
*
*   FUNCTION
*		obtains information about the named file
*
*		Fills out the stat structure pointer to by buf.  The following
*		fields are filled in:
*		
*		u_short		st_mode;
*		short		st_uid;
*		short		st_gid;
*		long		st_size;
*		long		st_mtime;
*		long		st_atime;	 same as st_mtime 
*		short		st_nlink;	 always 1 
*
*	INPUTS
*		filename	full or relative pathname of file
*
*		buf			pointer to a stat structure which is filled in by stat()
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*		struct stat buf;
*
*		if (stat("devs:foo",&buf)==-1) {
*			printf("Error: file not found\n");
*			exit(1);
*		}
*		
*   NOTES
*		On Amiga files, st_uid and st_gid will be set to that
*		of the local user.  The networking software must have been
*		started or -1 will be returned for both.
*
*   BUGS
*
*   SEE ALSO
*		fstat()
*
******************************************************************************
*
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dosextens.h>
#include <action.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <exec/memory.h>


#ifdef LATTICE
#include <proto/exec.h>
#endif


/* this is a FileInfoBlock with added stuff for NFS */

struct MyInfoBlock {
   LONG	  fib_DiskKey;
   LONG	  fib_DirEntryType;  /* Type of Directory. If < 0, then a plain file.
			      * If > 0 a directory */
   char	  fib_FileName[108]; /* Null terminated. Max 30 chars used for now */
   LONG	  fib_Protection;    /* bit mask of protection, rwxd are 3-0.	   */
   LONG	  fib_EntryType;
   LONG	  fib_Size;	     /* Number of bytes in file */
   LONG	  fib_NumBlocks;     /* Number of blocks in file */
   struct DateStamp fib_Date;/* Date file last changed */
   char	  fib_Comment[80];  /* Null terminated comment associated with file */
   LONG   is_remote;		/* 1 if remote file, 0 otherwise */
   LONG   uid;				/* userid */
   LONG   gid;				/* group id */
   LONG   mode;				/* mode */
   char	  fib_Reserved[20]; /* pad to size of real FileInfoBlock */
}; /* MyInfoBlock */

int
stat(name, buf)
	char *name;
	struct stat *buf;
{
	long l, Lock();
	struct MyInfoBlock *fp=NULL;
	void *AllocMem();
	long mode = 0;

	Chk_Abort();

	if ((l=Lock(name, ACCESS_READ)) == 0)
		return(-1);

	if ((fp = (struct MyInfoBlock *)AllocMem((long)sizeof *fp, 0L)) == 0) {
		UnLock(l);
		return(-1);
	}

	if (GetMyFib(l, fp) == 0 ) {
		UnLock(l);
		return(-1);
	}

	bzero(buf, sizeof(*buf));
	buf->st_mtime = fp->fib_Date.ds_Days * 24 * 60 * 60 +
					fp->fib_Date.ds_Minute * 60 +
					fp->fib_Date.ds_Tick/TICKS_PER_SECOND;
	buf->st_atime = buf->st_mtime;
	buf->st_size = fp->fib_Size;

	if (fp->is_remote) {
		buf->st_uid = fp->uid;
		buf->st_gid = fp->gid;
		buf->st_mode = fp->mode;
	} else {
		buf->st_uid = getuid();
		buf->st_gid = getgid();
		if (!(fp->fib_Protection & FIBF_READ))
			mode |= 0444;
		if( !(fp->fib_Protection & FIBF_WRITE))
			mode |= 0222;
		if (!(fp->fib_Protection & FIBF_EXECUTE))
			mode |= 0111;

		if (fp->fib_DirEntryType > 0)	
			mode |= S_IFDIR;
		else 
			mode |= S_IFREG;

		buf->st_mode = mode;
	}

	buf->st_nlink = 1;	/* Hard coded */

	FreeMem(fp, (long) sizeof *fp);
	UnLock(l);
	return(0);
}


GetMyFib(lock,inf)
BPTR lock;
struct FileInfoBlock *inf;
{

	struct fattr fa;
	struct MsgPort *dev;
	ULONG result;

	if (lock != 0) {
		dev = btod(lock,struct FileLock *)->fl_Task;
	} else {
		dev = (struct MsgPort *)(((struct Process *)FindTask(0))->pr_FileSystemTask);
	}
		
	result = dos_packet(dev, ACTION_EX_OBJECT, lock, dtob(inf), &fa);
	
	if (!result) {
		result = dos_packet(dev, ACTION_EXAMINE_OBJECT, lock, dtob(inf),0);
		if(!result) {
			return 0;
		}
		((struct MyInfoBlock *)inf)->is_remote = 0; 
	}  else {
		((struct MyInfoBlock *)inf)->is_remote = 1; 
	} 
			
	if (((struct MyInfoBlock *)inf)->is_remote) {	
		((struct MyInfoBlock *)inf)->uid = fa.uid;
		((struct MyInfoBlock *)inf)->gid = fa.gid;
		((struct MyInfoBlock *)inf)->mode = fa.mode;
	} 
	return 1;
}

