/*
 * $Id: workbenchbase.h,v 38.11 93/01/21 14:48:57 mks Exp $
 *
 * $Log:	workbenchbase.h,v $
 * Revision 38.11  93/01/21  14:48:57  mks
 * Cleaned up the disk init code...  (Removed much redundant code)
 * 
 * Revision 38.10  92/08/20  10:00:01  mks
 * Added wb_Copyright4
 *
 * Revision 38.9  92/07/13  18:34:09  mks
 * Changed to contain a config lock
 *
 * Revision 38.8  92/06/11  09:25:39  mks
 * renamed some of the bits as unused
 *
 * Revision 38.7  92/04/16  09:16:59  mks
 * Added the copyright string pointers
 *
 * Revision 38.6  92/04/14  13:21:46  mks
 * Removed the WaitPointer field
 *
 * Revision 38.5  92/02/18  10:16:46  mks
 * Private WB base changed a bit
 *
 * Revision 38.4  92/01/07  16:09:20  mks
 * Major rework of the workbenchbase for the new config stuff.
 *
 * Revision 38.3  91/11/12  18:42:02  mks
 * Updated to support even better locale stuff
 *
 * Revision 38.2  91/09/12  10:44:57  mks
 * Added two long words to the library base for locale support
 *
 * Revision 38.1  91/06/24  11:39:56  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#ifndef WORKBENCHBASE_H
#define WORKBENCHBASE_H

#include	<exec/types.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>
#include	<libraries/dos.h>
#include	<libraries/dosextens.h>
#include	<graphics/rastport.h>
#include	<devices/timer.h>
#include	<intuition/intuition.h>
#include	<utility/hooks.h>

#ifndef WORKBENCH_WORKBENCH_H
#include "workbench.h"
#endif

#ifndef WBINTERNAL_H
#include "wbinternal.h"
#endif

#ifndef	WBSTART_H
#include "wbstart.h"
#endif

#define EMBOSEWIDTH	(wb->wb_EmboseBorderLeft + wb->wb_EmboseBorderRight)
#define EMBOSEHEIGHT	(wb->wb_EmboseBorderTop + wb->wb_EmboseBorderBottom)
#define VIEWBYTEXTLEFTOFFSET	(4)

#define WBBUFSIZE	260	/* sizeof(struct FileInfoBlock) ??? */

struct IconMsg {
    struct Message im_Message;	/* msg struct */
    LONG im_dobj;	/* Flag set to TRUE if Put and FALSE is Delete */
    BPTR im_lock;		/* lock on parentdir of 'name' */
    char im_name;		/* storage for name starts here */
};

struct TypedIconMsg {
    WORD tim_type;		/* msg type */
    struct IconMsg tim_IconMsg;	/* IconMsg struct */
};

#define	WBConfig_Version	1

struct WBConfig
{
	LONG	wbc_Version;	/* This needs to be n... */
	LONG	wbc_Reserved;	/* This is currently 0 */
	SHORT	wbc_LeftEdge;	/* These values are the left/top/width/height */
	SHORT	wbc_TopEdge;	/* of the backdrop window if it is not a */
	SHORT	wbc_Width;	/* backdrop.  These values *MUST* remain in */
	SHORT	wbc_Height;	/* This location if Version > 0 */
	WORD	wbc_Backdrop;	/* Backdrop flag */
	WORD	wbc_Doubleclick;/* Not used currently... */
};

struct BitMap_Sem
{
struct	SignalSemaphore	bms_Semaphore;	/* The semaphore for this bitmap */
struct	PatternBitMap	*bms_PatternBitMap;
	UWORD		bms_ToMinX;
	UWORD		bms_ToMinY;
	UWORD		bms_FromMinX;
	UWORD		bms_FromMinY;
	UWORD		bms_FromMaxX;
	UWORD		bms_FromMaxY;
	UWORD		bms_SaveMinX;
	UWORD		bms_SaveMaxX;
	UWORD		bms_SaveMinX1;
	UWORD		bms_pad;	/* Longword allign... */
};

struct WB_Semaphore
{
struct	SignalSemaphore	wbs_Semaphore;	/* The semaphore structure */
struct	BitMap_Sem	wbs_RootPattern;
struct	BitMap_Sem	wbs_WindowPattern;
struct	WorkbenchFont	*wbs_Icon;
struct	WorkbenchFont	*wbs_Text;
};

struct WorkbenchBase {

    struct Library	wb_Library;

    BYTE		wb_UpdateNestCnt;
    BYTE		wb_DiskIONestCnt;
    /* now longword aligned */

    APTR		wb_SegList;	/* ptr to wb's seglist */

    /* the workbench task */
    void *		wb_Task;

    /* the work bench status flags */
    /* Flags1 (assembler) */
    int	wb_unused_0 : 1;	/* unused bit... */
    int	wb_Dragging : 1;	/* on when dragging icon(s) */
    int	wb_DoubleClick : 1;	/* we just double clicked */
    int	wb_unused_1 : 1;	/* unused bit... */
    int	wb_InputTrashed : 1;	/* Don't believe SELECTUP */
    int	wb_ErrorDisplayed : 1;	/* ErrorTitle is up */
    int	wb_NameChange : 1;	/* Name Change win is up */
    int	wb_Closed : 1;		/* WB screen is closed */

    /* Flags2 (assembler) */
    int	wb_KPrintfOK : 1;	/* ok to send kprintf's */
    int	wb_unused_2 : 1;	/* unused bit... */
    int	wb_unused_3 : 1;	/* unused bit... */
    int	wb_DragSelect : 1;	/* drag selecting */
    int	wb_Quit : 1;		/* quit WB */
    int	wb_Backdrop : 1;	/* backdrop disk window is enabled */
    int	wb_WorkbenchStarted : 1; /* workbench has been started */
    int	wb_unused_4 : 1;	/* unused bit... */

    /* Flags3 (Magic stuff for assembler... Do not Touch!) */
    int wb_PadBit01 : 1;
    int	wb_PadBit02 : 1;
    int	wb_PadBit03 : 1;
    int	wb_PadBit04 : 1;
    int	wb_PadBit05 : 1;
    int	wb_PadBit06 : 1;
    int	wb_PadBit07 : 1;
    int	wb_PadBit08 : 1;
    int	wb_PadBit09 : 1;
    int	wb_PadBit10 : 1;
    int	wb_PadBit11 : 1;
    int	wb_PadBit12 : 1;
    int	wb_PadBit13 : 1;
    int	wb_PadBit14 : 1;
    int	wb_PadBit15 : 1;
    int	wb_PadBit16 : 1;

    /* three message ports -- two for intuition, and one for all reasonable
     * applications that let you allocate your messages yourself....
     */
    struct MsgPort	wb_WorkbenchPort;	/* workbench style msgs */

    struct MsgPort	wb_IntuiPort;		/* msgs from intuition */
    struct MsgPort	wb_SeenPort;		/* delayed msgs from intuition */
    struct MsgPort	wb_NotifyPort;		/* msgs from notification handler */

    struct MsgPort     *wb_CopyReaderPort;	/* The reader port for Copy... */
    struct MsgPort     *wb_CopyWriterPort;	/* The writer port for Copy... */

    /* all of our library bases */
    struct Library *	wb_SysBase;
    struct Library *	wb_GfxBase;
    struct Library *	wb_IntuitionBase;
    struct Library *	wb_IconBase;
    struct Library *	wb_DOSBase;
    struct Library *	wb_LayersBase;
    struct Library *	wb_GadToolsBase;
    struct Library *	wb_UtilityBase;
    struct Library *	wb_TimerBase;

    /* The three copyright pointers */
    char	       *wb_Copyright1;
    char	       *wb_Copyright2;
    char	       *wb_Copyright3;
    char               *wb_Copyright4;

    /* our three major lists.  Every object is on the master list.
     * The highlighted (selected) icons are on the select list.
     * all disks that we have "active" are on the active list.
     */
    struct List		wb_MasterList;
    struct List		wb_SelectList;
    struct List		wb_ActiveDisks;
    struct List		wb_UtilityList;

    struct List		wb_AppWindowList;	/* list of App Windows */
    struct List		wb_AppIconList;		/* list of App Icons */
    struct List		wb_AppMenuItemList;	/* list of App MenuItems */

    struct List		wb_BobList;		/* all bobs are on this list */

    struct List		wb_InfoList;		/* list of .info files */
    struct List		wb_NonInfoList;		/* list of non .info files */

	/* Note: We must be long-word alinged here... */
    BPTR		wb_OldPath;		/* Old path... */
    LONG		wb_OldPathTick;		/* Tick count... */

    LONG		wb_ByteCount;		/* Count bytes used in copy... */

    /* the mouse position of the last select press */
    UWORD		wb_XOffset;
    UWORD		wb_YOffset;

    /* For locale... */
    struct Library *	wb_LocaleBase;
    void *		wb_Catalog;

    /* a pointer to a timer request, so we can free it later */
    struct timerequest *wb_TimerRequest;

    /* the base of the workbench object tree */
    struct WBObject *	wb_RootObject;

    /* some current pointers */
    struct Gadget *	wb_CurrentGadget;
    struct Window *	wb_CurrentWindow;
    struct WBObject *	wb_CurrentObject;

    /* Last IDCMPWindow to get a CloseWindow event. */
    struct Window *	wb_LastCloseWindow;

    /* images for arrow gadgets */
    struct Image 	*wb_LeftImage;
    struct Image 	*wb_RightImage;
    struct Image 	*wb_UpImage;
    struct Image 	*wb_DownImage;

    /*
     * Now, we need the two call-back hooks here...
     */
    struct Hook		wb_WBHook;
    struct Hook		wb_WinHook;

    /* two string buffers to save us from allocating memory */
    /* THESE MUST BE LONG WORD ALLIGNED */
    UBYTE 		wb_Buf0[WBBUFSIZE];
    UBYTE 		wb_Buf1[WBBUFSIZE];

    /* misc stuff */
    ULONG		wb_LastFreeMem;
    struct TextFont *	wb_IconFont;	/* for icon names */
    struct TextAttr *	wb_IconAttr;	/* for icon names */
    struct TextFont *	wb_TextFont;	/* for viewbytext */
    struct TextAttr *	wb_TextAttr;	/* for viewbytext */

    /* the last time we saw a key-down */
    struct timeval	wb_Tick;

    /* what we need to paint text */
    struct RastPort	wb_IconRast;	/* for icon text */
    struct RastPort	wb_TextRast;	/* for viewby text */

    /* the argument that we were initialized with */
    ULONG		wb_Argument;

    /* stuff for moving bobs dynamically */
    struct FreeList	wb_BobMem;	/* all bob mem is here */
    WORD		wb_LastX;	/* last X of mouse was here */
    WORD		wb_LastY;	/* last Y of mouse was here */

    struct Screen *	wb_Screen;	/* pointer to workbench screen */
    struct Window *	wb_BackWindow;	/* pointer to workbench window */
    struct GelsInfo *	wb_GelsInfo;	/* pointer for screen gels stuff */

    UWORD		wb_IconFontHeight;	/* height of icon font */

    /* these two lists are used for 'Show All' */
    UWORD		wb_MaxTextWidth;	/* used by viewbytext for drawers */

    struct Menu        *wb_MenuStrip;		/* main menu */
    struct Menu        *wb_ToolMenu;		/* tool menu */
    struct Menu        *wb_HiddenMenu;		/* debugging menu */
    struct timerequest *wb_LayerDemonRequest;	/* timer request for locked layer checking */
    struct MsgPort	wb_LayerPort;		/* msgs from locked layer request */

    APTR		wb_VisualInfo;		/* GadTools VisualInfo */

    UWORD		wb_AsyncCopyCnt;	/* # of outstanding async copies going on */
    UWORD		wb_Word2;		/* used during testing */

    struct WBConfig	wb_Config;		/* My config structure */
    ULONG		wb_ConfigLock;		/* Lock count of config... */

    ULONG		wb_ToolExitCnt;		/* # of outstanding launched programs */
    UWORD		wb_AppMenuItemCnt;	/* # of app menu items */
    UBYTE		wb_shinePen;		/* shinePen for embosing icons */
    UBYTE		wb_shadowPen;		/* shadowPen for embosing icons */

    UBYTE		wb_EmboseBorderTop;	/* height of embose top border */
    UBYTE		wb_EmboseBorderBottom;	/* height of embose bottom border */
    UBYTE		wb_EmboseBorderLeft;	/* width of embose left border */
    UBYTE		wb_EmboseBorderRight;	/* width of embose right border */

    UBYTE		wb_APen;	/* foreground color for text */
    UBYTE		wb_BPen;	/* background color for text */
    UBYTE		wb_DrawMode;	/* draw mode for text */

    /* the screen title */
    UBYTE		wb_ScreenTitle[190];
    UBYTE		wb_CurrentError[190];

    UBYTE 		wb_ExecuteBuf[WBBUFSIZE];	/* one line history for execute (Amiga-E) */
};

/* When icons are being dragged, they are tracked via the BobList.
 * Each icon has a BobNode on the BobList.
 */
struct BobNode {
    struct Node		bn_Node;
    int			bn_Drawn : 1;	/* this bob is drawn at least once */
    UBYTE		bn_PeterPad;
    UBYTE		bn_pad;
    struct Bob *	bn_Bob;		/* pointer to bob for obj */
    struct WBObject *	bn_Obj;		/* pointer to workbench obj */
};

struct ActiveDisk {
    struct Node		ad_Node;
    UBYTE		ad_Active;
    UBYTE		ad_Flags;
    struct MsgPort *	ad_Handler;
    UBYTE *		ad_Name;
    struct WBObject *	ad_Object;
    BPTR 		ad_Volume;
    struct InfoData	ad_Info;
    struct DateStamp	ad_CreateTime;
};

/* defines for ad_Flags */
#define	ADB_FILLBACKDROP	0	/* FillBackdrop has been called (bit posn) */
#define	ADF_FILLBACKDROP	1	/* FillBackdrop has been called (flag) */

struct CreateToolMsg {
    struct Message ctm_Message;			/* message structure itself */
    void (*ctm_Handler)();			/* ptr to the cleanup function */
    struct typedmsg *ctm_StartupMsg;		/* ptr to tool's startup message */
    ULONG ctm_Lock;				/* a lock for the tool */
    char *ctm_Name;				/* ptr to tool's name */
    ULONG ctm_Priority;				/* tool's priority */
    ULONG ctm_StackSize;			/* tool's stacksize */
    struct CommandLineInterface *ctm_Cli;	/* ptr to wb cli (used to search path) */
    BPTR ctm_LoadLock;				/* lock tool was loaded from */
};

#endif /* ! WORKBENCHBASE_H */
