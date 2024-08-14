/*
 * workbenches private data structures
 *
 * $Id: wbinternal.h,v 38.3 92/01/08 14:14:01 mks Exp $
 *
 * $Log:	wbinternal.h,v $
 * Revision 38.3  92/01/08  14:14:01  mks
 * Fixed nested comment problem (for those who don't like that)
 * 
 * Revision 38.2  92/01/07  14:04:49  mks
 * Added the defined and structures for the new configuration method
 * in the V39 Workbench.
 *
 * Revision 38.1  91/06/24  11:39:23  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#ifndef WBINTERNAL_H
#define WBINTERNAL_H

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif /* EXEC_TYPES_H */

#ifndef INTUITION_INTUITION_H
#include "intuition/intuition.h"
#endif /* INTUITION_INTUITION_H */

#ifndef LIBRARIES_DOS_H
#include "libraries/dos.h"
#endif /* LIBRARIES_DOS_H */

#ifndef WORKBENCH_WORKBENCH_H
#include "workbench/workbench.h"
#endif /* WORKBENCH_WORKBENCH_H */

/*
** These values are for the WBConfig() LVO
*/
#define	WORKBENCH_RESET		0
#define	WORKBENCH_ROOTPATTERN	1
#define	WORKBENCH_WINDOWPATTERN	2
#define	WORKBENCH_ICON_FONT	3
#define	WORKBENCH_TEXT_FONT	4
#define	WORKBENCH_NEW_LOCALE	5

struct WorkbenchFont
{
struct	TextAttr *wf_Attr;
struct	TextFont *wf_Font;
	UWORD    wf_DrawMode;
	UBYTE    wf_FrontPen;
	UBYTE    wf_BackPen;
};

struct PatternBitMap
{
struct	BitMap	*pbm_BitMap;
	UWORD	pbm_Width;
	UWORD	pbm_Height;
};

/* each message that comes into the WorkBenchPort must have a type field
** in the preceeding short.  These are the defines for this type
*/

#define MTYPE_PSTD		1	/* a "standard Potion" message */
#define MTYPE_TOOLEXIT		2	/* exit message from our tools */
#define MTYPE_DISKCHANGE	3	/* dos telling us of a disk change */
#define MTYPE_TIMER		4	/* we got a timer tick */
#define MTYPE_CLOSEDOWN		5	/* <unimplemented> */
#define MTYPE_IOPROC		6	/* <unimplemented> */
#define MTYPE_APPWINDOW		7	/* msg from an app window */
#define MTYPE_APPICON		8	/* msg from an app icon */
#define MTYPE_APPMENUITEM	9	/* msg from an app menuitem */
#define MTYPE_COPYEXIT		10	/* exit msg from copy process */
#define MTYPE_ICONPUT		11	/* msg from PutDiskObject in icon.library */
#define	MTYPE_APP_FUNC		12	/* msg for the App ADD/DEL functions */

/*
 * The following structure is use by all of the AppXXX features...
 */
struct WorkbenchAppInfo
{
struct	Node	wai_Node;		/* linkage */
	void	*wai_Ptr;		/* used internally by Workbench */
struct	MsgPort	*wai_MsgPort;		/* application's message port */
	ULONG	wai_UserData;		/* application specific */
	ULONG	wai_ID;			/* application definable ID */
};

struct NewDD { /* ram drawer data */
/*
    The next 5 entries are the members of a DrawerData structure
    and MUST remain in this exact order.  This is the information
    that gets written to the disk.
*/
    struct NewWindow	dd_NewWindow;	/* args to open window */
    LONG		dd_CurrentX;	/* current x coordinate of origin */
    LONG		dd_CurrentY;	/* current y coordinate of origin */
    ULONG		dd_Flags;	/* flags for the dir */
    UWORD		dd_ViewModes;	/* how does he want to see the dir? */
    /* note: above info is written/read to/from disk as DrawerData */
    LONG		dd_MinX;	/* smallest x coordinate in window */
    LONG		dd_MinY;	/* smallest y coordinate in window */
    LONG		dd_MaxX;	/* largest x coordinate in window */
    LONG		dd_MaxY;	/* largest y coordinate in window */
    struct Gadget	dd_HorizScroll;
    struct Gadget	dd_VertScroll;
    struct Gadget	dd_UpMove;
    struct Gadget	dd_DownMove;
    struct Gadget	dd_LeftMove;
    struct Gadget	dd_RightMove;
    struct Image	dd_HorizImage;
    struct Image	dd_VertImage;
    struct PropInfo	dd_HorizProp;
    struct PropInfo	dd_VertProp;
    struct Window *	dd_DrawerWin;	/* pointer to drawers window */
    struct WBObject *	dd_Object;	/* back pointer to drawer object */
    struct List		dd_Children;	/* where our children hang out */
    LONG		dd_Lock;	/* lock on our drawer */

	/*
	 * The next entries are used to place icons faster.  These are
	 * new entries for this purpose...  (Ugly kludge to fix a very
	 * ugly routine/problem.)
	 */
    LONG		dd_LastColumn; /* Last column used */
    LONG		dd_LastObjY;   /* Last object Y size */
    LONG		dd_LastMin;    /* Last Min */
    LONG		dd_LastMax;    /* Last Max */
    LONG		dd_LastUse;    /* Last Use count */
};

/* definitions for dd_ViewModes */
#define DDVM_BYDEFAULT	0
#define DDVM_BYICON	1
#define DDVM_BYNAME	2
#define DDVM_BYDATE	3
#define DDVM_BYSIZE	4

/* definitions for dd_Flags */
#define	DDFLAGS_SHOWDEFAULT	0	/* default (show only icons) */
#define	DDFLAGS_SHOWICONS	1	/* show only icons */
#define	DDFLAGS_SHOWALL		2	/* show all files */

struct WBObject {
    struct Node		wo_MasterNode;	/* all objects are on this list */
    struct Node		wo_Siblings;	/* list of drawer members */
    struct Node		wo_SelectNode;	/* list of all selected objects */
    struct Node		wo_UtilityNode;	/* function specific linkages */
    struct WBObject *	wo_Parent;	/* ptr to this childs parent */
    /*
	The next 9 entries are the members of a MyDiskObject structure
	and MUST remain in this exact order.  This is the information
	that gets written to the disk.
    */
    char *		wo_DefaultTool; /* ptr to icon's default tool */
    char *		wo_ToolWindow;	/* char string for tool's window */
    char **		wo_ToolTypes;	/* the types for this tool */
    struct NewDD *	wo_DrawerData;	/* if this is a drawer or disk */
    LONG		wo_CurrentX;	/* virtual X in drawer */
    LONG		wo_CurrentY;	/* virtual Y in drawer */
    LONG		wo_StackSize;	/* how much stack to give to this */
    struct Gadget	wo_IOGadget;	/* the gadget to write to the disk */
    UBYTE		wo_Type;	/* what flavor object is this? */

    /* rest of V1.3 fields (albeit re-arranged) */

    int		wo_DrawerSilent:1;	/* drawer is silent (has open children) */
    int		wo_DrawerOpen:1;	/* drawer is partially open (for close/open wb) */
    int		wo_Selected:1;		/* our icon is selected */
    int		wo_Background:1;	/* set if icon is in background */
    int		wo_Invisible:1;		/* can't see this icon */
    int		wo_AppIcon:1;		/* this is an application icon */
    int		wo_Startup:1;		/* icon came from startup drawer */
    int		wo_FakeIcon:1;		/* icon has no .info file */
    int		wo_LeftOut:1;		/* object is currently left out */
    int		wo_PutAway:1;		/* object needs to be put away */

    USHORT		wo_UseCount;	/* number of references to this obj */
    char *		wo_Name;	/* this object's textual name */
    struct Window *	wo_IconWin; 	/* each object's icon lives here */
    /*
	'wo_Lock' is NULL unless the object is either in the Disk window
	(formerly called the backdrop window) or has no parent
	(ie. the object's parent window was closed.  If non-NULL,  it is
	a lock on the object's parent.
   */
    LONG		wo_Lock;	/* see above description */
    SHORT		wo_NameXOffset;	/* where to put the name */
    SHORT		wo_NameYOffset; /* where to put the name */
    struct FreeList	wo_FreeList;	/* this objects free list */

    /* new fields for V1.4 */

    char *		wo_NameBuffer;	/* for view by name/size/date etc */
    struct DiskObject*	wo_DiskObject;	/* valid only for GetDefDiskObject icons */
    /*
	When we switch to ViewByText (ie. Name, Date, Size, etc.) we must
	save the position of the icon which is specified by CurrentX
	and CurrentY.  We need to save it because we must use these two
	variables to specify the text position of the icon thus otherwise
	destroying the graphical position.  If we didn't save the graphical
	position, when we switched back to ViewByIcon, the icons would have
	lost their position.  So we save it here. Whew!
    */
    LONG		wo_SaveX;	/* see explanation above */
    LONG		wo_SaveY;	/* see explanation above */

    ULONG		wo_FileSize;	/* last size that we know about */
    ULONG		wo_Protection;	/* the protection for the file */
    struct Gadget	wo_Gadget;	/* NOT a ptr, but an instance of it */
    struct DateStamp	wo_DateStamp;	/* modification time */
    struct Image	wo_NameImage;	/* special image for view by name */
    USHORT		wo_ImageSize;	/* size of bitmap for image data */
};

#define WONAME_SIZE		80	/* the total size of the name buffer */
#define WONAME_ACTIVE	25	/* active part of name gadget */

/* we use the gadget id field to encode some special information */
#define GID_WBOBJECT	0	/* a normal workbench object */
#define GID_HORIZSCROLL	1	/* the horizontal scroll gadget for a drawer */
#define GID_VERTSCROLL	2	/* the vertical scroll gadget for a drawer */
#define GID_LEFTSCROLL	3	/* move one window left */
#define GID_RIGHTSCROLL	4	/* move one window right */
#define GID_UPSCROLL	5	/* move one window up */
#define GID_DOWNSCROLL	6	/* move one window down */
/*#define GID_NAME	7*/	/* the name field for an object (not used) */


#define DEFAULT_TOOLPRI		0	/* default tool priority */
#define DEFAULT_STACKSIZE	4096	/* default stack size */

#endif /* WBINTERNAL_H */
