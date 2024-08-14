/*
 * $Id: icon_def.c,v 38.2 91/11/08 14:08:43 mks Exp $
 *
 * $Log:	icon_def.c,v $
 * Revision 38.2  91/11/08  14:08:43  mks
 * Autodoc cleanup
 * 
 * Revision 38.1  91/06/24  19:01:32  mks
 * Changed to V38 source tree - Trimmed Log
 *
 */

#include <exec/types.h>
#include <workbench/workbench.h>
#include <dos/dosextens.h>

#include <string.h>

#include "support.h"
#include "internal.h"

/* Proto's for our functions */

/* External INTERNAL DiskObject's:  icon_def_images.c */
extern struct DiskObject disk, drawer, tool, project;
extern struct DiskObject trashcan, device, kick;

UBYTE *def_read_path = "ENV:sys/def_";
UBYTE *def_write_path = "ENVARC:sys/def_";

UBYTE *def_name[] = {
    "disk",	/* name for disk image */
    "drawer",	/* name for drawer image */
    "tool",	/* name for tool image */
    "project",	/* name for project image */
    "trashcan",	/* name for trashcan (garbage) image */
    NULL,	/* name for device image (not used) */
    "kick",	/* name for kickstart disk image */
};

struct DiskObject *def_dob[] = {
    &disk,	/* defaultimage for a disk */
    &drawer,	/* default image for a drawer */
    &tool,	/* default image for a tool */
    &project,	/* default image for a project */
    &trashcan,	/* default image for the trashcan */
    NULL,	/* default image for a device (not used) */
    &kick,	/* default image for a kickstart disk */
};

/*
******* icon.library/GetDefDiskObject ***************************************
*
*   NAME
*	GetDefDiskObject - read default wb disk object from disk.       (V36)
*
*   SYNOPSIS
*	diskobj = GetDefDiskObject(def_type)
*         D0                          D0
*
*	struct DiskObject *GetDefDiskObject(LONG);
*
*   FUNCTION
*	This routine reads in a default Workbench disk object from disk.
*	The valid def_types can be found in workbench/workbench.h and
*	currently include WBDISK thru WBGARBAGE.  If the call fails,
*	it will return zero.  The reason for the failure may be obtained
*	via IoErr().
*
*	Using this routine protects you from any future changes to
*	the way default icons are stored within the system.
*
*   INPUTS
*	def_type - default icon type (WBDISK thru WBKICK).  Note that the
*		   define 'WBDEVICE' is not currently supported.
*
*   RESULTS
*	diskobj -- the default Workbench disk object in question
*
*   SEE ALSO
*	PutDefDiskObject
*
*   BUGS
*	None
*
*****************************************************************************
*/
struct DiskObject *IGetDefDiskObject(LONG def_type)
{
struct DiskObject *dob = NULL;
struct Process *pr;
APTR oldwindowptr;
UBYTE icon_name[32];

    DP(("GDDO: enter, def_type=$%lx\n", def_type));

    if (def_type >= WBDISK && def_type < WBAPPICON)
    {
	/* prevent error messages */
	pr = (struct Process *)FindTask(NULL);
	oldwindowptr = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR)-1;

	/* Attempt to get the default icon from ENV:sys/ */
	strcpy (icon_name, def_read_path);
	strcat (icon_name, def_name[(def_type - 1)]);
	DP(("GDDO: icon_name='%s'\n", icon_name));
	if (dob = GetDiskObjectStub(icon_name))
	{
		/* Now, check that the type matches... */
		if (dob->do_Type!=def_type)
		{
			/* The type did not match, so reject it */
			FreeDiskObjectStub(dob);
			dob=NULL;
		}
	}
	if (!dob)
	{
	    /* We didn't get the default, so let's return
	     * a FreeDiskObject()-able DiskObject pointer
	     * obtained from one of our internal images.
	     */
	    if (dob=IAllocateDiskObject())
	    {
	    	if (!IGetIconCommon(icon_name,dob,(struct FreeList *)&dob[1],def_dob[(def_type-1)],36))
	    	{
	    	    DP(("GDDO: Could not get default object\n"));
	    	    FreeDiskObjectStub(dob);
	    	    dob=NULL;
	    	}
	    }
	    else
	    {
	    	DP(("GDDO: could not allocate disk object\n"));
	    }
	}

#ifdef DEBUGGING
	else
	{
	    DP(("GDDO: got default icon from disk\n"));
	}
#endif DEBUGGING

	/* re-enable error messages */
	pr->pr_WindowPtr = oldwindowptr;
    }

#ifdef DEBUGGING
    else
    {
	DP(("GDDO: wrong def_type!\n"));
    }
#endif DEBUGGING

    /*
     * Make sure that default icons have no position.
     */
    if (dob)
    {
	dob->do_CurrentX=NO_ICON_POSITION;
	dob->do_CurrentY=NO_ICON_POSITION;
    }

    return (dob);
}

/*
******* icon.library/PutDefDiskObject ***************************************
*
*   NAME
*	PutDefDiskObject - write disk object as the default for its type.  (V36)
*
*   SYNOPSIS
*       status = PutDefDiskObject(diskobj)
*         D0                        A0
*
*	BOOL PutDefDiskObject(struct DiskObject *);
*
*   FUNCTION
*	This routine writes out a DiskObject structure, and its
*	associated information.  If the call fails, a zero will
*	be returned.  The reason for the failure may be obtained
*	via IoErr().
*
*	Note that this function calls PutDiskObject internally which means
*	that this call (if sucessful) notifies workbench than an icon has
*	been created/modified.
*
*	Using this routine protects you from any future changes to
*	the way default icons are stored within the system.
*
*   INPUTS
*	diskobj -- a pointer to a DiskObject
*
*   RESULTS
*	status -- TRUE if the call succeeded else FALSE
*
*   SEE ALSO
*	GetDefDiskObject
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IPutDefDiskObject(struct DiskObject *dob)
{
    struct Process *pr;
    APTR oldwindowptr;
    BOOL ret_val = FALSE;
    UBYTE icon_name[32];
    UBYTE def_type = dob->do_Type;

    DP(("PDDO: enter, obj=$%lx\n", dob));

    if (def_type >= WBDISK && def_type < WBAPPICON)
    {
	/* prevent error messages */
	pr = (struct Process *)FindTask(NULL);
	oldwindowptr = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR)-1;

	/* Attempt to put the default icon into ENV:sys/ */
	strcpy (icon_name, def_read_path);
	strcat (icon_name, def_name[(def_type - 1)]);
	DP(("PDDO: icon_name='%s'\n", icon_name));
	/* PutDiskObject and PutIcon are the same */
	/* We don't care if this one works... */
	PutIconStub(icon_name, dob);

	/* Attempt to put the default icon into ENVARC:sys/ */
	strcpy (icon_name, def_write_path);
	strcat (icon_name, def_name[(def_type - 1)]);
	DP(("PDDO: icon_name='%s'\n", icon_name));
	/* PutDiskObject and PutIcon are the same */
	if (PutIconStub(icon_name, dob))
	{
	    ret_val = TRUE;
	}
	else
	{
	    DP(("PDDO: could not PutIcon\n"));
	    ret_val = FALSE;
	}

	/* re-enable error messages */
	pr->pr_WindowPtr = oldwindowptr;
    }

#ifdef DEBUGGING
    else
    {
	DP(("PDDO: wrong def_type\n"));
    }
#endif DEBUGGING

    return (ret_val);
}
