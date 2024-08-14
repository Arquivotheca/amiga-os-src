
/*
** chmod(name, unixmask)
*/

#include <dos/dosextens.h>
#include <exec/ports.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <nfs/perm.h>
#include <action.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
extern struct DosLibrary *DOSBase;

#include <errno.h>



#define ALIGNLONG(p) (((long)p) + 4 - (((long)p) & 3))

int
chmod( register char *name , int unixmask )
{
	register char	*np, *cp;
	char	namebuf[256 + 4];
	struct MsgPort *dev;
	u_long	result, dos_packet(), amigamask, parent, lock;
	char	*tail, *basename();

	cp = (char *)ALIGNLONG(namebuf);
	if((lock = Lock(name, MODE_OLDFILE)) == 0)
		return(-1);

	parent = ParentDir(lock);

	tail = basename ( name );
	np = &cp[1];
	while(*tail)
		*np++ = *tail++;
	cp[0] = np - &cp[1];
	*np++ = 0;
	dev = btod(lock, struct FileLock *)->fl_Task;
	result = DoPkt(dev, ACTION_CHMOD, 0L, parent, dtob(cp),unixmask,0L);
	if(result==0 && IoErr()==ERROR_ACTION_NOT_KNOWN){
		amigamask = 0;
		if((unixmask & 0400)==0)
			amigamask |= FIBF_READ;
		if((unixmask & 0200)==0)
			amigamask |= FIBF_WRITE;
		if((unixmask & 0100)==0)
			amigamask |= FIBF_EXECUTE;
		result = DoPkt(dev, ACTION_SET_PROTECT, 0, parent,dtob(cp), amigamask,0L);
		if( result == 0 ) {
			errno = ENOENT ;
	/*		_OSERR = ERROR_OBJECT_NOT_FOUND ; */
			return( -1 ) ;
		}
	}
	UnLock(lock);
	UnLock(parent);
	return(0);
}

