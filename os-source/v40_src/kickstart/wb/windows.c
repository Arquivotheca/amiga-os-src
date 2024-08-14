/*
 * $Id: windows.c,v 39.5 93/05/24 15:39:24 mks Exp $
 *
 * $Log:	windows.c,v $
 * Revision 39.5  93/05/24  15:39:24  mks
 * Saved 184 bytes by hardcoding the screen->WBorTop/Left/Bottom/Right
 * values in the window open routine.  These values are currently
 * hard coded in Intuition and we need the ROM space so Workbench
 * now hardcodes the values.
 * 
 * The source code still shows the reference but it is commented out.
 * 
 * Revision 39.4  92/07/31  15:22:59  mks
 * No longer sets WA_AutoAdjust on the backdrop window
 *
 * Revision 39.3  92/07/20  12:02:31  mks
 * Now dynamically figures the size of the gadgets based on the
 * size of the size gadgets which is obtained via the Height of
 * the Left arrow and the Width of the Up arrow...
 *
 * Revision 39.2  92/06/23  09:34:19  mks
 * New window prop gadgets again.  Re-installed the borderless versions
 *
 * Revision 39.1  92/06/11  15:47:51  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.11  92/06/11  10:55:42  mks
 * Made code smaller and re-ordered the gadgets to match the dd structure
 * and the GID values.
 *
 * Revision 38.10  92/06/02  17:34:18  mks
 * Saved some bytes and removed PROPBORDERLESS
 *
 * Revision 38.9  92/06/01  16:20:27  mks
 * Added a comment
 *
 * Revision 38.8  92/06/01  16:12:48  mks
 * New prop look along with some major space savings...
 *
 * Revision 38.7  92/04/16  10:03:21  mks
 * Changed to use the new copyright string method
 *
 * Revision 38.6  92/04/14  12:18:33  mks
 * Now uses the magic tags for busy pointers
 *
 * Revision 38.5  92/04/09  07:11:36  mks
 * Now makes use of Intiution WA_AutoAdjust... Saved code (ROM) space.
 *
 * Revision 38.4  92/02/27  16:52:14  mks
 * Now pre-inits the screen title bar buffer to flush any old text
 *
 * Revision 38.3  91/09/12  11:40:09  mks
 * Added calls to open and close the catalog as the screen is opened/closed.
 *
 * Revision 38.2  91/08/27  16:35:20  mks
 * Added changes to make menus come up in new-look.
 *
 * Revision 38.1  91/06/24  11:39:40  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "intuition/classes.h"
#include "intuition/icclass.h"
#include "intuition/gadgetclass.h"
#include "intuition/imageclass.h"
#include "utility/tagitem.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "quotes.h"

/* width and height of intuition's close gadget */
#define CLOSEWIDTH	18

/*
    NOTE: The vertical prop gadget MUST be the first gadget for
    this code to work properly.  Not only that BUT the horizontal
    prop gadget MUST be the next gadget.  Followed by the
    left, right, up and down gadget.  Damm!
*/
struct Window *
OpenPWindow(nw, flags, gadv, text, extraIDCMP, diskwin, viewmodes)
struct NewWindow *nw;
ULONG flags;
struct Gadget *gadv;
BYTE *text;
LONG extraIDCMP;
BOOL diskwin;
UWORD viewmodes;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Screen *screen = wb->wb_Screen;
    struct Window *windowptr;
    struct Gadget *gad;
    WORD   VSize=0;
    UWORD  propflags=0;
    struct TagItem tagitem[6] =
    {
	{WA_BackFill, NULL},		/* 0 */
	{WA_BusyPointer, FALSE},	/* 1 */
	{WA_ScreenTitle, NULL},		/* 2 */
	{WA_NewLookMenus, TRUE},	/* 3 */
	{WA_AutoAdjust, FALSE},		/* 4 */
	{TAG_DONE, NULL}
    };

    DP(("OpenPWindow: enter\n"));
    nw->Flags = flags;
    nw->FirstGadget = gadv;

    nw->Title = text;
    nw->IDCMPFlags = 0;
    nw->Screen = 0;
    nw->BitMap = 0;
    nw->CheckMark = 0;
    nw->Type = WBENCHSCREEN;

    /** mks:  Make sure that the detail and block pens are -1 **/
    nw->DetailPen=-1;
    nw->BlockPen=-1;

    /*
	The minimum window width is:
		(1) the width of the window left border (screen->WBorLeft)
	plus	(2) the width of the close gadget (CLOSEWIDTH)
	plus	(3) enough space for 2 letters of the window title (TextLength("MM"))
	plus	(4) the width of the left arrow gadget (wb->wb_LeftImage->Width)
	plus	(5) the width of the right arrow gadget (wb->wb_RightImage->Width)
	plus	(6) the width of the sizing gadget (SIZEWIDTH)
	plus	(7) the width of the window right border (screen->WBorRight).
    */
    nw->MinWidth = /*screen->WBorLeft*/4 + CLOSEWIDTH + TextLength(&wb->wb_Screen->RastPort,"MM",2) + wb->wb_LeftImage->Width + wb->wb_RightImage->Width + wb->wb_UpImage->Width+ /*screen->WBorRight*/4;

    /*
	The minimum window height is:
		(1) the height of the window top border (screen->WBorTop)
	plus	(2) the height of the font (screen->Font->ta_YSize)
	plus	(3) some room to show part of vert prop gadget (20)
	plus	(4) the height of the up arrow gadget (wb->wb_UpImage->Height)
	plus	(5) the height of the down arrow gadget (wb->wb_UpImage->Height)
	plus	(6) the height of the sizing gadget (SIZEHEIGHT)
	plus	(7) the height of the window bottom border (screen->WBorBottom).
    */
    nw->MinHeight = /*screen->WBorTop*/2 + screen->Font->ta_YSize + 20 + wb->wb_UpImage->Height + wb->wb_DownImage->Height + wb->wb_UpImage->Height + /*screen->WBorBottom*/2;

    nw->MaxWidth = ~0; /* full size */
    nw->MaxHeight = ~0; /* full size */

    /* make sure width and height are not too small */
    if (nw->Width < nw->MinWidth) nw->Width = nw->MinWidth;
    if (nw->Height < nw->MinHeight) nw->Height = nw->MinHeight;

    if (screen->BitMap.Depth>1) propflags=PROPBORDERLESS;

    if (gad=gadv)
    { /* if there are any gadgets... */

	/* Auto-adjust only if a real window and not backdrop */
	tagitem[4].ti_Data=TRUE;

DP(("OpenPWindow: calculating gadh\n"));
	/* get ptr to gadh */
	/* Already there... */
	gad->LeftEdge = /*screen->WBorLeft*/4 - 1;
	gad->Width = -(gad->LeftEdge + wb->wb_UpImage->Width + wb->wb_LeftImage->Width + wb->wb_RightImage->Width + /*screen->WBorRight*/4 - 2);
	gad->Height = wb->wb_LeftImage->Height - 4;
	gad->TopEdge = -(gad->Height + 1);
	((struct PropInfo *) gad->SpecialInfo)->Flags = propflags|FREEHORIZ|AUTOKNOB|PROPNEWLOOK;

DP(("OpenPWindow: calculating gadv\n"));
	/* get ptr to gadv */
	gad = gad->NextGadget;
	if (wb->wb_UpImage->Width > 15) VSize=1;	/* Adjust for large resolutions */
	gad->TopEdge = /*screen->WBorTop*/2 + screen->Font->ta_YSize + 2;
	gad->Height = -(gad->TopEdge + wb->wb_LeftImage->Height-1 + wb->wb_UpImage->Height + wb->wb_DownImage->Height + /*screen->WBorBottom*/2);
	gad->Width = wb->wb_UpImage->Width - 6 - VSize - VSize;
	gad->LeftEdge = -(gad->Width + 2 + VSize);
	((struct PropInfo *) gad->SpecialInfo)->Flags = propflags|FREEVERT|AUTOKNOB|PROPNEWLOOK;

DP(("OpenPWindow: calculating gadu\n"));
	/* get ptr to gadu */
	gad = gad->NextGadget;
	gad->Width = wb->wb_UpImage->Width;
	gad->Height = wb->wb_UpImage->Height;
	gad->LeftEdge = 1 - wb->wb_UpImage->Width;
	gad->TopEdge =VSize= -(wb->wb_LeftImage->Height-1 + wb->wb_UpImage->Height + wb->wb_DownImage->Height);
	gad->GadgetRender = gad->SelectRender = (APTR)wb->wb_UpImage;

DP(("OpenPWindow: calculating gadd\n"));
	/* get ptr to gadd */
	gad = gad->NextGadget;
	gad->Width = wb->wb_DownImage->Width;
	gad->Height = wb->wb_DownImage->Height;
	gad->LeftEdge = 1 - wb->wb_DownImage->Width;
	gad->TopEdge = VSize+wb->wb_UpImage->Height;
	gad->GadgetRender = gad->SelectRender = (APTR)wb->wb_DownImage;

DP(("OpenPWindow: calculating gadl\n"));
	/* get ptr to gadl */
	gad = gad->NextGadget;
	gad->Width = wb->wb_LeftImage->Width;
	gad->Height = wb->wb_LeftImage->Height;
	gad->LeftEdge =VSize= -(wb->wb_UpImage->Width-1 + wb->wb_LeftImage->Width + wb->wb_RightImage->Width);
	gad->TopEdge = 1 - wb->wb_LeftImage->Height;
	gad->GadgetRender = gad->SelectRender = (APTR)wb->wb_LeftImage;

DP(("OpenPWindow: calculating gadr\n"));
	/* get ptr to gadr */
	gad = gad->NextGadget;
	gad->Width = wb->wb_RightImage->Width;
	gad->Height = wb->wb_RightImage->Height;
	gad->LeftEdge = VSize+wb->wb_LeftImage->Width;
	gad->TopEdge = 1 - wb->wb_RightImage->Height;
	gad->GadgetRender = gad->SelectRender = (APTR)wb->wb_RightImage;
    }

    DP(("OpenPWindow: calling OpenWindow(nw=$%lx)\n", nw));

    tagitem[1].ti_Data=wb->wb_DiskIONestCnt;
    tagitem[2].ti_Data=(ULONG)(wb->wb_ScreenTitle);

    /* only ViewByIcon windows get a backfill hook */
    if (viewmodes <= DDVM_BYICON) tagitem[0].ti_Data = PrepareBackFill(diskwin);
    if (windowptr = OpenWindowTagList(nw, tagitem))
    {
	windowptr->UserPort = &wb->wb_IntuiPort;
	DP(("OpenPWindow: calling ModifyIDCMP\n"));
	if (ModifyIDCMP(windowptr,MOUSEBUTTONS|GADGETDOWN|REFRESHWINDOW|ACTIVEWINDOW|GADGETUP|MENUPICK|CLOSEWINDOW|NEWSIZE|INACTIVEWINDOW|extraIDCMP))
	{
            if (!wb->wb_DiskIONestCnt)
            {
                DP(("OpenPWindow: calling SetMenuStrip\n"));
                SetMenuStrip(windowptr, wb->wb_MenuStrip);
            }
	}
	else
	{
		/*
		 * Now close since we did not get the ModifyIDCMP to work!
		 */
	    windowptr->Title=NULL;
	    ClosePWindow(windowptr,NULL);
	    windowptr=NULL;
	}
    }

    DP(("OpenPWindow: exit, returning $%lx\n", windowptr));

    return(windowptr);
}

struct PropInfo PropInfo =
{
    0,			/* Flags */
    0, 0,		/* HorizPot, VertPort */
    0xffff, 0xffff,	/* HorizBody, VertBody */
    0xffff, 0xffff,	/* CWidth, CHeight */
    0, 0, 0, 0		/* HPotRes, VPotRes, LeftBorder, TopBorder */
};

const UWORD GadgetFillIn[] =
{
/* HorizScroll */
    GADGHCOMP|GADGIMAGE|GRELWIDTH|GRELBOTTOM,	/* flags */
    GADGIMMEDIATE|RELVERIFY|BOTTOMBORDER,		/* activation */
    PROPGADGET,						/* type */
/* VertScroll */
    GADGHCOMP|GADGIMAGE|GRELRIGHT|GRELHEIGHT,	/* flags */
    GADGIMMEDIATE|RELVERIFY|RIGHTBORDER,		/* activation */
    PROPGADGET,						/* type */
/* Up */
    GADGHIMAGE|GADGIMAGE|GRELRIGHT|GRELBOTTOM,	/* flags */
    GADGIMMEDIATE|RELVERIFY|RIGHTBORDER,		/* activation */
    BOOLGADGET,						/* type */
/* Down */
    GADGHIMAGE|GADGIMAGE|GRELRIGHT|GRELBOTTOM,	/* flags */
    GADGIMMEDIATE|RELVERIFY|RIGHTBORDER,		/* activation */
    BOOLGADGET,						/* type */
/* Left */
    GADGHIMAGE|GADGIMAGE|GRELBOTTOM|GRELRIGHT,	/* flags */
    GADGIMMEDIATE|RELVERIFY|BOTTOMBORDER,		/* activation */
    BOOLGADGET,						/* type */
/* Right */
    GADGHIMAGE|GADGIMAGE|GRELBOTTOM|GRELRIGHT,	/* flags */
    GADGIMMEDIATE|RELVERIFY|BOTTOMBORDER,		/* activation */
    BOOLGADGET,						/* type */
/* End of list */
    NULL
};

void InitGadgets(struct NewDD *dd)
{
struct	Gadget	*gad;
	int	index=0;
	int	id=GID_HORIZSCROLL;

	gad = &dd->dd_HorizScroll;

	while (gad)
	{
		memset((char *)gad,0,sizeof(struct Gadget));
		gad->Flags	=GadgetFillIn[index++];
		gad->Activation	=GadgetFillIn[index++];
		gad->GadgetType	=GadgetFillIn[index++];
		gad->GadgetID	=id++;

		if (GadgetFillIn[index]) gad->NextGadget=&gad[1];
		gad=gad->NextGadget;
	}

	dd->dd_HorizScroll.GadgetRender=(APTR)&(dd->dd_HorizImage);
	dd->dd_VertScroll.GadgetRender=(APTR)&(dd->dd_VertImage);

	dd->dd_HorizScroll.SpecialInfo=(APTR)&(dd->dd_HorizProp);
	dd->dd_VertScroll.SpecialInfo=(APTR)&(dd->dd_VertProp);

	dd->dd_HorizProp=PropInfo;
	dd->dd_VertProp=PropInfo;
}

int InitScreenAndWindows()
{
struct	WorkbenchBase	*wb = getWbBase();
	int		result=FALSE;
	ULONG		tags[]=	{	SYSIA_DrawInfo,0,
					SYSIA_Which,LEFTIMAGE,
					TAG_END
				};


    /* Set up the default screen title that we will use */
    /* (This is so that it is not junk) */
    sprintf(wb->wb_ScreenTitle,CopyrightFormat,wb->wb_Copyright1,wb->wb_Copyright2,wb->wb_Copyright3);
    wb->wb_LastFreeMem=0;	/* So the system knows to set it */

    if (wb->wb_Screen = (struct Screen *)LockPubScreen(SystemWorkbenchName))
    {
	OpenWBCat();

        if (wb->wb_VisualInfo = GetVisualInfo(wb->wb_Screen,TAG_DONE))
        {
	    if (tags[1]=(ULONG)GetScreenDrawInfo(wb->wb_Screen))
	    {
                wb->wb_LeftImage = (struct Image *)NewObjectA(NULL,"sysiclass",(struct TagItem *)tags);

	        tags[3]=RIGHTIMAGE;
                wb->wb_RightImage = (struct Image *)NewObjectA(NULL,"sysiclass",(struct TagItem *)tags);

	        tags[3]=UPIMAGE;
                wb->wb_UpImage = (struct Image *)NewObjectA(NULL,"sysiclass",(struct TagItem *)tags);

	        tags[3]=DOWNIMAGE;
                wb->wb_DownImage = (struct Image *)NewObjectA(NULL,"sysiclass",(struct TagItem *)tags);

		if ((wb->wb_LeftImage) &&
                    (wb->wb_RightImage) &&
                    (wb->wb_UpImage) &&
                    (wb->wb_DownImage)) result=TRUE;

                FreeScreenDrawInfo(wb->wb_Screen,(struct DrawInfo *)tags[1]);
	    }
	}
    }

    return(result);
}

void UnInitScreenAndWindows()
{
struct	WorkbenchBase	*wb = getWbBase();
struct	Image		**wbimage;
	long		loop;

    DP(("UISAW: enter\n"));
    if (wb->wb_Screen)
    {
        if (wb->wb_VisualInfo)
        {
	    wbimage=&(wb->wb_LeftImage);

	    for (loop=4;loop>0;loop--)
	    {
	    	if (*wbimage)
	    	{
	    	    DisposeObject(*wbimage);
	    	    *wbimage=NULL;
	    	}
	    	wbimage++;
	    }

            FreeVisualInfo(wb->wb_VisualInfo);
        }
	CloseWBCat();
	UnlockPubScreen(NULL, wb->wb_Screen);
    }
}

#define	ROLLOVER_POINT	9999

VOID FormatDrawerUse(struct WBObject *obj,char *s)
{
struct WorkbenchBase *wb = getWbBase();
struct InfoData *id;
ULONG lock, used, free, percent, total;
char freechar, usedchar;

    *s='\0';
    if (obj!=wb->wb_RootObject) if (obj->wo_Type==WBDISK)
    {
        if (lock=MakeDiskLock(obj))
        {
            if (id = (struct InfoData *)ALLOCVEC(sizeof(struct InfoData), MEMF_PUBLIC))
            {
                if (Info(lock,id))
                {
                    total = id->id_NumBlocks;
                    used = id->id_NumBlocksUsed;
                    percent = (((used * 100) + (total>>1)) / total);
                    total *= id->id_BytesPerBlock;
                    used *= id->id_BytesPerBlock;
                    free = total - used;

		    freechar=usedchar='K';

		    if ((free >>= 10) > ROLLOVER_POINT)
		    {
			freechar='M';
			free >>= 10;
		    }

		    if ((used >>= 10) > ROLLOVER_POINT)
		    {
			usedchar='M';
			used >>= 10;
		    }

                    sprintf(s, Quote(Q_VOLUME_TITLE_FORMAT), percent, free, freechar, used, usedchar);
                }
                FREEVEC(id);
            }
        }
    }
}

VOID FormatWindowName(struct WBObject *obj,UBYTE **pointer,struct Window *win)
{
struct WorkbenchBase *wb = getWbBase();
char *p;

    {
    APTR oldwin;

        oldwin=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
        ((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)(-1);     /* No requesters */

        FormatDrawerUse(obj,wb->wb_Buf1);

        ((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwin;
    }

    if (p=ALLOCVEC(strlen(obj->wo_Name)+strlen(wb->wb_Buf1)+1,MEMF_PUBLIC))
    {
        strcpy(p, obj->wo_Name);
        strcat(p, wb->wb_Buf1);
    }

    if (win) if (win->Title)
    {
    char *oldtitle;

        oldtitle = win->Title;

	if (p)
	{
            /*
             * Check if title changed
             * If not, we do not set the window title again
             * (SetWindowTitles is slow)
             */
            if (oldtitle) if (!stricmp(oldtitle,p))
            {
                FREEVEC(p);
                p=oldtitle;
            }

            if (oldtitle!=p)
            {
                SetWindowTitles(win, p, (UBYTE *)-1);
                FREEVEC(oldtitle);
            }
	}
	else p=oldtitle;
    }

    *pointer=p;
}

/*
 * This is used with master-search to see if we have a drawer of the
 * same lock connected to a window elsewhere...  (Due to links)
 */
LookForSameDrawer(struct WBObject *obj, struct WBObject *new,BPTR lock)
{
int result=FALSE;
BPTR olock;
struct NewDD *dd;

	if ((obj!=new) && (dd=obj->wo_DrawerData))
	{
		if ((obj->wo_DrawerOpen) || (dd->dd_DrawerWin))
		{
			LockDrawer(obj);
			if (olock=dd->dd_Lock)
			{
				if (SAMELOCK(olock,lock)==LOCK_SAME) result=TRUE;
			}
		}
	}
	return(result);
}

/*
 * This is used with Sibling searching to move the objects from
 * the old parent to the new one...
 * It also clears the window pointer which will be rebuilt later...
 */
MoveToLink(struct WBObject *obj, struct WBObject *old, struct WBObject *new)
{
	DP(("MoveToLink: Moving %s from %s to %s\n",obj->wo_Name,old->wo_Name,new->wo_Name));

	/*
	 * Remove it from the old list
	 */
	Remove(&(obj->wo_Siblings));
	--old->wo_UseCount;

	/*
	 * Add it to the new list
	 */
	ADDTAIL(&(new->wo_DrawerData->dd_Children),&(obj->wo_Siblings));
	++new->wo_UseCount;

	/*
	 * Make the back-pointer (parent) correct...
	 */
	obj->wo_Parent=new;
	obj->wo_IconWin=NULL;	/* No window for you yet... */

	return(0);
}

/* Does an Open of a Drawer
 * if the Object is a Disk:
 *  - ignore all other extend-selections
 *  - if the Disk's TopDrawer is closed, open it
 *  - if the Disk's TopDrawer is opened, bring it to the front (?)
 * if the Object is a Drawer:
 *  - ignore all other extend-selections
 *  - if the Drawer is closed, open it
 *  - if the Drawer is opened, bring it to the front (?)
 */
PotionOpen(drawerobj)
struct WBObject *drawerobj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *dd = drawerobj->wo_DrawerData;

    ULONG WindowFlags = SIMPLE_REFRESH|WINDOWSIZING|WINDOWDRAG|WBENCHWINDOW|
	WINDOWDEPTH|SIZEBBOTTOM|SIZEBRIGHT|REPORTMOUSE|ACTIVATE|WINDOWCLOSE;

    ULONG extraIDCMP = NULL;
    UBYTE *ptr;

    DP(("PotionOpen: enter, obj=$%lx (%s)\n", drawerobj, drawerobj->wo_Name));
#ifdef MEMDEBUG
    PrintAvailMem("PotionOpen", MEMF_CHIP);
#endif MEMDEBUG

    /* if this drawer's window is not open */
    if (dd->dd_DrawerWin == 0)
    {
	/* if this drawer is open, re-open the window */
	if (drawerobj->wo_DrawerOpen)
	{
	    DP(("PotionOpen: drawer is already open, re-opening window\n"));
	    /* clear silent flag as it may have been set */
	    drawerobj->wo_DrawerSilent = 0;
	    /* re-open the window */
	    wbReopen(drawerobj);
	    goto exit;
	}

	DP(("PotionOpen: window not open, calling BeginDiskIO..."));
	BeginDiskIO();
	DP(("ok\n"));

	InitGadgets(dd);

	/* new for V1.4 (ask for NULL initial window)*/
	FormatWindowName(drawerobj, &ptr, NULL);

	if (drawerobj == wb->wb_RootObject) extraIDCMP=DISKINSERTED|DISKREMOVED;

        /*
         * We need to check if the drawer we are about to open is linked to
         * another, already open drawer and if so, to yank that one and place
         * over to this one.  (And thus removing it from the other icon.)
         * This should also make the drawer open almost instantly...
         */
	LockDrawer(drawerobj);
        if (dd->dd_Lock)
        {
        struct WBObject *old;
        APTR oldwin;

            oldwin=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
            ((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)(-1);     /* No requesters */

            if (old=MasterSearch(LookForSameDrawer,drawerobj,dd->dd_Lock))
            {
            struct WBObject *look=drawerobj;
	    struct NewDD *olddd=old->wo_DrawerData;

		DP(("PotionOpen: Found linked open drawer...  Checking for loop...\n"));
                /*
                 * Ok, no make sure we don't fall into a loop'ed link issue...
                 * That is, don't try to yank object away if the object is a
                 * parent of this object.
                 */
		while (look && (look!=old))
		{
		    look=look->wo_Parent;
		    if (look==wb->wb_RootObject) look=NULL;
		}
		if (look)
		{
		    DP(("PotionOpen: Loop link...  Will just bring parent link up...\n"));
		    /*
		     * Ok, this is a loop, so just bring up the old one...
		     */
		    drawerobj=old;
		    dd=olddd;
		}
		else
		{
                    DP(("PotionOpen: Found linked open drawer...  Moving it...\n"));
                    SiblingSuccSearch(olddd->dd_Children.lh_Head,MoveToLink,old,drawerobj);

                    /*
                     * We need to make sure that the new drawer is of the same
                     * type/mode/view as the old one...
                     */
                    dd->dd_Flags=olddd->dd_Flags;
                    dd->dd_ViewModes=olddd->dd_ViewModes;

                    drawerobj->wo_DrawerOpen=1; /* Look like we were not open */
                    drawerobj->wo_UseCount++;   /* Make us look like we are real */

                    CloseDrawer(old);           /* Make the old one go away... */
		}

                PotionOpen(drawerobj);      /* Come back around and open it */
            }

            ((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwin;
        }

	if (!(dd->dd_DrawerWin))
	{
	    if ((drawerobj!=wb->wb_RootObject) && (!(dd->dd_Lock)))
	    {
		goto openerrmsg;
	    }

            dd->dd_DrawerWin=OpenPWindow(&dd->dd_NewWindow,WindowFlags,&dd->dd_HorizScroll,
					 ptr,extraIDCMP,
					 (BOOL)(drawerobj==wb->wb_RootObject),
					 dd->dd_ViewModes);

            /* handle the intuition failures (e.g. out of mem?) */
            if (!dd->dd_DrawerWin)
            {
                goto openerrmsg;
            }
            else if (drawerobj == wb->wb_RootObject)
            {
                /* make sure right pattern is used to clear window */
                wb->wb_BackWindow = dd->dd_DrawerWin;
            }
            drawerobj->wo_DrawerOpen = 0; /* drawer is now fully open */
            ptr=NULL;	/* Open must have worked, NULL the pointer */

            /*
               moved from after MinMaxDrawer call (below) to here so that
               the gauge gets drawn before the objects in the window.  This
               was done for asthetic purposes (ie. it looks better)
               Added note: there is no longer a gauge, instead RedrawGauge
               updates the title bar info (# files, blocks, etc.).  I wonder
               if this call should be moved back to after MinMaxDrawer?
            */
            DP(("PotionOpen: calling RedrawGuage\n"));
            RedrawGauge(drawerobj);

            DP(("PotionOpen: calling RefreshDrawer\n"));
            RefreshDrawer(drawerobj); /* added for pattern support */

            DP(("PotionOpen: calling OpenDrawer\n"));
            /* and fill it up */
            if (!OpenDrawer(drawerobj))
            {
                DP(("PotionOpen: could not open drawer %lx (%s)\n",drawerobj, drawerobj->wo_Name));
                CloseDrawer( drawerobj );
                /*
                    VERY IMPORTANT!  The 'openerrmsg' label MUST
                    be AFTER 'CloseDrawer' and BEFORE 'ErrorTitle'
                    otherwise the parent drawer will get closed too!
                */
                if (!(wb->wb_LastCloseWindow))
                {
openerrmsg:
                    ErrorTitle(Quote(Q_DRW_CANT_BE_OPENED));
		}
                goto openfailed;
            }

	    DP(("PotionOpen: calling MinMaxDrawer\n"));
	    /* fix proportional gadgets */
	    MinMaxDrawer( drawerobj );
	}

openfailed:
	FREEVEC(ptr);	/* We free this if not NULL  (FreeVec works on NULL) */

	DP(("PotionOpen: calling EndDiskIO\n"));
	EndDiskIO();
    }
    else
    {
	DP(("PotionOpen: %s already Open\n", drawerobj->wo_Name));
	WindowToFront( dd->dd_DrawerWin );
	ActivateWindow( dd->dd_DrawerWin );
    }
exit:
    RethinkMenus();
    DP(("PotionOpen: exit\n"));
#ifdef MEMDEBUG
    PrintAvailMem("PotionOpen", MEMF_CHIP);
#endif MEMDEBUG
    return( 0 );
}

struct Window *
WhichWindow( x, y )
int x, y;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Window *newwin;
    struct Layer *newlayer;
    struct LayerInfo *li;

    DP(("WhichWindow: enter, x=%ld, y=%ld\n", x, y));
    li = &wb->wb_Screen->LayerInfo;
    newlayer = WhichLayer( li, x, y );

    newwin = newlayer? (struct Window *)newlayer->Window : NULL;
    DP(("WhichWindow: exit, returning $%lx (%s)\n",
	newwin, newwin->Title));
    return( newwin );
}

winCmp( obj, win )
struct WBObject *obj;
struct Window *win;
{
    struct NewDD *dd;

    if( dd = obj->wo_DrawerData ) {
	if( dd->dd_DrawerWin == win ) return( 1 );
    }
    return( 0 );
}

struct WBObject *
WindowToObj( win )
struct Window *win;
{
    if( ! win ) return( 0 );
    return( MasterSearch( winCmp, win ) );
}


UpdateWindowSize(obj)
struct WBObject *obj;
{
struct WorkbenchBase *wb = getWbBase();
register struct NewDD *dd;
register struct Window *win;

    DP(("UpdateWindowSize: enter, obj=%lx (%s), dd=$%lx\n",obj, obj->wo_Name, obj->wo_DrawerData));
    if (dd = obj->wo_DrawerData)
    {
	DP(("\twin=$%lx (%s)\n", dd->dd_DrawerWin,dd->dd_DrawerWin ? dd->dd_DrawerWin->Title : "NULL"));
	if (win = dd->dd_DrawerWin)
	{
	    DP(("\tLE=%ld,%ld, TE=%ld,%ld, W=%ld,%ld, H=%ld,%ld\n",win->LeftEdge, dd->dd_NewWindow.LeftEdge,win->TopEdge, dd->dd_NewWindow.TopEdge,win->Width, dd->dd_NewWindow.Width,win->Height, dd->dd_NewWindow.Height));

	    COPY_WBOX(&(dd->dd_NewWindow.LeftEdge),&(win->LeftEdge));

	    if (obj==wb->wb_RootObject)
	    {
	    register struct WBConfig *wbc=&(wb->wb_Config);

                if (!(wbc->wbc_Backdrop=wb->wb_Backdrop))
                {
		register struct Screen *sc=wb->wb_Screen;

                    /*
                     * We are not a backdrop window, so save size and possition too
                     */
                    wbc->wbc_Version=0;

		    COPY_WBOX(&(wbc->wbc_LeftEdge),&(win->LeftEdge));

                    /* Check if we are full sized.  If so, we do not save the size */
                    if ((win->Width!=sc->Width) || (win->Height!=(sc->Height-(sc->BarHeight+1))))
                    {
			DP(("We are not full sized: Width=%ld/%ld  Height=%ld/%ld  Bar=%ld\n",win->Width,sc->Width,win->Height,sc->Height,sc->BarHeight));
                        wbc->wbc_Version=WBConfig_Version;
                    }
                }
	    }
	    MinMaxDrawer(obj);
	}
    }
    DP(("UpdateWindowSize: exit\n"));
    return(NULL);
}

UpdateWindow(obj)
struct WBObject *obj;
{
    struct NewDD *dd;
    struct Window *win;
    struct Layer *layer;

    DP(("UpdateWindow: enter, obj=%lx (%s)\n",
	obj, obj ? obj->wo_Name : "NULL"));
    if (obj) {
	if (dd = obj->wo_DrawerData) {
	    if (win = dd->dd_DrawerWin) {
		layer = win->RPort->Layer;
		if (layer->Flags & LAYERREFRESH)
		{
		    UpdateWindowSize(obj);
		    /* refresh the window */
		    BeginIconUpdate(obj, win, FALSE);
                    SiblingPredSearch(dd->dd_Children.lh_TailPred, RenderIcon);
		    EndIconUpdate(win);
		}
	    }
	}
    }
#ifdef DEBUGGING
    else {
	DP(("UpdateWindow: null object!\n"));
    }
#endif DEBUGGING
    DP(("UpdateWindow: exit\n"));
    return(NULL);
}


RemIntuiMsg( msg, win )
struct IntuiMessage *msg;
struct Window *win;
{
    if (msg->IDCMPWindow == win) {
	REMOVE( (struct Node *)msg );

	REPLYMSG( (struct Message *)msg );
    }
    return( 0 );
}

void ClosePWindow(win, dd)
struct Window *win;
struct NewDD *dd; /* may be NULL! */
{
    struct WorkbenchBase *wb = getWbBase();
    UBYTE *ptr;

    DP(("ClosePWindow: FORBID!\n"));
    FORBID;
    /* remove any pending msgs for this window from SeenPort */
    SearchList( &wb->wb_SeenPort.mp_MsgList, RemIntuiMsg, win );
    /* remove any pending msgs for this window from IntuiPort */
    SearchList( &wb->wb_IntuiPort.mp_MsgList, RemIntuiMsg, win );
    /* and turn off all new messages */
    win->UserPort = NULL;

    ModifyIDCMP( win, NULL );

    DP(("ClosePWindow: PERMIT!\n"));
    PERMIT;

    ptr = win->Title;

    DP(("ClosePWindow: calling ClearMenuStrip\n"));
    ClearMenuStrip( win );
    DP(("ClosePWindow: calling CloseWindow(%lx)\n",win));
    CloseWindow( win );
    DP(("ClosePWindow: returned from CloseWindow\n"));
    FREEVEC(ptr);
    DP(("ClosePWindow: exit\n"));
}

#ifdef DEBUGGING
void PrintNewWindow(nw)
struct NewWindow *nw;
{
DP(("PrintNewWindow: nw=%lx\n"));
DP(("\tLeftEdge=%ld, TopEdge=%ld\n", nw->LeftEdge, nw->TopEdge));
DP(("\tWidth=%ld, Height=%ld\n", nw->Width, nw->Height));
DP(("\tDetailPen=%ld, BlockPen=%ld\n", nw->DetailPen, nw->BlockPen));
DP(("\tIDCMPFlags=%lx, Flags=%lx\n", nw->IDCMPFlags, nw->Flags));
DP(("\tFirstGadget=%lx, CheckMark=%lx\n", nw->FirstGadget, nw->CheckMark));
DP(("\tTitle='%s'\n", nw->Title));
DP(("\tScreen=%lx, BitMap=%lx\n", nw->Screen, nw->BitMap));
DP(("\tMinWidth=%ld, MinHeight=%ld\n", nw->MinWidth, nw->MinHeight));
DP(("\tMaxWidth=%ld, MaxHeight=%ld, Type=%lx\n",
    nw->MaxWidth, nw->MaxHeight, nw->Type));
}
#endif DEBUGGING
