/*
 * $Id: update.c,v 38.5 92/07/29 12:19:41 mks Exp $
 *
 * $Log:	update.c,v $
 * Revision 38.5  92/07/29  12:19:41  mks
 * Now correctly updates the icon and will free memory in the one
 * case that used to not free the memory...
 * 
 * Revision 38.4  92/06/29  11:47:45  mks
 * Changed so less SameLock packets are sent...
 *
 * Revision 38.3  92/06/22  14:11:17  mks
 * Fixed the UpdateWorkbench code so that the disk window will work right.
 *
 * Revision 38.2  92/04/27  14:22:49  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.1  91/06/24  11:39:10  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "graphics/text.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "intuition/intuition.h"
#include "macros.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

FindParent(obj, lock)
struct WBObject *obj;	/* potential parent (dir) */
BPTR lock;		/* lock on dir (parent) we are looking for */
{
    struct NewDD *dd;
    BPTR parlock;
    int result = NULL; /* assume no parent */

    DP(("FP: enter, obj=$%lx (%s), lock=$%lx\n", obj, obj->wo_Name, lock));
    if (dd = obj->wo_DrawerData)
    { /* object is a drawer */
	if (obj->wo_Type == WBDISK)
	{
	    parlock = MakeDiskLock(obj);
	    DP(("FP: lock ($%lx) comes from MakeDiskLock\n", parlock));
	}
	else
	{
	    parlock = dd->dd_Lock;
	    DP(("FP: lock ($%lx) comes from dd->dd_Lock\n", parlock));
	}

	if (SAMELOCK(parlock, lock) == LOCK_SAME)
	{	/* locks same */
	    DP(("\t\t\tMATCH FOUND!\n"));
	    result = 1; /* return ptr to object */
	}
    }
    DP(("FP: exit, returning $%lx\n", result));
    return(result);
}

CheckForChildOnBackdrop(obj, lock, name)
struct WBObject *obj;	/* potential parent (dir) (obj on backdrop) */
BPTR lock;		/* lock on dir (parent) we are looking for */
char *name;		/* name of file residing in dir 'lock' */
{
    int result = 0; /* assume no match */

    DP(("CFCOB: enter, obj=$%lx (%s), obj->wo_Lock=$%lx, lock=$%lx, name=%s\n",obj, obj->wo_Name, obj->wo_Lock, lock, name));
    if (obj->wo_Type != WBDISK)
    {
	if (stricmp(obj->wo_Name, name) == 0)
	{
	    if (SAMELOCK(obj->wo_Lock, lock) == LOCK_SAME)
	    {
		DP(("\t\t\tMATCH FOUND!\n"));
		result = 1; /* got a match */
	    }
	}
    }
    DP(("CFCOB: exit, returning $%lx\n", result));
    return(result);
}

void TransferObj(struct WBObject *old,struct WBObject *new)
{
struct	NewDD	*o_dd;
struct	NewDD	*n_dd;
	/*
	 * Check if we have an object that has a drawer connected...
	 */
	if (o_dd=old->wo_DrawerData)
	{
		if (n_dd=new->wo_DrawerData)
		{
		struct	Window	*win;

			/*
			 * Transfer the window data...
			 */
			n_dd->dd_NewWindow=o_dd->dd_NewWindow;
			n_dd->dd_CurrentX=o_dd->dd_CurrentX;
			n_dd->dd_CurrentY=o_dd->dd_CurrentY;
			n_dd->dd_Flags=o_dd->dd_Flags;
			n_dd->dd_ViewModes=o_dd->dd_ViewModes;

			if (win=o_dd->dd_DrawerWin)
			{
				n_dd->dd_NewWindow.LeftEdge = win->LeftEdge;
				n_dd->dd_NewWindow.TopEdge = win->TopEdge;
				n_dd->dd_NewWindow.Width = win->Width;
				n_dd->dd_NewWindow.Height = win->Height;
			}

			/*
			 * If we don't have a lock yet, transfer it...
			 */
			if (!(n_dd->dd_Lock))
			{
				n_dd->dd_Lock=o_dd->dd_Lock;
				o_dd->dd_Lock=NULL;
			}

			/*
			 * Make sure we have the gadgets set up...
			 */
			InitGadgets(n_dd);

			MP(("Old use count: %ld\n",old->wo_UseCount));

			SiblingSuccSearch(o_dd->dd_Children.lh_Head,MoveToLink,old,new);

			new->wo_DrawerOpen=old->wo_DrawerOpen;
			new->wo_DrawerSilent=old->wo_DrawerSilent;
			new->wo_Parent=NULL;

			new->wo_UseCount+=old->wo_UseCount-1;

			if (win)
			{
				old->wo_UseCount++;
				CloseDrawer(old);
				new->wo_DrawerOpen=1;
				PotionOpen(new);
			}

			MP(("New use count: %ld\n",new->wo_UseCount));
		}
	}
}

void UpdateParent(struct WBObject *parent)
{
    if (parent->wo_DrawerData->dd_DrawerWin)
    {
        /*
            if parent is in view by text mode then must refresh
            the entire drawer in order to display this icon.
        */
        if (parent->wo_DrawerData->dd_ViewModes > DDVM_BYICON)
        {
            DP(("IU: calling RefreshDrawer\n"));
            RefreshDrawer(parent);
        }
        else
        {
            DP(("IU: calling MinMaxDrawer\n"));
            MinMaxDrawer(parent); /* update scroll bars */
        }
    }
}

void IconUpdate(im)
struct IconMsg *im;
{
struct WorkbenchBase *wb = getWbBase();
struct WBObject *obj, *parent, *oldobj;
struct TypedIconMsg *tim;
struct ActiveDisk *ad;
char *name;
BOOL DiskIcon = FALSE; /* assume not trying to update a disk icon */
APTR oldwindow;

    /*
     * Shut off requesters for a bit...
     */
    oldwindow=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
    ((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)-1;

    tim = (struct TypedIconMsg *)(((WORD *)im)- 1);
    name = &im->im_name;
    DP(("IU: enter, im=$%lx, tim=$%lx\n", im, tim));
    DP(("\tdobj=$%lx\n", im->im_dobj));
    DP(("\tlock=$%lx\n", im->im_lock));
    DP(("\tname\'%s'\n", name));

    if (im->im_lock)
    {
        if (im->im_dobj)
        {
            /* We did a Put so add/update icon... */
            /* must get parent */
            if (!(parent = MasterSearch(FindParent, im->im_lock)))
            {
                DP(("IU: FindParent returned NULL\n"));
                if (parent = SiblingPredSearch(wb->wb_RootObject->wo_DrawerData->dd_Children.lh_TailPred,CheckForChildOnBackdrop, im->im_lock, name))
                {
                    /*
                        SiblingxxxxSearch returns either a ptr to the object
                        it last passed to the function or NULL.  It does
                        NOT return the return value of the function.  Thus
                        I want the object's parent pointer here since I
                        know the object is on the backdrop.  Actually, I
                        know that the parent is going to be wb->wb_RootObject
                        but this is more correct.
                    */
                    parent = parent->wo_Parent;
                }
            }

            if (parent)
            {
                CURRENTDIR(im->im_lock);
                obj=WBGetWBObject(name);

                if (obj) if (obj->wo_Type==WBDISK)
                {
                    if ((parent->wo_Type==WBDISK) && (stricmp(name,DiskIconName)==0))
                    {
                        DiskIcon = TRUE; /* trying to update a disk icon */
                        name = parent->wo_Name; /* get name of disk */
                        parent = parent->wo_Parent; /* get disk's parent */
                        obj->wo_Name=objscopy(obj,name);
                        SetIconNames(obj);
                    }
                    else obj->wo_Name=NULL;

                    if (!(obj->wo_Name))
                    {
                        WBFreeWBObject(obj);
                        obj=NULL;
                    }
                }

                if (obj)
                {
                    if (oldobj = MasterSearch(FindDupObj, im->im_lock, name, (obj->wo_Type==WBDISK) ? ISDISKLIKE : ~ISDISKLIKE ))
                    {
                        obj->wo_PutAway = oldobj->wo_PutAway; /* save PutAway status */

                        if (obj->wo_CurrentX == NO_ICON_POSITION)
                        {
                            /* inherit old position */
                            obj->wo_CurrentX = oldobj->wo_CurrentX;
                            obj->wo_CurrentY = oldobj->wo_CurrentY;
                            obj->wo_SaveX = oldobj->wo_SaveX;
                            obj->wo_SaveY = oldobj->wo_SaveY;
                        }

                        if (oldobj->wo_Background)
                        {
                            DP(("IU: old object was on background\n"));
                            /* flag that new object is also on background */
                            obj->wo_Background = 1;
                            obj->wo_LeftOut=oldobj->wo_LeftOut;
                            /* give the new object a lock to its true parent */
                            obj->wo_Lock = DUPLOCK(im->im_lock);
                            parent = wb->wb_RootObject; /* parent is root not dir obj lives in */
                        }

                        if (DiskIcon)
                        {
                            if (ad = ObjectToActive(oldobj))
                            {
                            struct ActiveDisk *newad;

                                if (newad = (struct ActiveDisk *)ALLOCMEM(sizeof(struct ActiveDisk), MEMF_PUBLIC))
                                {
                                    /* Add the AD object to the action WB object... */
                                    AddFreeList(&obj->wo_FreeList, newad, sizeof(struct ActiveDisk));

                                    DP(("IU: copying ad to newad, adding newad, removing old ad\n"));
                                    memcpy(newad,ad,sizeof(struct ActiveDisk));
                                    newad->ad_Object=obj;

                                    ad->ad_Object=NULL;
                                    ad->ad_Node.ln_Name=ad->ad_Name=NULL;
                                    RemoveActiveDisk(ad); /* remove old */

                                    TransferObj(oldobj,obj);
                                    ADDTAIL(&wb->wb_ActiveDisks, (struct Node *)newad); /* add new */
                                }
                                else goto cancelobj;
                            }
                            else goto cancelobj;
                        }
                        else TransferObj(oldobj,obj);
                    }
                    else if (DiskIcon) goto cancelobj;

                    DP(("IU: calling AddIcon\n"));
                    if ((parent->wo_DrawerData->dd_DrawerWin) || (parent->wo_DrawerOpen))
                    {
			if (MasterSearch(ObjOnList,oldobj)) FullRemoveObject(oldobj);
                        DP(("IU: calling AddIcon\n"));
                        AddIcon(obj, parent);
                        UpdateParent(parent);
                        obj=NULL;
                    }

		    /* If we still have an object, free it */
cancelobj:	    WBFreeWBObject(obj);
                }
            }
        }
        else if (oldobj=MasterSearch(FindDupObj,im->im_lock,name,~ISDISKLIKE))
        {    /* We did a Delete so find and remove the object... */
            parent=oldobj->wo_Parent;
	    if (MasterSearch(ObjOnList,oldobj)) FullRemoveObject(oldobj);
	    UpdateParent(parent);
        }
    }

bailout:
    DP(("IU: unlocking $%lx\n", im->im_lock));
    UNLOCK(im->im_lock);

    FREEVEC(tim);

    /*
     * Restore requesters...
     */
    ((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwindow;

    DP(("IU: exit\n"));
}

/*
*****i* workbench.library/UpdateWorkbench ************************************
*
*   NAME
*	UpdateWorkbench - Tell workbench of a new or deleted icon.       (V37)
*
*   SYNOPSIS
*	UpdateWorkbench(name, parentlock, flag)
*	                A0    A1          D0
*
*	VOID UpdateWorkbench(char *, BPTR, BOOL);
*
*   FUNCTION
*	This function does the "magic" of letting workbench know that
*	an object has been added, changed, or removed.  The name is
*	the name of the object, the lock is a lock on the directory that
*	contains the object.  The flag determines what has happened.
*	If TRUE, the object is either NEW or CHANGED.  If FALSE, the
*	object is deleted.
*
*   INPUTS
*	name - Name of the object (without the .info)
*
*	parentlock - Lock on the object's parent directory.
*
*	flag - TRUE for a new or changed object
*	       FALSE for a deleted object
*
*   RESULTS
*	Workbench will update its display, if needed.
*
*   NOTES
*	Note that saying that a DISK icon has been delete will not do
*	much as disk icons must continue to be visible.  Thus, this
*	is currently a NO-OP.  At some future date (maybe) it will change
*	the disk icon to the default.
*
*   SEE ALSO
*
*   BUGS
*
******************************************************************************
*/
VOID UpdateWorkbench(char *filename,BPTR parentlock,BOOL PutFlag)
{
struct WorkbenchBase *wb = getWbBase();
struct TypedIconMsg *tim;
struct IconMsg *im;

    /* Check if workbench is ready for this... */
    if ((wb->wb_WorkbenchStarted) && (!(wb->wb_Quit)))
    {
        /* allocate memory for msg and filename */
        if (tim = (struct TypedIconMsg *)ALLOCVEC(sizeof(struct TypedIconMsg) + strlen(filename) + 1, MEMF_CLEAR|MEMF_PUBLIC))
        {
            /* fill in msg */
            tim->tim_type = MTYPE_ICONPUT;
            im = &tim->tim_IconMsg;
            im->im_dobj = PutFlag;
            im->im_lock = DUPLOCK(parentlock);
            strcpy(&im->im_name, filename);

            DP(("name = '%s'\n",filename));
            DP(("filelock=$%lx, parentlock=$%lx\n",filelock,parentlock));
            DP(("tim_type = %ld\n", tim->tim_type));
            DP(("im_lock = $%lx\n", im->im_lock));
            DP(("im_name = '%s'\n", &im->im_name));

            DP(("calling PutMsg($%lx, $%lx)...",&wb->wb_WorkbenchPort, im));

            /* send msg */
            PUTMSG(&wb->wb_WorkbenchPort, im);
            DP(("ok\n"));

            /* wb will free mem for msg */
            tim=NULL;
        }

        /* If we did not send the message, free it */
        FREEVEC(tim);
    }

    DP(("UW: exit\n"));
}
