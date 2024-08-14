/*
 * NFS I/O routines.  Just serve as frontends to kill off potential
 * requesters
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "nfsd.h"

static struct Process *nfsd_pr;
static APTR old_win_ptr;

#define NOREQUESTER 	{old_win_ptr = nfsd_pr->pr_WindowPtr; \
				nfsd_pr->pr_WindowPtr = (APTR)-1;}
#define REQUESTER 	{nfsd_pr->pr_WindowPtr = old_win_ptr;}

BPTR nfs_lock(char *name, LONG how)
{
	BPTR lock;

	NOREQUESTER;
	lock = Lock(name, how);
	REQUESTER;
	return lock;
}

BPTR nfs_open(char *name, LONG how)
{
	BPTR fh;

	NOREQUESTER;
	fh = Open(name, how);
	REQUESTER;
	return fh;
}

BPTR nfs_createdir(char *path)
{
	BPTR rtn;

	NOREQUESTER;
	rtn = CreateDir(path);
	REQUESTER;
	return rtn;
}

long nfs_wr(BPTR fh, void *buf, long len)
{
	long	rtn;

	NOREQUESTER;
	rtn = Write(fh, buf, len);
	REQUESTER;
	return rtn;
}

long nfs_rd(BPTR fh, void *buf, long len)
{
	long	rtn;

	NOREQUESTER;
	rtn = Read(fh, buf, len);
	REQUESTER;
	return rtn;
}

long nfs_seek(BPTR fh, long pos, long how)
{
	long	rtn;

	NOREQUESTER;
	rtn = Seek(fh, pos, how);
	REQUESTER;
	return rtn;
}

BOOL nfs_examine(BPTR lk, struct FileInfoBlock *fib)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = Examine(lk, fib);
	REQUESTER;
	return rtn;
}

BOOL nfs_exnext(BPTR lk, struct FileInfoBlock *fib)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = ExNext(lk, fib);
	REQUESTER;
	return rtn;
}

void nfs_unlock(BPTR lk)
{

	NOREQUESTER;
	UnLock(lk);
	REQUESTER;
}

BOOL nfs_close(BPTR fh)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = Close(fh);
	REQUESTER;
	return rtn;
}

BOOL nfs_rename(char *from, char *to)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = Rename(from, to);
	REQUESTER;
	return rtn;
}

BOOL nfs_info(BPTR lk, struct InfoData *id)
{
	BOOL rtn;
	NOREQUESTER;
	rtn = Info(lk, id);
	REQUESTER;
	return rtn;
}

void init_io()
{
	nfsd_pr = (struct Process *)FindTask(0L);
}
