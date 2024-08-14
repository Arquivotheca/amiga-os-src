/*
 * $Id: info.h,v 39.3 93/02/04 11:03:26 mks Exp $
 *
 * $Log:	info.h,v $
 * Revision 39.3  93/02/04  11:03:26  mks
 * Cleaned up the structure a bit and removed unused parts
 * 
 * Revision 39.2  92/05/31  16:23:26  mks
 * Removed the lock fields since I am not going to use them...
 *
 * Revision 39.1  92/05/31  01:52:25  mks
 * Now is the ASYNC, LVO based INFO...
 *
 * Revision 38.5  92/05/26  11:39:48  mks
 * Changed the mask of items that get the comment field...
 *
 * Revision 38.4  92/05/22  16:41:35  mks
 * Removed the RMBTrap from Info...
 *
 * Revision 38.3  92/04/24  16:20:01  mks
 * Removed unused field
 *
 * Revision 38.2  92/03/10  11:33:33  mks
 * Added the data space for the path
 *
 * Revision 38.1  91/06/24  11:36:11  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include "workbench.h"
#include "wbinternal.h"

/*------------------------------------------------------------------------*/

#define BLUE	0
#define WHITE	1
#define BLACK	2
#define ORANGE	3

/*------------------------------------------------------------------------*/

#define Empty(list)	(!((list)->lh_Head->ln_Succ))
#define HeadNode(list)	((list)->lh_Head)
#define TailNode(list)	((list)->lh_TailPred)

/*  Extract the StringInfo of a string gadget: */
#define StrInfo(gad)	((struct StringInfo *) gad->SpecialInfo)

/*------------------------------------------------------------------------*/

/*  Max length of string gadgets (numbers cloned from Info V34): */
#define COMMENTLENGTH		80
#define DEFAULTTOOLLENGTH	256
#define TOOLTYPESLENGTH		128
#define NUMSTRINGLENGTH		10
#define DATESTRINGLENGTH	50
#define STRINGLENGTH		255

/*------------------------------------------------------------------------*/

/*  Info's window parameters: */

#define INFOW_LEFTEDGE	0
#define INFOW_TOPEDGE	0
#define INFOW_WIDTH	472
#define INFOW_HEIGHT	182
#define INFOW_IDCMPFLAGS	(CLOSEWINDOW | REFRESHWINDOW | LISTVIEWIDCMP | GADGETUP)
#define INFOW_FLAGS	(WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH | SIMPLE_REFRESH | ACTIVATE)
#define INFOW_MINWIDTH	1
#define INFOW_MINHEIGHT	1
#define INFOW_MAXWIDTH	0
#define INFOW_MAXHEIGHT	0

#define ZOOMLEFT	INFOW_LEFTEDGE
#define ZOOMTOP		INFOW_TOPEDGE
#define ZOOMWIDTH	200

/*------------------------------------------------------------------------*/

/*  Each Info Window has its UserData field pointing to a struct InfoTag,
    which holds assorted information on the icon, etc. */

struct InfoTag
    {
    struct DiskObject *it_Obj;
    LONG it_ObjFake;
    char *it_Name;
    char *it_NameInfo;
    struct FileInfoBlock *it_FIB;
    struct InfoData *it_InfoData;
    struct Window *it_Window;
    ULONG it_MaskType;
    ULONG it_Flags;	/*  See below */

	struct	Rectangle	it_IconRect;	/* Icon display window */

    LONG it_Protection;		/*  This is the protection bits of the file,
				    or of the .info if there was no file. */
    struct List it_ToolTypeList;
    struct Gadget *it_FirstGadget;
    struct Gadget *it_Stack;
    struct Gadget *it_Comment;
    struct Gadget *it_DefaultTool;
    struct Gadget *it_TTList;
    struct Gadget *it_TTString;
    struct Gadget *it_TTDel;
    struct Node *it_CurrentTTNode;
    struct Node *it_AfterNode;
    struct TextFont *it_InfoFont;
    struct DateStamp it_CreateDate;
    void *it_VisualInfo;
    UBYTE it_Title[200];	/* For those really long file names */
    UBYTE it_Path[256];		/* For the path to the file */
    };

/*  it_Flags: */

#define IT_GOT_FIB	0x00000001	/*  Read the FIB ok */
#define IT_GOT_INFODATA	0x00000002	/*  Got the InfoData ok */
#define IT_GOT_VOLNODE	0x00000004	/*  Got the volume node ok */

/*------------------------------------------------------------------------*/

#define MASK_DISK	(1 << WBDISK)
#define MASK_DRAWER	(1 << WBDRAWER)
#define MASK_TOOL	(1 << WBTOOL)
#define MASK_PROJECT	(1 << WBPROJECT)
#define MASK_GARBAGE	(1 << WBGARBAGE)
#define MASK_KICK	(1 << WBKICK)

/*  Now we can define which types of icon have what features in the
    Info window: */

/*  These have FileInfoBlock we care to read: */
#define MASK_FIB		(MASK_DRAWER | MASK_TOOL | MASK_PROJECT |\
				MASK_GARBAGE)

/*  These have an InfoData structure we care to read: */
#define MASK_INFODATA		(MASK_DISK)

/*  These have a "Last Changed" date: */
#define MASK_LASTCHANGED	(MASK_DRAWER | MASK_TOOL | MASK_PROJECT | \
				MASK_GARBAGE)

/*  These have a "Created" date: */
#define MASK_CREATEDDATE	(MASK_DISK)

/*  These have a "stack" gadget and blocks/bytes size info: */
#define MASK_SIZESTACK		(MASK_PROJECT | MASK_TOOL)

/*  These have a disk-type status display (read-only vs. read-write): */
#define MASK_DISKSTATUS		(MASK_DISK)

/*  These have protection bits: */
#define MASK_PROBITS		(MASK_DRAWER | MASK_TOOL | MASK_PROJECT | \
				MASK_GARBAGE)

/*  These have a "Comment" field: */
#define MASK_COMMENT		(MASK_DRAWER | MASK_TOOL | MASK_PROJECT)

/*  These have a "Default Tool" field: */
#define MASK_DEFAULTTOOL	(MASK_DISK | MASK_PROJECT)

/*  These have "Tool Types": */
#define MASK_TOOLTYPES		(MASK_DRAWER | MASK_TOOL | MASK_PROJECT)

/*  These have their name displayed beside their type: */
#define MASK_HASNAME		(MASK_DRAWER | MASK_TOOL | MASK_PROJECT | MASK_GARBAGE)

/*------------------------------------------------------------------------*/

#define PROBITSET	(FIBF_READABLE)
#define PROBITUSE	(FIBF_WRITABLE | FIBF_DELETABLE)

/*------------------------------------------------------------------------*/

/*  GadgetID definitions: */
#define G_STACK		1
#define G_COMMENT	2
#define G_DEFAULTTOOL	3
#define G_TTLIST	4
#define G_TTSTRING	5
#define G_TTNEW		6
#define G_TTDEL		7
#define G_SAVE		8
#define G_QUIT		9
#define G_SCRIPT	10
#define G_ARCHIVED	11
#define G_READABLE	12
#define G_WRITABLE	13
#define G_EXECUTABLE	14
#define G_DELETABLE	15

/*------------------------------------------------------------------------*/

extern struct TextAttr defaultAttr;
