/*
 * pointerdemo.c - shows off new pointer features
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * This example shows off some of the V39 pointer features available
 * through the boopsi pointerclass and through the new SetWindowPointer()
 * call.  These features include AA-specific pointer features, as well
 * as the standard busy pointer and the pointer-delay feature.  This
 * feature is useful if the application may be busy for an indeterminate
 * but possibly short period of time.  If the application restores the
 * original pointer before a short time has elapsed, no busy pointer
 * will appear.  This removes annoying short flashes of the busy pointer.
 */

/*------------------------------------------------------------------------*/

#include <intuition/intuition.h>
#include <intuition/pointerclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

/*------------------------------------------------------------------------*/

void doPointerStuff( struct Window *win, ULONG code );
void bail_out(long code);
struct Gadget *createAllGadgets( struct Gadget **glistptr, void *vi,
    struct Screen *sc );
BOOL createPointers();

/*------------------------------------------------------------------------*/

struct NewWindow newwin =
{
    0,0,		/*  LeftEdge, TopEdge */
    320,200,            /*  Width, Height */
    -1, -1,             /*  DetailPen, BlockPen */
    VANILLAKEY | CLOSEWINDOW | REFRESHWINDOW | BUTTONIDCMP,	/*  IDCMPFlags */
    WINDOWDRAG | WINDOWSIZING | WINDOWDEPTH | WINDOWCLOSE |
    WFLG_ACTIVATE|SIMPLE_REFRESH,	/* Flags */
    NULL,		/*  FirstGadget */
    NULL,		/*  CheckMark */
    (UBYTE *) "New Pointer Demo",	/*  Title */
    NULL,		/*  Screen */
    NULL,		/*  BitMap */
    100,50,		/*  MinWidth, MinHeight */
    640,200,		/*  MaxWidth, MaxHeight */
    WBENCHSCREEN,	/*  Type */
};

/*------------------------------------------------------------------------*/

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;

struct Screen *mysc = NULL;
struct VisualInfo *vi = NULL;
struct Gadget *glist = NULL;

struct Window *win = NULL;
struct BitMap *bm = NULL;
struct BitMap simplebm;

VOID *simple_pointer = NULL;
VOID *wide_pointer = NULL;

UWORD __chip simplePointer0[] =
{
    0x0180,    /* .......##....... */
    0x0180,    /* .......##....... */
    0x0180,    /* .......##....... */
    0x0ff0,    /* ....########.... */
    0x3ffc,    /* ..############.. */
    0x7ffe,    /* .##############. */
    0xffff,    /* ################ */
    0xffff,    /* ################ */
    0xffff,    /* ################ */
    0xffff,    /* ################ */
    0xffff,    /* ################ */
    0xffff,    /* ################ */
    0x7ffe,    /* .##############. */
    0x7ffe,    /* .##############. */
    0x3ffc,    /* ..############.. */
    0x0ff0     /* ....########.... */
};

UWORD __chip simplePointer1[] =
{
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0660,    /* .....##..##..... */
    0x0440,    /* .....#...#...... */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0000,    /* ................ */
    0x0ef0,    /* ....###.####.... */
    0x07e0,    /* .....######..... */
    0x0340,    /* ......##.#...... */
    0x0000,    /* ................ */
    0x0000     /* ................ */
};

struct TextAttr topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

/*------------------------------------------------------------------------*/

void main( void )
{
    BOOL terminated;
    struct IntuiMessage *imsg;
    struct Gadget *gad;

    terminated = FALSE;
    win = NULL;

    if (!(GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library",39L)))
	bail_out(20);

    if (!(IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library",39L)))
	bail_out(20);

    if (!(GadToolsBase = OpenLibrary("gadtools.library",39L)))
	bail_out(20);

    if ( !( mysc = LockPubScreen( NULL ) ) )
	bail_out(20);

    if ( !( vi = GetVisualInfo( mysc, NULL ) ) )
	bail_out(20);

    if ( ! createPointers() )
	bail_out(20);

    if ( !( gad = createAllGadgets( &glist, vi, mysc ) ) )
	bail_out(20);

    if (!(win = OpenWindowTags(&newwin,
	WA_Pointer, simple_pointer,
	WA_PubScreen, mysc,
	WA_Gadgets, glist,
	TAG_DONE)))
	bail_out(20);

    while (!terminated)
    {
	Wait (1 << win->UserPort->mp_SigBit);
	while (imsg = GT_GetIMsg(win->UserPort))
	{
	    if (imsg->Class == CLOSEWINDOW)
		terminated = TRUE;
	    else if (imsg->Class == REFRESHWINDOW)
	    {
		GT_BeginRefresh(win);
		GT_EndRefresh(win,TRUE);
	    }
	    else if (imsg->Class == GADGETUP)
	    {
		doPointerStuff( win, ((struct Gadget *)imsg->IAddress)->GadgetID );
	    }
	    else if (imsg->Class == VANILLAKEY)
	    {
		doPointerStuff( win, imsg->Code );
	    }
	    GT_ReplyIMsg(imsg);
	}
    }
    bail_out(0);
}


/*------------------------------------------------------------------------*/

void doPointerStuff( struct Window *win, ULONG code )
{
    switch ( toupper(code) )
    {
    case 'C':	/* Clear pointer */
	SetWindowPointer( win,
	    TAG_DONE );
	break;

    case 'S':	/* Set custom */
	SetWindowPointer( win,
	    WA_Pointer, simple_pointer,
	    TAG_DONE );
	break;

    case 'W':	/* Set wide custom */
	SetWindowPointer( win,
	    WA_Pointer, wide_pointer,
	    TAG_DONE );
	break;

    case 'B':	/* Set Busy */
	SetWindowPointer( win,
	    WA_BusyPointer, TRUE,
	    TAG_DONE );
	break;

    case 'D':	/* Busy w/ delay */
	SetWindowPointer( win,
	    WA_BusyPointer, TRUE,
	    WA_PointerDelay, TRUE,
	    TAG_DONE );
	break;
    }
}


/*------------------------------------------------------------------------*/

struct Gadget *createAllGadgets( struct Gadget **glistptr, void *vi,
    struct Screen *sc )
{
    struct NewGadget ng;
    struct Gadget *gad;

    gad = CreateContext( glistptr );

    ng.ng_LeftEdge = 20;
    ng.ng_TopEdge = sc->WBorTop + sc->Font->ta_YSize + 11;
    ng.ng_Width = 120;
    ng.ng_Height = 14;
    ng.ng_Flags = 0;
    ng.ng_VisualInfo = vi;
    ng.ng_TextAttr = &topaz80;
    ng.ng_GadgetText = "_Clear Pointer";
    ng.ng_GadgetID = 'C';
    gad = CreateGadget( BUTTON_KIND, gad, &ng,
	GT_Underscore, '_',
	TAG_DONE );

    ng.ng_LeftEdge += 130;
    ng.ng_GadgetText = "_Simple Pointer";
    ng.ng_GadgetID = 'S';
    gad = CreateGadget( BUTTON_KIND, gad, &ng,
	GT_Underscore, '_',
	TAG_DONE );

    ng.ng_TopEdge += 20;
    ng.ng_LeftEdge = 20;
    ng.ng_GadgetText = "_Wide Pointer";
    ng.ng_GadgetID = 'W';
    gad = CreateGadget( BUTTON_KIND, gad, &ng,
	GT_Underscore, '_',
	TAG_DONE );

    ng.ng_TopEdge += 20;
    ng.ng_LeftEdge = 20;
    ng.ng_GadgetText = "_Busy Pointer";
    ng.ng_GadgetID = 'B';
    gad = CreateGadget( BUTTON_KIND, gad, &ng,
	GT_Underscore, '_',
	TAG_DONE );

    ng.ng_LeftEdge += 130;
    ng.ng_GadgetText = "_Delayed Busy";
    ng.ng_GadgetID = 'D';
    gad = CreateGadget( BUTTON_KIND, gad, &ng,
	GT_Underscore, '_',
	TAG_DONE );

    return( gad );
}

/*------------------------------------------------------------------------*/

void bail_out(long code)
{
    if (win)
	CloseWindow(win);

    if ( glist )
	FreeGadgets( glist );

    if ( vi )
	FreeVisualInfo( vi );

    if ( mysc )
    	UnlockPubScreen( NULL, mysc );

    if (wide_pointer)
	DisposeObject(wide_pointer);

    if (simple_pointer)
	DisposeObject(simple_pointer);

    if (bm)
	FreeBitMap(bm);

    if (GadToolsBase)
	CloseLibrary(GadToolsBase);

    if (IntuitionBase)
	CloseLibrary(IntuitionBase);

    if (GfxBase)
	CloseLibrary(GfxBase);

    exit(code);
}


/*------------------------------------------------------------------------*/

#define PWORDWIDTH	4
#define PHEIGHT		64

BOOL createPointers()
{
    struct RastPort rport;
    int i;

    /* First, let's make a traditional bitmap, then make a
     * pointer out of it:
     */
    InitBitMap( &simplebm, 2, 16, 16 );
    simplebm.Planes[0] = (PLANEPTR)simplePointer0;
    simplebm.Planes[1] = (PLANEPTR)simplePointer1;
    simple_pointer = NewObject( NULL, "pointerclass",
	POINTERA_BitMap, &simplebm,
	POINTERA_XOffset, -6,
	POINTERA_WordWidth, 1,
	POINTERA_XResolution, POINTERXRESN_HIRES,
	POINTERA_YResolution, POINTERYRESN_HIGH,
	TAG_DONE );

    /* Now, we'll make a V39-style bitmap, and render into
     * a RastPort we throw around it:
     */

    if ( bm = AllocBitMap( 16*PWORDWIDTH, PHEIGHT, 2, BMF_CLEAR, NULL ) )
    {
	InitRastPort( &rport );
	rport.BitMap = bm;

	SetAPen( &rport, 1 );
	for ( i = 0; i < PHEIGHT; i++ )
	{
	    Move( &rport, 0, i );
	    Draw( &rport, (i*(PWORDWIDTH*16))/PHEIGHT, i );
	}

	SetAPen( &rport, 3 );
	Move( &rport, 0 ,0 );
	Draw( &rport, PWORDWIDTH*16-1, PHEIGHT-1 );
	SetAPen( &rport, 2 );
	Move( &rport, 0 ,0 );
	Draw( &rport, 0, PHEIGHT-1 );

	wide_pointer = NewObject( NULL, "pointerclass",
	    POINTERA_BitMap, bm,
	    POINTERA_WordWidth, PWORDWIDTH,
	    POINTERA_XResolution, POINTERXRESN_HIRES,
	    POINTERA_YResolution, POINTERYRESN_HIGHASPECT,
	    TAG_DONE );
    }

    return( simple_pointer && wide_pointer );
}

/*------------------------------------------------------------------------*/

