/*
 * $Id: drawers.c,v 38.6 92/04/27 14:23:20 mks Exp $
 *
 * $Log:	drawers.c,v $
 * Revision 38.6  92/04/27  14:23:20  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 * 
 * Revision 38.5  92/02/13  14:21:07  mks
 * Fixed the propgadget display problem.
 * Cleaned up some code
 *
 * Revision 38.4  92/01/28  11:48:16  mks
 * Some more MinMax cleanup...
 *
 * Revision 38.3  92/01/28  11:41:24  mks
 * Cleaned up some code on the drawer MinMax and Prop setting...
 *
 * Revision 38.2  92/01/07  13:55:38  mks
 * Removed the text initialization code from this module and put
 * it with the rest of the prefs code
 *
 * Revision 38.1  91/06/24  11:34:57  mks
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
#include "errorindex.h"
#include "support.h"
#include "fontenv.h"
#include "quotes.h"

struct InfoNode
{
struct	Node		node;
	long		prot;
	long		size;
struct	DateStamp	date;
	long		type;
	char		name[1];
};

OpenDrawer(drawer)
struct WBObject *drawer;
{
    struct WorkbenchBase *wb = getWbBase();
    int result=1;

    wb->wb_LastCloseWindow=NULL;
    MP(("OpenDrawer: enter, drawer=%lx (%s)\n", drawer, drawer->wo_Name));
    if (drawer!=wb->wb_RootObject)
    {
	drawer->wo_UseCount++; /* bump use count */
	result=OpenCommon(drawer, drawer->wo_DrawerData->dd_Flags, FALSE);
    }
    MP(("OpenDrawer: exit, result=%ld\n", result));
    return(result);
}

LockDrawer(struct WBObject *drawerobj)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd = drawerobj->wo_DrawerData;
LONG lock=NULL;

    if (drawerobj == wb->wb_RootObject)
    {
	lock=1;
    }
    else if (dd)
    {
	if (!(lock=dd->dd_Lock))
	{
	    if (drawerobj->wo_Type==WBDISK) dd->dd_Lock=MakeDiskLock(drawerobj);
	    else
	    {
		if (lock=GetParentsLock(drawerobj))
		{
		    CURRENTDIR(lock);
		    dd->dd_Lock=LOCK(drawerobj->wo_Name,SHARED_LOCK);
		}
	    }
	    lock=dd->dd_Lock;
	}
	CURRENTDIR(lock);
    }

    return(lock);
}

FindDupObj(struct WBObject *obj, BPTR fl, char *name,ULONG type)
{
struct WorkbenchBase *wb = getWbBase();
BPTR lock;
int result = FALSE; /* assume failure */

    MP(("FindDupObj: enter, obj=$%lx (%s), fl=$%lx, name=%s\n",obj, obj->wo_Name, fl, name));
    /* if object is not the root object  && 'fl' is valid */
    if (obj != wb->wb_RootObject && fl)
    {
	/* Check that it matches our type needs... */
        if (CheckForType(obj,type))
        {
            /* if the object has a valid lock */
            if (lock = GetParentsLock(obj))
            {
		if (!stricmp(obj->wo_Name,name))
		{
                    /*
                     * NOTE:  To really check if they are the same
                     * object, we should just use the object's lock
                     * and get a lock on the name and see if they are
                     * the same.  String compairs are much faster
                     * but may not be correct for the file system.
                     * (such as case problems...)
                     */

		    /* Now check the directory parent locks... */
                    if (SAMELOCK(fl,lock)==LOCK_SAME) result=TRUE;
                }
            }
        }
    }
    MP(("FindDupObj: exit, result=$%lx\n", result));
    return(result);
}

BOOL AddChildCommon(BPTR lock,char *name,struct WBObject *parent,struct WBObject *child,struct FileInfoBlock *fib)
{
struct	WorkbenchBase	*wb = getWbBase();
struct	WBObject	*obj;
	BOOL		result=TRUE;	/* Assume it works... */

    MP(("ACC: enter, lock=%lx\n", lock));
    MP(("\t: name = %lx, (%s)\n", name, (name ? name : "NULL")));
    MP(("\t: parent = %lx (%s)\n", parent, (parent ? parent->wo_Name : "NULL")));
    MP(("\t: child = %lx (%s)\n", child, child ? child->wo_Name : "NULL"));

    CheckDiskIO();

    if (obj = MasterSearch(FindDupObj, lock, name,~ISDISKLIKE))
    {
	/*
	    The Object we are interested in already exists.
	    This could happen in the following cases:
		- the object was a drawer, had it's parent closed and then
		  re-opened.
		- the object was moved to the workbench (disk) window,
		  had its parent closed and then re-opened.
		- we are doing a 'See All Files' and are trying to add
		  this icon again (we obviously should not).
	    If the object is in the workbench (disk) window, leave him be.
	    If we already have the object (ie. the child) we want to use
	    the new child and FreeWBObject on the old obj.
	    If not, we want to use (re-attach) this object instead of
	    calling GetWBObject to create it.
	*/
	if (child)
	{
	    MP(("ACC: RemoveObject on old object %lx (%s)\n", obj, obj->wo_Name));
	    RemoveObject(obj); /* remove old object from system */
	    obj=child;
	}
	else if (obj->wo_Parent == wb->wb_RootObject)
	{
	    /* he is in the backdrop.  Leave him be... */
	    MP(( "ACC: passing on backdrop icon, obj=%lx (%s)\n",obj, obj->wo_Name));
	    obj=NULL;
	}
	else
	{
            /* We will reattach it... */
            MP(( "ACC: got a re-attach match for '%s'\n", obj->wo_Name));

            /* take this icon out of the ozone... */
            MP(("ACC: freeing this objects lock\n"));
            UNLOCK(obj->wo_Lock);
            obj->wo_Lock=NULL;
            if (obj->wo_Parent==parent) obj=NULL;
	}
    }
    else if (!(obj = child))
    {
        if (obj = NewWBGetWBObject(name,fib))
        {
        /*
         * kludge:  This gets the real file size from the noninfolist
	 * if that file was already seen.  If it was not, it gets handled
	 * later in the game.
	 */
	struct InfoNode *ln;

		if (ln=(struct InfoNode *)SiblingSuccSearch(wb->wb_NonInfoList.lh_Head,OnList,name))
		{
	                obj->wo_FileSize = ln->size;
	                obj->wo_Protection = ln->prot;
	                obj->wo_DateStamp = ln->date;
		}
        }
        else
        {
            MP(("ACC: GetWBObject failed (IoErr %ld)\n", IOERR()));
            result=FALSE;
        }
    }

    if (obj)
    {
	if (obj->wo_Type==WBDISK)
	{
	    MP(("ACC: FreeWBObject on %lx (%s), rejecting a disk\n",obj, obj->wo_Name));
	    WBFreeWBObject(obj);
	}
	else if (obj->wo_FakeIcon && !(parent->wo_DrawerData->dd_Flags & DDFLAGS_SHOWALL))
	{
	    MP(("ACC: freeing %lx (%s) as it is a fake icon!\n",obj, obj->wo_Name));
	    WBFreeWBObject(obj);
	}
	else
	{
	    /* process unplaced icons LAST */
	    if (obj->wo_CurrentX == NO_ICON_POSITION)
	    {
		AlphaEnqueue(&wb->wb_UtilityList, &obj->wo_UtilityNode);
	    }
	    else
	    {
                MP(("ACC: calling AddIcon, obj=%lx (%s), parent=%lx (%s)\n",obj, obj->wo_Name, parent, parent->wo_Name));
                AddIcon(obj, parent);
            }
	}
    }

    MP(("ACC: exit\n"));
    return(result);
}

FindSameName(struct WBObject *obj,char *name)
{
	return(!stricmp(obj->wo_Name,name));
}

/* this routine is called if we dont' have a valid ".info" icon cache.
** It is called once for each entry in the directory.  It passes all
** files that end in ".info" on to AddChildCommon()
** This routine also implements the 'See All Files' option which allows WB
** to display files which do not have a .info file associated with them.
*/

BOOL AddChild(BPTR lock,char *name,struct WBObject *parent,struct FileInfoBlock *fib)
{
struct	WorkbenchBase	*wb=getWbBase();
	BOOL		result=TRUE;
struct	NewDD		*dd=parent->wo_DrawerData;
	char		*s;
	char		oldc;

    MP(("AddChild: name='%s', parent=%lx (%s)\n",name, parent, parent->wo_Name));

    /* if does not end in '.info', do this */
    if (!(s = suffix(name, InfoSuf)))
    {
    struct WBObject *obj;

        obj=SiblingSuccSearch(wb->wb_UtilityList.lh_Head,FindSameName,name);
        if (!obj) obj=SiblingSuccSearch(dd->dd_Children.lh_Head,FindSameName,name);
        if ((fib) && (obj))
        {
            obj->wo_FileSize = fib->fib_Size;
            obj->wo_Protection = fib->fib_Protection;
            obj->wo_DateStamp = fib->fib_Date;
        }
        else InfoListsCommon(&wb->wb_NonInfoList,name,fib,&wb->wb_InfoList);
    }
    else if (s!=name) /* Don't bother with ".info" */
    {
        oldc = *s;
        *s = '\0'; /* replace '.' with a null */
        result=AddChildCommon(lock,name,parent,NULL,fib);
        *s = oldc; /* restore '.' */

        /* save name on info list */
        if (result)
        {
            InfoListsCommon(&wb->wb_InfoList,name,fib,NULL);
            /* remove name from noninfolist (if its on there) */
            RemoveDupInfoEntry(&wb->wb_NonInfoList,&wb->wb_InfoList,name);
        }
    }
    return(result);
}

BOOL AddAllChildren(BPTR lock,struct FileInfoBlock *fib,struct WBObject *parent,BOOL *alldone)
{
struct	WorkbenchBase *wb = getWbBase();
BPTR	oldlock;
BOOL	result=FALSE;

    if (lock)
    {
    	oldlock=CURRENTDIR(lock);

	if (EXAMINE(lock,fib))
	{
	    if (fib->fib_DirEntryType>0)
	    {
		*alldone=result=TRUE;
		while (EXNEXT(lock,fib) && result)
		{
		    /*
		     * Make sure this only stays true if all .info files
		     * are ok.
		     */
		    *alldone&=AddChild(lock,fib->fib_FileName,parent,fib);
		    result=(wb->wb_LastCloseWindow!=parent->wo_DrawerData->dd_DrawerWin);
		}
	    }
	}
	else
	{
	    ErrorDos(ERR_EXAM_DIR,NULL);
	}

	CURRENTDIR(oldlock);
    }
    return(result);
}


void FreeVecList(struct List *list)
{
struct	Node	*ln;

	while (ln=RemHead(list)) FREEVEC(ln);
}

int LocalAddIcon(struct WBObject *obj, struct WBObject *obj2)
{
struct WorkbenchBase *wb = getWbBase();

	AddIcon(obj,obj2);
	CheckDiskIO();
	return(wb->wb_LastCloseWindow == obj2->wo_DrawerData->dd_DrawerWin);
}

OpenCommon(drawerobj, flags, rescan)
struct WBObject *drawerobj;
int flags;
BOOL rescan;
{
struct WorkbenchBase *wb = getWbBase();
struct FileInfoBlock *fib;
struct NewDD *dd;
int result = 0;
BOOL writeobj=FALSE;
BOOL showall;

    if (!LockDrawer(drawerobj))
    {
	MP(("OpenCommon: Warning, could not LockDrawer!  returning 0\n"));
    }
    else if (!(fib = ALLOCVEC(sizeof(struct FileInfoBlock), MEMF_PUBLIC)))
    {
    	MP(("OpenCommon: Warning, could not allocate FIB...\n"));
    }
    else
    {
	BeginDiskIO();
        NewList( &wb->wb_UtilityList );

        dd = drawerobj->wo_DrawerData;

        MP(("OpenCommon: enter, name='%s', ViewModes=%lx, Flags=%lx\n",drawerobj->wo_Name, dd->dd_ViewModes, dd->dd_Flags));

        if (!EXAMINE(dd->dd_Lock, fib))
        {
            ErrorDos( ERR_EXAM, drawerobj->wo_Name );
        }
        else if (fib->fib_DirEntryType>0)
        {
            { /* scan directory and build list */
                showall = dd->dd_Flags & DDFLAGS_SHOWALL; /* get dir showall status */
                result = AddAllChildren(dd->dd_Lock,fib,drawerobj,&writeobj);
            }

	    if (result)
	    {
                /* now place all the unassigned icons */
                result=(!SiblingSuccSearch( wb->wb_UtilityList.lh_Head, LocalAddIcon, drawerobj ));

		if (result)
		{
                    /* now place all the non info file icons */
                    if (showall)
                    {
                        result=(!SiblingSuccSearch(wb->wb_NonInfoList.lh_Head, CreateIconFromNode,drawerobj));
                    }

		    if (result)
		    {
                        /* ***** temp. kludge for non ViewByIcon ***** */
                        if (dd->dd_ViewModes > DDVM_BYICON)
                        {
                            MP(("OpenCommon: calling RefershDrawer\n"));
                            RefreshDrawer(drawerobj);
                        }
		    }
		}
            }

        }
        else
        {
            /* this is not a directory */
            ErrorTitle(Quote(Q_DRW_IS_NOT_A_DIR));
        }

	EndDiskIO();
        FREEVEC(fib);
    }

    FreeVecList(&wb->wb_InfoList);
    FreeVecList(&wb->wb_NonInfoList);

    MP(("Open Common: exit, returning $%lx\n", result));
    return( result );
}

CloseChild(obj, drawer)
struct WBObject *obj, *drawer;
{
    struct NewDD *dd = obj->wo_DrawerData;

    MP(("CloseChild: removing $%lx (%s) 's siblings\n",
	obj, obj->wo_Name));
    MP(("\tdrawer = $%lx (%s)\n",
	drawer, drawer->wo_Name));
    Remove(&obj->wo_Siblings);

    --drawer->wo_UseCount;

#ifdef DEBUGGING
    MP(("CloseChild: decrementing %s's UseCount to %ld.\n",
	obj->wo_Name, obj->wo_UseCount - 1));
#endif DEBUGGING

    if (--obj->wo_UseCount == 0) {
	if (dd) UNLOCK(dd->dd_Lock);	/* UNLOCK checks for NULL */
	UNLOCK(obj->wo_Lock);

	MP(("CloseChild: FreeWBObject on $%lx (%s)\n",
	    obj, obj->wo_Name));
	WBFreeWBObject(obj);
    }
    else {
	MP(("CloseChild: obj=%lx, '%s', count=%ld\n",
	    obj, obj->wo_Name, obj->wo_UseCount));
	if (!obj->wo_Lock) {
	    obj->wo_Lock = DUPLOCK(GetParentsLock(obj));
	}
	obj->wo_Parent = 0; /* object has no parent */
	obj->wo_IconWin = NULL; /* object has no window */
    }

    return(0);
}

#define PARENT_CHILD


#ifdef PARENT_CHILD
NoIconWin(obj)
struct WBObject *obj;
{
	obj->wo_IconWin = NULL;
	return(NULL);
}

CheckForOpenChildren(obj)
struct WBObject *obj;
{
    return(obj->wo_UseCount > 1);
}

#endif PARENT_CHILD

void CloseDrawer(drawer)
struct WBObject *drawer;
{
    struct WBObject *parent;
    struct NewDD *dd;
    BOOL keep_going = TRUE;

    MP(("CloseDrawer: enter, drawer=$%lx (%s)\n",
	drawer, drawer->wo_Name));
    do {
	if (!(dd = drawer->wo_DrawerData)) {
	    MP(("CloseDrawer: ERROR, null dd!\n"));
	    return;
	}
	parent = drawer->wo_Parent;
	MP(("0: UseCount=%ld\n", drawer->wo_UseCount));
	if (dd->dd_DrawerWin) { /* if drawer has a window */
	    MP(("\tdrawer $%lx (%s) has a window, closing it\n",
		drawer, drawer->wo_Name));
	    /* figure out our current position */
	    UpdateWindowSize(drawer);
	    /* tell intuition to stop sending messages to us */
	    ClosePWindow(dd->dd_DrawerWin, dd);
	    dd->dd_DrawerWin = NULL; /* this drawer no longer has a window */
	}
	MP(("1: UseCount=%ld\n", drawer->wo_UseCount));
	/* if drawer has any open children */
	if (SiblingSuccSearch(dd->dd_Children.lh_Head, CheckForOpenChildren)) {
	    MP(("\tdrawer $%lx (%s) has open children, stopping\n",
		drawer, drawer->wo_Name));
	    drawer->wo_DrawerOpen = 1; /* drawer is open */
	    drawer->wo_DrawerSilent = 1; /* drawer is silent */
	    /* all the children have no output window */
	    SiblingSuccSearch(dd->dd_Children.lh_Head, NoIconWin);
	    keep_going = FALSE; /* stop (drawer has open children) */
	}
	else { /* no open children, try to really close drawer */
	    MP(("\tdrawer $%lx (%s) has no open children, trying to close it\n",
		drawer, drawer->wo_Name));
	    MP(("2: UseCount=%ld\n", drawer->wo_UseCount));
	    SiblingSuccSearch(dd->dd_Children.lh_Head, CloseChild, drawer);
	    MP(("3: UseCount=%ld\n", drawer->wo_UseCount));
	    if (--drawer->wo_UseCount == 0) {
		MP(("\tRemovingObject $%lx (%s), count=%ld\n",
		    drawer, drawer->wo_Name, drawer->wo_UseCount));
		RemoveObject(drawer);
	    }
	    else {
		MP(("CloseDrawer: can't free drawer.  drawer=%lx, '%s', count=%ld\n",
		    drawer, drawer->wo_Name, drawer->wo_UseCount));
		/* if we have no children then give up the lock */
		UNLOCK(dd->dd_Lock);
		dd->dd_Lock = 0;

		PhantomLockCleanup(drawer);
		if (drawer->wo_IconWin) keep_going = FALSE;
		drawer->wo_DrawerOpen = 0; /* drawer is closed */
	    }
	}
	MP(("4: UseCount=%ld\n", drawer->wo_UseCount));

	/*
	 * mks:  If there is no parent, we don't keep going...
	 */
	if (!(drawer = parent)) /* parent is now the drawer */
	{
	    keep_going=FALSE;
	    MP(("\tbottom, drawer=$%lx (%s), DrawerWin=$%lx, DrawerOpen=%lx\n",
	        drawer, drawer->wo_Name, drawer->wo_DrawerData->dd_DrawerWin,
	        drawer->wo_DrawerOpen));
	}
#ifdef	DEBUGGING
	else
	{
	    MP(("\tbottom, no parent...\n"));
	}
#endif

    } while (keep_going);
    MP(("CloseDrawer: exit\n"));
}

int MinMaxChild(struct WBObject *obj,struct WBObject *drawer)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd = drawer->wo_DrawerData;
LONG curx, cury, textDeltaX;

    /* Check to see if we are in a "sane" location (if needed) */
    IconBoxBackdrop(obj,drawer);

    if ((curx = obj->wo_CurrentX) == NO_ICON_POSITION) curx = 0;
    if ((cury = obj->wo_CurrentY) == NO_ICON_POSITION) cury = 0;

    /* take the text string into account -- delta is how much the
     * name extends beyond the gadget box.  See MakeNameGadget for
     * the setting of wo_NameXOffset...
     */
    textDeltaX = - wb->wb_EmboseBorderLeft - (obj->wo_NameXOffset-1);
    if (textDeltaX < 1) textDeltaX=1;

    if (dd->dd_MinX > curx - textDeltaX) dd->dd_MinX = curx - textDeltaX;
    if (dd->dd_MinY > cury) dd->dd_MinY = cury;

    curx += obj->wo_Gadget.Width;
    cury += obj->wo_Gadget.Height;

    if (dd->dd_ViewModes <= DDVM_BYICON)
    {
	curx += textDeltaX + EMBOSEWIDTH;
	cury += wb->wb_IconFontHeight + EMBOSEHEIGHT + 1;
    }
    else curx += wb->wb_EmboseBorderLeft + TextLength(&wb->wb_TextRast, obj->wo_NameBuffer, strlen(obj->wo_NameBuffer));

    if (dd->dd_MaxX < curx) dd->dd_MaxX = curx;
    if (dd->dd_MaxY < cury) dd->dd_MaxY = cury;

    return(NULL);
}

void MinMaxDrawer(struct WBObject *drawer)
{
struct NewDD *dd = drawer->wo_DrawerData;
struct Window *win = dd->dd_DrawerWin;
int winwidth, winheight;

    if (win)
    {
        winwidth = win->Width - win->BorderLeft - win->BorderRight;
        winheight = win->Height - win->BorderTop - win->BorderBottom;

        dd->dd_MinX = dd->dd_CurrentX;
        dd->dd_MinY = dd->dd_CurrentY;
        dd->dd_MaxX = dd->dd_CurrentX + winwidth;
        dd->dd_MaxY = dd->dd_CurrentY + winheight;

        SiblingSuccSearch(dd->dd_Children.lh_Head, MinMaxChild, drawer);

	if (win->FirstGadget)
	{
	    MinMaxProp(win, &dd->dd_HorizScroll, &dd->dd_HorizProp, dd->dd_CurrentX,dd->dd_MinX, dd->dd_MaxX, winwidth, 1);
	    MinMaxProp(win, &dd->dd_VertScroll, &dd->dd_VertProp, dd->dd_CurrentY,dd->dd_MinY, dd->dd_MaxY, winheight, 0);
	}
    }
}

void MinMaxProp(win, gad, prop, current, min, max, size, horizFlag)
struct Window *win;
struct Gadget *gad;
struct PropInfo *prop;
LONG current, min, max, size;
int horizFlag;
{
    ULONG mintomax;
    UWORD pot, body;
    int diff;

    MP(("MinMaxProp: enter, %s\n", horizFlag ? "HORIZ" : "VERT"));
    MP(("\tcurrent=%ld ($%lx), min=%ld ($%lx), max=%ld ($%lx)\n",
	current, current, min, min, max, max));
    MP(("\tsize=%ld ($%lx), horizFlag=%ld\n", size, size, horizFlag));

    /* compute the body size.  don't let the body get bigger than the box */
    mintomax = max - min;
    body = (MAXBODY * size) / mintomax;
    MP(("\tbody = (%ld * %ld) / (%ld - %ld)\n",MAXBODY, size, max, min));

    /* now do the pot position */
    diff = current - min;

    if (diff < 0) diff = 0;

    if (mintomax == size) {
	MP(("\tpot = %ld / 2\n", MAXPOT));
	pot = MAXPOT / 2;
    }
    else {
	MP(("\tpot = (%ld * %ld) / (%ld - %ld)\n",MAXPOT, diff, mintomax, size));
	pot = (MAXPOT * diff) / (mintomax - size);
    }

    NewModifyProp(gad, win, NULL, prop->Flags, pot, pot, body, body, 1);
    MP(("MinMaxProp: exit\n"));
}

ComputeMaxTextWidth(obj)
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    int i;

    MP(("CMTW: obj=$%lx (%s), max was %ld, ",
	obj, obj->wo_Name, wb->wb_MaxTextWidth));
    if ((i = TextLength(&wb->wb_TextRast, obj->wo_Name, strlen(obj->wo_Name))) > wb->wb_MaxTextWidth) {
	wb->wb_MaxTextWidth = i;
    }
    MP(("is %ld\n", wb->wb_MaxTextWidth));
    return(0);
}

InsureTextAlignment(obj, viewmode)
struct WBObject *obj;
int viewmode;
{
    MakeNameGadget(obj, viewmode);
    growNameImage(obj);
    return(0);
}

int RefreshDrawer(struct WBObject *drawer)
{
    struct WorkbenchBase *wb = getWbBase();
    struct Window *win;
    struct NewDD *dd;

    dd = drawer->wo_DrawerData;

    MP(("RefreshDrawer: enter, drawer=$%lx (%s), dd=$%lx, win=$%lx\n",drawer, drawer->wo_Name, dd,dd->dd_DrawerWin));

    if (dd) if (win = dd->dd_DrawerWin)
    {
	/* if in view by text mode, calc maximum width name */
	if (dd->dd_ViewModes > DDVM_BYICON)
	{
	    wb->wb_MaxTextWidth = 0;
	    SiblingPredSearch(dd->dd_Children.lh_TailPred, ComputeMaxTextWidth);
	    SiblingPredSearch(dd->dd_Children.lh_TailPred, InsureTextAlignment, dd->dd_ViewModes);
	}
	else
	{ /* must MinMaxDrawer first for viewbyicon */
	    MP(("RefreshDrawer: calling MinMaxDrawer\n"));
	    MinMaxDrawer(drawer);
	}
	MP(("RefreshDrawer: calling BeginIconUpdate\n"));
	BeginIconUpdate(drawer, win, TRUE);
	MP(("RefreshDrawer: calling ClearDrawerWindow\n"));
	ClearDrawerWindow(dd);
	MP(("RefreshDrawer: calling SPS(RenderIcon)\n"));
	SiblingPredSearch(dd->dd_Children.lh_TailPred, RenderIcon);
	MP(("RefreshDrawer: calling EndIconUpdate\n"));
	EndIconUpdate(win);
	/* must MinMaxDrawer last for viewbytext */
	if (dd->dd_ViewModes > DDVM_BYICON)
	{
	    MP(("RefreshDrawer: calling MinMaxDrawer\n"));
	    MinMaxDrawer(drawer);
	}
    }
    MP(("RefreshDrawer: exiting\n"));
    return(NULL);
}

/*----------------------------------------------------------------------*/
/* Routines added for 'See All Files' support */

/* create a node and add to list along with name */
void InfoListsCommon(struct List *list,char *name,struct FileInfoBlock *fib,struct List *otherlist)
{
struct InfoNode *ln=NULL;

	MP(("InfoListsCommon: enter, name='%s'", name));

	if (otherlist)
	{
		ln=(struct InfoNode *)SiblingSuccSearch(otherlist->lh_Head,OnList,name);
	}

	if (!ln)
	{
		if (ln=ALLOCVEC(strlen(name) + sizeof(struct InfoNode), MEMF_CLEAR|MEMF_PUBLIC))
		{
			MP(("ILC: calling AddHead\n"));
			strcpy(ln->name, name);
			ln->node.ln_Name=ln->name;
			AlphaEnqueue(list,&(ln->node));
		}
	}

	if (ln)
	{
		if (fib)
		{
			ln->size = fib->fib_Size;
			ln->prot = fib->fib_Protection;
			ln->date = fib->fib_Date;
			ln->type = fib->fib_DirEntryType;
		}
	}
	MP(("InfoListsCommon: exit\n"));
}

/* if names are the same, remove entry from list */
void RemoveDupInfoEntry(struct List *list,struct List *otherlist,char *name)
{
char *s;
struct InfoNode *ln;
struct InfoNode *oln;

	if (s=suffix(name,InfoSuf)) *s='\0';

	if (ln=(struct InfoNode *)SiblingSuccSearch(list->lh_Head,OnList,name))
	{
		if (oln=(struct InfoNode *)SiblingSuccSearch(otherlist->lh_Head,OnList,name))
		{
			oln->size=ln->size;
			oln->prot=ln->prot;
			oln->date=ln->date;
			oln->type=ln->type;
		}
		REMOVE(ln);
	}

	if (s) *s='.';

	FREEVEC(ln);
}

/* return status of whether names are the same */
OnList(ln, name)
struct Node *ln;	/* from infolist (a name with '.info' appended) */
char *name;		/* a name */
{
int result;
char *s;

	if (s=suffix(ln->ln_Name,InfoSuf)) *s='\0';

	result=(stricmp(name,ln->ln_Name) == 0);

	if (s) *s='.';

	return(result); /* terminate search if match found */
}

/*
	Create an icon from a name and add it to the system.
	This routine is used by the 'Copy' and
	the 'Show All Files' logic.  The file exists but an
	icon must be created for it since the file has no
	corresponding .info file.
*/

struct WBObject *CreateIconFromNameFib(char *name,struct WBObject *parent,BPTR fromlock,struct FileInfoBlock *fib)
{
struct WorkbenchBase *wb = getWbBase();
struct WBObject *obj;
int error=FALSE;

	if (obj=WBAllocWBObject(name))
	{
		obj->wo_Protection = fib->fib_Protection;
		obj->wo_FileSize = fib->fib_Size;
		obj->wo_DateStamp = fib->fib_Date;
		obj->wo_Type=WBTOOL;

		if (fib->fib_DirEntryType>0)
		{
			obj->wo_Type=WBDRAWER;
		}
		else if (fib->fib_Protection & FIBF_EXECUTE)
		{
			obj->wo_Type=WBPROJECT;
		}

		if (!error && initgadget2(obj))
		{
			obj->wo_FakeIcon=1;
			if (parent==wb->wb_RootObject)
			{
				obj->wo_Background=1;
				obj->wo_Lock=DUPLOCK(fromlock);
			}
		}
		else
		{
			WBFreeWBObject(obj);
			obj=NULL;
		}
	}
	return(obj);
}

struct WBObject *CreateIconFromName(char *name,struct WBObject *parent,BPTR fromlock)
{
struct WBObject *obj = NULL; /* assume failure */
struct FileInfoBlock *fib;
BPTR lock;

	if (lock=LOCK(name,ACCESS_READ))
	{
		if (fib=ALLOCVEC(sizeof(struct FileInfoBlock),MEMF_PUBLIC))
		{
			if (EXAMINE(lock,fib))
			{
				obj=CreateIconFromNameFib(name,parent,fromlock,fib);
			}
			FREEVEC(fib);
		}
		UNLOCK(lock);
	}
	return(obj);
}

/*
	Create an icon from a node (which contains the name)
	and add it to the system.  This routine is used by
	the 'Show All Files' logic.  The file exists but an
	icon must be created for it since the file has no
	corresponding .info file.
*/
CreateIconFromNode(struct InfoNode *ln,struct WBObject *parent)
{
struct WBObject *obj;
struct FileInfoBlock fib;	/* A Fake one... */
int result=FALSE;

    /* if object does not already exist */
    if (!MasterSearch(FindDupObj, parent->wo_DrawerData->dd_Lock, ln->name,~ISDISKLIKE))
    {
	fib.fib_Size = ln->size;
	fib.fib_Protection = ln->prot;
	fib.fib_Date = ln->date;
	fib.fib_DirEntryType = ln->type;

	/* if we could create the object */
	if (obj = CreateIconFromNameFib(ln->name, parent, NULL,&fib))
	{
	    result=LocalAddIcon(obj, parent);
	}
    }
    return(result);
}

#ifdef DEBUGGING
void DisplayInfoList()
{
	struct WorkbenchBase *wb = getWbBase();
	struct Node *ln;

	MP(("DisplayInfoList:\n"));
	for (ln = wb->wb_InfoList.lh_Head; ln->ln_Succ; ln = ln->ln_Succ) {
		MP(("\tln->ln_Name='%s'\n", ln->ln_Name));
	}
}

void DisplayNonInfoList()
{
	struct WorkbenchBase *wb = getWbBase();
	struct Node *ln;

	MP(("DisplayNonInfoList:\n"));
	for (ln=wb->wb_NonInfoList.lh_Head; ln->ln_Succ; ln = ln->ln_Succ) {
		MP(("\tln->ln_Name='%s'\n", ln->ln_Name));
	}
}
#endif DEBUGGING
