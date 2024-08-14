/*
 * $Id: global.h,v 38.5 93/02/15 14:53:05 mks Exp $
 *
 * $Log:	global.h,v $
 * Revision 38.5  93/02/15  14:53:05  mks
 * Added the global strings needed
 * 
 * Revision 38.4  92/05/30  15:54:09  mks
 * Changed some enternals
 *
 * Revision 38.3  92/05/14  17:13:25  mks
 * Removed #define of index() to SAS function
 *
 * Revision 38.2  92/04/16  09:18:28  mks
 * Removed the global copyright string pointers
 *
 * Revision 38.1  91/06/24  11:35:46  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "dos.h"
#include "string.h"

extern char OutputWindow1[];
extern char OutputWindow2[];
extern char KickstartName[];
extern char VersionFormat[];
extern char CopyrightFormat[];
extern char InformationString[];
extern char ScreenTitle[];
extern char WorkbenchName[];
extern char Level0Prefix[];
extern char DiskIconName[];
extern char InfoSuf[];
extern char TopazName[];
extern char WB_FontPref[];
extern char SYS_FontPref[];
extern char CopyTaskName[];
extern char NumbersString[];
extern char MM_String[];
extern char M_String[];

extern char SystemDiskCopyName[];
extern char SystemFormatName[];
extern char SystemWorkbenchName[];

#define printf kprintf

/* some funky support functions -- for list management */
#define LISTEMPTY( lh )		((lh) == (lh)->lh_TailPred)
#define LISTNOTEMPTY( lh )	((lh) != (lh)->lh_TailPred)

#define SiblingToObj( n )	((struct WBObject *)((struct Node *)n-1))
#define SelNodeToObj( n )	((struct WBObject *)((struct Node *)n-2))

/* bit masks for the corresponding workbench object */
#define WBF_DISK	0x002
#define WBF_DRAWER	0x004
#define WBF_TOOL	0x008
#define WBF_PROJECT	0x010
#define WBF_GARBAGE	0x020
#define WBF_DEVICE	0x040
#define WBF_KICK	0x080
#define	WBF_APPICON	0x100

#define ISDRAWER	(WBF_DISK | WBF_DRAWER | WBF_GARBAGE)
#define ISDISKLIKE	(WBF_DISK | WBF_KICK | WBF_DEVICE)
#define	ISTRASHLIKE	(WBF_GARBAGE)
#define WINDOWCHANGE	(WBF_DRAWER | WBF_TOOL | WBF_PROJECT)

/* does the object support "drawer" operations? */
#define Drawer_P(obj)	CheckForType(obj,ISDRAWER)

#define DISKGAUGEWIDTH	12

#define	PUTAWAY_NORMAL	0
#define	PUTAWAY_QUICK	1
#define	PUTAWAY_NODEL	2

struct WindowBox
{
	SHORT	LeftEdge;
	SHORT	TopEdge;
	SHORT	Width;
	SHORT	Height;
};

#define	COPY_WBOX(t,f)	*((struct WindowBox *)(t))=*((struct WindowBox *)(f))

#include <intuition/intuition.h>
extern struct TextAttr defaultAttr;
