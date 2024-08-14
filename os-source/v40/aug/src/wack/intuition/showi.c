
/* showi.c : wack "show intuition" functions.
 * $Id: showi.c,v 40.1 93/09/10 11:11:12 peter Exp $
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>
#include <exec/resident.h>
#include <exec/execbase.h>

#include <exec/semaphores.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <string.h>

extern struct MsgPort *WackBase;

/* amiga core address for field in sun-buffered structure */

#include "/special.h"
#include "iwack_proto.h"

#include "/protos/wack_protos.h"
#include "/pragmas/wack_pragmas.h"

#include "I:cghooks.h"
#include "I:sghooks.h"
#include "I:iprefs.h"
#include "I:sc.h"
#include "I:i.h"
#include "I:intuinternal.h"
#include "I:ifunctions.h"

#include "V39:src/kickstart/graphics/displayinfo_internal.h"

#include <intuition/classusr.h>
#include <intuition/classes.h>

extern APTR SpareAddr;

#define SEMFMT0 "Address  NestCount    Owner  Queue  Waiters ... <Name>\n"
#define SEMFMT1 "%08lx     %4ld %8lx   %4ld  "

void
semHeader( void )
{
    Wack_Printf(SEMFMT0);
}

ULONG
dumpSemReq( APTR addr, struct SemaphoreRequest *sr )
{
    Wack_Printf(" %8lx ", sr->sr_Waiter);
    return (0);
}

ULONG
dumpSem( APTR addr, struct SignalSemaphore *sem, char *name )
{
    /* dump semaphore */
    Wack_Printf(SEMFMT1, addr, sem->ss_NestCount, sem->ss_Owner, sem->ss_QueueCount);

    /* print out waiters */
    WalkMinList(CADDR(addr, sem, &sem->ss_WaitQueue),
		sizeof(struct SemaphoreRequest), dumpSemReq, FALSE );

    Wack_Printf("%s\n", name);
    return (0);
}


void
showSem( APTR addr )
{
    doNode(addr, sizeof (struct SignalSemaphore), dumpSem);
}

void
showSemList( APTR addr )
{
    WalkList(addr, sizeof (struct SignalSemaphore), dumpSem, FALSE );
    NewLine();
}


/** Screens **/

ULONG
dumpXScreen( APTR addr, struct Screen *s )
{
    struct ViewPortExtra vpextra;
    struct PubScreenNode	psn;
    char	psname[MAXPUBSCREENNAME];

    Wack_Printf("Screen %lx", addr);
    if (s->Title)
    {
	char	titlebuff[ 256 ];
	Wack_ReadBlock( s->Title, titlebuff, sizeof( titlebuff )  );
	Wack_Printf(" \"%s\"\n", titlebuff );
    }
    else
    {
	Wack_Printf(" (untitled)\n");
    }

    Wack_Printf("\tNextScreen: %8lx\tTitle: %8lx\n", s->NextScreen, s->Title);

    if ( XSC(s)->PSNode )
    {
	Wack_ReadBlock( XSC(s)->PSNode, &psn, sizeof psn );
	Wack_ReadBlock( psn.psn_Node.ln_Name, psname, sizeof psname );
	Wack_Printf("\tPSNode: %lx\tPublic name: \"%s\"\n", XSC(s)->PSNode, psname  );
    }

    Wack_Printf("\tFirstWindow: %8lx\tFlags: %lx\tPrivateFlags: %lx\n",
	s->FirstWindow, s->Flags, XSC(s)->PrivateFlags );

    Wack_Printf("\tL/T/W/H: %ld/%ld/%ld/%ld\n",
	s->LeftEdge, s->TopEdge, s->Width, s->Height);
    Wack_Printf("\tMouseX/Y: %ld/%ld\n", s->MouseX, s->MouseY);

    Wack_Printf("\tViewPort: %8lx\tVPExtra: %8lx\n",
	CADDR(addr, s, &s->ViewPort),XSC(s)->VPExtra);
    ShowAddress("\tRastPort:", CADDR(addr, s, &s->RastPort));
    Wack_Printf("\tBitMap: %8lx\tRealBitMap: %8lx\n",
	CADDR(addr, s, &s->BitMap), XSC(s)->RealBitMap );

    Wack_Printf("\tLayerInfo: %8lx\tBarLayer: %8lx\n",
	CADDR(addr, s, &s->LayerInfo), s->BarLayer );
    Wack_Printf("\tClipLayerInfo: %8lx\tClipLayer: %8lx\n",
	XSC(s)->ClipLayer_Info, XSC(s)->ClipLayer );

    {
	struct TextAttr	tattr;
	char	nameb[ 44 ];

	Wack_ReadBlock( s->Font, &tattr, sizeof tattr );
	Wack_ReadBlock( tattr.ta_Name, nameb, sizeof nameb );

	Wack_Printf("\tTextAttr: %lx (%s %ld)\n", s->Font, nameb, tattr.ta_YSize);
    }

    Wack_ReadBlock( XSC(s)->VPExtra, &vpextra, sizeof vpextra);

    dumpRect("\tScreen DisplayClip:", &XSC(s)->DClip);
    dumpRect("\tViewport DisplayClip:", &vpextra.DisplayClip);

    Wack_Printf("\tScale factors: %ld/%ld",
	XSC(s)->ScaleFactor.X, XSC(s)->ScaleFactor.Y );
    Wack_Printf("\tSprite factors: %ld/%ld\n",
	XSC(s)->SpriteFactor.X, XSC(s)->SpriteFactor.Y );
    Wack_Printf("\tNaturalDRecord: %lx, NaturalMSpec: %lx, NaturalDisplayID: %lx\n",
	XSC(s)->NaturalDRecord,
	XSC(s)->NaturalMSpec,
	XSC(s)->NaturalDisplayID );

    Wack_Printf("\tDProperties: %lx ModeStash: %lx\n", 
	XSC(s)->DProperties,
	XSC(s)->VPModeStash );

    Wack_Printf("\tColorMap: %lx\n", XSC(s)->ColorMap);

    {
	struct ColorMap colormap;
	struct DisplayInfoRecord direcord;
	ULONG normalID;
	ULONG coerceID;

	Wack_ReadBlock( XSC(s)->ColorMap, &colormap, sizeof colormap);

	Wack_ReadBlock( colormap.NormalDisplayInfo, &direcord, sizeof direcord );
	normalID = *((ULONG *)&direcord.rec_MajorKey);
	Wack_ReadBlock( colormap.CoerceDisplayInfo, &direcord, sizeof direcord );
	coerceID = *((ULONG *)&direcord.rec_MajorKey);

	Wack_Printf("\t(CM)NormalDisplayID: %lx\t(CM)CoercedDisplayID: %lx\n",
	    normalID, coerceID );
    }

    if ( XSC(s)->ParentScreen )
    {
	Wack_Printf("\tThis screen's parent: %8lx\tParentYOffset: %ld\n",
	    XSC(s)->ParentScreen, XSC(s)->ParentYOffset );
    }
    else
    {
	struct MinNode *node;
	struct MinNode *succ;
	struct Screen *child;
	long firstchild = 1;

	for ( node = XSC(s)->Family.mlh_Head;
	    succ = Wack_ReadPointer( &node->mln_Succ );
	    node = succ )
	{
	    /* One of the screens on the Family list is the
	     * screen itself, and we don't want to act on it.
	     */
	    if ( ( child = SCREENFROMNODE( node ) ) != addr )
	    {
		if ( firstchild )
		{
		    Wack_Printf("\tThis screen's children:\n");
		    firstchild = 0;
		}
		Wack_Printf("\t\t%8lx\n", child );
	    }
	}
    }

/* missing:
	SpriteFactor
	VPModeStash
	SysImages
	SDraw stuff
	*/

    return(0);
}

void ShowXScreen( char *arg )
{
    struct XScreen	*w;

    ULONG	dumpXScreen();
    if (hexArgAddr(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct XScreen), dumpXScreen);
    }
    else BadSyntax();
}

/* windows */
ULONG
dumpXWindow( APTR addr, struct Window *w )
{
    Wack_Printf("Window %lx", addr );
    if (w->Title)
    {
	char	titlebuff[ 256 ];
	Wack_ReadBlock( w->Title, titlebuff, sizeof( titlebuff ) );
	Wack_Printf(" \"%s\"\n", titlebuff );
    }
    else
    {
	Wack_Printf(" (untitled)\n");
    }

    Wack_Printf("\tNextWindow: %8lx\tTitle: %8lx\n",
	w->NextWindow, w->Title);

    Wack_Printf("\tWScreen: %8lx\tRPort: %8lx\tFirstGadget: %8lx\n",
	w->WScreen, w->RPort, w->FirstGadget);
    Wack_Printf("\tL/T/W/H: %ld/%ld/%ld/%ld\n", w->LeftEdge, w->TopEdge,
	w->Width, w->Height);
    Wack_Printf("\tMouse X/Y: %ld/%ld\n", w->MouseX, w->MouseY);
    Wack_Printf("\tMin W/H: %ld/%ld\tMax W/H: %ld/%ld\n",
	w->MinWidth, w->MinHeight, w->MaxWidth, w->MaxHeight);
    Wack_Printf("\tFlags: %lx MoreFlags: %lx\n", w->Flags, w->MoreFlags);
    Wack_Printf("\tIDCMPFlags: %lx\tIDCMP user port: %8lx\n", w->IDCMPFlags, w->UserPort );
    Wack_Printf("\tWLayer: %8lx\n", w->WLayer);
    ShowAddress("\tMenuStrip:", w->MenuStrip);

    Wack_Printf("\tFirstRequest: %lx\tDMRequest: %lx\tReqCount: %ld\n",
	w->FirstRequest, w->DMRequest, w->ReqCount);
    Wack_Printf("\tParent: %lx\tDescendant: %lx\n", w->Parent, w->Descendant);

    Wack_Printf("\tBorder l/t/r/b: %ld/%ld/%ld/%ld\tBorderRPort: %lx\n",
    w->BorderLeft, w->BorderTop, w->BorderRight, w->BorderBottom,
	w->BorderRPort);
    Wack_Printf("\tMouseQueueLimit: %ld\tMousePending: %ld\n",
	XWINDOW(w)->MouseQueueLimit, XWINDOW(w)->MousePending );
    Wack_Printf("\tRptQueueLimit: %ld\tRptPending: %ld\n",
	XWINDOW(w)->RptQueueLimit, XWINDOW(w)->RptPending );
    Wack_Printf("\tPointer: %08lx\t Width: %ld Height:%ld XOffset: %ld YOffset: %ld\n",
	w->Pointer, w->PtrHeight, w->PtrWidth, w->XOffset, w->YOffset );
    Wack_Printf("\t&window->Pointer: %08lx\n", &(((struct Window*)addr)->Pointer) );

    return (0);
}

void ShowXWindow( char *arg )
{
    struct XWindow	*w;

    ULONG	dumpXWindow();
    if (hexArgAddr(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct XWindow), dumpXWindow);
    }
    else BadSyntax();
}


void ShowWindowFlags( char *arg )
{
    struct Window	*w;

    ULONG	dumpWindowFlags();
    if (hexArgAddr(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct Window), dumpWindowFlags);
    }
    else
    {
	BadSyntax();
    }
}

char *WFlagLabel[] =
{
    "WFLG_SIZEGADGET",
    "WFLG_DRAGBAR",
    "WFLG_DEPTHGADGET",
    "WFLG_CLOSEGADGET",
    "WFLG_SIZEBRIGHT",
    "WFLG_SIZEBBOTTOM",
    "WFLG_SIMPLE_REFRESH",
    "WFLG_SUPER_BITMAP",
    "WFLG_BACKDROP",
    "WFLG_REPORTMOUSE",
    "WFLG_GIMMEZEROZERO",
    "WFLG_BORDERLESS",
    "WFLG_ACTIVATE",
    "WFLG_RMBTRAP",
    "WFLG_NOCAREREFRESH",
    "WFLG_NW_EXTENDED",
    "WFLG_NEWLOOKMENUS",
    "WFLG_WINDOWACTIVE",
    "WFLG_INREQUEST",
    "WFLG_MENUSTATE",
    "WFLG_WINDOWREFRESH",
    "WFLG_WBENCHWINDOW",
    "WFLG_WINDOWTICKED",
    "WFLG_VISITOR",
    "WFLG_ZOOMED",
    "WFLG_HASZOOM",
};

ULONG WFlagMask[] =
{
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000080,
    0x00000100,
    0x00000200,
    0x00000400,
    0x00000800,
    0x00001000,
    0x00010000,
    0x00020000,
    0x00040000,
    0x00200000,
    0x00002000,
    0x00004000,
    0x00008000,
    0x01000000,
    0x02000000,
    0x04000000,
    0x08000000,
    0x10000000,
    0x20000000,
    0,
};


char *WIDCMPLabel[] =
{
    "IDCMP_SIZEVERIFY",
    "IDCMP_NEWSIZE",
    "IDCMP_REFRESHWINDOW",
    "IDCMP_MOUSEBUTTONS",
    "IDCMP_MOUSEMOVE",
    "IDCMP_GADGETDOWN",
    "IDCMP_GADGETUP",
    "IDCMP_REQSET",
    "IDCMP_MENUPICK",
    "IDCMP_CLOSEWINDOW",
    "IDCMP_RAWKEY",
    "IDCMP_REQVERIFY",
    "IDCMP_REQCLEAR",
    "IDCMP_MENUVERIFY",
    "IDCMP_NEWPREFS",
    "IDCMP_DISKINSERTED",
    "IDCMP_DISKREMOVED",
    "IDCMP_WBENCHMESSAGE",
    "IDCMP_ACTIVEWINDOW",
    "IDCMP_INACTIVEWINDOW",
    "IDCMP_DELTAMOVE",
    "IDCMP_VANILLAKEY",
    "IDCMP_INTUITICKS",
    "IDCMP_IDCMPUPDATE",
    "IDCMP_MENUHELP",
    "IDCMP_CHANGEWINDOW",
    "IDCMP_GADGETHELP",
    "IDCMP_LONELYMESSAGE",
};

ULONG WIDCMPMask[] =
{
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000080,
    0x00000100,
    0x00000200,
    0x00000400,
    0x00000800,
    0x00001000,
    0x00002000,
    0x00004000,
    0x00008000,
    0x00010000,
    0x00020000,
    0x00040000,
    0x00080000,
    0x00100000,
    0x00200000,
    0x00400000,
    0x00800000,
    0x01000000,
    0x02000000,
    0x04000000,
    0x80000000,
    0,
};

char *WMoreFlagLabel[] =
{
    "WMF_NEEDMENUCLEAR",
    "WMF_USEMENUHELP",
    "WMF_NOTIFYDEPTH",
    "WMF_TABLETMESSAGES",
    "WMF_GADGETHELP",
    "WMF_DEFERREDPOINTER",
    "WMF_BEGINUPDATE",
};

ULONG WMoreFlagMask [] =
{
    0x00000001,
    0x00000002,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000100,
    0,
};

ULONG
dumpWindowFlags( APTR addr, struct Window *w )
{
    int printed = 0;
    int i;

    Wack_Printf( "Window %lx\n", addr );
    Wack_Printf( "Window->Flags:\n");
    for ( i = 0; WFlagMask[ i ]; i++ )
    {
	if ( w->Flags & WFlagMask[ i ] )
	{
	    if ( !printed )
	    {
		Wack_Printf("    ");
	    }
	    Wack_Printf("%s ", WFlagLabel[ i ] );
	    if ( printed++ == 3 )
	    {
		NewLine();
		printed = 0;
	    }
	}
    }
    if ( printed )
	NewLine();

    Wack_Printf( "Window->IDCMPFlags:\n");
    printed = 0;
    for ( i = 0; WIDCMPMask[ i ]; i++ )
    {
	if ( w->IDCMPFlags & WIDCMPMask[ i ] )
	{
	    if ( !printed )
	    {
		Wack_Printf("    ");
	    }
	    Wack_Printf("%s ", WIDCMPLabel[ i ] );
	    if ( printed++ == 3 )
	    {
		NewLine();
		printed = 0;
	    }
	}
    }
    if ( printed )
	NewLine();


    Wack_Printf( "Window->MoreFlags:\n");
    printed = 0;
    for ( i = 0; WMoreFlagMask[ i ]; i++ )
    {
	if ( w->MoreFlags & WMoreFlagMask[ i ] )
	{
	    if ( !printed )
	    {
		Wack_Printf("    ");
	    }
	    Wack_Printf("%s ", WMoreFlagLabel[ i ] );
	    if ( printed++ == 3 )
	    {
		NewLine();
		printed = 0;
	    }
	}
    }
    if ( printed )
	NewLine();

    return( 0 );
}

void
dumpBox( UBYTE *s, struct IBox *b )
{
    Wack_Printf("[%s]\t", s);
    Wack_Printf("\tl/t: %ld, %ld; w/h: %ld, %ld\n",
	b->Left, b->Top, b->Width, b->Height);
}


void
ShowISems( void )
{
    int i;
    struct SignalSemaphore *sem;
    struct IntuitionBase *ibase;

    if (!(ibase = (struct IntuitionBase *) Wack_FindLibrary("intuition.library")))
    {
	Wack_Printf("Couldn't find intuition.library.\n");
	return;
    }

    semHeader();

    sem = &ibase->ISemaphore[0];

    for ( i = 0; i < NUMILOCKS; i++ )
    {
	showSem( sem );
	sem++;
    }
}

struct Thing {struct Thing *NextThing;};

ULONG
dumpThing( APTR addr, struct Thing *p )
{
    if (addr == Wack_ReadCurrAddr())
    {
	Wack_Printf("* ");
    }
    else
    {
	Wack_Printf("  ");
    }
    ShowAddress("Thing", addr);

    return (0);
}

void ShowThing( char *arg )
{
    struct Thing	*w;

    ULONG	dumpThing();
    if (hexArgAddr(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct Thing), dumpThing);
    }
    else BadSyntax();
}

void ShowThingList( char *arg )
{
    struct Thing	*w;

    ULONG	dumpThing();
    if (hexArgAddr(arg, (ULONG *)&w))
    {
	WalkSimpleList( w, sizeof (struct Thing), dumpThing, TRUE );
    }
    else BadSyntax();
}


ULONG
dumpIClass( APTR addr, struct IClass *cl )
{
#define CLASSNAMESIZE	100
    char name[ CLASSNAMESIZE ];
    GetName( cl->cl_ID, name );
    Wack_Printf("%lx (%s)\n", addr, name );

    strcpy( name, "---" );
    if ( cl->cl_Super )
    {
	GetName( Wack_ReadPointer( &cl->cl_Super->cl_ID ), name );
    }
    Wack_Printf("Subclass of %lx (%s)\n", cl->cl_Super, name );
    Wack_Printf("Dispatcher Hook h_Entry %lx\n", cl->cl_Dispatcher.h_Entry );
    Wack_Printf("Instance Data offset %lx, size %lx\n", cl->cl_InstOffset, cl->cl_InstSize );
    Wack_Printf("SubClass Count %ld, Object Count %ld\n", cl->cl_SubclassCount, cl->cl_ObjectCount );
    return( 0 );
}

void
ShowIClasses( void )
{
    struct IntuitionBase *ibase;

    if (!(ibase = (struct IntuitionBase *) Wack_FindLibrary("intuition.library")))
    {
	Wack_Printf("Couldn't find intuition.library.\n");
	return;
    }

    WalkMinList( (struct MinList *)&ibase->IClassList, sizeof( struct IClass ), dumpIClass, TRUE );
}
