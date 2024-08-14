/*
 * Amiga Grand Wack
 *
 * showi.c -- Show intuition.library structures.
 *
 * $Id: showi.c,v 39.6 93/08/19 16:34:36 peter Exp $
 *
 * This module contains code to display intuition.library structures on
 * the target Amiga, including screens, windows, gadgets, menus, requesters,
 * classes, and objects.
 *
 */

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "exec/semaphores.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

#include "sys_proto.h"
#include "parse_proto.h"
#include "show_proto.h"
#include "showlock_proto.h"
#include "link_proto.h"
#include "special_proto.h"
#include "showi_proto.h"
#include "ops_proto.h"
#include "io_proto.h"

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/cghooks.h>
#include <intuition/sghooks.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>

extern APTR CurrAddr;
extern APTR SpareAddr;

/** Screens **/

ULONG
dumpScreen( APTR addr, struct Screen *s )
{
    PPrintf("Screen %lx", addr);
    if (s->Title)
    {
	char	titlebuff[ 256 ];
	ReadBlock( s->Title, titlebuff, sizeof( titlebuff )  );
	PPrintf(" \"%s\"\n", titlebuff );
    }
    else
    {
	PPrintf(" (untitled)\n");
    }

    PPrintf("\tNextScreen: %8lx\tTitle: %8lx\n", s->NextScreen, s->Title);

    PPrintf("\tFirstWindow: %8lx\tFlags: %lx\n",
	s->FirstWindow, s->Flags );

    PPrintf("\tL/T/W/H: %ld/%ld/%ld/%ld\n",
	s->LeftEdge, s->TopEdge, s->Width, s->Height);
    PPrintf("\tMouseX/Y: %ld/%ld\n", s->MouseX, s->MouseY);

    PPrintf("\tViewPort: %8lx\n",
	CADDR(addr, s, &s->ViewPort));
    ShowAddress("\tRastPort:", CADDR(addr, s, &s->RastPort));
    PPrintf("\tBitMap: %8lx\n",
	CADDR(addr, s, &s->BitMap) );

    PPrintf("\tLayerInfo: %8lx\tBarLayer: %8lx\n",
	CADDR(addr, s, &s->LayerInfo), s->BarLayer );

    {
	struct TextAttr	tattr;
	char	nameb[ 44 ];

	ReadBlock( s->Font, &tattr, sizeof tattr );
	ReadBlock( tattr.ta_Name, nameb, sizeof nameb );

	PPrintf("\tTextAttr: %lx (%s %ld)\n", s->Font, nameb, tattr.ta_YSize);
    }

    return(0);
}

SHOWONE(Screen)

ULONG
dumpExtGadget( APTR addr, struct Gadget *g )
{
    int lf;

    PPrintf("Gadget %lx, ID: %lx", addr, g->GadgetID);
    if ( g->GadgetText )
    {
	struct IntuiText gtext;
	ReadBlock( g->GadgetText, &gtext, sizeof( gtext ) );
	if ( gtext.IText )
	{
	    char	labelbuff[ 256 ];
	    ReadBlock( gtext.IText, labelbuff, sizeof( labelbuff ) );
	    PPrintf(", \"%s\"", labelbuff );
	}
    }
    NewLine();
    ShowAddress("NextGadget", g->NextGadget);
    PPrintf("L/T/W/H:\t\t%ld/%ld/%ld/%ld\n",
	g->LeftEdge, g->TopEdge, g->Width, g->Height);
    PPrintf("Flags %lx Activation %lx GadgetType %lx\n",
	g->Flags, g->Activation, g->GadgetType);

    switch ( g->GadgetType & GTYP_SYSTYPEMASK )
    {
    case GTYP_SIZING:
	PPrintf("Window-Sizing ");
	break;

    case GTYP_WDRAGGING:
	PPrintf("Window-Drag ");
	break;

    case GTYP_SDRAGGING:
	PPrintf("Screen-Drag ");
	break;

    case GTYP_WUPFRONT:
	PPrintf("Window-Depth ");
	break;

    case GTYP_SUPFRONT:
	PPrintf("Screen-Depth ");
	break;

    case GTYP_WDOWNBACK:
	PPrintf("Window-Zoom ");
	break;

    case GTYP_CLOSE:
	PPrintf("Window-Close ");
	break;

    case 0:
	break;

    default:
	PPrintf("Unknown-SysType ");
	break;
    }

    switch (g->GadgetType & GTYP_GTYPEMASK )
    {
	case GTYP_BOOLGADGET:
	    PPrintf("Boolean ");
	    break;

	case GTYP_PROPGADGET:
	    PPrintf("Prop ");
	    break;

	case GTYP_STRGADGET:
	    PPrintf("String ");
	    break;

	case GTYP_CUSTOMGADGET:
	    PPrintf("Custom ");
	    break;

	default:
	    PPrintf("Unknown-Type ");
    }

    if ( g->GadgetType & GTYP_SYSGADGET )
	PPrintf("System-Alloc ");
    if ( g->GadgetType & GTYP_SCRGADGET )
	PPrintf("Screen ");
    if ( g->GadgetType & GTYP_GZZGADGET )
	PPrintf("GZZ ");
    if ( g->GadgetType & GTYP_REQGADGET )
	PPrintf("Requester ");
    PPrintf("Gadget\n");


    PPrintf("Activation:");
    if ( g->Activation & GACT_TOGGLESELECT )
	PPrintf(" Toggle");
    if ( g->Activation & GACT_IMMEDIATE )
	PPrintf(" Immediate");
    if ( g->Activation & GACT_FOLLOWMOUSE )
	PPrintf(" FollowMouse");
    if ( g->Activation & GACT_RELVERIFY )
	PPrintf(" RelVerify");
    if ( g->Activation & GACT_ENDGADGET )
	PPrintf(" EndGadget");

    if ( g->Activation & ( GACT_RIGHTBORDER|GACT_LEFTBORDER|GACT_TOPBORDER|GACT_BOTTOMBORDER|GACT_BORDERSNIFF ) )
    {
	PPrintf(", Border:");
	if ( g->Activation & GACT_RIGHTBORDER )
	    PPrintf(" Right");
	if ( g->Activation & GACT_LEFTBORDER )
	    PPrintf(" Left");
	if ( g->Activation & GACT_TOPBORDER )
	    PPrintf(" Top");
	if ( g->Activation & GACT_BOTTOMBORDER )
	    PPrintf(" Bottom");
	if ( ( g->Activation & ( GACT_RIGHTBORDER|GACT_LEFTBORDER|GACT_TOPBORDER|GACT_BOTTOMBORDER|GACT_BORDERSNIFF ) )
	 == GACT_BORDERSNIFF )
	    PPrintf(" (sniffed)");
    }
    NewLine();

    PPrintf("Highlight: ");
    switch ( g->Flags & GFLG_GADGHIGHBITS )
    {
    case GFLG_GADGHCOMP:
	PPrintf("Complement");
	break;

    case GFLG_GADGHBOX:
	PPrintf("Box");
	break;

    case GFLG_GADGHIMAGE:
	PPrintf("Alternate");
	break;

    case GFLG_GADGHNONE:
	PPrintf("None");
	break;
    }
    if ( g->Flags & GFLG_GADGIMAGE )
	PPrintf(" (image)");

    if ( g->Flags & GFLG_SELECTED )
	PPrintf(", Selected");

    if ( g->Flags & GFLG_DISABLED )
	PPrintf(", Disabled");

    if ( g->Flags & ( GFLG_RELBOTTOM|GFLG_RELRIGHT|GFLG_RELWIDTH|GFLG_RELHEIGHT|GFLG_RELSPECIAL ) )
    {
	PPrintf(", ");
	if ( g->Flags & GFLG_RELBOTTOM )
	    PPrintf("Bottom ");
	if ( g->Flags & GFLG_RELRIGHT )
	    PPrintf("Right ");
	if ( g->Flags & GFLG_RELWIDTH )
	    PPrintf("Width ");
	if ( g->Flags & GFLG_RELHEIGHT )
	    PPrintf("Height ");
	if ( g->Flags & GFLG_RELSPECIAL )
	    PPrintf("Special ");
	PPrintf("relative");
    }
    NewLine();

    lf = 0;
    if ( g->Flags & GFLG_TABCYCLE )
    {
	PPrintf("TabCycle ");
	lf = 1;
    }
    if ( g->Flags & GFLG_STRINGEXTEND )
    {
	PPrintf("StringExtend(flag) ");
	lf = 1;
    }
    if ( g->Flags & GFLG_IMAGEDISABLE )
    {
	PPrintf("ImageDisable ");
	lf = 1;
    }
    if ( g->Flags & GFLG_EXTENDED )
    {
	PPrintf("Extended ");
	if ( ((struct ExtGadget *)g)->MoreFlags & GMORE_BOUNDS )
	    PPrintf("GBounds ");
	if ( ((struct ExtGadget *)g)->MoreFlags & GMORE_GADGETHELP )
	    PPrintf("GHelp ");
	if ( ((struct ExtGadget *)g)->MoreFlags & GMORE_SCROLLRASTER )
	    PPrintf("ScrollRaster ");

	lf = 1;
    }
    if ( lf )
    {
	NewLine();
    }

    lf = 0;
    if ( g->Activation & GACT_ACTIVEGADGET )
    {
	PPrintf("ActiveGadget ");
	lf = 1;
    }
    if ( g->Activation & GACT_STRINGCENTER )
    {
	PPrintf("StringCenter ");
	lf = 1;
    }
    if ( g->Activation & GACT_STRINGRIGHT )
    {
	PPrintf("StringRight ");
	lf = 1;
    }
    if ( g->Activation & GACT_LONGINT )
    {
	PPrintf("LongInt ");
	lf = 1;
    }
    if ( g->Activation & GACT_ALTKEYMAP )
    {
	PPrintf("AltKeyMap ");
	lf = 1;
    }
    if ( g->Activation & GACT_STRINGEXTEND )
    {
	PPrintf("(String|Bool)Extend ");
	lf = 1;
    }
    if ( lf )
    {
	NewLine();
    }

    PPrintf("GadgetRender %lx, SelectRender %lx\n",
	g->GadgetRender, g->SelectRender);
    ShowAddress("SpecialInfo", g->SpecialInfo);
    ShowAddress("UserData", g->UserData);

    return (0);
}

SHOWSIMPLE(ExtGadget,TRUE)

/* menu items */
#define NAMESIZE 100

ULONG
dumpMenuItem( APTR addr, struct MenuItem *i )
{
    PPrintf("MenuItem at\t%8lx", addr);
    if ( ( i->ItemFill ) && ( i->Flags & ITEMTEXT ) )
    {
	struct IntuiText itext;	
	char   name[NAMESIZE];
	ReadBlock( i->ItemFill, &itext, sizeof( struct IntuiText ) );
	if ( itext.IText )
	{
	    GetName( itext.IText, name );
	    PPrintf( "\t\"%s\"\n", name );
	}
    }
    else
    {
	PPrintf("\t(graphical item)\n");
    }

    ShowAddress("NextItem", i->NextItem);
    PPrintf("L/T/W/H:\t\t%ld/%ld/%ld/%ld\n", i->LeftEdge, i->TopEdge, i->Width, i->Height);
    PPrintf("Flags %lx, MutualExclude %lx\n", i->Flags, i->MutualExclude);
    ShowAddress("ItemFill", i->ItemFill);
    ShowAddress("SelectFill", i->SelectFill);
    PPrintf("Command: %lx, NextSelect %lx\n", i->Command, i->NextSelect);
    ShowAddress("SubItem", i->SubItem);
    NewLine();

    return (0);
}

SHOWSIMPLE(MenuItem,TRUE)

/* menus */
ULONG
dumpMenu( APTR addr, struct Menu *m )
{
    char   name[NAMESIZE];

    GetName (m->MenuName, name);
    PPrintf("Menu at\t\t%8lx\t\"%s\"\n", addr, name);
    PPrintf("NextMenu at\t%8lx\t\n", m->NextMenu);
    PPrintf("L/T/W/H:\t\t%ld/%ld/%ld/%ld\n", m->LeftEdge, m->TopEdge, m->Width, m->Height);
    PPrintf("Flags\t\t%lx\n", m->Flags);
    ShowAddress("MenuName", m->MenuName);
    ShowAddress("FirstItem", m->FirstItem);
    PPrintf("JazzX/Y BeatX/Y %ld/%ld  %ld/%ld\n",m->JazzX,m->JazzY,m->BeatX,m->BeatY);

    return (0);
}

SHOWSIMPLE(Menu,TRUE)

/* windows */
ULONG
dumpWindow( APTR addr, struct Window *w )
{
    PPrintf("Window %lx", addr );
    if (w->Title)
    {
	char	titlebuff[ 256 ];
	ReadBlock( w->Title, titlebuff, sizeof( titlebuff ) );
	PPrintf(" \"%s\"\n", titlebuff );
    }
    else
    {
	PPrintf(" (untitled)\n");
    }

    PPrintf("\tNextWindow: %8lx\tTitle: %8lx\n",
	w->NextWindow, w->Title);

    PPrintf("\tWScreen: %8lx\tRPort: %8lx\tFirstGadget: %8lx\n",
	w->WScreen, w->RPort, w->FirstGadget);
    PPrintf("\tL/T/W/H: %ld/%ld/%ld/%ld\n", w->LeftEdge, w->TopEdge,
	w->Width, w->Height);
    PPrintf("\tMouse X/Y: %ld/%ld\n", w->MouseX, w->MouseY);
    PPrintf("\tMin W/H: %ld/%ld\tMax W/H: %ld/%ld\n",
	w->MinWidth, w->MinHeight, w->MaxWidth, w->MaxHeight);
    PPrintf("\tFlags: %lx\n", w->Flags);
    PPrintf("\tIDCMPFlags: %lx\tIDCMP user port: %8lx\n", w->IDCMPFlags, w->UserPort );
    PPrintf("\tWLayer: %8lx\n", w->WLayer);
    ShowAddress("\tMenuStrip:", w->MenuStrip);

    PPrintf("\tFirstRequest: %lx\tDMRequest: %lx\tReqCount: %ld\n",
	w->FirstRequest, w->DMRequest, w->ReqCount);
    PPrintf("\tParent: %lx\tDescendant: %lx\n", w->Parent, w->Descendant);

    PPrintf("\tBorder l/t/r/b: %ld/%ld/%ld/%ld\tBorderRPort: %lx\n",
    w->BorderLeft, w->BorderTop, w->BorderRight, w->BorderBottom,
	w->BorderRPort);
    PPrintf("\tPointer: %08lx\t Width: %ld Height:%ld XOffset: %ld YOffset: %ld\n",
	w->Pointer, w->PtrHeight, w->PtrWidth, w->XOffset, w->YOffset );
    PPrintf("\t&window->Pointer: %08lx\n", &(((struct Window*)addr)->Pointer) );

    return (0);
}

SHOWONE(Window)


STRPTR
ShowWindowFlags( char *arg )
{
    struct Window	*w;

    ULONG	dumpWindowFlags();
    if (parseAddress(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct Window), dumpWindowFlags);
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
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

ULONG
dumpWindowFlags( APTR addr, struct Window *w )
{
    int printed = 0;
    int i;

    PPrintf( "Window %lx\n", addr );
    PPrintf( "Window->Flags:\n");
    for ( i = 0; WFlagMask[ i ]; i++ )
    {
	if ( w->Flags & WFlagMask[ i ] )
	{
	    if ( !printed )
	    {
		PPrintf("    ");
	    }
	    PPrintf("%s ", WFlagLabel[ i ] );
	    if ( printed++ == 3 )
	    {
		NewLine();
		printed = 0;
	    }
	}
    }
    if ( printed )
    {
	NewLine();
    }

    PPrintf( "Window->IDCMPFlags:\n");
    printed = 0;
    for ( i = 0; WIDCMPMask[ i ]; i++ )
    {
	if ( w->IDCMPFlags & WIDCMPMask[ i ] )
	{
	    if ( !printed )
	    {
		PPrintf("    ");
	    }
	    PPrintf("%s ", WIDCMPLabel[ i ] );
	    if ( printed++ == 3 )
	    {
		NewLine();
		printed = 0;
	    }
	}
    }
    if ( printed )
    {
	NewLine();
    }

    return( 0 );
}


ULONG
dumpRequester( APTR addr, struct Requester *r )
{
    ShowAddress("Requester", addr);
    ShowAddress ("OlderRequest", r->OlderRequest);
    ShowAddress ("ReqLayer", r->ReqLayer);
    ShowAddress ("RWindow", r->RWindow);
    ShowAddress("ReqGadget", r->ReqGadget);
    PPrintf("Flags %lx\n", r->Flags);
    dumpBox ("reqbox", (struct IBox *)&r->LeftEdge );
    PPrintf("rel left/top %ld %ld\n", r->RelLeft, r->RelTop);

    return (0);
}


SHOWONE(Requester)

void
dumpBox( UBYTE *s, struct IBox *b )
{
    PPrintf("[%s]\t", s);
    PPrintf("\tl/t: %ld, %ld; w/h: %ld, %ld\n",
	b->Left, b->Top, b->Width, b->Height);
}


STRPTR
ShowLayerInfoSems( void )
{
    struct IntuitionBase *ibase;
    struct Screen *screen;

    if (!(ibase = (struct IntuitionBase *) FindBase("intuition.library")))
    {
	PPrintf("Couldn't find intuition.library.\n");
    }
    else
    {
	for (screen = ReadPointer(&ibase->FirstScreen); screen; screen = ReadPointer(&screen->NextScreen))
	{
	    PPrintf ("Screen: %lx, LayerInfo: %lx\n", screen, &screen->LayerInfo);
	    PPrintf ("Locks: (Layer_Info.Lock first)\n");
	    semHeader();
	    ShowSemNoHeader(&screen->LayerInfo.Lock);
	    ShowSemListNoHeader(&screen->LayerInfo.gs_Head);
	}
    }

    return( NULL );
}




ULONG
dumpPropInfo( APTR addr, struct PropInfo *pi )
{

    ShowAddress("PropInfo", addr);
    PPrintf("\tflags %lx, pots %lx/%lx\n", pi->Flags, pi->HorizPot, pi->VertPot);
    PPrintf("\tbodies: %lx/%lx\n", pi->HorizBody, pi->VertBody);
    PPrintf("\tCWidth/Height %ld/%ld, H/VPotRes %ld/%ld\n", 
	pi->CWidth, pi->CHeight, pi->HPotRes, pi->VPotRes);

    PPrintf("\tL/T Borders %ld/%ld\n", pi->LeftBorder, pi->TopBorder);

    return (1);	/* make it stop */
}

SHOWONE(PropInfo)

ULONG
dumpImage( APTR addr, struct Image *im )
{
    ShowAddress("Image", addr);
    PPrintf("\tL/T/W/H %ld/%ld/%ld/%ld\n",
	im->LeftEdge, im->TopEdge, im->Width, im->Height);
    PPrintf("\tDepth %ld, ImageData %lx\n", im->Depth, im->ImageData);
    PPrintf("\tPlanePick/OnOff %ld - %ld\n", im->PlanePick, im->PlaneOnOff);

    return (1);
}

SHOWONE(Image)


ULONG
dumpIntuiText( APTR addr, struct IntuiText *it )
{
    ShowAddress("IntuiText", addr);
    if (it->IText)
    {
	char	textbuff[ 256 ];
	ReadBlock( it->IText, textbuff, sizeof( textbuff ) );
	PPrintf("\tText: \"%s\"\n", textbuff );
    }
    PPrintf("\tL/T %ld/%ld\n",
	it->LeftEdge, it->TopEdge);
    PPrintf("\tFrontPen %ld, BackPen %ld, DrawMode %ld\n", it->FrontPen,
	it->BackPen, it->DrawMode);
    PPrintf("\tITextFont at %lx\n", it->ITextFont);
    PPrintf("\tNextText at %lx\n", it->NextText);

    return (1);
}

SHOWONE(IntuiText)

void
dumpRect( UBYTE *s, struct Rectangle *r )
{
    PPrintf("%s\t", s);
    PPrintf("\tX: [%ld/%ld]\tY: [%ld/%ld]\n", r->MinX, r->MaxX, r->MinY, r->MaxY);
}

STRPTR
ShowAScreen( void )
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  FindBase ("intuition.library"))
    {
	ReadBlock(ibase, &IBase, sizeof IBase);
	SpareAddr = (APTR) IBase.ActiveScreen;
	PPrintf ("Active Screen at %08lx\n", IBase.ActiveScreen );
    }
    else
    {
	PPrintf("Couldn't find intuition.library.\n");
    }

    return( NULL );
}

STRPTR
ShowAWindow( void )
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  FindBase ("intuition.library"))
    {
	ReadBlock(ibase, &IBase, sizeof IBase);
	SpareAddr = (APTR) IBase.ActiveWindow;
	PPrintf ("Active Window at %08lx\n", IBase.ActiveWindow );
    }
    else
    {
	PPrintf("Couldn't find intuition.library.\n");
    }

    return( NULL );
}

STRPTR
ShowFirstGad( char *arg )
{
    struct Window *w;
    struct Gadget *firstgad;

    if (parseAddress(arg, (ULONG *)&w))
    {
	firstgad = ReadPointer( &w->FirstGadget );
	SpareAddr = (APTR) firstgad;
	PPrintf( "First Gadget of window at %08lx is at %08lx\n", w, firstgad );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

ULONG
dumpIClass( APTR addr, struct IClass *cl )
{
#define CLASSNAMESIZE	100
    char name[ CLASSNAMESIZE ];
    GetName( cl->cl_ID, name );
    PPrintf("%lx (%s)\n", addr, name );

    strcpy( name, "---" );
    if ( cl->cl_Super )
    {
	GetName( ReadPointer( &cl->cl_Super->cl_ID ), name );
    }
    PPrintf("Subclass of %lx (%s)\n", cl->cl_Super, name );
    PPrintf("Dispatcher Hook h_Entry %lx\n", cl->cl_Dispatcher.h_Entry );
    PPrintf("Instance Data offset %lx, size %lx\n", cl->cl_InstOffset, cl->cl_InstSize );
    PPrintf("SubClass Count %ld, Object Count %ld\n", cl->cl_SubclassCount, cl->cl_ObjectCount );
    return( 0 );
}

SHOWONE(IClass)

STRPTR
ShowObject( char *arg )
{
#define CLASSNAMESIZE	100
    char name[ CLASSNAMESIZE ];
    struct _Object *o;
    struct _Object obj;
    struct IClass *cl;

    if (parseAddress(arg, (ULONG *)&o))
    {
	ReadBlock( _OBJECT(o), &obj, sizeof( struct _Object ) ); 
	cl = obj.o_Class;

	PPrintf("Object %lx of class ", o );

	while ( cl )
	{
	    UWORD idoffset, idsize;

	    GetName( ReadPointer( &cl->cl_ID ), name );
	    PPrintf("%lx (%s)", cl, name );
	    idoffset = ReadWord( &cl->cl_InstOffset );
	    idsize = ReadWord( &cl->cl_InstSize );
	    if ( idsize )
	    {
		PPrintf(", IData at %lx, size %lx\n", ((ULONG)o)+idoffset, idsize );
	    }
	    else
	    {
		PPrintf(", no IData\n");
	    }
	    cl = ReadPointer( &cl->cl_Super );
	    if ( cl )
	    {
		PPrintf("  Subclass of ");
	    }
	}
	PPrintf("Object Succ %lx, Pred %lx\n",obj.o_Node.mln_Succ, obj.o_Node.mln_Pred );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

STRPTR
ShowIBase( void )
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  FindBase("intuition.library"))
    {
	SpareAddr = (APTR) ibase;
	ReadBlock(ibase, &IBase, sizeof IBase);

	/*** Public Information ***/

	PPrintf( "IntuitionBase at %08lx\n", ibase);
	PPrintf( "ViewLord: %lx\n", IBADDR( &IBase.ViewLord ));
	PPrintf( "Active Window: %lx AScreen: %lx, FirstScreen: %lx\n",
	     IBase.ActiveWindow, IBase.ActiveScreen, IBase.FirstScreen);
    }
    else
    {
	PPrintf("Couldn't find intuition.library.\n");
    }

    return( NULL );
}
