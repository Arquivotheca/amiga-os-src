/*
 * $Id: patch.c,v 40.1 93/06/07 16:51:15 mks Exp $
 *
 * $Log:	patch.c,v $
 * Revision 40.1  93/06/07  16:51:15  mks
 * *** empty log message ***
 * 
 */

/*
 * Magic patch to find workbench.library somewhere on the system and
 * to make calls to OpenLibrary("workbench.library") find it.
 */

#include	<exec/types.h>
#include	<exec/libraries.h>
#include	<exec/execbase.h>
#include	<exec/memory.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

extern	char	PatchData[];
extern	char	EndPatch[];

extern	ULONG	LVOOpenLibrary;

typedef	ULONG	(* __asm OpenLib)(register __a1 char *,register __d0 ULONG,register __a6 void *);

ULONG __asm Entry(register __a6 struct ExecBase *SysBase)
{
struct	Library	*DOSBase;

	if (DOSBase=OpenLibrary("dos.library",37))
	{
	char	*patch;
	ULONG	size=EndPatch-PatchData;
	BPTR	lock;

		/* Check for special case */
		if (lock=Lock("LIBS:workbench.library",ACCESS_READ)) UnLock(lock);
		else if (patch=AllocVec(EndPatch-PatchData+100,MEMF_PUBLIC|MEMF_CLEAR))
		{
		struct	DosList	*p;

			CopyMem(PatchData,patch,size);
			CacheClearU();

			p=LockDosList(LDF_VOLUMES | LDF_READ);
			while ((p=NextDosEntry(p,LDF_VOLUMES | LDF_READ)) && (!lock))
			{
			char	*bstr;
			char	*newWB;
			ULONG	len;

				newWB=&patch[size];
				bstr=BADDR(p->dol_Name);
				len=*bstr++;
				while (len--) *newWB++=*bstr++;
				bstr=":libs/workbench.library";
				while (*newWB++=*bstr++);

				/* Now check if this is it... */
				newWB=&patch[size];
				if (lock=Lock(newWB,ACCESS_READ)) UnLock(lock);
			}
			UnLockDosList(LDF_VOLUMES | LDF_READ);

			/*
			 * If we have a match, install the patch...
			 */
			if (lock)
			{
				Forbid();
				*((ULONG *)patch)=(ULONG)SetFunction(SysBase,(LONG)&LVOOpenLibrary,(VOID *)&patch[4]);
				Permit();
			}
			else FreeVec(patch);
		}
		CloseLibrary(DOSBase);
	}
	return(0);
}

ULONG __asm DoPatch(register __a0 char *patch, register __a1 char *name, register __d0 ULONG version, register __a6 struct ExecBase *SysBase)
{
ULONG	lib;
OpenLib	oldOpenLib;

	oldOpenLib=*(OpenLib *)((ULONG *)patch);
	if (!(lib=(*oldOpenLib)(name,version,SysBase)))
	{
	char	*newWB;

		newWB="workbench.library";
		while ((*name) && (*name==*newWB))
		{
			name++;
			newWB++;
		}
		if (*name==*newWB)
		{
			newWB=&patch[EndPatch-PatchData];
			lib=(*oldOpenLib)(newWB,version,SysBase);
		}
	}
	return(lib);
}
