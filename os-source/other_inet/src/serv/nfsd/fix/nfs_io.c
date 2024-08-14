/*
 * NFS I/O routines.  Just serve as frontends to kill off potential
 * requesters
 */

#define DEBUG 1

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include "nfsd.h"
#include "debug.h"

extern BPTR debug_fh ;

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
	DBS("nfs_io:nfs_lock: %s '\n", name) ;
	return lock;
}

BPTR nfs_open(char *name, LONG how)
{
	BPTR fh;

	NOREQUESTER;
	fh = Open(name, how);
	REQUESTER;
	DBS("nfs_io:nfs_open: %s\n", name) ;
	return fh;
}

BPTR nfs_createdir(char *path)
{
	BPTR rtn;

	NOREQUESTER;
	rtn = CreateDir(path);
	REQUESTER;
	DBS("nfs_io:nfs_createdir: %s\n", path) ;
	return rtn;
}

long nfs_wr(BPTR fh, void *buf, long len)
{
	long	rtn;
#ifdef DEBUG	
	char buffer[256] ;
#endif

	NOREQUESTER;
	rtn = Write(fh, buf, len);
	REQUESTER;
//	CopyMem(buf,buffer,len > 256L ? 255L : len) ;
//	*(buffer + 256) = 0 ;
//	DBS("nfs_io:nfs_write: %s\n",buffer) ;
	DB("nfs_io:nfs_write:\n") ;
	return rtn;
}

long nfs_rd(BPTR fh, void *buf, long len)
{
	long	rtn;
#ifdef DEBUG	
	char buffer[256] ;
#endif

	NOREQUESTER;
	rtn = Read(fh, buf, len);
	REQUESTER;
	DB("nfs_io:nfs_rd\n") ;
//	CopyMem(buffer, buf, len > 256 ? 256L : len) ;
//	*(buffer + 256) = 0 ;
//	DBS("nfs_io:nfs_read: %s\n",buffer) ;
	return rtn;
}

long nfs_seek(BPTR fh, long pos, long how)
{
	long	rtn;
#ifdef DEBUG	
	char buffer[256] ;
#endif

	NOREQUESTER;
	rtn = Seek(fh, pos, how);
	REQUESTER;
	NameFromFH(fh, buffer, 256L) ;
	DBS("nfs_io:nfs_seek: %s", buffer) ;
	return rtn;
}

BOOL nfs_examine(BPTR lk, struct FileInfoBlock *fib)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = Examine(lk, fib);
	REQUESTER;
	DB("nfs_io:nfs_examine: '\n") ;
	DB(fib->fib_FileName) ;
	DB("'\n") ;
	return rtn;
}

BOOL nfs_exnext(BPTR lk, struct FileInfoBlock *fib)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = ExNext(lk, fib);
	REQUESTER;
	DB("nfs_io:nfs_exnext:\n") ;
	DB(fib->fib_FileName) ;
	DB("\n") ;
	return rtn;
}

void nfs_unlock(BPTR lk)
{
#ifdef DEBUG
	char buffer[256] ;
#endif

	NOREQUESTER;
	UnLock(lk);
	DB("nfs_io:nfs_unlock\n") ;
	NameFromLock(lk, buffer, 256L) ;
	DB(buffer) ;
	DB("\n") ;
	REQUESTER;
}

BOOL nfs_close(BPTR fh)
{
	BOOL rtn;
#ifdef DEBUG	
	char buffer[256] ;
#endif
	
	NOREQUESTER;
	rtn = Close(fh);
	DB("nfs_io:nfs_close: \n") ;
	NameFromFH(fh, buffer, 256L) ;
	DB(buffer) ;
	DB("\n") ;
	REQUESTER;
	return rtn;
}

BOOL nfs_rename(char *from, char *to)
{
	BOOL rtn;

	NOREQUESTER;
	rtn = Rename(from, to);
	REQUESTER;
	DB("nfs_io:nfs_rename: from '\n") ;
	DB(from) ;
	DB("' to '") ;
	DB(to) ;
	DB("'\n") ;
	return rtn;
}

BOOL nfs_info(BPTR lk, struct InfoData *id)
{
	BOOL rtn;
	NOREQUESTER;
	rtn = Info(lk, id);
	REQUESTER;
	DB("nfs_io:nfs_info\n") ;
	return rtn;
}

void init_io()
{
	nfsd_pr = (struct Process *)FindTask(0L);
	DB("nfs_io:init_io\n") ;
}
