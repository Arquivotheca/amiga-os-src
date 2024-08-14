/* -----------------------------------------------------------------------
 * unixprotect.c (chmod)  SAS 5.10a
 *
 * $Locker:  $
 *
 * $Id: unixprotect.c,v 1.3 92/11/24 15:56:38 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: Hog:Other/inet/src/c/chmod/RCS/unixprotect.c,v 1.3 92/11/24 15:56:38 bj Exp $
 *
 *------------------------------------------------------------------------
 */

// #define DEBUG 1


/*
** SetUnixProt(name, unixmask)
*/
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/ports.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <nfs/perm.h>
#include <action.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "chmod.h"

u_long GetUnixProtect(char *name, long *maskp, struct glob *g);
u_long SetUnixProtect(char *name, long unixmask, struct glob *g);

u_long SetUnixProtect(char *name, long unixmask, struct glob *g)
{
	REGISTER char *np;
	REGISTER char *cp;
	REGISTER struct MsgPort *dev;
	REGISTER char  *tail;
	REGISTER struct DosLibrary *DOSBase = g->g_DOSBase ;
	u_long amigamask, parent, lock;
	LONG  result = 20L ;
	long  error ;

	if(lock = Lock(name, MODE_OLDFILE))
	{
		if(cp = AllocMem(256L, MEMF_CLEAR))
		{
			parent = ParentDir(lock) ;

			tail = FilePart( name ) ;
			np = &cp[1];
			while(*tail)
			{
				*np++ = *tail++;
			}
			cp[0] = np - &cp[1];
			*np++ = 0;
			dev = btod(lock, struct FileLock *)->fl_Task;
			result = DoPkt(dev, ACTION_CHMOD, 0, parent, dtob(cp), unixmask,0);
            error = IoErr() ;
            DBp2("SUP: result = %ld error = %ld\n", result, error) ;
            
			if((result==0) && (error ==ERROR_ACTION_NOT_KNOWN))
			{
				amigamask = 0;
				if((unixmask & 0400)==0)
					amigamask |= FIBF_READ;
				if((unixmask & 0200)==0)
					amigamask |= FIBF_WRITE;
				if((unixmask & 0100)==0)
					amigamask |= FIBF_EXECUTE;
				result = DoPkt(dev, ACTION_SET_PROTECT, 0, parent, 
						dtob(cp), amigamask,0);
			}
			UnLock(parent);
			FreeMem(cp, 256L) ;
		}
		UnLock(lock);
	}
#ifdef DEBUG	
	else
	{
		DB("SUP: Lock failed\n") ;
	}
#endif	
	return((u_long)result) ;
}

u_long GetUnixProtect(char *name, long *maskp, struct glob *g)
{
	REGISTER u_long mode ;
	REGISTER struct DosLibrary *DOSBase = g->g_DOSBase ;
	REGISTER struct FileInfoBlock *fib;
	struct fattr fa;
	struct MsgPort *dev;
	u_long	result, lock ;

	if((lock = Lock(name, MODE_OLDFILE)) == 0)
	{
		return(0);
	}
	if(fib = AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR))
	{
		dev = btod(lock, struct FileLock *)->fl_Task;
		result = DoPkt(dev, ACTION_EX_OBJECT, lock, dtob(fib), (LONG)&fa,0,0);
		if(result==0 && IoErr()==ERROR_ACTION_NOT_KNOWN)
		{
			result = DoPkt(dev, ACTION_EXAMINE_OBJECT, lock, dtob(fib),0,0,0);
			if(fib->fib_EntryType > 0)
				mode = NFSMODE_DIR|0700;
			else
				mode = NFSMODE_REG|0700;
			if(fib->fib_Protection & FIBF_READ)
				mode &= ~0400;
			if(fib->fib_Protection & FIBF_WRITE)
				mode &= ~0200;
			if(fib->fib_Protection & FIBF_EXECUTE)
				mode &= ~0100;
			*maskp = mode;
		} 
		else
		{
			*maskp = fa.mode & 06777;
		}
		FreeMem(fib, sizeof(struct FileInfoBlock)) ;
	}
	UnLock(lock);
	return(result);
}
	
