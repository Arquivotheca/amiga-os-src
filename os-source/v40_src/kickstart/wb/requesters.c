/*
 * $Id: requesters.c,v 39.1 93/01/11 16:54:03 mks Exp $
 *
 * $Log:	requesters.c,v $
 * Revision 39.1  93/01/11  16:54:03  mks
 * Now supports a string filter for the RENAME requester...
 * No longer needs to check for ":" or "/" after the fact.
 * 
 * Revision 38.1  91/06/24  11:38:15  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "intuition/intuition.h"
#include "intuition/sghooks.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "quotes.h"

/* Function used as a diskname editing hook for a string gadget. It doesn't
 * allow : or / to be typed in.
 */
ULONG __asm NameHook(register __a0 struct Hook *hook,
                     register __a2 struct SGWork *sgw,
                     register __a1 ULONG *msg)
{
ULONG result;

    result = 0;
    if (*msg == SGH_KEY)
    {
        result = 1;
        if ((sgw->Code == ':') || (sgw->Code == '/'))
        {
            sgw->EditOp  = EO_BADFORMAT;
            sgw->Actions = SGA_BEEP;
        }
    }

    return (result);
}

RenameObject(obj, name)
struct WBObject *obj;
char *name;
{
struct WorkbenchBase *wb = getWbBase();
struct Hook nameHook;
BPTR holdlock = NULL;
STRPTR oldname;
int success, result = 0; /* assume no error */

    nameHook.h_Entry=NameHook;

    /* !!! check for read only file system */

    if (!obj) goto seterr;

    if (obj == wb->wb_RootObject)
    {
	DP(("RenameObject: cannot rename the root object\n"));
	goto seterr;
    }

    /*
	In order to be able to rename a left out icon the .backdrop file
	would have to be updated.  It was decided that this should be
	punted on for now since it is only two hours from final V2.0 lockdown.
    */
    if (obj->wo_LeftOut)
    {
	ErrorTitle(Quote(Q_RENAME_LEFTOUT));
	goto seterr;
    }

    /* prevent object from going away while in rename */
    holdlock = DUPLOCK(GetParentsLock(obj));

    oldname = obj->wo_Name;

    /*
     * If we are doing an interactive requester, do the following
     */
    if (!name)
    {
        strcpy(wb->wb_Buf0, obj->wo_Name);
        sprintf(wb->wb_Buf1, Quote(Q_RENAME_GREETING), obj->wo_Name);
        wb->wb_LastFreeMem = 0;

        success = SyncExecute(Quote(Q_RENAME_REQ_TITLE),
            wb->wb_Buf1, Quote(Q_RENAME_LABEL), wb->wb_Buf0,
            &wb->wb_IntuiPort,&nameHook);

	/*
	 * Kludge:  Don't let names over 25 char file (or 30 volume) name
	 */
	wb->wb_Buf0[((obj->wo_Type == WBDISK) ? 30 : 25)]='\0';

        if (!success)
        {
            ErrorTitle(Quote(Q_RENAME_FAILED)); /* rename failed */
        }
        else if (success < 0) goto end;
    }
    else strcpy(wb->wb_Buf0, name);

    /* update the object graphics */

    obj->wo_Name = objscopy(obj, wb->wb_Buf0);
    if (!obj->wo_Name) goto err;
    SetIconNames(obj);

    /* rename the sucker */
    if (!name)
    {
	if (CheckForType(obj,WBF_TOOL|WBF_PROJECT|WBF_GARBAGE|WBF_DRAWER))
	{
	    if (!WBRename(obj,oldname)) goto err;
	}
	else if (CheckForType(obj,WBF_DISK))
	{
	    if (!WBRenameDisk(obj,oldname)) goto err;
	}
    }

    /* Make sure the window is updated... */
    if (obj->wo_DrawerData) if (obj->wo_DrawerData->dd_DrawerWin) RedrawGauge(obj);

    /* force viewbytext imagery to get rebuilt */
    obj->wo_ImageSize = 0;

    /* and update the icon */
    if (obj->wo_Parent)
    {
	MakeNameGadget(obj, obj->wo_Parent->wo_DrawerData->dd_ViewModes);

	if (obj->wo_Parent->wo_DrawerData->dd_ViewModes > DDVM_BYICON)
	{
	    SortByText(obj->wo_Parent);
	}
	else RefreshDrawer(obj->wo_Parent);
    }
    else result = -1;

    /* clean up and go away */
end:
    SetIconNames(obj);
    UNLOCK(holdlock); /* rename is done with using this object */
    DP(("RenameObject: at end:, result=%ld\n", result));
    return(result);
err:
    DP(("RenameObject: at err:\n"));
    obj->wo_Name = oldname;
seterr:
    result = 1;
    goto end;
}
