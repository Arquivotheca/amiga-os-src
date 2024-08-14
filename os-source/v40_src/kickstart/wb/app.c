/*
 * $Id: app.c,v 39.2 92/07/02 14:29:03 mks Exp $
 *
 * $Log:	app.c,v $
 * Revision 39.2  92/07/02  14:29:03  mks
 * Fixed autodoc typos...
 * 
 * Revision 39.1  92/06/11  15:48:56  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.4  92/05/26  13:57:40  mks
 * Did a bit of ROM space savings here too
 *
 * Revision 38.3  92/02/13  14:09:41  mks
 * Changed the call to PlaceObj()
 *
 * Revision 38.2  91/11/06  15:42:19  mks
 * Autodoc cleanup
 *
 * Revision 38.1  91/06/24  11:33:32  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "utility/tagitem.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "startup.h"
#include "global.h"
#include "support.h"
#include "wbinternal.h"

#define	APP_NOP		0
#define	ADD_ICON	1
#define	ADD_MENU	2
#define	ADD_WINDOW	3
#define	DEL_ICON	4
#define	DEL_MENU	5
#define	DEL_WINDOW	6


struct MyAppIcon
{
	SHORT atype;
	SHORT mtype;
	struct Message msg;
	struct WorkbenchAppInfo ai;
	struct DiskObject dobj;
	struct Image image1;
	struct Image image2;
	char name[1];
};

struct MyAppMenu
{
	SHORT atype;
	SHORT mtype;
	struct Message msg;
	struct WorkbenchAppInfo ai;
	char text[1];
};

struct MyAppWin
{
	SHORT atype;
	SHORT mtype;
	struct Message msg;
	struct WorkbenchAppInfo ai;
};

struct MyAppCommon
{
	SHORT atype;
	SHORT mtype;
	struct Message msg;
	struct WorkbenchAppInfo ai;
};

struct typeamsg
{
	SHORT mtype;		/* holder for message type */
	struct AppMessage msg;	/* application message */
};

FindAppIcon(obj)
struct WBObject *obj;
{
	return(obj->wo_Type == WBAPPICON);
}

struct WorkbenchAppInfo *
MenuToAppMenuItem(code)
int code;
{
struct WorkbenchBase *wb = getWbBase();
struct WorkbenchAppInfo *ami = (struct WorkbenchAppInfo *)wb->wb_AppMenuItemList.lh_Head;
int i = ITEMNUM(code);

	while (--i) {
		ami = (struct WorkbenchAppInfo *)ami->wai_Node.ln_Succ;
	};
	return(ami);
}

IconToAppIcon(ai, obj)
struct WorkbenchAppInfo *ai;
struct WBObject *obj;
{
	return(obj == ai->wai_Ptr);
}

WindowToAppWindow(aw, win)
struct WorkbenchAppInfo *aw;
struct Window *win;
{
	return(win == aw->wai_Ptr);
}

/*
	We are finished with this AppMessage.  Free up all the
	memory that we allocated for it.
*/
void CleanupAppMsg(am)
struct AppMessage *am;
{
	/* free up WBArg mem */
	ExitArgs(am->am_NumArgs, am->am_ArgList);
	/* free up struct typeamsg mem */
	FREEVEC( (void *)(((ULONG)am) - sizeof(SHORT)));
}

/*
 * Common code for App stuff...
 *
 * This also fixes the problem of the AppMenu not getting
 * the selected objects...
 */
void DoAppCommon(struct WorkbenchAppInfo *appstuff,long numargs,struct IntuiMessage *msg,UWORD mType)
{
struct WorkbenchBase *wb = getWbBase();
struct WBArg *arglist, *argorig;
struct AppMessage *am;
struct typeamsg *amsg;

	if (amsg = ALLOCVEC(sizeof(struct typeamsg), MEMF_CLEAR|MEMF_PUBLIC))
	{
		amsg->mtype=mType;
		am=&amsg->msg;
		am->am_Type=mType;
		am->am_UserData=appstuff->wai_UserData;
		am->am_ID=appstuff->wai_ID;

		if (numargs) numargs=SizeList(&wb->wb_SelectList);
		if (numargs) arglist=ALLOCVEC(numargs * sizeof(struct WBArg), MEMF_CLEAR|MEMF_PUBLIC);
		else arglist=NULL;

		argorig=arglist;

		if (arglist)
		{
			if (SelectSearch(PrepareArg, &arglist, -1, SystemWorkbenchName))
			{
				ExitArgs(numargs, argorig);
				argorig=NULL;
				numargs=0;
			}
		}

		am->am_Message.mn_ReplyPort = &wb->wb_WorkbenchPort;
		am->am_NumArgs = numargs;
		am->am_ArgList = argorig;
		am->am_Version = AM_VERSION;
		if (msg)
		{
			if (mType==MTYPE_APPWINDOW)
			{
				/* convert from wb window co-ords to screen co-ords */
				/* convert from screen co-ords to app's window co-ords */

				am->am_MouseX = msg->MouseX + msg->IDCMPWindow->LeftEdge -
						((struct Window *)(appstuff->wai_Ptr))->LeftEdge;
				am->am_MouseY = msg->MouseY + msg->IDCMPWindow->TopEdge -
						((struct Window *)(appstuff->wai_Ptr))->TopEdge;
			}

			am->am_Seconds = msg->Seconds;
			am->am_Micros  = msg->Micros;
		}

		/*
		 * We need to protect this section.
		 * If there is no message port, we bail out by
		 * replying the message ourselves.
		 */
		FORBID;
		if (appstuff->wai_MsgPort)
		{
			PUTMSG(appstuff->wai_MsgPort, &(am->am_Message));
		}
		else
		{
			REPLYMSG(&(am->am_Message));
		}
		PERMIT;
	}
}

/*
	An appmenuitem has been accessed, let's send a msg
	to the application to let him now about it.
*/
void SendAppMenuItemMsg(ami, msg)
struct WorkbenchAppInfo *ami;
struct IntuiMessage *msg;
{
	DoAppCommon(ami,TRUE,msg,MTYPE_APPMENUITEM);
}

/*
	An icon has been dropped in this window, let's send a message
	to the application to let him now about it.
*/
void SendAppWindowMsg(aw, msg)
struct WorkbenchAppInfo *aw;
struct IntuiMessage *msg;
{
	DoAppCommon(aw,TRUE,msg,MTYPE_APPWINDOW);
}

/*
	An appicon has been accessed, let's send a msg
	to the application to let him now about it.
*/
void SendAppIconMsg(ai, msg, doubleclick)
struct WorkbenchAppInfo *ai;
struct IntuiMessage *msg;
/*
    0 - got here via an icon dropped on an app icon.
   !0 - got here via app icon getting double-clicked on.
*/
int doubleclick;
{
	DoAppCommon(ai,(!doubleclick),msg,MTYPE_APPICON);
}

/* This assumes that *new is already NULL... */
BOOL CopyImage(struct Image *new,struct Image *old)
{
BOOL result=FALSE;
ULONG size;

	new->Width=old->Width;
	new->Height=old->Height;
	new->PlanePick=(1L << (new->Depth=old->Depth)) - 1;
	size=RASSIZE(new->Width,new->Height)*(new->Depth);
	if (new->ImageData=ALLOCVEC(size,MEMF_CHIP))
	{
		memcpy(new->ImageData,old->ImageData,size);
		result=TRUE;
	}
	return(result);
}

void CommonRemoveApp(struct MyAppCommon *app,SHORT atype)
{
struct WorkbenchBase *wb = getWbBase();

	if (app)
	{
		FORBID;

		if (app->atype==APP_NOP) PUTMSG(&(wb->wb_WorkbenchPort),&(app->msg));

		app->atype=atype;

		if (atype==DEL_WINDOW) app->ai.wai_Ptr=NULL;
		app->ai.wai_MsgPort=NULL;

		PERMIT;
	}
}

/*
******* workbench.library/AddAppIconA *****************************************
*
*   NAME
*	AddAppIconA - add an icon to workbench's list of appicons.       (V36)
*
*   SYNOPSIS
*	AppIcon = AddAppIconA(id, userdata, text, msgport,
*	   D0		      D0     D1      A0     A1
*
*					lock, diskobj, taglist)
*					 A2      A3      A4
*
*	struct AppIcon *AddAppIconA(ULONG, ULONG, char *,
*		struct MsgPort *, struct FileLock *, struct DiskObject *,
*		struct TagItem *);
*
*	Alternate, varargs version:
*	struct AppIcon *AddAppIcon(ULONG, ULONG, char *,
*			struct MsgPort *, struct FileLock *,
*			struct DiskObject *,
*			tag1, data1,
*			tag2, data2,
*			...
*			TAG_END );
*
*   FUNCTION
*	Attempt to add an icon to workbench's list of appicons.  If
*	successful, the icon is displayed on the workbench (the same
*	place disk icons are displayed).
*	This call is provided to allow applications to be notified when
*	a graphical object (non neccessarely associated with a file)
*	gets 'manipulated'. (explained later).
*	The notification consists of an AppMessage (found in workbench.h/i)
*	of type 'MTYPE_APPICON' arriving at the message port you specified.
*	The types of 'manipulation' that can occur are:
*	1. Double-clicking on the icon.  am_NumArgs will be zero and
*	   am_ArgList will be NULL.
*	2. Dropping an icon or icons on your appicon.  am_NumArgs will
*          be the number of icons dropped on your app icon plus one.
*          am_ArgList will be an array of ptrs to WBArg structures.
*          Refer to the 'WBStartup Message' section of the RKM for more info.
*	3. Dropping your appicon on another icon.  NOT SUPPORTED.
*
*   INPUTS
*	id - this variable is strictly for your own use and is ignored by
*	     workbench.  Typical uses in C are in switch and case statements,
*	     and in assembly language table lookup.
*	userdata - this variable is strictly for your own use and is ignored
*		   by workbench.
*	text - name of icon (char *)
*	lock - NULL    (Currently unused)
*	msgport - pointer to message port workbench will use to send you an
*		  AppMessage message of type 'MTYPE_APPICON' when your icon
*		  gets 'manipulated' (explained above).
*	diskobj - pointer to a DiskObject structure filled in as follows:
*	    do_Magic - NULL
*	    do_Version - NULL
*	    do_Gadget - a gadget structure filled in as follows:
*		NextGadget - NULL
*		LeftEdge - NULL
*		TopEdge - NULL
*		Width - width of icon hit-box
*		Height - height of icon hit-box
*		Flags - NULL or GADGHIMAGE
*		Activation - NULL
*		GadgetType - NULL
*		GadgetRender - pointer to Image structure filled in as follows:
*		    LeftEdge - NULL
*		    TopEdge - NULL
*		    Width - width of image (must be <= Width of hit box)
*		    Height - height of image (must be <= Height of hit box)
*		    Depth - # of bit-planes in image
*		    ImageData - pointer to actual word aligned bits (CHIP MEM)
*		    PlanePick - Plane mask ((1 << depth) - 1)
*		    PlaneOnOff - 0
*		    NextImage - NULL
*		SelectRender - pointer to alternate Image struct or NULL
*		GadgetText - NULL
*		MutualExclude - NULL
*		SpecialInfo - NULL
*		GadgetID - NULL
*		UserData - NULL
*	    do_Type - NULL
*	    do_DefaultTool - NULL
*	    do_ToolTypes - NULL
*	    do_CurrentX - NO_ICON_POSITION (recommended)
*	    do_CurrentY - NO_ICON_POSITION (recommended)
*	    do_DrawerData - NULL
*	    do_ToolWindow - NULL
*	    do_StackSize - NULL
*
*	(an easy way to create one of these (a DiskObject) is to create an icon
*        with the V2.0 icon editor and save it out.  Your application can then
*        call GetDiskObject on it and pass that to AddAppIcon.)
*
*	taglist - ptr to a list of tag items.  Must be NULL for V2.0.
*
*   RESULTS
*	AppIcon - a pointer to an appicon structure which you pass to
*		  RemoveAppIcon when you want to remove the icon
*		  from workbench's list of appicons.  NULL
*		  if workbench was unable to add your icon; typically
*		  happens when workbench is not running or under low
*		  memory conditions.
*
*   EXAMPLE
*	You could design a print-spooler icon and add it to the workbench.
*	Any file dropped on the print spooler would be printed.  If the
*	user double-clicked (opened) your printer-spooler icon, you could
*	open a window showing the status of the print spool, allow changes
*	to print priorities, allow deletions, etc.  If you registered this
*	window as an 'appwindow' (explained in workbench.library AddAppWindow)
*	files could also be dropped in the window and added to the spool.
*
*   SEE ALSO
*	RemoveAppIcon()
*
*   BUGS
*	Currently Info cannot be obtained on appicons.
*
******************************************************************************
*/
void *AddAppIcon(	ULONG id,
			ULONG userdata,
			char *name,
			struct MsgPort *msgport,
			BPTR lock,
			struct DiskObject *dobj,
			struct TagItem *taglist)
{
struct WorkbenchBase *wb = getWbBase();
struct WorkbenchAppInfo *ai;
struct DiskObject *newobj;
struct MyAppIcon *apicon;
VOID *result=NULL;

	if ((wb->wb_WorkbenchStarted) && !(wb->wb_Quit))
	{
		if (apicon=ALLOCVEC(sizeof(struct MyAppIcon) + strlen(name),MEMF_PUBLIC|MEMF_CLEAR))
		{
			result=apicon;
			apicon->mtype=MTYPE_APP_FUNC;
			ai=&(apicon->ai);
			newobj=&(apicon->dobj);
			strcpy(apicon->name,name);

			/*
			 * Now, we copy the diskobject...
			 */
			newobj->do_Magic=WB_DISKMAGIC;
			newobj->do_Version=WB_DISKVERSION;
			newobj->do_Gadget.Width=dobj->do_Gadget.Width;
			newobj->do_Gadget.Height=dobj->do_Gadget.Height;
			newobj->do_Gadget.Flags=(dobj->do_Gadget.Flags & GADGHIMAGE) | GADGIMAGE;
			newobj->do_Gadget.Activation=GADGIMMEDIATE | RELVERIFY;

			if (!CopyImage(newobj->do_Gadget.GadgetRender=&(apicon->image1),dobj->do_Gadget.GadgetRender)) result=NULL;

			if (newobj->do_Gadget.Flags & GADGHIMAGE)
			{
				if (!CopyImage(newobj->do_Gadget.SelectRender=&(apicon->image2),dobj->do_Gadget.SelectRender)) result=NULL;
			}

			newobj->do_Gadget.GadgetType=BOOLGADGET;
			newobj->do_Type=WBAPPICON;
			newobj->do_CurrentX=dobj->do_CurrentX;
			newobj->do_CurrentY=dobj->do_CurrentY;

			if (result)
			{
				ai->wai_MsgPort=msgport;
				ai->wai_UserData=userdata;
				ai->wai_ID=id;
				apicon->atype=ADD_ICON;
			}
			else
			{
				apicon->atype=DEL_ICON;
			}

			/* Have workbench do what I want */
			PUTMSG(&(wb->wb_WorkbenchPort),&(apicon->msg));
		}
	}
	return(result);
}

/*
******* workbench.library/RemoveAppIcon *************************************
*
*   NAME
*	RemoveAppIcon - remove an icon from workbench's list            (V36)
*                         of appicons.
*
*   SYNOPSIS
*	error = RemoveAppIcon(AppIcon)
*         D0                     A0
*	BOOL RemoveAppIcon(struct AppIcon *);
*
*   FUNCTION
*	Attempt to remove an appicon from workbench's list of appicons.
*
*   INPUTS
*	AppIcon - pointer to an AppIcon structure returned by AddAppIcon.
*
*   RESULTS
*	error - Currently always TRUE...
*
*   NOTES
*	As with anything that deals with async operation, you will need to
*	do a final check for messages on your App message port for messages
*	that may have come in between the last time you checked and the
*	call to removed the App.
*
*   SEE ALSO
*	AddAppIconA()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL RemoveAppIcon(struct MyAppIcon *apicon)
{
	CommonRemoveApp((struct MyAppCommon *)apicon,DEL_ICON);
	return((BOOL)apicon);
}

/*
******* workbench.library/AddAppMenuItemA *************************************
*
*   NAME
*	AddAppMenuItemA - add a menuitem to workbench's list             (V36)
*                         of appmenuitems.
*
*   SYNOPSIS
*	AppMenuItem = AddAppMenuItemA(id, userdata, text, msgport, taglist)
*	D0			      D0     D1      A0     A1       A2
*
*	struct AppMenuItem *AddAppMenuItemA(ULONG, ULONG, char *,
*						struct MsgPort *,
*						struct TagItem *);
*
*	Alternate, varargs version:
*	struct AppMenuItem *AddAppMenuItem(ULONG, ULONG, char *,
*					struct MsgPort *,
*					tag1, data1,
*					tag2, data2,
*					...
*					TAG_END );
*
*   FUNCTION
*	Attempt to add the text as a menuitem to workbench's list
*	of appmenuitems (the 'Tools' menu strip).
*
*   INPUTS
*	id - this variable is strictly for your own use and is ignored by
*	     workbench.  Typical uses in C are in switch and case statements,
*	     and in assembly language table lookup.
*	userdata - this variable is strictly for your own use and is ignored
*		   by workbench.
*	text - text for the menuitem (char *)
*	msgport - pointer to message port workbench will use to send you an
*		  AppMessage message of type 'MTYPE_APPMENUITEM' when your
*		  menuitem gets selected.
*	taglist - ptr to a list of tag items.  Must be NULL for V2.0.
*
*   RESULTS
*	AppMenuItem - a pointer to an appmenuitem structure which you pass to
*		      RemoveAppMenuItem when you want to remove the menuitem
*		      from workbench's list of appmenuitems.  NULL if
*		      workbench was unable to add your menuitem; typically
*		      happens when workbench is not running or under low
*		      memory conditions.
*
*   SEE ALSO
*	RemoveAppMenuItem()
*
*   BUGS
*	Currently does not limit the system to 63 menu items...
*	Any menu items after the 63rd will not be selectable.
*
******************************************************************************
*/
void *AddAppMenuItem(	ULONG id,
			ULONG userdata,
			char *text,
			struct MsgPort *msgport,
			struct TagItem *taglist)
{
struct WorkbenchBase *wb = getWbBase();
struct MyAppMenu *apmenu=NULL;

	if ((wb->wb_WorkbenchStarted) && !(wb->wb_Quit))
	{
		if (apmenu=ALLOCVEC(sizeof(struct MyAppMenu) + strlen(text),MEMF_PUBLIC|MEMF_CLEAR))
		{
			apmenu->mtype=MTYPE_APP_FUNC;
			strcpy(apmenu->text,text);
			apmenu->ai.wai_MsgPort=msgport;
			apmenu->ai.wai_UserData=userdata;
			apmenu->ai.wai_ID=id;
			apmenu->atype=ADD_MENU;
			PUTMSG(&(wb->wb_WorkbenchPort),&(apmenu->msg));
		}
	}
	return(apmenu);
}

/*
******* workbench.library/RemoveAppMenuItem **********************************
*
*   NAME
*	RemoveAppMenuItem - remove a menuitem from workbench's list      (V36)
*			    of appmenuitems.
*
*   SYNOPSIS
*	error = RemoveAppMenuItem(AppMenuItem)
*         D0                            A0
*	BOOL RemoveAppMenuItem(struct AppMenuItem *);
*
*   FUNCTION
*	Attempt to remove an appmenuitem from workbench's list
*	of appmenuitems.
*
*   INPUTS
*	AppMenuItem - pointer to an AppMenuItem structure returned by
*		      AddAppMenuItem.
*
*   RESULTS
*	error - Currently always TRUE...
*
*   NOTES
*	As with anything that deals with async operation, you will need to
*	do a final check for messages on your App message port for messages
*	that may have come in between the last time you checked and the
*	call to removed the App.
*
*   SEE ALSO
*	AddAppMenuItemA()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL RemoveAppMenuItem(struct MyAppMenu *apmenu)
{
	CommonRemoveApp((struct MyAppCommon *)apmenu,DEL_MENU);
	return((BOOL)apmenu);
}

/*
******* workbench.library/AddAppWindowA ***************************************
*
*   NAME
*	AddAppWindowA - add a window to workbench's list of appwindows.  (V36)
*
*   SYNOPSIS
*	AppWindow = AddAppWindowA(id, userdata, window, msgport, taglist)
*	D0			  D0     D1       A0      A1       A2
*
*	struct AppWindow *AddAppWindowA(ULONG, ULONG, struct Window *,
*					struct MsgPort *, struct TagItem *);
*
*	Alternate, varargs version:
*	struct AppWindow *AddAppWindow(ULONG, ULONG, struct Window *,
*					struct MsgPort *
*					tag1, data1,
*					tag2, data2,
*					...
*					TAG_END );
*
*   FUNCTION
*	Attempt to add the window to workbench's list of appwindows.
*	Normally non-workbench windows (those not opened by workbench)
*	cannot have icons dropped in them.  This call is provided to
*	allow applications to be notified when an icon or icons get
*	dropped inside a window that they have registered with workbench.
*	The notification consists of an AppMessage (found in workbench.h/i)
*	of type 'MTYPE_APPWINDOW' arriving at the message port you specified.
*	What you do with the list of icons (pointed to by am_ArgList) is
*	up to you, but generally you would want to call GetDiskObjectNew on
*	them.
*
*   INPUTS
*	id - this variable is strictly for your own use and is ignored by
*	     workbench.  Typical uses in C are in switch and case statements,
*	     and in assembly language table lookup.
*	userdata - this variable is strictly for your own use and is ignored
*		   by workbench.
*	window - pointer to window to add.
*	msgport - pointer to message port workbench will use to send you an
*		  AppMessage message of type 'MTYPE_APPWINDOW' when your
*		  window gets an icon or icons dropped in it.
*	taglist - ptr to a list of tag items.  Must be NULL for V2.0.
*
*   RESULTS
*	AppWindow - a pointer to an appwindow structure which you pass to
*		    RemoveAppWindow when you want to remove the window
*		    from workbench's list of appwindows.  NULL
*		    if workbench was unable to add your window; typically
*		    happens when workbench is not running or under low
*		    memory conditions.
*
*   SEE ALSO
*	RemoveAppWindow()
*
*   NOTES
*       The V2.0 icon editor is an example of an app window.  Note that app
*       window applications generally want to call GetDiskObjectNew
*       (as opposed to GetDiskObject) to get the disk object for the icon
*       dropped in the window.
*
*   BUGS
*	None
*
******************************************************************************
*/
void *AddAppWindow(	ULONG id,
			ULONG userdata,
			struct Window *window,
			struct MsgPort *msgport,
			struct TagItem *taglist)
{
struct WorkbenchBase *wb = getWbBase();
struct MyAppWin *apwin=NULL;

	if ((wb->wb_WorkbenchStarted) && !(wb->wb_Quit))
	{
		if (apwin=ALLOCVEC(sizeof(struct MyAppWin),MEMF_PUBLIC|MEMF_CLEAR))
		{
			apwin->mtype=MTYPE_APP_FUNC;
			apwin->ai.wai_Ptr=window;
			apwin->ai.wai_MsgPort=msgport;
			apwin->ai.wai_UserData=userdata;
			apwin->ai.wai_ID=id;
			apwin->atype=ADD_WINDOW;
			PUTMSG(&(wb->wb_WorkbenchPort),&(apwin->msg));
		}
	}
	return(apwin);
}

/*
******* workbench.library/RemoveAppWindow ***********************************
*
*   NAME
*	RemoveAppWindow - remove a window from workbench's list         (V36)
*                          of appwindows.
*
*   SYNOPSIS
*	error = RemoveAppWindow(AppWindow)
*         D0                       A0
*	BOOL RemoveAppWindow(struct AppWindow *);
*
*   FUNCTION
*	Attempt to remove an appwindow from workbench's list of appwindows.
*
*   INPUTS
*	AppWindow - pointer to an AppWindow structure returned by
*		    AddAppWindow.
*
*   RESULTS
*	error - Currently always TRUE...
*
*   NOTES
*	As with anything that deals with async operation, you will need to
*	do a final check for messages on your App message port for messages
*	that may have come in between the last time you checked and the
*	call to removed the App.
*
*   SEE ALSO
*	AddAppWindowA()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL RemoveAppWindow(struct MyAppWin *apwin)
{
	CommonRemoveApp((struct MyAppCommon *)apwin,DEL_WINDOW);
	return((BOOL)apwin);
}

/*
 * Now, when a message comes in from the app add/remove routines, we
 * just do what we can here...
 */
VOID DoAppWork(struct Message *msg)
{
struct WorkbenchBase *wb = getWbBase();
SHORT *p;
SHORT id;
struct WBObject *obj;

	p=(SHORT *)(((ULONG)msg)-(2*sizeof(SHORT)));

	/* Change the type of work to do in atomic way */
	FORBID;
	id=*p;
	*p=APP_NOP;
	PERMIT;

	switch (id)
	{
	case ADD_ICON:		if (obj=WBAllocWBObject(((struct MyAppIcon *)p)->name))
				{
					((struct MyAppIcon *)p)->ai.wai_Ptr=obj;
					AssignDOtoWBO(obj,&(((struct MyAppIcon *)p)->dobj));
					PlaceObj(obj,wb->wb_RootObject);
					PrepareIcon(obj);
					AddIcon(obj,wb->wb_RootObject);
					MinMaxDrawer(wb->wb_RootObject);
					ADDTAIL(&(wb->wb_AppIconList),&(((struct MyAppIcon *)p)->ai.wai_Node));
				}
				break;

	case ADD_MENU:		if (((struct MyAppMenu *)p)->ai.wai_Ptr=AddToolMenuItem(((struct MyAppMenu *)p)->text))
				{
					ADDTAIL(&(wb->wb_AppMenuItemList),&(((struct MyAppMenu *)p)->ai.wai_Node));
					RethinkMenus();
				}
				break;

	case ADD_WINDOW:	ADDTAIL(&(wb->wb_AppWindowList),&(((struct MyAppMenu *)p)->ai.wai_Node));
				break;

	case DEL_ICON:		FREEVEC(((struct MyAppIcon *)p)->image1.ImageData);
				FREEVEC(((struct MyAppIcon *)p)->image2.ImageData);
				if (((struct MyAppIcon *)p)->ai.wai_Ptr) RemoveObject(((struct MyAppIcon *)p)->ai.wai_Ptr);
				((struct MyAppIcon *)p)->ai.wai_Ptr=NULL;
				/* Fall into the one below... */

	case DEL_MENU:		if (((struct MyAppMenu *)p)->ai.wai_Ptr)
				{
				struct MenuItem *mi;
				int i;

					mi=wb->wb_ToolMenu->FirstItem;
					i=1;
					while (mi)
					{
						if (mi == ((struct MyAppMenu *)p)->ai.wai_Ptr)
						{
							DeleteToolMenuItem(i);
							RethinkMenus();
							mi=NULL;
						}
						else
						{
							i++;
							mi=mi->NextItem;
						}
					}
				}
				/* Fall into the one below... */

	case DEL_WINDOW:	if (((struct MyAppWin *)p)->ai.wai_Node.ln_Succ) REMOVE(&(((struct MyAppWin *)p)->ai.wai_Node));
				FREEVEC(p);
				break;
	}
}
