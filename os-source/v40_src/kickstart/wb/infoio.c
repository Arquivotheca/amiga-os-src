/*
 * $Id: infoio.c,v 39.2 92/05/31 16:32:47 mks Exp $
 *
 * $Log:	infoio.c,v $
 * Revision 39.2  92/05/31  16:32:47  mks
 * Now checks to see if the icon still exists before it just
 * writes all over it.
 * 
 * Revision 39.1  92/05/31  01:51:35  mks
 * Now is the ASYNC, LVO based INFO...
 *
 * Revision 38.4  92/05/26  17:21:28  mks
 * Now uses the UpdateWorkbench code to update the icon after a save.
 * This makes the icon get reloaded and solves the problem with icons
 * that have tool types changed and thus "loose" memory until the icon
 * is released completely.  This also means that the old best-fit code
 * was able to go away.
 *
 * Revision 38.3  92/05/26  13:27:51  mks
 * Now calls UpdateWorkbench() to update the icon information
 *
 * Revision 38.2  92/04/27  14:23:38  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.1  91/06/24  11:36:24  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include <string.h>

#include <exec/memory.h>
#include <intuition/intuition.h>
#include "info.h"
#include "support.h"
#include "global.h"

#include "proto.h"

/*------------------------------------------------------------------------*/

void InfoSave(struct InfoTag *itag)
{
struct	DiskObject	*dobj=NULL;
struct	DiskObject	LocalDOBJ;
	char		**NewTools=NULL;

	LocalDOBJ=*(itag->it_Obj);
	if (!(itag->it_ObjFake))
	{
		dobj=&LocalDOBJ;

		/* Check if object is still there... */
		if (!SETPROTECTION(itag->it_NameInfo,FIBF_EXECUTE))
		{
			if (IOERR()==ERROR_OBJECT_NOT_FOUND) dobj=NULL;
		}
	}

	if (dobj)
	{
		if (itag->it_Stack) dobj->do_StackSize=(((StrInfo(itag->it_Stack)->LongInt)+3) >> 2) * 4;
		if (itag->it_DefaultTool) dobj->do_DefaultTool=StrInfo(itag->it_DefaultTool)->Buffer;
		if (itag->it_MaskType & MASK_TOOLTYPES)
		{
		struct	Node	*node;
			ULONG	size=4;

			for (node = HeadNode(&itag->it_ToolTypeList); node->ln_Succ; node = node->ln_Succ) size+=4;

			if (NewTools=ALLOCVEC(size,MEMF_PUBLIC))
			{
				size=0;
				for (node = HeadNode(&itag->it_ToolTypeList); node->ln_Succ; node = node->ln_Succ) NewTools[size++]=node->ln_Name;
				NewTools[size]=NULL;
				dobj->do_ToolTypes=NewTools;
			}
		}

		PutDiskObject(itag->it_Name,dobj);
		if (itag->it_Flags & IT_GOT_FIB) SETPROTECTION(itag->it_NameInfo,(itag->it_Protection & (FIBF_DELETE|FIBF_WRITE)) | FIBF_EXECUTE);

		FREEVEC(NewTools);
	}

	if (itag->it_Flags & IT_GOT_FIB)
	{
		SETPROTECTION(itag->it_Name,itag->it_Protection);
		if (itag->it_Comment) SETCOMMENT(itag->it_Name,StrInfo(itag->it_Comment)->Buffer);
	}
}
