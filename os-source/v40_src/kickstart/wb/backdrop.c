/*
 * $Id: backdrop.c,v 38.2 92/04/27 14:22:31 mks Exp $
 *
 * $Log:	backdrop.c,v $
 * Revision 38.2  92/04/27  14:22:31  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 * 
 * Revision 38.1  91/06/24  11:33:53  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbenchbase.h"
#include "global.h"
#include "quotes.h"
#include "support.h"

#define BACKDROPFILE	".backdrop"
#define TMPBACKDROPFILE	"~backdrop"

#define	BUF_SIZE	260

/*
 * External functions:
 *
 * void FillBackdrop(BPTR)		- Load the backdrop for this volume
 * int LeaveOut(struct WBObject *)	- LeaveOut this object
 * int PutAway(struct WBObject *,LONG)	- PutAway this object
 */

/*
 * This routine trims off the volume name of a path...
 */
void TrimPath(char *name)
{
register	char	*p;

	p=name;
	while (*p)
	{
		p++;
		if (*p==':')
		{
			strcpy(name,p);
			p=name;
		}
	}
}

/*
 * Code to set the protection bits of the backdrop file
 * (Without causing a requester...
 */
void ProtectBackdrop(void)
{
struct	WorkbenchBase	*wb=getWbBase();
APTR	oldwin;

	oldwin=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
	((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)(-1);     /* No requesters */

	SETPROTECTION(BACKDROPFILE,FIBF_EXECUTE);

	((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwin;
}

/*
 * Code to open a backdrop file...
 */
BPTR OpenBackdrop(long Mode)
{
	ProtectBackdrop();
	return(OPEN_fh(BACKDROPFILE,Mode));
}

/*
 * This routine closes and sets the protection bits on the backdrop file
 */
void CloseBackdrop(BPTR fh)
{
	CLOSE_fh(fh);
	ProtectBackdrop();
}

void FillBackdrop(BPTR volumelock)
{
struct	WorkbenchBase	*wb=getWbBase();
register	char	*name;
register	char	*path;
register	short	tmp;
register	BPTR	fh;
		BPTR	olddir;
		BPTR	tmplock;
		char	buf[BUF_SIZE];
		char	tmpc;

	/* Make sure we really have a lock... */
	if (volumelock)
	{
		/* Make sure we are at this volume... */
		olddir=CURRENTDIR(volumelock);

		if (fh=OpenBackdrop(MODE_OLDFILE))
		{
			while (FGets(fh,name=buf,BUF_SIZE))
			{
				TrimPath(name);
				tmp=strlen(name)-1;
				if (name[tmp]==10) name[tmp]=0;
				path=PATHPART(name);

				tmpc=*path;
				*path=0;
				tmplock=LOCK(name,ACCESS_READ);
				*path=tmpc;
				if (tmplock)
				{
				struct	WBObject	*obj;

					name=FILEPART(name);

					/*
					 * Make sure we don't have doubles...
					 */
					if (!MasterSearch(FindDupObj,tmplock,name,WINDOWCHANGE))
					{
						tmplock=CURRENTDIR(tmplock);
						obj=WBGetWBObject(name);
						tmplock=CURRENTDIR(tmplock);

						if (obj)
						{
							obj->wo_Lock=tmplock;
							obj->wo_Background=1;
							obj->wo_LeftOut=1;
							AddIcon(obj,wb->wb_RootObject);
							tmplock=NULL;
						}
					}

					/* UnLock() checks for NULL... */
					UNLOCK(tmplock);
				}
			}
			CloseBackdrop(fh);
		}
		CURRENTDIR(olddir);	/* Return home... */
	}
}

/*
 * This routine builds the icon path into buf
 */
BOOL BuildBackdropPath(struct WBObject *obj,char *buf)
{
BOOL	result=FALSE;

	if (NameFromLock(GetParentsLock(obj),buf,BUF_SIZE-(strlen(obj->wo_Name)+1)))
	{
		TrimPath(buf);
		ADDPART(buf,obj->wo_Name,BUF_SIZE);
		result=TRUE;
	}
	return(result);
}

int LeaveOut(struct WBObject *obj,long autoPut)
{
struct	WorkbenchBase	*wb=getWbBase();
	int	result=0;
	char	buf[BUF_SIZE];

	if (obj!=wb->wb_RootObject)
	{
		result=1;
		if (CheckForNotType(obj,WINDOWCHANGE) || (obj->wo_FakeIcon) || (!BuildBackdropPath(obj,buf)))
		{
			ErrorTitle(Quote(Q_CANNOT_LEAVE_OUT));
		}
		else if (obj->wo_LeftOut) ErrorTitle(Quote(Q_ALREADY_LEFT_OUT));
		else
		{
		BPTR	tmpdir;
		BPTR	rootlock;

			tmpdir=CURRENTDIR(GetParentsLock(obj));
			if (rootlock=LOCK(":",ACCESS_READ))
			{
			BPTR	fh;

				rootlock=CURRENTDIR(rootlock);
				if (fh=OpenBackdrop(MODE_OLDFILE)) SEEK_fh(fh,0,OFFSET_END);
				else fh=OpenBackdrop(MODE_NEWFILE);

				if (fh)
				{
				long	len;
				long	oldpos;

					/* Make sure there is a LF at the end */
					strcat(buf,"\n");
					len=strlen(buf);

					oldpos=SEEK_fh(fh,0,OFFSET_CURRENT);

					if (WRITE_fh(fh,buf,len)!=len)
					{	/* we had a write error! */
						SetFileSize(fh,oldpos,OFFSET_BEGINNING);
						ErrorTitle(Quote(Q_CANNOT_LEAVE_OUT));
					}
					else
					{
						if (autoPut!=PUTAWAY_NODEL) ObjMove(obj,obj->wo_Parent,wb->wb_RootObject,NO_ICON_POSITION,NO_ICON_POSITION);
						obj->wo_LeftOut=1;
						result=0;
					}
					CloseBackdrop(fh);
				}

				rootlock=CURRENTDIR(rootlock);
				UNLOCK(rootlock);
			}
			CURRENTDIR(tmpdir);
		}
	}
	return(result);
}

int ReturnObjectHome(struct WBObject *drawer,struct WBObject *obj)
{
int	result=FALSE;

	if (CheckForType(drawer,ISDRAWER))
	{
	struct	NewDD	*dd;

		if (dd=drawer->wo_DrawerData)
		{
			if ((drawer->wo_DrawerOpen) || (dd->dd_DrawerWin))
			{
				LockDrawer(drawer);
				if (SAMELOCK(dd->dd_Lock,obj->wo_Lock)==LOCK_SAME)
				{
					ObjMove(obj,obj->wo_Parent,drawer,NO_ICON_POSITION,NO_ICON_POSITION);
					result=TRUE;
				}
			}
		}
	}
	return(result);
}

void FullRemoveObject(struct WBObject *obj)
{
	if (obj->wo_DrawerData)
	{
		SiblingSuccSearch(obj->wo_DrawerData->dd_Children.lh_Head,RemoveIconsLoop);
	}
	RemoveObject(obj);
}

/*
 * This function returns the object or just throws it away if it can't be
 * returned.
 */
void PutObjectBack(struct WBObject *obj)
{
	if (!MasterSearch(ReturnObjectHome,obj)) FullRemoveObject(obj);
}

/*
 * This routine cleans up the buffer, removing volume names, and
 * the match name(s) if it finds them.
 */
void CleanUpBuffer(char *buffer,char *match)
{
register	char	*start;
register	char	*end;
		char	tmpch;

	start=buffer;
	while (*start)
	{
		end=start;
		while (*end && (*end!=10)) end++;
		tmpch=*end;
		*end=NULL;
		TrimPath(start);

		if (stricmp(start,match))
		{
			while (*start) start++;
			*start=tmpch;
		}
		else
		{	/* We found the match */
			start--;
			if (!tmpch) end--;
		}

		/* Check if we need to move the stuff down... */
		if (*start)
		{
			start++;
			end++;
			strcpy(start,end);
		}
	}
}

int PutAway(struct WBObject *obj,long autoPut)
{
struct	WorkbenchBase	*wb=getWbBase();
	int	result=0;
	char	buf[BUF_SIZE];

	if (obj!=wb->wb_RootObject)
	{
		if (autoPut==PUTAWAY_QUICK) FullRemoveObject(obj);
		else if (CheckForNotType(obj,WINDOWCHANGE) || (!obj->wo_LeftOut && !obj->wo_Background)) result=1;
		else if (!obj->wo_LeftOut) PutObjectBack(obj);
		else if (!BuildBackdropPath(obj,buf)) result=1;
		else
		{
		BPTR	rootdir;
		BPTR	tmpdir;

			tmpdir=CURRENTDIR(GetParentsLock(obj));
			if (rootdir=LOCK(":",ACCESS_READ))
			{
			BPTR	fh;

				rootdir=CURRENTDIR(rootdir);

				if (fh=OpenBackdrop(MODE_OLDFILE))
				{
				long	size;
				char	*buffer;

					result=1;
					SEEK_fh(fh,0,OFFSET_END);
					size=SEEK_fh(fh,0,OFFSET_BEGINNING);
					if (buffer=ALLOCVEC(size+1,MEMF_PUBLIC|MEMF_CLEAR))
					{
						if (READ_fh(fh,buffer,size)==size)
						{
							CloseBackdrop(fh);
							CleanUpBuffer(buffer,buf);
							size=strlen(buffer);	/* Find new size */
							/*
							 * Now check if we need to write again
							 */
							if (RENAME(BACKDROPFILE,TMPBACKDROPFILE))
							{
								if (size)
								{
									if (fh=OpenBackdrop(MODE_NEWFILE))
									{
										if (WRITE_fh(fh,buffer,size)==size) result=0;
										CloseBackdrop(fh);
										if (result) DELETEFILE(BACKDROPFILE);
									}
								}
								else result=0;
								size=0;
							}
							else size=(IOERR()==ERROR_DISK_WRITE_PROTECTED);

							/*
							 * What we did here was set the size to 0 if
							 * we should do some more work here...
							 * Under write protected cases, this would have
							 * caused two more requesters...
							 */
							if (!size)
							{
								if (result) RENAME(TMPBACKDROPFILE,BACKDROPFILE);
								DELETEFILE(TMPBACKDROPFILE);
							}
							fh=NULL;
						}
						FREEVEC(buffer);
					}
					if (fh) CloseBackdrop(fh);
				}

				UNLOCK(CURRENTDIR(rootdir));
			}
			CURRENTDIR(tmpdir);

			/*
			 * If things still look good, we return the object home
			 */
			if (!result)
			{
				obj->wo_LeftOut=0;
				if (autoPut!=PUTAWAY_NODEL) PutObjectBack(obj);
			}
		}
	}

	if (result) ErrorTitle(Quote(Q_CANNOT_PUT_AWAY));
	return(result);
}
