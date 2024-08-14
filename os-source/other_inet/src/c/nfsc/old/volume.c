/* -----------------------------------------------------------------------
 * VOLUME.c (NFSC)  SAS
 *
 * $Locker:  $
 *
 * $Id: volume.c,v 1.6 92/12/07 16:58:11 bj Exp $
 *
 * $Revision: 1.6 $
 *
 * $Header: Hog:Other/inet/src/c/nfsc/RCS/volume.c,v 1.6 92/12/07 16:58:11 bj Exp $
 *
 *------------------------------------------------------------------------
 */

//#define DEBUG 1

/*
 * Amiga volume handling
 */
#include "fs.h"

#include <exec/ports.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>

extern struct mount_pt *nfs_mtlist;

/*
 * voltomt - convert given volume name to mount point.
 */
struct mount_pt *
voltomt(char *volume)
{
	register struct mount_pt *mt;
	int len, mt_len ;

	len = strlen(volume);
	if(len == 0)
	{
		return 0;
	}

	if( *(volume + (len - 1)) == ':' )
	{
		len-- ;
	}
	for(mt = nfs_mtlist; mt; mt = mt->mt_next)
	{
		mt_len = (int)(*(btod(mt->mt_vol->dol_Name, UBYTE *))) ;
#ifdef DEBUG
		kprintf("len = %ld  mtlen = %ld\n", len, (long)(*(btod(mt->mt_vol->dol_Name, UBYTE *)))) ;
		kprintf("nfsc: voltomt: list name = %s  volume = %s\n", btod(mt->mt_vol->dol_Name,char *)+1, volume) ;
#endif
		if( mt_len == len )
		{
			if(strnicmp(btod(mt->mt_vol->dol_Name, char *)+1, volume, mt_len) == 0)
			{
				break;
			}
		}
	}
	
	return mt;
}


/*
 * Delete specified volume from DOS info chain.
 */
void deletevol(struct DosList *dl)
{
	LockDosList(LDF_VOLUMES|LDF_WRITE);
	RemDosEntry(dl);
	UnLockDosList(LDF_VOLUMES|LDF_WRITE);
	FreeDosEntry(dl);
}

/*
 * volname is considered valid iff it contains no slashes, is of non zero
 * length and is not in use.
 */
int validvol(char *volname)
{
	struct Process *pr;
	char vname[MAXVOL];
	APTR Old;
	int len;

	if((len = strlen(volname)) > MAXVOL-2){
		len = MAXVOL-2;
	}
	if(len == 0 || strchr(volname, '/') != NULL){
		return EBADVOL;
	}

	strncpy(vname, volname, len);
	vname[len] = 0;
	strcat(vname, ":");
	pr = (struct Process *)FindTask(0L);
	Old = pr->pr_WindowPtr;

	pr->pr_WindowPtr = (APTR)-1L;
	if(DeviceProc(vname)){
		pr->pr_WindowPtr = Old;
		return EBUSY;
	}
	pr->pr_WindowPtr = Old;

	return 0;
}

/*
 * Insert volume name into DOS volume list
 */

struct mount_pt *insert_vname(char *volname)
{
	struct mount_pt	*mt;
	struct DosList *dl;

	dl = MakeDosEntry(volname,DLT_VOLUME);

	mt = (struct mount_pt *)AllocMem(sizeof(*mt),MEMF_PUBLIC|MEMF_CLEAR);
	if(!mt){
		return (NULL);
	}

	mt->mt_port = CreateMyPort();
	dl->dol_Task = mt->mt_lock.fd_lock.fl_Task = mt->mt_port;
	dl->dol_Lock = dtob(&mt->mt_lock);
	mt->mt_vol = dl;

	if (!AddDosEntry(dl)) {
		FreeDosEntry(dl);
		FreeMem(mt,sizeof(*mt));
		return (NULL);
	}

	return (mt);
}

/*
 * Rename disk volume to dp->dp_Arg1
 */
void nfs_rename_dsk (mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	u_char	*p, *newname;
	u_int	len;

	p = btod(dp->dp_Arg1, u_char *);
	len = *p++;

	newname = (u_char *)AllocMem(len + 2, MEMF_PUBLIC);
	if(!newname){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}

	Forbid();

	bcopy(p, newname + 1, len);
	newname[len + 1] = 0;		/* just to be safe */

/*	if(p = (u_char *)index(newname+1, ':')){
		*p = 0;
		len--;
	}
*/

	newname[0] = len;
	p = btod(mt->mt_vol->dol_Name, unsigned char *);
	mt->mt_vol->dol_Name = (BSTR)dtob(newname);

	FreeMem(p,p[0]+2);

	Permit();

	dp->dp_Res1 = DOS_TRUE;
}

