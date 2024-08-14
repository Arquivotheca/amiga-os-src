/*
 * $Id: ioloop.c,v 39.3 93/02/10 10:43:25 mks Exp $
 *
 * $Log:	ioloop.c,v $
 * Revision 39.3  93/02/10  10:43:25  mks
 * some cleanup of return (un-needed) values
 * 
 * Revision 39.2  92/07/13  18:35:18  mks
 * Added config locking...
 *
 * Revision 39.1  92/06/11  15:49:19  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.7  92/05/30  17:58:59  mks
 * Now uses ThisTask()
 *
 * Revision 38.6  92/05/30  16:56:23  mks
 * Now uses the GETMSG/PUTMSG stubs
 *
 * Revision 38.5  92/02/21  10:24:37  mks
 * Now handles the WORKBENCH_NEW_LOCALE message
 *
 * Revision 38.4  92/01/08  16:39:53  mks
 * Added the needed cleanup when the icon text changed.  It was required
 * sizes could change and only cleanup would be able to get the text
 * back under the icons correctly...
 *
 * Revision 38.3  92/01/07  17:29:11  mks
 * Updated autodocs...
 *
 * Revision 38.2  92/01/07  13:59:20  mks
 * Completely new preferences control.  Most features are enabled
 * already.  Just WORKBENCH_RESET and WORKBENCH_NEW_LOCALE
 * are left to go.
 *
 * Revision 38.1  91/06/24  11:36:47  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/ports.h"
#include "exec/io.h"
#include "exec/tasks.h"
#include "exec/errors.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "dos/notify.h"
#include "dos/var.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "fontenv.h"
#include "quotes.h"

struct iomessage {
    struct Message msg;
    void (*handler)();
};

struct NewWBConfig
{
struct	Message	msg;
	ULONG	tag;	/* The data that changed */
};

void NewConfig(struct NewWBConfig *);

void ioloopinit()
{
struct WorkbenchBase *wb = getWbBase();
struct MsgPort *wbp, *ip, *sp, *np, *lp;

    wbp = &wb->wb_WorkbenchPort;
    ip = &wb->wb_IntuiPort;
    sp = &wb->wb_SeenPort;
    np = &wb->wb_NotifyPort;
    lp = &wb->wb_LayerPort;

    wbp->mp_SigTask = (struct Task *) wb->wb_Task;
    wbp->mp_SigBit = AllocSignal(-1L);
    wbp->mp_Node.ln_Type = NT_MSGPORT;

    *ip = *wbp;
    *sp = *wbp;
    *np = *wbp;
    *lp = *wbp;

    NewList(&wbp->mp_MsgList);
    NewList(&ip->mp_MsgList);
    NewList(&sp->mp_MsgList);
    NewList(&np->mp_MsgList);
    NewList(&lp->mp_MsgList);
}

void ioloopuninit()
{
struct WorkbenchBase *wb = getWbBase();

	FreeSignal(wb->wb_WorkbenchPort.mp_SigBit);
}

/* this is the main loop -- it waits for all IO and input
 *
 * Currently the DOS calls do not use this loop -- they wait
 * for their IO to complete.  As soon as the message formats
 * are completed then I will add them too.  Neil
 */

void ioloop()
{
struct WorkbenchBase *wb = getWbBase();
struct MsgPort *wbp, *ip, *sp, *np, *lp;
struct Message *msg;
SHORT *typeptr;
ULONG WbSig;
int gotmsg;
SHORT type;

    wbp = &wb->wb_WorkbenchPort;
    ip = &wb->wb_IntuiPort;
    sp = &wb->wb_SeenPort;
    np = &wb->wb_NotifyPort;
    lp = &wb->wb_LayerPort;

    WbSig = 1L << wbp->mp_SigBit;

    MP(("ioloop: enter\n"));
    /*
     * the idea is to only wait when we have no input
     */
    do {
	MP(("ioloop: top of loop\n"));
	gotmsg = 0; /* do not have a message yet */

	/* see if we may be in danger of hanging due to a locked layers problem */
	if (msg = GETMSG(lp)) {
	    gotmsg = 1;
	    MP(("ld..."));
	    DoLayerDemon();
	    MP(("ok\n"));
	}

	/*
	    While in the middle of an asynchronous copy, we can allow
	    only refresh-type intuition events (handled via CheckDiskIO)
	    and workbench port messages.
	*/
	if (wb->wb_AsyncCopyCnt) {
	    MP(("ioloop: calling CheckDiskIO..."));
	    CheckDiskIO();
	    MP(("ok\n"));
	    goto checkwbport;
	}

	/* first look for messages that we've already seen (hence the name
	   SeenPort) but were unable to handle when they first came in.
	*/
	if (msg = GETMSG(sp)) {
	    gotmsg = 1;
	    MP(("ioloop: got seen msg, (class=$%lx) calling IncomingEvent...",
		((struct IntuiMessage *)msg)->Class));
	    MP(("sm, $%lx...", ((struct IntuiMessage *)msg)->Class));
	    IncomingEvent((struct IntuiMessage *)msg);
	    MP(("ok\n"));
	}
	/* only look at IntuiPort if SeenPort is empty */
	else if (msg = GETMSG(ip)) {
	    gotmsg = 1;
	    MP(("ioloop: got intuiport msg, (class=$%lx) calling IncomingEvent...",
		((struct IntuiMessage *)msg)->Class));
	    MP(("im, $%lx...", ((struct IntuiMessage *)msg)->Class));
	    IncomingEvent( (struct IntuiMessage *)msg );
	    MP(("ok\n"));
	}

	/*
	   if we're not dragging icon(s) or drag selecting,
	   we can look for messages from the device and wb ports
	*/
	if (!(wb->wb_Dragging)) {

	    if (!(wb->wb_ConfigLock)) if (msg = GETMSG(np)) {
		gotmsg = 1;
		MP(("ioloop: got notification msg, calling ProcessNotification..."));
		NewConfig((struct NewWBConfig *)msg);
		MP(("ok\n"));
	    }

checkwbport:
	    MP(("ioloop: calling GETMSG(wbp) ($%lx)\n", wbp));
	    if (msg = GETMSG(wbp)) {
		gotmsg = 1;

		typeptr = (SHORT *)msg;
		type = *--typeptr;

		switch( type ) {

		case MTYPE_TOOLEXIT:
		    MP(("ioloop: got a MTYPE_TOOLEXIT message..."));
		    ExitTool( (struct WBStartup *)msg );
		    MP(("ok\n"));
		    break;

		case MTYPE_TIMER:
		    MP(("ioloop: got a MTYPE_TIMER message..."));
		    TimeTick( (struct timerequest *)msg );
		    MP(("ok\n"));
		    break;

		case MTYPE_IOPROC:
		    MP(("ioloop: got a MTYPE_IOPROC message..."));
		    (*((struct iomessage *)msg)->handler)(msg);
		    MP(("ok\n"));
		    break;

		case MTYPE_APPWINDOW:
		case MTYPE_APPICON:
		case MTYPE_APPMENUITEM:
		    MP(("ioloop: got a AppMessage message..."));
		    CleanupAppMsg( (struct AppMessage *) msg);
		    MP(("ok\n"));
		    break;

		case MTYPE_COPYEXIT:
		    MP(("ioloop: got a MTYPE_COPYEXIT message..."));
		    DosCopyExit(msg);
		    MP(("ok\n"));
		    break;

		case MTYPE_ICONPUT:
		    MP(("ioloop: got a MTYPE_ICONPUT message..."));
		    IconUpdate((struct IconMsg *)msg);
		    MP(("ok\n"));
		    break;

		case MTYPE_APP_FUNC:
		    DoAppWork(msg);
		    break;

		default:
		    MP(("ioloop: illegal msg: type=%ld, msg=$%lx\n",
			type, msg));
		    Alert(AN_Workbench|AN_WBBadIOMsg);
		    MP(("ok\n"));
		    break;

		}
	    }
	}
	if (!gotmsg) Wait(WbSig);
    } while (!wb->wb_Quit);
}

/* notification code follows */

ForceRebuildViewByTextImagery(obj)
struct WBObject *obj;
{
    obj->wo_ImageSize = 0; /* force viewbytext imagery to get rebuilt */
    return(0);
}

void RefreshDrawerIconNames(drawer)
struct WBObject *drawer;
{
    struct Window *win;
    struct NewDD *dd;

    if (dd = drawer->wo_DrawerData) {
	if (win = dd->dd_DrawerWin) {
	    BeginIconUpdate(drawer, win, TRUE);
	    SiblingPredSearch(dd->dd_Children.lh_TailPred, RenderName);
	    EndIconUpdate(win);
	}
    }
}

#define	WBCONFIG_FILE	"ENVARC:sys/wbconfig.prefs"

#define	IFF_Data_Size	34
#define	IFF_HEADER_WB	"FORM" "\x00\x00\x00\x2E" "PREF" "PRHD" "\x00\x00\x00\x06" "\x00\x00\x00\x00\x00\x00" "WBCF" "\x00\x00\x00\x14"

void WriteWBPrefs(void)
{
struct WorkbenchBase *wb = getWbBase();
register struct WBConfig *wbc=&(wb->wb_Config);
BPTR fh;
UBYTE IFF_Data[IFF_Data_Size+1]=IFF_HEADER_WB;

    MP(("WriteWBPrefs: enter\n"));

    /* Setup the WBConfig data first... */
    UpdateWindowSize(wb->wb_RootObject);

    if (fh = OPEN_fh(WBCONFIG_FILE, MODE_NEWFILE))
    {
        if (WRITE_fh(fh,IFF_Data,IFF_Data_Size)==IFF_Data_Size)
        {
            WRITE_fh(fh,wbc,sizeof(struct WBConfig));
        }
        CLOSE_fh(fh);
        SETPROTECTION(WBCONFIG_FILE,FIBF_EXECUTE);
    }

    MP(("WriteWBPrefs: exit\n"));
}

void ReadWBPrefs(void)
{
struct	WorkbenchBase	*wb = getWbBase();
struct	WBConfig	*wbc=&(wb->wb_Config);
struct	Window		*oldwin;
	BPTR		fh;

	/* prevent error messages */
	oldwin=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
	((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)(-1);     /* No requesters */

	wbc->wbc_Backdrop=TRUE;

	MP(("ReadWBPrefs: enter\n"));
	if (fh = OPEN_fh(WBCONFIG_FILE, MODE_OLDFILE))
	{
		MP(("\tfile opended, seeking\n"));
		if ((SEEK_fh(fh, IFF_Data_Size, OFFSET_BEGINNING)) != -1)
		{
			if (READ_fh(fh,wbc,sizeof(struct WBConfig))!=sizeof(struct WBConfig))
			{
				/*
				 * Read error, so clear the data...
				 */
				wbc->wbc_Version=0;	/* Sizes are invalid */
				wbc->wbc_Backdrop=0;	/* Main window normal */
			}
		}
		CLOSE_fh(fh);
	}
	wb->wb_Backdrop = wbc->wbc_Backdrop; /* just init it */

	/* re-enable error messages */
	((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwin;
	MP(("ReadWBPrefs: exit\n"));
}


struct TextAttr defaultAttr = {
    TopazName,	/* name (filled in to 'topaz.font') */
    8,		/* ysize */
    0,		/* style */
    0		/* flags */
};

void initSomeText(struct WorkbenchFont *pref,struct RastPort *rp,struct TextFont **theFont,struct TextAttr **theAttr)
{
struct WorkbenchBase *wb = getWbBase();
struct WorkbenchFont junk;

	if (!pref)
	{
		pref=&junk;
		pref->wf_Attr=&defaultAttr;
		pref->wf_Font=OpenFont(&defaultAttr);
		pref->wf_DrawMode=JAM2;
		pref->wf_FrontPen=1;
		pref->wf_BackPen=0;
	}

	if (*theAttr==&defaultAttr)
	{
		CloseFont(*theFont);
		*theAttr=NULL;
	}

	*theAttr=pref->wf_Attr;
	if (*theFont=pref->wf_Font)
	{
		SetFont(rp,*theFont);

		/* Check for the special case... */
		if (theFont == &(wb->wb_IconFont))
		{
			wb->wb_IconFontHeight=(*theFont)->tf_YSize;
			wb->wb_APen=pref->wf_FrontPen;
			wb->wb_BPen=pref->wf_BackPen;
			wb->wb_DrawMode=pref->wf_DrawMode;
		}
	}
	else
	{
		/* There is no way out since we did not even get TOPAZ-8! */
		Alert(AN_NoFonts | AT_DeadEnd);
	}
}

void termText()
{
struct WorkbenchBase *wb = getWbBase();

	if (wb->wb_IconAttr==&defaultAttr)
	{
		CloseFont(wb->wb_IconFont);
		wb->wb_IconAttr=NULL;
	}
	if (wb->wb_TextAttr==&defaultAttr)
	{
		CloseFont(wb->wb_TextFont);
		wb->wb_TextAttr=NULL;
	}
}

struct WB_Semaphore *FindWB_Sem(void)
{
struct	WB_Semaphore	*sem;

	/* Protect the finding/adding of the semaphore... */
	FORBID;
	if (!(sem=(struct WB_Semaphore *)FindSemaphore(SystemWorkbenchName)))
	{
		if (sem=ALLOCMEM(sizeof(struct WB_Semaphore)+10,MEMF_CLEAR|MEMF_PUBLIC))
		{
			InitSemaphore(&(sem->wbs_Semaphore));

			/* Init the pattern semaphores */
			InitSemaphore(&(sem->wbs_RootPattern.bms_Semaphore));
			InitSemaphore(&(sem->wbs_WindowPattern.bms_Semaphore));

			strcpy(sem->wbs_Semaphore.ss_Link.ln_Name=(char *)&(sem[1]),SystemWorkbenchName);
			sem->wbs_Semaphore.ss_Link.ln_Pri=-127;
			AddSemaphore(&(sem->wbs_Semaphore));
		}
	}
	PERMIT;

	return(sem);
}

void initIconText(void)
{
struct	WorkbenchBase	*wb = getWbBase();
struct	WB_Semaphore	*sem;

	ObtainSemaphore(sem=FindWB_Sem());
	initSomeText(sem->wbs_Icon,&(wb->wb_IconRast),&(wb->wb_IconFont),&(wb->wb_IconAttr));
	ReleaseSemaphore(sem);
}

void initTextText(void)
{
struct	WorkbenchBase	*wb = getWbBase();
struct	WB_Semaphore	*sem;

	ObtainSemaphore(sem=FindWB_Sem());
	initSomeText(sem->wbs_Text,&(wb->wb_TextRast),&(wb->wb_TextFont),&(wb->wb_TextAttr));
	ReleaseSemaphore(sem);
}

void initText(void)
{
	initIconText();
	initTextText();
}

int InitPattern(void)
{
struct	WorkbenchBase	*wb = getWbBase();
struct	WB_Semaphore	*sem;

	if (sem=FindWB_Sem())
	{
		wb->wb_WBHook.h_Data=(void *)&(sem->wbs_RootPattern);
		wb->wb_WinHook.h_Data=(void *)&(sem->wbs_WindowPattern);
	}

	return(sem!=NULL);
}

void NewConfig(struct NewWBConfig *msg)
{
struct WorkbenchBase *wb = getWbBase();
BOOL reset=FALSE;

	switch (msg->tag)
	{
	case WORKBENCH_ROOTPATTERN:	RefreshDrawer(wb->wb_RootObject);
					break;
	case WORKBENCH_WINDOWPATTERN:	MasterSearch(RefreshDrawer);
					break;
	case WORKBENCH_TEXT_FONT:	initTextText();
					MasterSearch(ForceRebuildViewByTextImagery);
					MasterSearch(RefreshDrawer);
					break;
	case WORKBENCH_ICON_FONT:	initIconText();
					MasterSearch(Cleanup);
					break;
	case WORKBENCH_RESET:
	case WORKBENCH_NEW_LOCALE:	reset=TRUE;
					break;
	}
	REPLYMSG((struct Message *)msg);

	if (reset) if (wb->wb_BackWindow) if (wb->wb_BackWindow->MenuStrip) ResetWB();
}

/*
 * This is the internal routine that does the updating of the semaphore as
 * needed.
 */
ULONG WBConfig_Update(ULONG tag,ULONG data)
{
struct	WB_Semaphore	*sem;
struct	BitMap_Sem	*bms=NULL;
	ULONG		*p=NULL;
	ULONG		olddata=data;

	if (sem=FindWB_Sem())
	{
		ObtainSemaphore(&(sem->wbs_Semaphore));

		switch (tag)
		{
		case WORKBENCH_ROOTPATTERN:	bms=&(sem->wbs_RootPattern);
						break;
		case WORKBENCH_WINDOWPATTERN:	bms=&(sem->wbs_WindowPattern);
						break;
		case WORKBENCH_ICON_FONT:	p=(ULONG *)&(sem->wbs_Icon);
						break;
		case WORKBENCH_TEXT_FONT:	p=(ULONG *)&(sem->wbs_Text);
						break;
		}

		/* If BMS, obtain the semaphore */
		if (bms)
		{
			ObtainSemaphore(&(bms->bms_Semaphore));
			p=(ULONG *)&(bms->bms_PatternBitMap);
		}

		/* Install the new data if we have a data pointer... */
		if (p)
		{
			olddata=*p;	/* Get old value */
			*p=data;	/* Install new value */
		}

		/* If BMS, release the semaphore */
		if (bms) ReleaseSemaphore(&(bms->bms_Semaphore));

		ReleaseSemaphore(&(sem->wbs_Semaphore));
	}

	return(olddata);
}

/*
*****i* workbench.library/WBConfig ********************************************
*
*   NAME
*	WBConfig - Set a configuration item within Workbench...          (V39)
*
*   SYNOPSIS
*	OldVal = WBConfig(id, newval)
*	d0                d0  d1
*
*	ULONG WBConfig(ULONG, ULONG);
*
*   FUNCTION
*	This is a private function used with IPrefs and Workbench.library
*	The ID given defines what the data (newval) is.
*	If the ID is unknown, the routine returns the newval as the oldval.
*	This function may Wait() for a while as it may need to obtain
*	semaphores and/or sync with the Workbench process.
*
*   INPUTS
*	id - The value type ID.  This ID is currently (V39) one of the
*	following:
*		WORKBENCH_ROOTPATTERN
*			In this case, the data is a pointer to a pattern
*			bitmap which is a Width/Height plus bitmap.
*			The structure looks like this:
*			struct PatternBitMap
*			{
*				struct	BitMap	*pbm_BitMap;
*					UWORD	pbm_Width;
*					UWORD	pbm_Height;
*			};
*
*		WORKBENCH_WINDOWPATTERN
*			This is the pattern used for windows.  The data
*			structure is the same as above.
*
*		WORKBENCH_ICON_FONT
*			This defines the font to be used in the window
*			when a proportional font is available.
*			The structure likes like this:
*			struct WorkbenchFont
*			{
*				struct	TextAttr *wf_Attr;
*				struct	TextFont *wf_Font;
*					UWORD    wf_DrawMode;
*					UBYTE    wf_FrontPen;
*					UBYTE    wf_BackPen;
*			};
*
*		WORKBENCH_TEXT_FONT
*			This defines the font to be used in the window
*			when it must be mono-spaced.  The structure is the
*			WorkbenchFont structure.
*
*		WORKBENCH_NEW_LOCALE
*			The data fiels is not used.  This tells workbench
*			that a new locale was set as the system locale.
*			Workbench will endever to do whatever it needs to
*			make itself use that local.
*
*		WORKBENCH_RESET
*			The data field is not used here.  This is just used
*			to have workbench reset itself.  It is exactly like
*			the RESET menu item.
*
*	newval - The new value to be set for whichever of these id's are
*	sent.  The value many times is a pointer.
*
*   RESULTS
*	OldVal - This is the old value that was stored in the configuration
*	just sent.  If the configuration is unknown, this will be the same
*	as newval.  If the configuration was never set before, it will be NULL.
*
*   NOTE
*	This is the prototype and pragma for SAS/C such that
*	the function can be used in our private work.  (So we can get IPrefs)
*
*		ULONG WBConfig( unsigned long tag, unsigned long value );
*		#pragma libcall WorkbenchBase WBConfig 54 1002
*
*   SEE ALSO
*
*   BUGS
*
******************************************************************************
*/
ULONG __stdargs __asm WBConfig(register __d0 ULONG tag,register __d1 ULONG data)
{
struct	WorkbenchBase	*wb = getWbBase();
	ULONG		olddata;
struct	MsgPort		tmpPort;
struct	NewWBConfig	tmpMsg;

	/* Update the configuration structure */
	olddata=WBConfig_Update(tag,data);

	/* Now, signal workbench as needed to update its structures... */
	FORBID;
	if ((wb->wb_WorkbenchStarted) && !(wb->wb_Quit))
	{
    		tmpPort.mp_Flags=PA_SIGNAL;
		tmpPort.mp_SigTask=ThisTask();
		tmpPort.mp_SigBit=SIGB_SINGLE;
		NewList(&tmpPort.mp_MsgList);
		tmpMsg.msg.mn_ReplyPort=&tmpPort;
		tmpMsg.tag=tag;
		PUTMSG(&(wb->wb_NotifyPort),&(tmpMsg.msg));
		WAITPORT(&tmpPort);
	}
	PERMIT;

	return(olddata);
}
