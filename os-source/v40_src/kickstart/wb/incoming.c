/*
 * $Id: incoming.c,v 39.3 93/02/10 10:33:53 mks Exp $
 *
 * $Log:	incoming.c,v $
 * Revision 39.3  93/02/10  10:33:53  mks
 * Saved some code by removing an unused return value
 * 
 * Revision 39.2  92/06/23  18:08:48  mks
 * Fixed very nasty bug in the way workbench tracked gadgets
 *
 * Revision 39.1  92/06/11  15:49:07  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.2  92/06/11  10:55:03  mks
 * Made code smaller and made roll-off work.
 *
 * Revision 38.1  91/06/24  11:36:03  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "devices/inputevent.h"
#include "devices/timer.h"
#include "libraries/dos.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "graphics/clip.h"
#include "graphics/layers.h"

#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "intuimsg.h"
#include "quotes.h"

/*
    Inheritence - if the drawer's ViewBy or Show mode has yet to be defined,
    then it inherits its parent's ViewBy or Show mode BUT only if the drawer
    is not already open (otherwise we'd be changing the viewmode on the fly).
*/
DoOpen2(drawer)
struct WBObject *drawer;
{
    struct WBObject *parent;
    struct NewDD *dd;

    MP(("CPOFDO: enter, drawer=$%lx (%s)\n",
	drawer, drawer ? drawer->wo_Name : "NULL"));
#ifdef MEMDEBUG
    PrintAvailMem("DoOpen2", MEMF_CHIP);
#endif MEMDEBUG

    /* if this drawer does not have a window, is not open, and is not silent */
    if ((drawer->wo_DrawerData->dd_DrawerWin == 0) &&
	!drawer->wo_DrawerOpen && !drawer->wo_DrawerSilent) {
	if (parent = drawer->wo_Parent) { /* if drawer has a parent */
	    if (dd = drawer->wo_DrawerData) { /* if drawer data */
		if (parent->wo_DrawerData) { /* if parent drawer data */
		    MP(("CPOFDO: old ViewModes=%ld\n", dd->dd_ViewModes));
		    if (dd->dd_ViewModes == DDVM_BYDEFAULT) {
			dd->dd_ViewModes = parent->wo_DrawerData->dd_ViewModes;
			MP(("CPOFDO: new ViewModes=%ld\n", dd->dd_ViewModes));
		    }
		    MP(("CPOFDO: old Flags=%ld\n", dd->dd_Flags));
		    if (dd->dd_Flags == DDFLAGS_SHOWDEFAULT) {
			dd->dd_Flags = parent->wo_DrawerData->dd_Flags;
			MP(("CPOFDO: new Flags=%ld\n", dd->dd_Flags));
		    }
		}
	    }
	}
    }
    PotionOpen(drawer);
    MP(("CPOFDO: exit\n"));
#ifdef MEMDEBUG
    PrintAvailMem("DoOpen2", MEMF_CHIP);
#endif MEMDEBUG
    return(NULL);
}

/* Open menu performed
 * if the Object is a Tool or Project, handle it starting at Object
 */
void DoOpen()
{
    MP(("DoOpen: enter\n"));
    /* beware the double negative */
    if (!SelectSearch(CheckForNotType,ISDRAWER))
    {
	/* all selected files are drawer or drawer like */
	SelectSearch( DoOpen2 );
    }
    else
    {
	PotionRunTool();
    }
    MP(("DoOpen: exit\n"));
}

void DoClose2( obj )
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();

    if (obj == wb->wb_RootObject) {
	QuitCheck();
    }
    else if (Drawer_P(obj)) {
	ResetPotion();
	CloseDrawer(obj);
    }
}

#define GOTSHIFT 0x10000

void ProcessGadget(nextState, NextEvent)
int nextState;
struct IntuiMessage *NextEvent;
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd;
struct WBObject *obj;
struct Window *win;
ULONG proportion;
ULONG gadnum;
int curx, cury;
ULONG width, height;

	if (obj = wb->wb_CurrentObject) if (dd = obj->wo_DrawerData) if (win = dd->dd_DrawerWin)
	{
	    curx = dd->dd_CurrentX;
	    cury = dd->dd_CurrentY;

	    if (!nextState)
	    {
	    	ModifyIDCMP(win, win->IDCMPFlags & ~INTUITICKS);
		wb->wb_CurrentObject=NULL;
	    }

	    width = win->Width - win->BorderLeft - win->BorderRight;
	    height = win->Height - win->BorderTop - win->BorderBottom;

	    gadnum=(((ULONG)wb->wb_CurrentGadget) & (~GOTSHIFT));
	    if (gadnum==GID_HORIZSCROLL)
	    {
		proportion = dd->dd_HorizProp.HorizPot * (dd->dd_MaxX - dd->dd_MinX - width);
		curx = (proportion / MAXPOT) + dd->dd_MinX;
		nextState=1;
	    }
	    else if (gadnum==GID_VERTSCROLL)
	    {
		proportion = dd->dd_VertProp.VertPot * (dd->dd_MaxY - dd->dd_MinY - height);
		cury = (proportion / MAXPOT) + dd->dd_MinY;
		nextState=1;
	    }
		/*
		 * Very slimy trick here:  We use the gadget ID as an
		 * index into the array of gadget in the dd structure.
		 * This lets us check the state of the "selected" flag
		 * and lets "roll-off" work correctly...
		 * This means that GID_xxx and dd structure must match
		 */
	    else if (((struct Gadget *)&(dd->dd_HorizScroll))[gadnum-1].Flags & GFLG_SELECTED)
	    {
		    switch ((ULONG)wb->wb_CurrentGadget)
		    {
		    case GID_LEFTSCROLL:
			curx -= width >> 3;
			break;

		    case GOTSHIFT | GID_LEFTSCROLL:
			curx--;
			break;

		    case GID_RIGHTSCROLL:
			curx += width >> 3;
			break;

		    case GOTSHIFT | GID_RIGHTSCROLL:
			curx++;
			break;

		    case GID_UPSCROLL:
			cury -= height >> 3;
			break;

		    case GOTSHIFT | GID_UPSCROLL:
			cury--;
			break;

		    case GID_DOWNSCROLL:
			cury += height >> 3;
			break;

		    case GOTSHIFT | GID_DOWNSCROLL:
			cury++;
			break;
		    }
	    }

	    if (nextState) if (dd->dd_CurrentX != curx || dd->dd_CurrentY != cury)
	    {
		dd->dd_CurrentX = curx;
		dd->dd_CurrentY = cury;
		RefreshDrawer(obj);
	    }
	}
}

/*
	Hilight an icon.
	- flag that this icon is selected
	- bring it to the front of the list
	- render it
	- if object is a drawer, hilight its window
*/
void HighIcon(object)
struct WBObject *object;
{
    struct WorkbenchBase *wb = getWbBase();

	if (!object->wo_Selected) { /* if unselected */
		object->wo_Selected = 1; /* this icon is now selected */
		BringToFront(object); /* put it at the front of the list */
		if (object != wb->wb_RootObject) {
			if (object->wo_Parent) {
				RenderIcon(object);
			}
		}
	}
#ifdef DEBUGGING
	else {
		MP(("HighIcon: WARNING: obj already hilighted!\n"));
		MP(("\tobj=%lx (%s)\n", object, object->wo_Name));
	}
#endif DEBUGGING
}

/*
	UnHilight an icon.
	- flag that this icon is not selected
	- bring it to the front of the list
	- render it
	- if object is a drawer, unhilight its window
*/
void UnHighIcon(object)
struct WBObject *object;
{
    struct WorkbenchBase *wb = getWbBase();

	if( object->wo_Selected ) { /* if selected */
		object->wo_Selected = 0; /* this icon is now unselected */
		BringToFront( object ); /* put it at the front of the list */
		if (object != wb->wb_RootObject) {
			if (object->wo_Parent) {
				RenderIcon(object);
			}
		}
	}
#ifdef DEBUGGING
	else {
		MP(("UnHighIcon: WARNING:  obj already unhilighted!\n"));
		MP(("\tobj=%lx (%s)\n", object, object->wo_Name));
	}
#endif DEBUGGING
}

void BringToFront( obj )
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *dd;

    /*
	What a kludge!  I hate this!  In ViewByText mode the objects are
	kepted sorted in reverse order.  This is absolutely neccessary in
	order for RenderIcon to properly dynamically figure out thier
	position.  I MUST not change their position in the list
	(ie. BringToFront) otherwise they will be out of sorted order.
	Why Neil brings an object to the front of the SelectList everytime
	it gets selected is beyond my.  Perhaps he wants to be able to
	just pick off the head of the list to get the last selected item
	(he does have a routine called LastSelected I believe).  All this
	bullsh_t means that an icon selected in a ViewByText drawer won't
	be at the head of the list and won't get returned by LastSelected.
    */
	MP(("BringToFront: obj=$%lx (%s)\n", obj, obj->wo_Name));
	if (obj != wb->wb_RootObject) {
	    if (obj->wo_Parent)
	    {
		dd = obj->wo_Parent->wo_DrawerData;
		if (dd->dd_ViewModes <= DDVM_BYICON) {
				REMOVE(&obj->wo_Siblings);
				ADDHEAD(&dd->dd_Children, &obj->wo_Siblings);
		}
	    }
	}
}

ResetHighlight( object )
struct WBObject *object;
{
    UnHighIcon( object );
    return( FALSE );
}


void ResetPotion()
{
    struct WorkbenchBase *wb = getWbBase();

    SelectSearch(ResetHighlight);
    NewList(&wb->wb_SelectList);
    RethinkMenus();
    wb->wb_Dragging = 0;
    wb->wb_DragSelect = 0;
}


ObjOnList( obj1, obj2 )
struct WBObject *obj1, *obj2;
{
    return( obj1 == obj2 );
}


/* see if (x,y) is over the objects hit box */
SelectObject(obj, x, y)
struct WBObject *obj;
int x, y;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Gadget *gad = &obj->wo_Gadget;
    struct NewDD *dd = obj->wo_Parent->wo_DrawerData;
#ifdef DEBUGGING
    struct Gadget *gio = &obj->wo_IOGadget;
#endif DEBUGGING

    int result = 0; /* assume failure */
    int Width = gad->Width;
    int Height = gad->Height;

    MP(("SelectObject: obj=%lx (%s), x=%ld, y=%ld\n",
	obj, obj->wo_Name, x, y));
    MP(("\tCurX=%ld, CurY=%ld, Width=%ld, Height=%ld, IOWidth=%ld, IOHeight=%ld\n",
	obj->wo_CurrentX, obj->wo_CurrentY, Width, Height,
	gio->Width, gio->Height));
    MP(("\tSaveX=%ld, SaveY=%ld\n", obj->wo_SaveX, obj->wo_SaveY));

    if (dd->dd_ViewModes <= DDVM_BYICON) {
	Width += EMBOSEWIDTH;
	Height += EMBOSEHEIGHT;
    }
    else {
	Width += VIEWBYTEXTLEFTOFFSET;
    }
/*
    y -= obj->wo_CurrentY;
    x -= obj->wo_CurrentX;
*/
    if (x >= obj->wo_CurrentX && y >= obj->wo_CurrentY &&
	x < (obj->wo_CurrentX + Width) && y < (obj->wo_CurrentY + Height)) {
	MP(("\tCO_ORDINATE IS WITHIN THE SELECT BOX\n"));
	MP(("\t(Flags & GADGHIGHBITS) = %ld, GADGBACKFILL=%ld\n",
	    (gad->Flags & GADGHIGHBITS), GADGBACKFILL));
	result = 1;
    }
end:
    MP(("SelectObject: exit, result = %ld\n", result));
    return(result);
}


/*
** Turn a mouse click into an object.  Special, late breaking hack:
** If we avoidselected is set, then we are looking to see who we have
** dropped an icon on.  We won't recognize any selected icon.
*/
struct WBObject *
TranslateSelect( msg, avoidselected )
struct IntuiMessage *msg;
int avoidselected;
{
    int x, y;
    struct WBObject *obj;
    struct WBObject *drawer;
    struct Window *win;
    struct NewDD *dd;

    win = msg->IDCMPWindow;
    drawer = WindowToObj(win);
    dd = drawer->wo_DrawerData;

    MP(("TS: msg=%lx, avoidselected=%ld, win=%lx (%s), dr=%lx (%s)\n",
	msg, avoidselected, win, win->Title, drawer, drawer->wo_Name));
    MP(("\tMX=%ld, BL=%ld, (%ld), MY=%ld, BT=%ld, (%ld)\n",
	msg->MouseX, win->BorderLeft, msg->MouseX - win->BorderLeft,
	msg->MouseY, win->BorderTop, msg->MouseY - win->BorderTop));
    MP(("\tddCX=%ld, ddCY=%ld\n", dd->dd_CurrentX, dd->dd_CurrentY));
#ifdef MEMDEBUG
    PrintAvailMem("TranslateSelect", MEMF_CHIP);
#endif MEMDEBUG
    if (drawer == NULL) {
#ifdef DEBUGGING
	MP(("TranslateSelect: can't find the right window!\n"));
	Debug(0L);
#endif DEBUGGING
	return(NULL);
    }

    if (msg->MouseX >= win->Width - win->BorderRight) {
	return( 0 );
    }
    if ((x = msg->MouseX - win->BorderLeft) < 0) {
	return( 0 );
    }

    if (msg->MouseY >= win->Height - win->BorderBottom) {
	return( 0 );
    }
    if ((y = msg->MouseY - win->BorderTop) < 0) {
	return( 0 );
    }

    /* mks:  kludge fix to the problem of "Last item on list is
     * under the mouse pointer and is being dropped on and we need
     * to skip it so we try again and loop problem...  (arg!!!)
     */
    if (obj=(struct WBObject *)dd->dd_Children.lh_Head)
    {
	if (obj = SiblingSuccSearch(obj, SelectObject,
					x + dd->dd_CurrentX,
					y + dd->dd_CurrentY))
	{
	    /*
	     * If we are to avoid selected objects, do so...
	     */
	    if ((avoidselected) && (obj->wo_Selected)) obj=NULL;
	}
    }

    MP(("TranslateSelect: exit, returning %lx (%s)\n",
	obj, obj ? obj->wo_Name : "<no selection>"));

#ifdef MEMDEBUG
    PrintAvailMem("TranslateSelect", MEMF_CHIP);
#endif MEMDEBUG
    return( obj );
}

void DoMouseMoves(struct IntuiMessage *msg)
{
struct WorkbenchBase *wb = getWbBase();
struct IntuiMessage *nextmsg=msg;
struct MsgPort *port = &wb->wb_IntuiPort;

    /* Flush extra mouse moves... */
    while(nextmsg)
    {
    	msg=nextmsg;
	FORBID;
	nextmsg=(struct IntuiMessage *)(port->mp_MsgList.lh_Head);
	if (!(nextmsg->ExecMessage.mn_Node.ln_Succ)) nextmsg=NULL;
	PERMIT;
	if (nextmsg)
	{
	    if (nextmsg->Class!=MOUSEMOVE) nextmsg=NULL;
	    else
	    {
	    	REPLYMSG((struct Message *)msg);
	    	FORBID;
	    	Remove(nextmsg);
	    	PERMIT;
	    }
	}
    }

    /* use the last message to draw the new bobs or drag box */
    if (wb->wb_Dragging) Redraw(msg);

    REPLYMSG((struct Message *)msg);
}

/*=== ===================================================================== */
/*=== ===================================================================== */
/*=== ===================================================================== */

#ifdef DEBUGGING
static char *wbcodes[32] = {
    "SIZEVERIFY", "NEWSIZE", "REFRESHWINDOW", "MOUSEBUTTONS", /* 1 - 4 */
    "MOUSEMOVE", "GADGETDOWN", "GADGETUP", "REQSET", /* 5 - 8 */
    "MENUPICK", "CLOSEWINDOW", "RAWKEY", "REQVERIFY", /* 9 - 12 */
    "REQCLEAR", "MENUVERIFY", "NEWPREFS", "DISKINSERTED", /* 13 - 16 */
    "DISKREMOVED", "WBENCHMESSAGE", "ACTIVEWINDOW", "INACTIVEWINDOW",
    "DELTAMOVE", "VANILLAKEY", "INTUITICKS", "800000", /* 21 - 24 */
    "1000000", "2000000", "4000000", "8000000", /* 25 - 28 */
    "10000000", "20000000", "40000000", "LONELYMESSAGE" /* 29 - 32 */
    };
#endif DEBUGGING

void IncomingEvent(NextEvent)
struct IntuiMessage *NextEvent;
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *object, *drawer;
    struct Window *eventwindow;
    struct timeval oldtick;
    struct IntuiMessage savemsg;
    int i, shifted, code;
    unsigned intuicode;

    /*
     * Do the extra event pre-processing...
     */
    QuickFilter(NextEvent);

    eventwindow = NextEvent->IDCMPWindow;

    /* turn IDCMP class into an index for more compact case code */
    for (i = 1, code = 0; i < NextEvent->Class; code++) i <<= 1;

    /* common code to notice if the shift key was down */
    shifted = (NextEvent->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? GOTSHIFT : 0;

    MP(("IE: code=%lx (%s), eventwindow=%lx (%s), NextEvent->Code=%lx\n",
    code, wbcodes[code], eventwindow, eventwindow->Title, NextEvent->Code));

    /*
	If the window cannot be converted to a drawer than it must be
	a non-drawer window (like Info, Execute, Rename, etc.).  These
	windows have their own event loop and I should never get any
	of their messages.  This code pretects me just in case I do.
	There is a case where I actually do get one of the messages
	for one of these windows.  If SAVE is selected in Info, the
	disk is write-protected, and the uses hits CANCEL, the EndDiskIO
	call in PutObject (which Info calls) does a CheckDiskIO.  This
	purges all previous messages one of which is a REFRESHWINDOW
	message for Info's window.  Since Info is going down anyway,
	it is safe to ignore it here.  Someday it would be better if
	Info had a way of calling a non-BeginDiskIO/EndDiskIO version
	of PutObject.  However, if the message is of type WB_WBENCHMESSAGE
	(which is from Intuition and has no window associated with it)
	then I do not want to abort.
    */
    if (code != WB_WBENCHMESSAGE) {
	if (!(drawer = WindowToObj(eventwindow))) goto reply;
    }

    switch(code)
    {

    case WB_MOUSEBUTTONS:
	MP(("\tWB_MOUSEBUTTONS\n"));
	switch(NextEvent->Code)
	{
	case SELECTUP: /* released mouse button inside a window */
	    MP(("IE: MOUSEBUTTONS: SELECTUP: (x=%ld, y=%ld)\n",NextEvent->MouseX, NextEvent->MouseY ));
	    MP(("SU..."));
	    /* go test for collisions */
	    if (wb->wb_Dragging)
	    {
		/*
		    Must call RethinkMenus AFTER calling ClearDragging since
		    ClearDragging unlocks the layers.  This sould be fixed as
		    of V1.4 Alpha 17 Intuition.
		*/
		/* turn off bobs/drag selecting, do bob move or drag select */
		MP(("cd..."));
		ClearDragging(NextEvent, 1);
		MP(("rm..."));
		RethinkMenus();
	    }
	    MP(("pg..."));
	    ProcessGadget(0, NextEvent);
	    MP(("BREAK\n"));
	    break;

	case SELECTDOWN: /* pressed mouse button inside a window */
	    MP(("IE: MOUSEBUTTONS: SELECTDOWN: (x=%ld, y=%ld)\n",NextEvent->MouseX, NextEvent->MouseY ));
	    DoLayerDemon(); /* cancel bobs/drag selecting */
	    wb->wb_InputTrashed = 0; /* clear input stream */
	    MP(("et..."));
	    ErrorTitle(wb->wb_ScreenTitle); /* clear title */

	    oldtick = wb->wb_Tick; /* get old time */
	    wb->wb_Tick.tv_secs = NextEvent->Seconds;
	    wb->wb_Tick.tv_micro = NextEvent->Micros; /* save new time */
	    wb->wb_DoubleClick = 0; /* clear doubleclick flag */

	    /* get object user clicked on (may be null) */
	    MP(("ts..."));
	    object = TranslateSelect(NextEvent, 0);
	    MP(("ok..."));
	    MP(("\tobject = %lx (%s)\n", object, object ? object->wo_Name : "NULL"));
	    if (!object)
	    { /* if didn't hit an object */
		if (!shifted)
		{ /* not adding, reset list */
		    MP(("rp..."));
		    ResetPotion();
		    MP(("ok..."));
		}
		/*
		    Must call RethinkMenus BEFORE calling SetDragging since
		    SetDragging locks the layers.  This sould be fixed as of
		    V1.4 Alpha 17 Intuition.
		*/
		MP(("rm..."));
		RethinkMenus();
		MP(("sd..."));
		SetDragging(NextEvent, 1); /* turn on drag selecting */
		MP(("BREAK\n"));
		break; /* thats all folks */
	    }

	    /* user hit an object */
	    /* if clicked fast enough */
	    if (DoubleClick(oldtick.tv_secs,oldtick.tv_micro,NextEvent->Seconds,NextEvent->Micros))
	    {
		wb->wb_DoubleClick = 1; /* flag we double clicked */
	    }

	    wb->wb_XOffset = NextEvent->MouseX;
	    wb->wb_YOffset = NextEvent->MouseY; /* save pointer co-ords */

	    /* if in selected list and want to open/run it */
	    if ((wb->wb_DoubleClick) && SelectSearch(ObjOnList,object))
	    {
		/* prevent this from being the first click of a double-click */
		wb->wb_Tick.tv_secs = 0;
		MP(("do..."));
		DoOpen(); /* open/run it */
		MP(("rm..."));
		RethinkMenus(); /* fix menus */
		MP(("ok..."));
	    }
	    else
	    {
		wb->wb_DoubleClick = 0;
		if (!shifted)
		{
		    /* we're not extending, so reset list head */
		    MP(("rp..."));
		    ResetPotion();
		}

		if (!SelectSearch(ObjOnList, object))
		{
		    ADDTAIL(&wb->wb_SelectList, &object->wo_SelectNode);
		    MP(("hi..."));
		    HighIcon(object);
		}
		MP(("rm..."));
		RethinkMenus();
		MP(("sd..."));
		SetDragging(NextEvent, 0); /* turn on bobs */
	    }
	    MP(("BREAK\n"));
	    break;
	}
	MP(("BREAK\n"));
	break;

    case WB_GADGETDOWN:
	MP(("\tWB_GADGETDOWN: drawer=%lx (%s)\n", drawer, drawer->wo_Name));

	DoLayerDemon(); /* cancel bobs/drag selecting */

	wb->wb_CurrentObject = drawer;

	code = ((struct Gadget *)NextEvent->IAddress)->GadgetID;

        if ((code >= GID_UPSCROLL) && (code <= GID_RIGHTSCROLL))
        {
	    MP(("GadgetDown: ModifyIDCMP(eventwin->IDCMPFlags | INTUITICKS)\n"));
	    ModifyIDCMP(eventwindow, eventwindow->IDCMPFlags | INTUITICKS);
	}
	code |= shifted;

	MP(("\tsetting CurrentGadget to %ld\n", code));
	/* backwards compatibility: we use it as a ULONG rather than ptr */
	wb->wb_CurrentGadget = (struct Gadget *)code;

    case WB_INTUITICKS:
	if (wb->wb_DragSelect)
	{ /* if drag selecting... */
	    AnimateDragSelectBox(NextEvent); /* animate drag select box */
	}
	else if (wb->wb_Dragging)
	{
	    PostLayerDemon();
	}
	else
	{
	/*
	 * If one of the arrow gadgets are pressed, process it...
	 */

	    if (eventwindow->IDCMPFlags & INTUITICKS) ProcessGadget(1, NextEvent);
	}
	break;

    case WB_INACTIVEWINDOW:
	MP(("\tWB_INACTIVEWINDOW:\n"));
	MP(("\teventwindow=%lx (%s)\n", eventwindow, eventwindow->Title));
	MP(("\tew->FirstGadget=%lx, ew->FirstGadget->GadgetID=%ld\n",eventwindow->FirstGadget, eventwindow->FirstGadget->GadgetID));
	/* Dropping into gadget up... */
    case WB_GADGETUP:
	MP(("\tWB_GADGETUP:\n"));
	DoLayerDemon(); /* cancel bobs/drag selecting */
	ProcessGadget(0, NextEvent);
        break;

    case WB_MENUPICK:
	MP(("\tWB_MENUPICK: calling ReplyMsg\n"));
	savemsg = *NextEvent; /* copy msg */
	/* reply first in case this is a close menu item */
	REPLYMSG( (struct Message *)NextEvent );
	/*
	    We SHOULD never get a MENUPICK while DragSelecting since menus
	    are disabled while DragSelecting BUT there is a window of
	    vunerability after DragSelecting starts and before the
	    menus are disabled (via a ModifyIDCMP).  So if I get a MENUPICK
	    while DragSelecting, I jump to the MENUVERIFY code which cancels
	    the DragSelecting (since that is what we have decided the right
	    mouse button does while DragSelecting).  DaveB Jan/90.
	*/
	if (wb->wb_Dragging) goto GotCancelButton;
	else {
	    MP(("ok, calling Menumeration\n"));
	    /* go do the menu response */
	    Menumeration(&savemsg);
	    MP(("ok, going to end\n"));
	    goto end;
	}
	break;

    case WB_CLOSEWINDOW:
	MP(("\tWB_CLOSEWINDOW: win=%lx (%s)\n",eventwindow, eventwindow->Title));
        /* must be very careful here: be sure to reply the message
         * BEFORE closing the window...
         */
        REPLYMSG( (struct Message *)NextEvent );
        if (eventwindow != wb->wb_BackWindow)
        {
            ResetPotion();
            CloseDrawer(drawer);
        }
        else
        { /* close back_window, same as QUIT */
            QuitCheck();
        }
        goto end;
	break;

    case WB_NEWSIZE:
	MP(("\tWB_NEWSIZE:\n"));
    case WB_REFRESHWINDOW:
	MP(("\tWB_REFRESHWINDOW:\n"));
	UpdateWindow(drawer);
	break;

    case WB_DISKINSERTED:
    case WB_DISKREMOVED:
	if (!(wb->wb_Dragging))
	{
	    FindDisks();
	    RethinkMenus();
	}
	break;

    case WB_WBENCHMESSAGE:
	MP(("\tWB_WBENCHMESSAGE:\n"));
	/* send the message back so intuition doesn't deadlock
	** (bless its little pointy head...).  Save the code -- its
	** the only thing we really need from the message.
	*/
	intuicode = (unsigned)NextEvent->Code;
	MP(("IncommingEvent: WB_WBENCHMESSAGE: intuicode=%lx\n", intuicode));

	if (intuicode == WBENCHOPEN)
	{ /* OpenWorkBench */
	    if (wb->wb_Closed) WBOpenWorkBench();
	}
	else if (intuicode == WBENCHCLOSE)
	{ /* CloseWorkBench */
	    /* refuse if already closed or dragging */
	    if (!(wb->wb_Closed || wb->wb_Dragging)) WBCloseWorkBench();
	}
	break;

    case WB_MOUSEMOVE:
	MP(("IncommingEvent: WB_\n"));
	DoMouseMoves(NextEvent);
	MP(("\tOK\n"));
	/* DoMouseMoves has replied to the message */
	goto end;
	break;

    case WB_ACTIVEWINDOW:
	MP(("\tWB_ACTIVEWINDOW: (%s)\n", eventwindow->Title));
	RethinkMenus();
	break;

    case WB_MENUVERIFY:
	MP(("\tWB_MENUVERIFY: code=%lx\n", NextEvent->Code));
	if (NextEvent->Code == MENUHOT) {
	    NextEvent->Code = MENUCANCEL;
	    savemsg = *NextEvent;
	    MP(("IE: MENUVERIFY: calling ReplyMsg..."));
	    REPLYMSG( (struct Message *)NextEvent );
	    MP(("ok, "));
	    if (wb->wb_Dragging) {
GotCancelButton:
		MP(("calling ClearDragging..."));
		DoLayerDemon(); /* cancel bobs/drag selecting */
		MP(("ok, "));
		if (shifted) { /* un-select object under pointer */
		    MP(("calling TranslateSelect..."));
		    object = TranslateSelect(&savemsg, 0);
		    MP(("ok, "));
		    if (object) {
			MP(("calling UnSelectIt..."));
			UnSelectIt(object);
			MP(("ok, "));
		    }
		}
		else { /* un-select all objects */
		    MP(("calling ResetPotion..."));
		    ResetPotion();
		    MP(("ok, "));
		}
	    }
	    MP(("goto end\n"));
	    goto end;
	}
	break;
    }
reply:
    MP(("IE: calling ReplyMsg(NextEvent)..."));
    REPLYMSG( (struct Message *)NextEvent );
    MP(("ok\n"));
end:
    return;
}

wbCleanup(obj)
struct WBObject *obj;
{
    struct NewDD *dd;
    struct Window *win;

    if (dd = obj->wo_DrawerData)
    {
	/* object has a drawer */
	MP(("wbCleanup: object has a drawer\n"));
	if (win = dd->dd_DrawerWin)
	{
	    /* this drawer has a window */
	    MP(("wbCleanup: this drawer has a window, closing it\n"));
	    ClosePWindow(win, dd); /* close it */
	    dd->dd_DrawerWin = 0; /* drawer has no window */
	    obj->wo_DrawerOpen = 1; /* drawer is open */
	}
    }
    MP(("wbCleanup: exit\n"));
    return(0);
}

resetIconWin(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();

	MP(("resetIconWin: obj=%lx (%s)\n", obj, obj ? obj->wo_Name : "NULL"));
	if (obj != wb->wb_RootObject)
	{
		if (obj->wo_Parent)
		{
			MP(("resetIconWin: assigning IconWin to %lx, %lx (%s) 's window\n",obj->wo_Parent->wo_DrawerData->dd_DrawerWin,obj->wo_Parent, obj->wo_Parent->wo_Name));
			obj->wo_IconWin = obj->wo_Parent->wo_DrawerData->dd_DrawerWin;
		}
	}
	return(0);
}

/*
 *	Called via 'MasterSearch(wbReopen)' from OpenWorkbench after a
 *	CloseWorkbench/ResetWB
 */
wbReopen(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd;
struct Window *win;
UBYTE *ptr;
int flag=0;	/* Assume all fine... */

    if (MasterSearch(ObjOnList,obj)) if (dd=obj->wo_DrawerData)
    {
	if (!dd->dd_DrawerWin)	/* I no window, we open it */
	{
            /* this drawer is open and not silent */
            if (obj->wo_DrawerOpen && !obj->wo_DrawerSilent)
            {
                /* this object has a drawer to be opened */
		/* Handle ROOT object in special case... */
                if (obj == wb->wb_RootObject) win = OpenDiskWin(obj);
                else
                {
                    /* null initial window */
                    FormatWindowName(obj, &ptr, NULL);

                    win = OpenPWindow(&dd->dd_NewWindow,SIMPLE_REFRESH|
                        WINDOWSIZING|WINDOWDRAG|WBENCHWINDOW|WINDOWDEPTH|
                        SIZEBBOTTOM|SIZEBRIGHT|REPORTMOUSE|ACTIVATE|
                        WINDOWCLOSE,
                        &dd->dd_HorizScroll,    /* gadgets */
                        ptr,                    /* title */
                        NULL,                   /* extraIDCMP */
                        FALSE,                  /* wb pattern flag */
                        dd->dd_ViewModes        /* view modes */
                    );
                }

                if (dd->dd_DrawerWin=win)
                {
                    obj->wo_DrawerOpen = 0; /* drawer is now fully open */
		    /* re-link children to the new open window */
		    SiblingSuccSearch(dd->dd_Children.lh_Head, resetIconWin);
		    /* re-draw the children */
		    RefreshDrawer(obj);
		}
                else
                {
                    /* if no drawer window was opened, cleanup and abort */
                    if (obj!=wb->wb_RootObject) CloseDrawer(obj); /* close drawer */
                    flag=1;           /* We failed! */
                }
            }
	}
    }
    MP(("wbReopen: exit\n"));
    return(flag);
}

void WBOpenWorkBench(void)
{
struct WorkbenchBase *wb = getWbBase();

    while (!InitScreenAndWindows())
    {
	UnInitScreenAndWindows();	/* Undo what we started */
        WaitTOF();
    }

	/*
	 * Note that wbReopen() returns TRUE if it did not work
	 */
    while(wbReopen(wb->wb_RootObject)) WaitTOF();

    MasterSearch(wbReopen); /* reopen everything */

    ResetMenus();

    wb->wb_Closed = 0; /* wb is now open */
    FindDisks();
    EndDiskIO();
}

void WBCloseWorkBench(void)
{
struct WorkbenchBase *wb = getWbBase();
register struct WBObject *obj;
register struct Layer *lay;

    MP(("WBCloseWorkBench: enter\n"));

    BeginDiskIO();
    /*
     * What we want to do here is to close down
     * the windows in order...
     */
    do
    {
	obj=NULL;
	LockLayerInfo(&(wb->wb_Screen->LayerInfo));
	lay=wb->wb_Screen->LayerInfo.top_layer;
	while (lay)
	{
	    if (obj=WindowToObj(lay->Window)) lay=NULL;
	    else lay=lay->back;
	}
	UnlockLayerInfo(&(wb->wb_Screen->LayerInfo));

	if (obj)
	{
	    REMOVE((struct Node *)obj);
	    ADDHEAD(&(wb->wb_MasterList),(struct Node *)obj);
	    UpdateWindowSize(obj);
	    wbCleanup(obj);
	}
    } while (obj);

    UnInitScreenAndWindows();
    wb->wb_Closed = 1; /* flag that wb is now closed */
    MP(("WBCloseWorkBench: exit\n"));
}

void ResetWB(void)
{
    WBCloseWorkBench();
    WBOpenWorkBench();
}
