/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

#include <sys/types.h>
#include <sys/stat.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dosextens.h>
#ifdef LATTICE
#include <proto/exec.h>
#endif
#include <bstr.h>

/*
** enhanced version of Manx stat.  Returns object type, plus other items.
*/
int
stat(name, buf)
	char *name;
	struct stat *buf;
{
	long l, Lock();
	struct FileInfoBlock *fp;
	void *AllocMem();
	long mode = 0;

	Chk_Abort();
	if ((l=Lock(name, ACCESS_READ)) == 0)
		return(-1);
	if ((fp = (struct FileInfoBlock *)AllocMem((long)sizeof *fp, 0L)) == 0) {
		UnLock(l);
		return(-1);
	}
	Examine(l, fp);

	bzero(buf, sizeof(*buf));
	buf->st_mtime = fp->fib_Date.ds_Days * 24 * 60 * 60 +
					fp->fib_Date.ds_Minute * 60 +
					fp->fib_Date.ds_Tick/TICKS_PER_SECOND;
	buf->st_size = fp->fib_Size;
	buf->st_uid = getuid();
	buf->st_gid = getgid();
	buf->st_nlink = 1;	/* Hard coded */
#define DALE
#ifdef DALE
	if (!(fp->fib_Protection & FIBF_READ)){
		mode |= 0444;
	}
	if( !(fp->fib_Protection & FIBF_WRITE)){
		mode |= 0222;
	}
	if (!(fp->fib_Protection & FIBF_EXECUTE)){
		mode |= 0111;
	}
	/*printf("prot=%lx mode=%lx\n",fp->fib_Protection,mode);*/

	if (fp->fib_DirEntryType > 0)	mode |= S_IFDIR;
	else mode |= S_IFREG;

	buf->st_mode = mode;
#else
	buf->st_mode = fp->fib_EntryType < 0 ? S_IFREG:S_IFDIR);
#endif

	FreeMem(fp, (long) sizeof *fp);
	UnLock(l);
	return(0);
}

