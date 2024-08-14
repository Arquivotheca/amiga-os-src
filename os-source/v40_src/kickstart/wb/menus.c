/*
 * $Id: menus.c,v 39.2 93/01/11 16:53:42 mks Exp $
 *
 * $Log:	menus.c,v $
 * Revision 39.2  93/01/11  16:53:42  mks
 * Now supports a string filter for the RENAME requester...
 * No longer needs to check for ":" or "/" after the fact.
 * 
 * Revision 39.1  92/06/11  15:48:35  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.6  92/05/30  17:30:56  mks
 * Changed the way it calls Information...
 *
 * Revision 38.5  92/05/26  17:57:18  mks
 * No real change yet... Just restructured for the future
 *
 * Revision 38.4  92/04/27  14:23:04  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.3  92/02/17  19:25:22  mks
 * Removed the About code (to save over 100 bytes!)
 *
 * Revision 38.2  91/08/27  16:35:51  mks
 * Added changes to make menus come up in new-look.
 *
 * Revision 38.1  91/06/24  11:37:24  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/libraries.h"
#include "exec/execbase.h"
#include "exec/alerts.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "libraries/gadtools.h"
#include "dos/dostags.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "diskchange.h"
#include "errorindex.h"
#include "support.h"
#include "quotes.h"

#define Empty(list)	(!((list)->lh_Head->ln_Succ))
#define V(x)	((void*)(x))

/* Size of the allocation to use for FlushLibs() */
#define	ALL_MEM	0x7ffffff0L

struct NewMenu wbnewmenu[] =
{
	{ NM_TITLE, NULL, 0, 0, 0,	V(Q_WORKBENCH_MENU),},
	{  NM_ITEM, NULL, 0, CHECKIT|MENUTOGGLE, 0,	V(Q_FLIP_WB_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_EXECUTE_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_REDRAW_ALL_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_RESCAN_ALL_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_LAST_ERROR_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_VERSION_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_QUIT_MENU),},

	{ NM_TITLE, NULL, 0, 0, 0,	V(Q_WINDOW_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_NEW_DRAWER_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_PARENT_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_CLOSE_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_RESCAN_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_SELECT_ALL_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_CLEAN_UP_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_SNAPSHOT_MENU),},
	{   NM_SUB, NULL, 0, 0, 0,	V(Q_SNAPSHOT_WINDOW_MENU),},
	{   NM_SUB, NULL, 0, 0, 0,	V(Q_SNAPSHOT_ALL_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_SHOW_MENU,)},
	{   NM_SUB, NULL, 0, CHECKIT, ~1,	V(Q_SHOW_ONLY_ICONS_MENU),},
	{   NM_SUB, NULL, 0, CHECKIT, ~2,	V(Q_SHOW_ALL_FILES_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_VIEW_BY_MENU),},
	{   NM_SUB, NULL, 0, CHECKIT, ~1,	V(Q_VIEW_BY_ICON_MENU),},
	{   NM_SUB, NULL, 0, CHECKIT, ~2,	V(Q_VIEW_BY_NAME_MENU),},
	{   NM_SUB, NULL, 0, CHECKIT, ~4,	V(Q_VIEW_BY_DATE_MENU),},
	{   NM_SUB, NULL, 0, CHECKIT, ~8,	V(Q_VIEW_BY_SIZE_MENU),},

	{ NM_TITLE, NULL, 0, 0, 0,	V(Q_ICON_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_OPEN_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_COPY_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_RENAME_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_INFO_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_SNAPSHOTI_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_UNSNAPSHOT_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_LEAVE_OUT_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_PUT_AWAY_MENU),},
	{  NM_ITEM, NM_BARLABEL, 0, 0, 0, 0,},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_DISCARD_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_FORMAT_DISK_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_EMPTY_TRASH_MENU),},
	{   NM_END, 0, 0, 0, 0, 0},
};

struct NewMenu toolnewmenu[] =
{
	{ NM_TITLE, NULL, 0, 0, 0,	V(Q_TOOLS_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_RESETWB_MENU),},
	{   NM_END, 0, 0, 0, 0, 0},
};

/*  This one is used to create a new appmenuitem: */
struct NewMenu toolnewmenuitem[] =
{
	{  NM_ITEM, NULL, 0, 0, 0,	0,},
	{   NM_END, 0, 0, 0, 0, 0},
};

#define MENU_WORKBENCH	(SHIFTMENU(0)|SHIFTITEM(NOITEM)|SHIFTSUB(NOSUB))
#define MENU_FLIPWB	(SHIFTMENU(0)|SHIFTITEM(0)|SHIFTSUB(NOSUB))
#define MENU_EXECUTE	(SHIFTMENU(0)|SHIFTITEM(1)|SHIFTSUB(NOSUB))
#define MENU_REDRAW_ALL	(SHIFTMENU(0)|SHIFTITEM(2)|SHIFTSUB(NOSUB))
#define MENU_RESCAN_ALL	(SHIFTMENU(0)|SHIFTITEM(3)|SHIFTSUB(NOSUB))
#define MENU_LASTERROR	(SHIFTMENU(0)|SHIFTITEM(4)|SHIFTSUB(NOSUB))
#define MENU_VERSION	(SHIFTMENU(0)|SHIFTITEM(5)|SHIFTSUB(NOSUB))
#define MENU_QUIT	(SHIFTMENU(0)|SHIFTITEM(6)|SHIFTSUB(NOSUB))

#define MENU_WINDOW	(SHIFTMENU(1)|SHIFTITEM(NOITEM)|SHIFTSUB(NOSUB))
#define MENU_NEW_DRAWER	(SHIFTMENU(1)|SHIFTITEM(0)|SHIFTSUB(NOSUB))
#define MENU_PARENT	(SHIFTMENU(1)|SHIFTITEM(1)|SHIFTSUB(NOSUB))
#define MENU_CLOSE	(SHIFTMENU(1)|SHIFTITEM(2)|SHIFTSUB(NOSUB))
#define MENU_RESCAN	(SHIFTMENU(1)|SHIFTITEM(3)|SHIFTSUB(NOSUB))
#define MENU_SELECT_ALL	(SHIFTMENU(1)|SHIFTITEM(4)|SHIFTSUB(NOSUB))
#define MENU_CLEANUP	(SHIFTMENU(1)|SHIFTITEM(5)|SHIFTSUB(NOSUB))
#define MENU_SNAPSHOT	(SHIFTMENU(1)|SHIFTITEM(6)|SHIFTSUB(NOSUB))
#define MENU_SHOW	(SHIFTMENU(1)|SHIFTITEM(7)|SHIFTSUB(NOSUB))
#define MENU_VIEWBY	(SHIFTMENU(1)|SHIFTITEM(8)|SHIFTSUB(NOSUB))

#define MENU_ICONS	(SHIFTMENU(2)|SHIFTITEM(NOITEM)|SHIFTSUB(NOSUB))
#define MENU_OPEN	(SHIFTMENU(2)|SHIFTITEM(0)|SHIFTSUB(NOSUB))
#define MENU_COPY	(SHIFTMENU(2)|SHIFTITEM(1)|SHIFTSUB(NOSUB))
#define MENU_RENAME	(SHIFTMENU(2)|SHIFTITEM(2)|SHIFTSUB(NOSUB))
#define MENU_INFO	(SHIFTMENU(2)|SHIFTITEM(3)|SHIFTSUB(NOSUB))
#define MENU_SNAPSHOTI	(SHIFTMENU(2)|SHIFTITEM(4)|SHIFTSUB(NOSUB))
#define MENU_UNSNAPSHOT	(SHIFTMENU(2)|SHIFTITEM(5)|SHIFTSUB(NOSUB))
#define MENU_LEAVE_OUT	(SHIFTMENU(2)|SHIFTITEM(6)|SHIFTSUB(NOSUB))
#define MENU_PUT_AWAY	(SHIFTMENU(2)|SHIFTITEM(7)|SHIFTSUB(NOSUB))
#define MENU_SEPARATOR	(SHIFTMENU(2)|SHIFTITEM(8)|SHIFTSUB(NOSUB))
#define MENU_DISCARD	(SHIFTMENU(2)|SHIFTITEM(9)|SHIFTSUB(NOSUB))
#define MENU_FORMAT	(SHIFTMENU(2)|SHIFTITEM(10)|SHIFTSUB(NOSUB))
#define MENU_EMPTYTRASH	(SHIFTMENU(2)|SHIFTITEM(11)|SHIFTSUB(NOSUB))

#define TOOLSMENU	3
#define MENU_RESETWB	(SHIFTMENU(TOOLSMENU)|SHIFTITEM(0)|SHIFTSUB(NOSUB))

struct NewMenu hiddennewmenu[] =
{
	{ NM_TITLE, NULL, 0, 0, 0,	V(Q_DEBUG_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_ROMWACK_MENU),},
	{  NM_ITEM, NULL, 0, 0, 0,	V(Q_FLUSHLIBS_MENU),},
	{  NM_END, 0, 0, 0, 0, 0,},
};

#define MENU_DEBUG	(SHIFTMENU(TOOLSMENU + 1)|SHIFTITEM(0)|SHIFTSUB(NOSUB))
#define MENU_FLUSHLIBS	(SHIFTMENU(TOOLSMENU + 1)|SHIFTITEM(1)|SHIFTSUB(NOSUB))

#define NUMSNAPSHOT	2	/* this MUST track the # of entries in array */
#define SNAPSHOTSUB(n)	((MENU_SNAPSHOT & ~SHIFTSUB(NOSUB))|SHIFTSUB(n))
#define MENU_SNAPSHOT_WINDOW	SNAPSHOTSUB(0)
#define MENU_SNAPSHOT_ALL	SNAPSHOTSUB(1)

#define NUMVIEWBY	4	/* this MUST track the # of entries in array */
#define VIEWBYSUB(n)	((MENU_VIEWBY & ~SHIFTSUB(NOSUB))|SHIFTSUB(n))
#define MENU_VIEWBYICON	VIEWBYSUB(0)
#define MENU_VIEWBYNAME	VIEWBYSUB(1)
#define MENU_VIEWBYDATE	VIEWBYSUB(2)
#define MENU_VIEWBYSIZE	VIEWBYSUB(3)

#define NUMSHOW		2	/* this MUST track the # of entries in array */
#define SHOWSUB(n)	((MENU_SHOW & ~SHIFTSUB(NOSUB))|SHIFTSUB(n))
#define MENU_SHOWONLYICONS	SHOWSUB(0)
#define MENU_SHOWALLFILES	SHOWSUB(1)

extern	void	*AboutSegment;


/* MenuCat():
**
** Appends the tailmenu after all the menus in the headmenu
**
*/

void MenuCat(struct Menu *tailmenu,struct Menu *headmenu)
{
    MP(("MenuCat: enter\n"));
    if (tailmenu)
    {
	while (headmenu->NextMenu)
	{
	    headmenu = headmenu->NextMenu;
	}

	headmenu->NextMenu = tailmenu;
    }
    MP(("MenuCat: exit\n"));
}

/* CreateWBMenus():
**
** Creates the three fixed menus, the toolmenu, and conditionally
** creates the hidden menu.  Links the three together.
**
*/

int CreateWBMenus()
{
struct WorkbenchBase *wb = getWbBase();
int status=FALSE;

    MP(("CreateWBMenus: enter\n"));

    wb->wb_MenuStrip = CreateMenus(wbnewmenu,TAG_DONE);

    wb->wb_ToolMenu = CreateMenus(toolnewmenu,TAG_DONE);

    if ((wb->wb_MenuStrip) && (wb->wb_ToolMenu))
    {
	status=TRUE;
        if (wb->wb_Argument & WBARGF_DEBUGON)
        {
            if (!(wb->wb_HiddenMenu = CreateMenus(hiddennewmenu,TAG_DONE)))
            {
		status=FALSE;
            }
        }

	if (status)
	{
            MenuCat(wb->wb_ToolMenu, wb->wb_MenuStrip);
            MenuCat(wb->wb_HiddenMenu, wb->wb_ToolMenu);

            status=LayoutWBMenus();
	}
    }
    MP(("CreateWBMenus: exit\n"));
    return(status);
}


/* FreeWBMenus():
**
** Frees all allocated menu memory in preparation for exit.
**
*/

void FreeWBMenus()
{
struct WorkbenchBase *wb = getWbBase();

    MP(("FreeWBMenus: enter\n"));
    MP(("FWBM:  Freeing menu strips\n"));
    FreeMenus(wb->wb_HiddenMenu);
    FreeMenus(wb->wb_ToolMenu);
    FreeMenus(wb->wb_MenuStrip);
    MP(("FreeWBMenus: exit\n"));
}

/* LayoutWBMenus():
**
** Installs language strings into menu items and lays out the menus.
**
*/

int LayoutWBMenus()
{
struct WorkbenchBase *wb = getWbBase();

    InstallMenuStrings(wb->wb_MenuStrip);
    return((int)LayoutMenus(wb->wb_MenuStrip, wb->wb_VisualInfo,
					GTMN_NewLookMenus,TRUE,
					TAG_DONE));
}


/* InstallMenuStrings():
**
** Walks the supplied menu structure, and installs the appropriate
** quotes for menu labels and key equivalents, depending on the
** index found in GTMENU(ITEM)_USERDATA(item).
**
*/

void InstallMenuStrings(menu)
struct Menu *menu;
{
    struct MenuItem *item;
    struct MenuItem *sub;

    MP(("InstallMenuStrings:  enter\n"));
    while (menu)
    {
	menu->MenuName = Quote((UWORD)GTMENU_USERDATA(menu));
	MP(("IMS:  Menu: '%s'\n", menu->MenuName));
	item = menu->FirstItem;

	while (item)
	{
	    InstallItemString(item);
	    sub = item->SubItem;
	    while (sub)
	    {
		InstallItemString(sub);
		sub = sub->NextItem;
	    }
	    item = item->NextItem;
	}
    menu = menu->NextMenu;
    }
    MP(("InstallMenuStrings:  exit\n"));
}


/* InstallItemString():
**
** Install string in a menuitem or subitem, based on quote number
** in menuitem UserData.
**
*/

void InstallItemString(item)

struct MenuItem *item;

{
    STRPTR commkey;

    if (GTMENUITEM_USERDATA(item))
    {
	((struct IntuiText *)item->ItemFill)->IText = Quote((UWORD)GTMENUITEM_USERDATA(item));

	item->Flags &= ~COMMSEQ;
	item->Command = 0;

	if ( commkey = Quote((UWORD) (((ULONG)GTMENUITEM_USERDATA(item)) +1)) )
	{
	    item->Flags |= COMMSEQ;
	    item->Command = *commkey;
	}
    }
}

/* AddToolMenuItem():
**
** Adds a menuitem with the given text to the end of tools menu.
**
*/

struct MenuItem *AddToolMenuItem(STRPTR text)
{
    struct WorkbenchBase *wb = getWbBase();
    struct MenuItem *item, *newitem;

    MP(("AddToolMenuItem: enter, text='%s'\n", text));
    /*  Clear all menu strips: */

    RemoveMenus();

    /*  CreateMenus and FreeMenus can handle menu items as well as menus */
    if (newitem = (struct MenuItem *)CreateMenus(toolnewmenuitem,TAG_DONE))
    {
        ((struct IntuiText *)newitem->ItemFill)->IText = text;

        /*  Add the new menu item after the existing ones in the ToolMenu */
        for (item = wb->wb_ToolMenu->FirstItem; item->NextItem; item = item->NextItem);
        item->NextItem = newitem;
    }

    /* Re-layout and set all the menu strips: */
    ReLayoutToolMenu();

    MP(("AddToolMenuItem: exit\n"));
    return(newitem);
}

/* DeleteToolMenuItem():
**
** Delete a menuitem of the given number from the tools menu.
**
*/

void DeleteToolMenuItem(n)

long n;

{
    struct WorkbenchBase *wb = getWbBase();
    struct MenuItem *previtem;
    struct MenuItem *item;

    MP(("DeleteToolMenuItem: enter, number=%ld\n", n));
    /*  Clear all menu strips */

    RemoveMenus();

    previtem = NULL;
    item = wb->wb_ToolMenu->FirstItem;

    /*  Delete the nth (counting from 1) menuitem: */
    while (--n)
    {
	previtem = item;
	item = item->NextItem;
    }

    if (previtem)
    {
	previtem->NextItem = item->NextItem;
    }
    else
    {
	wb->wb_ToolMenu->FirstItem = NULL;
    }

    /*  CreateMenus and FreeMenus can handle menu items as well as menus */
    FreeMenus((struct Menu *)item);

    /*  Re-lay out the tool menu and set all the menu strips: */
    ReLayoutToolMenu();

    MP(("DeleteToolMenuItem: exit\n"));
}


/* ReLayoutToolMenu():
**
** Standard operation after changing the tool menu, namely CreateMenus() the
** new ToolMenu, re-link it in, relayout the toolmenu, and set menu
** strips for everyone.
*/

void ReLayoutToolMenu()

{
struct WorkbenchBase *wb = getWbBase();

	/*
	 * This should never fail as we have the font open already
	 * and should be all set to go.
	 *
	 * If it does fail for some reason, there is nothing I can do
	 * to fix the problem.  In fact, at that point my structures
	 * will be so messed up that I will need to reboot anyway.
	 */
    if (!(LayoutMenuItems(wb->wb_ToolMenu->FirstItem, wb->wb_VisualInfo,
					GTMN_Menu,wb->wb_ToolMenu,
					GTMN_NewLookMenus,TRUE,
					TAG_DONE)))
    {
	Alert(AN_WBReLayoutToolMenu | AT_DeadEnd);	/* ZZZ: What do I do here? */
    }

    /* Make sure that we do not re-add the menus if we are in "busy" mode */
    if (!wb->wb_DiskIONestCnt) MasterSearch(WindowOperate, SetMenuStrip, wb->wb_MenuStrip);
}

/* RemoveMenus():
**
** Removes the menus from all windows.
**
*/

void RemoveMenus()
{
    MP(("RemoveMenus: enter\n"));

    MasterSearch( WindowOperate, ClearMenuStrip );

    MP(("RemoveMenus: exit\n"));
}


/* RethinkMenus():
**
** Enables/disables menu items according to the state of WB.
**
*/

void RethinkMenus()
{
    /*  Do it for each window: */
    MasterSearch(OnOffMenus);
}


/* AbleMenu():
**
** Enable or disable a menu or menu item.
**
*/

void AbleMenu(struct Window *win,UWORD menunumber,BOOL able)
{
    if (able)
    {
	MP(("AM:  OnMenu(win = $%lx, menunum = $%lx), MS=$%lx\n",win, menunumber, win->MenuStrip));
	OnMenu(win, menunumber);
    }
    else
    {
	MP(("AM:  OffMenu(win = $%lx, menunum = $%lx), MS=$%lx\n",win, menunumber, win->MenuStrip));
	OffMenu(win, menunumber);
    }
}

/*
 * The following routine returns TRUE if the object is
 * NOT of the type given.
 */
long CheckForNotType(struct WBObject *obj,ULONG thetype)
{
    return(!((1L << obj->wo_Type) & thetype));
}

long CheckForType(struct WBObject *obj,ULONG thetype)
{
    return((long)((1L << obj->wo_Type) & thetype));
}

/* OnOffMenus():
**
** Enables/disables menu items for a window of the supplied object
** according to the state of WB.
**
** Also makes sure the checkmarks are set correctly...
**
*/

int OnOffMenus(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd;
struct Window *win;
struct MenuItem *item;
ULONG menunum;
BOOL flag;

    MP(("OnOffMenus: enter, obj=$%lx (%s)\n",obj, obj ? obj->wo_Name : "NULL"));
    if (!wb->wb_DiskIONestCnt) if (dd = obj->wo_DrawerData) if (win = dd->dd_DrawerWin) if (win->Flags & WINDOWACTIVE)
    {
	/* Make sure the check marks are correct... */

        /*
         * mks:  We need to first clear the menu strip...
         */
        ClearMenuStrip(win);

        /* sync the backdrop checkmark... */
        item = ItemAddress(wb->wb_MenuStrip, MENU_FLIPWB);
        if (wb->wb_Backdrop) item->Flags |= CHECKED; /* Set it... */
        else item->Flags &= ~CHECKED; /* clear it... */

        /* sync checkmarks for show menu items */
        item = ItemAddress(wb->wb_MenuStrip, MENU_SHOWONLYICONS);
        item->Flags &= ~CHECKED; /* clear */
        item = item->NextItem; /* ShowAllFiles */
        item->Flags &= ~CHECKED; /* clear */

        menunum=MENU_SHOWONLYICONS;
        if (((dd->dd_Flags) & DDFLAGS_SHOWALL)==DDFLAGS_SHOWALL) menunum=MENU_SHOWALLFILES;

        item = ItemAddress(wb->wb_MenuStrip, menunum);
        item->Flags |= CHECKED;

        /* sync checkmark for viewby menu items */
        item = ItemAddress(wb->wb_MenuStrip, MENU_VIEWBYICON);
        item->Flags &= ~CHECKED; /* clear */
        item = item->NextItem; /* ViewByName */
        item->Flags &= ~CHECKED; /* clear */
        item = item->NextItem; /* ViewByDate */
        item->Flags &= ~CHECKED; /* clear */
        item = item->NextItem; /* ViewBySize */
        item->Flags &= ~CHECKED; /* clear */
        switch (dd->dd_ViewModes)
        {
            case DDVM_BYNAME:
                menunum = MENU_VIEWBYNAME;
                break;

            case DDVM_BYDATE:
                menunum = MENU_VIEWBYDATE;
                break;

            case DDVM_BYSIZE:
                menunum = MENU_VIEWBYSIZE;
                break;

            default:
                menunum = MENU_VIEWBYICON;
                break;
        }
        item = ItemAddress(wb->wb_MenuStrip, menunum);
        item->Flags |= CHECKED;

        /*
         * mks:  Now, re-add the menus to the strip...
         */
        ResetMenuStrip(win,wb->wb_MenuStrip);

	/*  Enable the cleanup option only if icons are shown */
	AbleMenu(win, MENU_CLEANUP, (menunum == MENU_VIEWBYICON));

	/*  Enable the Icons menu only if there are selected icons: */
	AbleMenu(win, MENU_ICONS, (BOOL)!Empty(&wb->wb_SelectList));

	flag=!(wb->wb_BackWindow->Flags & WFLG_WINDOWACTIVE);

	/* Enable the 'Close' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_CLOSE, flag);

	/* Enable the 'New Drawer' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_NEW_DRAWER, flag);

	/* Enable the 'Open Parent' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_PARENT, flag);

	/* Enable the 'Update' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_RESCAN, flag);

	/* Enable the 'View By' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_VIEWBY, flag);

	/* Enable the 'Show' menu only if the workbench window is not the active window */
	AbleMenu(win, MENU_SHOW, flag);

	/* Enable the 'Delete' menu only if no disk or trashcan is selected */
	AbleMenu(win, MENU_DISCARD, !SelectSearch(CheckForType,ISTRASHLIKE|ISDISKLIKE));

	/* Enable the 'Empty trash' menu only if trashcan is selected */
	AbleMenu(win, MENU_EMPTYTRASH, !SelectSearch(CheckForNotType,ISTRASHLIKE));

	/* Enable the 'Format' menu only if disks are selected */
	AbleMenu(win, MENU_FORMAT, !SelectSearch(CheckForNotType,ISDISKLIKE));
    }
    MP(("OnOffMenus: exit\n"));
    return(0);
}

/* ResetMenus():
**
** Removes menus from all windows, re-lays them out with
** the current font/language, adds them back, and rethinks which
** items are enabled.
**
*/

void ResetMenus()

{
struct WorkbenchBase *wb = getWbBase();

    MP(("ResetMenus: enter\n"));

    RemoveMenus();
    LayoutWBMenus();
    MasterSearch(WindowOperate, SetMenuStrip, wb->wb_MenuStrip);

    RethinkMenus();
    MP(("ResetMenus: exit\n"));
}

WindowOperate( obj, func, arg1, arg2 )
struct WBObject *obj;
int (*func)();
LONG arg1, arg2;
{
    struct NewDD *dd;
    struct Window *window;

    if( dd = obj->wo_DrawerData ) {
	if( window = dd->dd_DrawerWin ) {
	    (*func)( window, arg1, arg2 );
	}
    }
    return( 0 );
}

Snapshot(obj)
struct WBObject *obj;
{
struct WorkbenchBase *wb = getWbBase();
int result=0;

    if (obj == wb->wb_RootObject) WriteWBPrefs(); /* Snapshot the root... */
    else if (!(obj->wo_Parent->wo_DrawerData->dd_ViewModes > DDVM_BYICON))
    {
        /* this rule is softer and allows backdrop icons to be snapshottted */
        if (CheckForNotType(obj,WBF_DEVICE|WBF_KICK|WBF_APPICON))
        {
            UpdateWindowSize(obj);
            if (!PutObject(obj))
            {
                ErrorDos(ERR_WRITE, obj->wo_Name);
		result=1;
            }
        }
    }
    return(result);
}

CountItems(obj, filecount, drawercount)
struct WBObject *obj;
ULONG *filecount, *drawercount;
{
    if (obj->wo_Type == WBDRAWER) (*drawercount)++;
    else (*filecount)++;
    return(NULL);
}

void SnapshotAll(drawer)
struct WBObject *drawer;
{
    BeginDiskIO();
    /* snapshot all of this drawer's children */
    SiblingPredSearch(drawer->wo_DrawerData->dd_Children.lh_TailPred, Snapshot);
    Snapshot(drawer); /* snapshot this drawer */
    EndDiskIO();
}

DoEmptyTrash(obj)
struct WBObject *obj;
{
    if (obj->wo_Type == WBGARBAGE) EmptyTrash(obj);
    return(NULL);
}

UnSnapshot(obj)
struct WBObject *obj;
{
struct WorkbenchBase *wb = getWbBase();
LONG savecurrentx, savecurrenty, savesavex, savesavey;
int result=FALSE;

    if (obj != wb->wb_RootObject)
    {
	BeginDiskIO();
        savecurrentx = obj->wo_CurrentX;
        savecurrenty = obj->wo_CurrentY;
        savesavex = obj->wo_SaveX;
        savesavey = obj->wo_SaveY;
        obj->wo_CurrentX = obj->wo_CurrentY = obj->wo_SaveX = obj->wo_SaveY = NO_ICON_POSITION;
        result=Snapshot(obj);
        obj->wo_CurrentX = savecurrentx;
        obj->wo_CurrentY = savecurrenty;
        obj->wo_SaveX = savesavex;
        obj->wo_SaveY = savesavey;
        EndDiskIO();
    }
    return(result);
}

RescanRemoveObjectCommon(struct WBObject *obj)
{
    struct NewDD *dd;

    MP(("RROC: enter, obj=$%lx (%s), dd=$%lx, win=$%lx\n",
	obj, obj->wo_Name, obj->wo_DrawerData,
	obj->wo_DrawerData ? obj->wo_DrawerData->dd_DrawerWin : NULL));
    MP(("\tDrawerOpen=%ld, DrawerSilent=%ld\n",
	obj->wo_DrawerOpen, obj->wo_DrawerSilent));
    /* do not remove objects which have open drawers */
    if (dd = obj->wo_DrawerData) { /* if this is a drawer */
	/* if this drawer has an open window or is silent, do not remove it. */
	if (obj->wo_DrawerData->dd_DrawerWin || obj->wo_DrawerSilent) {
	    MP(("RROC: bypassing $%lx (%s)\n", obj, obj->wo_Name));
	    goto end;
	}
    }
    MP(("RROC: calling RemoveObjectCommon for $%lx (%s)\n",
	obj, obj->wo_Name));
    RemoveObjectCommon(obj);
end:
    MP(("RROC: exit\n"));
    return(0);
}

RescanCommon(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd;

    MP(("RescanCommon: enter, obj=$%lx (%s), Type=$%lx\n",obj, obj->wo_Name, obj->wo_Type));
    /* if not the root object */
    if (obj != wb->wb_RootObject)
    {
	if (dd = obj->wo_DrawerData)
	{ /* obj has a drawer */
	    MP(("RC: obj has a drawer\n"));
	    if (dd->dd_DrawerWin)
	    { /* drawer has a window */
		MP(("RC: removing all of $%lx (%s) 's children\n", obj, obj->wo_Name));
		/* remove all of this drawer's children */
		SiblingSuccSearch(dd->dd_Children.lh_Head, RescanRemoveObjectCommon);
		/* reset the window origin */
		dd->dd_CurrentX = dd->dd_CurrentY = 0;
		MP(("RC: refreshing drawer\n"));
		RefreshDrawer(obj); /* show empty drawer */
		MP(("RC: calling OpenCommon\n"));
		if (!OpenCommon(obj, NULL, TRUE))
		{
		    CloseDrawer(obj);
		}
		else
		{
		    MinMaxDrawer(obj); /* update scroll bars */
		}
	    }
	}
    }
    MP(("RescanCommon: exit\n"));
    return(0);
}

void Menumeration(msg)
struct IntuiMessage *msg;
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *drawer;
    struct WorkbenchAppInfo *ami;
    struct MenuItem *item;
    BPTR lock;
    USHORT code;

    /* convert IDCMPWindow to drawer, do sanity error check */
    drawer=WindowToObj(msg->IDCMPWindow);
    while ( (drawer) && ((code=msg->Code)!=MENUNULL) )
    {
        MP(("Menumeration: drawer=$%lx (%s)\n", drawer, drawer->wo_Name ));

        /* reset any errors that are displayed */
        ErrorTitle(wb->wb_ScreenTitle);

        item = ItemAddress(wb->wb_MenuStrip, code);
        msg->Code=item->NextSelect;

        switch(code) {

/****************************************************************************
            WORKBENCH MENU stuff
*****************************************************************************/

        case MENU_FLIPWB:
            {
	    struct Window *olddiskwin = wb->wb_BackWindow; /* save old window ptr */
	    struct NewDD *dd = wb->wb_RootObject->wo_DrawerData;
	    struct WBObject *obj = wb->wb_RootObject;

		if (((item->Flags & CHECKED) ? 1 : 0) != wb->wb_Backdrop)
		{
		    wb->wb_Backdrop ^= 1; /* we want the other type of window */
		    obj->wo_DrawerOpen = 1; /* flag that this drawer is only partially open */
		    dd->dd_DrawerWin=NULL;  /* Make it look like we are not open */

		    COPY_WBOX(&(dd->dd_NewWindow.LeftEdge),&(wb->wb_Config.wbc_LeftEdge));

		    if (!wbReopen(obj))	/* wbReopen() returns TRUE on failure */
		    { /* if it opened... */
			ClosePWindow(olddiskwin, dd); /* close old window */
		    }
		    else
		    {
			wb->wb_Backdrop ^= 1; /* we did not get the new window */
			dd->dd_DrawerWin=olddiskwin;
			obj->wo_DrawerOpen=0;
			NoMem(); /* tell user of failure */
		    }
		    OnOffMenus(obj);
		}
	    }
            break;

        case MENU_EXECUTE:
            MP(("Execute:\n"));
            lock = LockRamDisk();
            DoExecute(NULL,lock);
            UNLOCK(lock);
            break;

        case MENU_REDRAW_ALL:
            MP(( "Redraw\n" ));
            MasterSearch( RefreshDrawer );
            break;

        case MENU_RESCAN_ALL:
            {
	    struct WBObject *myobj=NULL;

		/*
		 * The following is a nasty trick...
		 * What I do, is to make sure that I go through the whole
		 * object list by going until the root object is
		 * found, which I had just placed on the end of the
		 * list.  This makes sure that I do not run into a
		 * object removed problem that can happen with
		 * the MasterSearch() routine.
		 */
                MP(("Rescan All:\n"));
                BeginDiskIO();
		REMOVE((struct Node *)(wb->wb_RootObject));
		ADDTAIL(&(wb->wb_MasterList),(struct Node *)(wb->wb_RootObject));
		while (!myobj)
		{
		    myobj=(struct WBObject *)RemHead(&(wb->wb_MasterList));
		    ADDTAIL(&(wb->wb_MasterList),(struct Node *)myobj);
		    RescanCommon(myobj);
		    if (myobj!=wb->wb_RootObject) myobj=NULL;
		}
                EndDiskIO();
	    }
            break;

        case MENU_LASTERROR:
            MP(("Last Error: (current <%s>)\n", wb->wb_CurrentError));
            ErrorTitle(wb->wb_CurrentError);
            break;

        case MENU_VERSION:
            MP(("Version:\n"));
	    CREATEPROC(Quote(Q_VERSION_MENU),5,MKBADDR(&AboutSegment),4096);
            break;

        case MENU_QUIT:
            MP(("Quit:\n"));
            QuitCheck();
            drawer=NULL;
            break;

/****************************************************************************
            WINDOW MENU stuff
*****************************************************************************/

        case MENU_NEW_DRAWER:
            MP(("New Drawer\n"));
            BeginDiskIO();
            NewDrawer(drawer);
            EndDiskIO();
            break;

        case MENU_PARENT:
            MP(("Parent:\n"));
            if (drawer->wo_Parent) DoOpen2(drawer->wo_Parent); /* open parent window */
            else ErrorTitle(Quote(Q_DRW_HAS_NO_PARENT)); /* there is no parent window */
            msg->Code=MENUNULL;
            break;

        case MENU_CLOSE:
            MP(("Close:\n"));
            if (drawer!=wb->wb_RootObject) DoClose2(drawer);
            break;

        case MENU_RESCAN:
            MP(("Rescan:\n"));
            BeginDiskIO();
            RescanCommon(drawer);
            EndDiskIO();
            break;

        case MENU_SELECT_ALL:
            MP(("SelectAll:\n"));
            SelectAll(msg, drawer);
            break;

        case MENU_CLEANUP:
            MP(("Clean Up:\n"));
            Cleanup(drawer);
            break;

        case MENU_SNAPSHOT_WINDOW:
            MP(("Snapshot All:\n"));
            Snapshot(drawer);
            break;

        case MENU_SNAPSHOT_ALL:
            MP(("Snapshot All:\n"));
            SnapshotAll(drawer);
            break;

        case MENU_SHOWONLYICONS:
            MP(("Show Only Icons:\n"));
            ChangeShowAll(drawer, DDFLAGS_SHOWICONS);
            break;

        case MENU_SHOWALLFILES:
            MP(("Show All Files:\n"));
            ChangeShowAll(drawer, DDFLAGS_SHOWALL);
            break;

        case MENU_VIEWBYICON:
            MP(("ViewBy Icon:\n"));
            ChangeViewBy(drawer, DDVM_BYICON);
            break;

        case MENU_VIEWBYNAME:
            MP(("ViewBy Name:\n"));
            ChangeViewBy(drawer, DDVM_BYNAME);
            break;

        case MENU_VIEWBYSIZE:
            MP(("ViewBy Size:\n"));
            ChangeViewBy(drawer, DDVM_BYSIZE);
            break;

        case MENU_VIEWBYDATE:
            MP(("ViewBy Date:\n"));
            ChangeViewBy(drawer, DDVM_BYDATE);
            break;

/****************************************************************************
            ICON MENU stuff
*****************************************************************************/

        case MENU_OPEN:
            MP(("Open:\n"));
            DoOpen(); /* does the selectsearch for me */
            msg->Code=MENUNULL;
            break;

        case MENU_COPY:
            MP(("Copy:\n"));
            SelectBracketDiskIO(Duplicate, NULL);
            break;

        case MENU_RENAME:
            MP(("Rename:\n"));
            SelectSearch(RenameObject, NULL);
            break;

        case MENU_INFO:
            MP(("Info:\n"));
            SelectBracketDiskIO(SyncInfo, NULL);
            msg->Code=MENUNULL;
            break;

        case MENU_SNAPSHOTI:
            MP(("Snapshot:\n"));
            SelectBracketDiskIO(Snapshot, NULL);
            break;

        case MENU_UNSNAPSHOT:
            MP(("UnSnapshot:\n"));
            SelectBracketDiskIO(UnSnapshot, NULL);
            break;

        case MENU_LEAVE_OUT:
            MP(("LeaveOut:\n"));
            SelectBracketDiskIO(LeaveOut, PUTAWAY_NORMAL);
            break;

        case MENU_PUT_AWAY:
            MP(("PutAway:\n"));
            /* put object away, do not bypass removing object from .backdrop file */
            SelectBracketDiskIO(PutAway, PUTAWAY_NORMAL);
            break;

        case MENU_DISCARD:
            {
            struct EasyStruct es = {sizeof(struct EasyStruct),NULL,SystemWorkbenchName,NULL,NULL};
	    ULONG drawercount[2];

                MP(("Discard:\n"));
                BeginDiskIO();
                drawercount[0] = drawercount[1] = 0;
                SelectSearch(CountItems, &drawercount[0], &drawercount[1]); /* count # of items to discard */
                MP(("\tfilecount = %ld, drawercount = %ld\n", drawercount[0], drawercount[1]));
                es.es_TextFormat = Quote(Q_DISCARD_REQ_TEXT);
                es.es_GadgetFormat = Quote(Q_OK_CANCEL_TEXT);
                if (EasyRequestArgs(wb->wb_BackWindow, &es, NULL, &drawercount[0])) SelectSearch(Discard);
                EndDiskIO();
            }
            break;

        case MENU_FORMAT:
            MP(("Format\n"));
            SelectSearch(WBFormat);
            msg->Code=MENUNULL;
            break;

        case MENU_EMPTYTRASH:
            MP(("Empty Trash\n"));
            SelectBracketDiskIO(DoEmptyTrash, NULL);
            break;

/****************************************************************************
            DEBUG MENU stuff
*****************************************************************************/

        case MENU_DEBUG:
            Debug(0L);
            break;

        case MENU_FLUSHLIBS:
            {
            void *tmp;

                MP(("Flushlibs:\n"));
                if (tmp=ALLOCVEC(ALL_MEM,MEMF_PUBLIC)) FREEVEC(tmp);
            }
            break;

/****************************************************************************
            TOOLS MENU stuff
*****************************************************************************/

        case MENU_RESETWB:
            MP(("menu ResetWB\n"));
            ResetWB();
            break;

        default:
            MP(("menu DEFAULT (maybe an appmenu item)\n"));
            /* see if this menu item is an application menu item */
            if (MENUNUM(code) == TOOLSMENU)
	    {
                ami = MenuToAppMenuItem(code);
                MP(("Menumeration: calling SendAppMenuItemMsg, ami=%06lx\n",ami));
                SendAppMenuItemMsg(ami, msg);
                msg->Code=MENUNULL;
            }
            break;

        }
        if (drawer) drawer=WindowToObj(msg->IDCMPWindow);
    }

    RethinkMenus();
}

void DoExecute(buf, lock)
char *buf;
BPTR lock;
{
    struct WorkbenchBase *wb = getWbBase();
#ifdef DEBUGGING
    struct Process *proc;
    struct CommandLineInterface *cli;
#endif DEBUGGING
    BPTR fhout, oldcd;
    long success;
    struct TagItem tagitem[8] = {
	    {SYS_Input, NULL},
	    {SYS_Output, NULL},
	    {SYS_Asynch, TRUE},
	/*
	 * We should execute the user commands in the user shell...
	 */
	    {SYS_UserShell, TRUE},

	/*
	 * User started items should be priority 0 and stack 4096...
	 */
	    {NP_Priority, 0},
	    {NP_StackSize, 4096},
	    {NP_WindowPtr, NULL},	/* No window pointer */
	    {TAG_DONE, NULL},
    };

    MP(("DoExecute: enter, buf=%s\n", buf));

    if (buf)
    {
    char *s;

	strcpy(wb->wb_Buf0,buf);

	/* Check for spaces... */
	for (s=wb->wb_Buf0;*s;s++) if (*s==' ') wb->wb_Buf0[0]='\0';

	/* If no string yet, we need to quote it... */
	s=wb->wb_Buf0;
	if (!*s)
	{
		strcpy(s,"\"");
		strcat(s,buf);
		strcat(s,"\"");
	}

	success=Q_EXECUTE_CMD; /* Enter Command Arguments: */
    }
    else
    {
	strcpy(wb->wb_Buf0, wb->wb_ExecuteBuf); /* get one line history */
	success=Q_EXECUTE_CMD_AND_ARGS; /* Enter Command and its Arguments: */
    }

    success=SyncExecute(Quote(Q_EXECUTE_REQ_TITLE),Quote(success),Quote(Q_COMMAND_TEXT),wb->wb_Buf0,&wb->wb_IntuiPort,NULL);

    /* if history enabled and user did not hit cancel */
    if ((success >= 0) && (!buf))
    {
	strcpy(wb->wb_ExecuteBuf, wb->wb_Buf0); /* update one line history */
    }

    if (!success) ErrorTitle(Quote(Q_EXECUTE_FAILED)); /* execute failed */
    else if (success > 0)
    { /* if not cancel */
	/*
	 * Build output CON: string with the title being the command...
	 */
	strcpy(wb->wb_Buf1,OutputWindow1);
	strcat(wb->wb_Buf1,Quote(Q_WB_OUTPUT_TITLE));
	strcat(wb->wb_Buf1,OutputWindow2);
	if (fhout = OPEN_fh(wb->wb_Buf1, MODE_NEWFILE))
	{
            tagitem[0].ti_Data = fhout;

            DP(("DoExecute: calling CURRENTDIR"));
            oldcd = CURRENTDIR(lock); /* System requires a valid currentdir */
            DP(("DoExecute: calling System..."));

            if ((!lock) || (System(wb->wb_Buf0, tagitem)))
            {
		ErrorTitle(Quote(Q_EXECUTE_FAILED)); /* execute failed */
            	CLOSE_fh(fhout);
	    }

            DP(("ok, cd'ing back to $%lx...", oldcd));
            CURRENTDIR(oldcd);
            DP(("ok\n"));
	}
    }
    MP(("DoExecute: exit\n"));
}

/*
	Un-select object (if selected)
*/
UnSelectIt(obj)
struct WBObject *obj;
{
	if (obj->wo_Selected) { /* if selected */
		/* remove from list of selected items */
		REMOVE(&obj->wo_SelectNode);
		UnHighIcon(obj); /* unhilight it */
	}
	return(NULL);
}

/*
	Select object (if not already selected)
*/
SelectIt(obj)
struct WBObject *obj;
{
	struct WorkbenchBase *wb = getWbBase();

	if (!obj->wo_Selected) { /* if not already selected */
		/* add to list of selected items */
		ADDTAIL(&wb->wb_SelectList, &obj->wo_SelectNode);
		HighIcon(obj); /* hilight it */
	}
	return(NULL);
}

/*
	Select all objects in this drawer.
*/
SelectAll(msg, obj)
struct IntuiMessage *msg;
struct WBObject *obj;
{
    struct NewDD *dd; /* drawer */

    /* do not reset select list if either shift key is held down */
    if (!((msg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 0)) {
	ResetPotion(); /* reset select list, etc. */
    }

    if (dd = obj->wo_DrawerData) { /* if this object is a drawer */
	if (dd->dd_DrawerWin) { /* if it has a window */
	    /* select all its children */
	    SiblingPredSearch(dd->dd_Children.lh_TailPred, SelectIt);
	}
    }
    return(NULL);
}

void reverse(s)
char *s;
{
	char *s2, c;

	MP(("reverse: s='%s', ", s));
	s2 = s + strlen(s) - 1;
	do {
		c = *s;
		*s++ = *s2;
		*s2-- = c;
	} while (s < s2);
}

void itoa(n, s)
int n;
char *s;
{
	int sign;
	char *s2 = s;

	MP(("itoa: n=%ld, ", n));
	if ((sign = n) < 0) n = -n;
	do { /* generate digits in reverse order */
		*s2++ = n % 10 + '0'; /* get next digit */
	} while ((n /= 10) > 0); /* delete it */
	if (sign < 0) *s2++ = '-';
	*s2 = NULL;
	reverse(s);
	MP(("s='%s'\n", s));
}

/*
	NewDrawer creates a new drawer named 'Unnamed' or 'Unnamedn'.
	The new drawer gets created and displayed inside the open drawer.
	It checks to make sure that the open drawer is NOT the wb root object.
*/
int NewDrawer(parentdrawer)
struct WBObject *parentdrawer;
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *obj, *tmpobj;
    struct NewDD *dd;
    BPTR newlock;
    int err = 1, revision = 0;

    MP(("NewDrawer: enter, obj=$%lx (%s), root=$%lx\n", parentdrawer,
	parentdrawer ? parentdrawer->wo_Name : "NULL", wb->wb_RootObject));

    if (parentdrawer == wb->wb_RootObject) {
	MP(("Create: passing on root object\n"));
	return(0); /* cannot create new drawer inside root object */
    }

    CURRENTDIR(parentdrawer->wo_DrawerData->dd_Lock);
    /* dummy call for first 'UNLOCK' in loop below */
    newlock = DUPLOCK(parentdrawer->wo_DrawerData->dd_Lock);
    do {
	UNLOCK(newlock);
	revision++;
	strcpy(wb->wb_Buf0, "Unnamed");
	itoa(revision, wb->wb_Buf1);
	strcat(wb->wb_Buf0, wb->wb_Buf1);
	/* make sure directory/file doesn't already exist */
    } while (newlock = LOCK(wb->wb_Buf0, ACCESS_READ));

    if (obj = WBAllocWBObject(wb->wb_Buf0)) {
	obj->wo_Type = WBDRAWER;
	obj->wo_Parent = parentdrawer;
	if (initgadget2(obj)) {
	    if (dd = obj->wo_DrawerData) {
		AddIcon(obj, parentdrawer);
                if (newlock = CREATEDIR(wb->wb_Buf0)) {
                    UNLOCK(newlock);
                    if (PutObject(obj)) { /* if put it */
                        if (tmpobj = WBGetWBObject(wb->wb_Buf0))
                        {
                            obj->wo_Protection = tmpobj->wo_Protection;
                            obj->wo_FileSize = tmpobj->wo_FileSize;
                            obj->wo_DateStamp = tmpobj->wo_DateStamp;
                            WBFreeWBObject(tmpobj);
                        }
                        err = 0; /* flag operation worked */
                    }
                    else {
                        MP(("Create: unable to PutObject\n"));
                        KillFile(wb->wb_Buf0);
                        ErrorDos(ERR_CREATE, wb->wb_Buf0);
                    }
                }
                else {
                    MP(("Create: unable to createdir '%s'\n", wb->wb_Buf0));
                    RemoveObject(obj);
                    ErrorDos(ERR_CREATE, wb->wb_Buf0);
                }
	    }
	    else {
		MP(("Create: unable to AllocDrawer\n"));
		goto freeobj;
	    }
	}
	else {
	    MP(("Create: unable to initgadget2\n"));
freeobj:    WBFreeWBObject(obj);
	    NoMem();
	}
    }
    RefreshDrawer(parentdrawer);
    if (!err) { /* bring up rename requester */
	RenameObject(obj, NULL);
    }
    return(0);
}

void QuitCheck()
{
    struct WorkbenchBase *wb = getWbBase();
    struct EasyStruct es = {
        sizeof(struct EasyStruct),
	NULL,
	SystemWorkbenchName,
	NULL,	/* filled in */
	NULL	/* filled in */
    };

    BeginDiskIO();
    FORBID;
    if (wb->wb_ToolExitCnt) {
	wb->wb_ErrorDisplayed = 0; /* force msg to appear */
        ErrorTitle(Quote(Q_TOOL_EXIT), wb->wb_ToolExitCnt);
    }
    else if (wb->wb_Library.lib_OpenCnt > 1) {
	wb->wb_ErrorDisplayed = 0; /* force msg to appear */
	ErrorTitle(Quote(Q_LIBRARY_OPEN), wb->wb_Library.lib_OpenCnt-1);
    }
    else {
        wb->wb_Quit=1;
	es.es_TextFormat = Quote(Q_QUIT_REQ_TEXT);
	es.es_GadgetFormat = Quote(Q_OK_CANCEL_TEXT);
	if (!EasyRequestArgs(wb->wb_BackWindow, &es, NULL, NULL)) wb->wb_Quit = 0;
    }
    PERMIT;
    EndDiskIO();
}
