/*
 * $Id: recurse.c,v 38.2 92/12/15 16:11:05 mks Exp $
 *
 * $Log:	recurse.c,v $
 * Revision 38.2  92/12/15  16:11:05  mks
 * Fixed the Enforcer hit/crash with deleting within a drawer
 * that was also being deleted.
 * 
 * Revision 38.1  91/06/24  11:38:11  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "macros.h"

#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "errorindex.h"
#include "support.h"
#include "quotes.h"


/*
 * This routine will change the name in the buffer to the "other" name.
 */
void MakeOtherName(char *name,char *other)
{
char *s;

	strcpy(other,name);
	if (s=suffix(other,InfoSuf)) *s='\0';
	else
	{
		strcat(other,InfoSuf);
		if (strlen(other)>30) other[0]='\0';
	}
}

/*
 * This routine will delete the file name given...
 */
LONG Kill1File(char *name)
{
struct	WBObject	*obj;
struct	NewDD		*dd;
	LONG		result=NULL;
	BPTR		lock;

	CheckDiskIO();	/* Empty out our ports... */

	DP(("Kill1File: [%s] - Enter\n",name));

	if (name[0])
	{
		CURRENTDIR(lock=CURRENTDIR(NULL));	/* Get the current directory */

		if (obj=MasterSearch(FindDupObj,lock,name,~ISDISKLIKE))
		{
			if (dd=obj->wo_DrawerData)
			{
				UNLOCK(dd->dd_Lock);
				dd->dd_Lock=NULL;
			}
		}

		if (!DELETEFILE(name))
		{
			result=IOERR();
			if (result==ERROR_OBJECT_NOT_FOUND) result=NULL;
		}

		if (obj)
		{
			if (result)
			{
				if (dd) dd->dd_Lock=LOCK(obj->wo_Name,ACCESS_READ);
			}
			else
			{
				/*
				 * If the object is on the backdrop, make
				 * it go away first...
				 */
				if (obj->wo_LeftOut) PutAway(obj,PUTAWAY_NODEL);

				FullRemoveObject(obj);
			}
		}
	}
	DP(("Kill1File: [%s] = %ld\n",name,result));

	return(result);
}

/* External */
/*
 * This routine will delete the file name given and it's "mate" if it
 * has one.
 */
LONG KillFile(char *name)
{
LONG	result;
char	other[128];

	MakeOtherName(name,other);

	result=Kill1File(name);
	if (!result) result=Kill1File(other);

	return(result);
}

#define	SKIP_OBJECT	(-1L)

/*
 * Routine to check if a file is deletable...
 * It will check both the icon and the file...
 * It will also allocate its own FIBs if you pass in a name...
 * If no name is passed in it will use the name from the first FIB...
 */
LONG CanDelete(char *name,struct FileInfoBlock *fib,struct FileInfoBlock *otherfib)
{
struct	FileInfoBlock	*myfib=NULL;
	LONG		result=NULL;
	BPTR		newlock;
	char		other[128];	/* Make sure it is big enough... */

	if (name)	/* Check if we are name based... */
	{
		if (myfib=ALLOCVEC(sizeof(struct FileInfoBlock),MEMF_PUBLIC|MEMF_CLEAR))
		{
			fib=otherfib=myfib;
			if (newlock=LOCK(name,ACCESS_READ))
			{
				if (!EXAMINE(newlock,fib)) result=IOERR();
				UNLOCK(newlock);
			}
			else result=IOERR();
		}
		else result=ERROR_NO_FREE_STORE;
	}
	else name=fib->fib_FileName;

	if (result==ERROR_OBJECT_NOT_FOUND) result=NULL;

	if (!result)
	{
		MakeOtherName(name,other);

		if (fib->fib_Protection & FIBF_DELETE) result=ERROR_DELETE_PROTECTED;
		else if (other[0])
		{
			if (newlock=LOCK(other,ACCESS_READ))
			{
				if (EXAMINE(newlock,otherfib))
				{
					if (otherfib->fib_DirEntryType>0) result=SKIP_OBJECT;
					else if (otherfib->fib_Protection & FIBF_DELETE) result=ERROR_DELETE_PROTECTED;
				}
				else result=IOERR();
				UNLOCK(newlock);
			}
			else result=IOERR();
		}
	}

	FREEVEC(myfib);

	if (result==ERROR_OBJECT_NOT_FOUND) result=NULL;
	return(result);
}

/*
 * This is the actual routine that does the delete...
 */
LONG KillDir(BPTR in_lock,struct FileInfoBlock *fib,struct FileInfoBlock *otherfib)
{
BPTR	tmpLock;
LONG	result=NULL;
char	*name=fib->fib_FileName;
BPTR	lock;
BPTR	newlock;
short	doOnce;

	tmpLock=CURRENTDIR(NULL);

	if (!(lock=DUPLOCK(in_lock))) result=IOERR();
	else if (EXAMINE(lock,fib))
	{
		CURRENTDIR(lock);	/* Move into the directory */
		DP(("KillDir: Enter = [%s]\n",fib->fib_FileName));
		if (fib->fib_DirEntryType>0)
		{
			doOnce=TRUE;
			while (doOnce && (!result))
			{
				doOnce=FALSE;
				while ((!result) && EXNEXT(lock,fib))
				{
					DP(("KillDir: EXNEXT=[%s]\n",name));

					result=CanDelete(NULL,fib,otherfib);

					if (!result)
					{
						/*
						 * Try to kill it what we have...
						 */
						result=KillFile(name);

						/*
						 * Check if it is a directory and if so, recurse down...
						 * We use the directory structure in the file system to
						 * do this...
						 */
						if ((result) && (fib->fib_DirEntryType>0))
						{
							if (newlock=LOCK(name,ACCESS_READ))
							{
								/*
								 * We now cause this whole thing to loop,
								 * and we save state in the directory
								 * structure itself!
								 */
								CURRENTDIR(newlock);
								UNLOCK(lock);
								lock=newlock;
								result=NULL;
							}
							else result=IOERR();
						}
					}

					DP(("KillDir: [%s] = %ld\n",name,result));

					if (result==SKIP_OBJECT) result=NULL;
					else if (result)
					{	/* We had a real error */
						SetResult2(result);
						ErrorDos(ERR_REMOVE,name);
					}
					else
					{	/*
						 * When we delete something, start
						 * the directory over again.
						 *
						 * This also happens when we enter
						 * a new directory...
						 */
						if (!EXAMINE(lock,fib)) result=IOERR();
					}
				}

				/* Get the directory name back again... */
				if (!result) if (!EXAMINE(lock,fib)) result=IOERR();

				/* Bounce back up a directory... */
				if (!result) if (SAMELOCK(in_lock,lock)!=LOCK_SAME)
				{
					doOnce=TRUE;	/* We need to come around again... */

					if (newlock=PARENTDIR(lock))
					{
						CURRENTDIR(newlock);
						UNLOCK(lock);
						lock=newlock;

						/*
						 * Kill old directory...
						 */
						if (!(result=KillFile(name)))
						{
							if (!EXAMINE(lock,fib)) result=IOERR();
						}
					}
					else result=IOERR();

					if (result)
					{	/* We had a real error */
						SetResult2(result);
						ErrorDos(ERR_REMOVE,name);
					}
				}
			}
		}
	}
	else result=IOERR();

	CURRENTDIR(tmpLock);		/* Restore old directory */
	UNLOCK(lock);			/* Free internal lock... */

	return(result);
}

/* External */
/*
 * This routine will return FLASE if it was able to empty a directory...
 * It will return IOERR() if it could not...
 * The LOCK should be on the directory.
 */
LONG KillDirectory(BPTR lock)
{
struct	FileInfoBlock	*fib;
	LONG		result;

	if (fib=ALLOCVEC(2 * sizeof(struct FileInfoBlock),MEMF_PUBLIC))
	{
		result=KillDir(lock,&fib[0],&fib[1]);
		FREEVEC(fib);
	}
	else result=ERROR_NO_FREE_STORE;

	return(result);
}

/* External */
EmptyTrash(struct WBObject *obj)
{
BPTR	lock;
int	result=ERROR_DIR_NOT_FOUND;

	MP(("EmptyTrash: enter, obj=$%lx (%s)\n", obj, obj->wo_Name));

	if (lock = GetParentsLock(obj))
	{
		/* this is a directory */
		CURRENTDIR(lock);

		if (lock=LOCK(obj->wo_Name,SHARED_LOCK))
		{
			result = KillDirectory(lock);
			UNLOCK(lock);
		}
		else result=IOERR();
	}

	MP(("EmptyTrash: exit, returning %ld\n", result));
	return(result);
}

/* External */
Discard(struct WBObject *obj)
{
struct	WorkbenchBase	*wb = getWbBase();
	int		result = 1; /* assume failure */

	if (obj != wb->wb_RootObject) if (MasterSearch(ObjOnList,obj))
	{
		if (CheckForNotType(obj,WINDOWCHANGE))
		{
			ErrorTitle(Quote(Q_CANNOT_DELETE),obj->wo_Name);
		}
		else
		{
		char	tmpname[128];
		BPTR	lock;

			CheckDiskIO();
			strcpy(tmpname,obj->wo_Name);
			if (lock=GetParentsLock(obj))
			{
				if (lock=DUPLOCK(lock))
				{
					lock=CURRENTDIR(lock);
					result=CanDelete(tmpname,NULL,NULL);
					if ((!result) || (result==SKIP_OBJECT))
					{
						result=KillFile(tmpname);
						if (result) if (MasterSearch(ObjOnList,obj))
						{
							lock=CURRENTDIR(lock);
							result=EmptyTrash(obj);
							lock=CURRENTDIR(lock);
							if ((!result) || (result == ERROR_OBJECT_NOT_FOUND))
							{
								result=KillFile(tmpname);
							}
						}
					}
					UNLOCK(CURRENTDIR(lock));
				}
				else result=IOERR();
			}
			else result=IOERR();

			if (result)
			{
				SetResult2(result);
				ErrorDos(ERR_REMOVE,tmpname);
			}
		}
	}

	return(result);
}
