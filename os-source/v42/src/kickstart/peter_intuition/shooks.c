/*** shooks.c ****************************************************************
 *
 *  String Gadget Hooks
 *
 *  $Id: shooks.c,v 38.3 92/08/02 12:45:13 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1988, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"

#include "shooks_protos.h"

/*---------------------------------------------------------------------------*/

static int stringRender(struct Gadget * g,
                        struct gpRender * cgp);

static int stringGoActive(struct Gadget * g,
                          struct gpInput * cgp);

static int safeDisplayString(struct Gadget * g,
                             struct GadgetIngo * gi);

static int stringGoInactive(struct Gadget * g,
                            struct gpGoInactive * cgp);

static int stringHandleInput(struct Gadget * g,
                             struct gpInput * cgp);

/*---------------------------------------------------------------------------*/

#define D(x)	;
#define DBOX(x)	;

#if 1
ULONG
stringHook( h, g, cgp )
struct Hook		*h;
struct Gadget		*g;
ULONG			*cgp;	/* only interested in HookID	*/
{
    ULONG retval = 0;

    switch ( (WORD) *cgp )
    {
#if 0
    case GM_SETUPINFO:
	stringSetupInfo( g, cgp );
	break;
#endif
    case GM_HITTEST:
        return ( GMR_GADGETHIT ); /* I have no special hit conditions	*/
	break;
    case GM_HELPTEST:
        retval = GMR_HELPHIT;
	break;
    case GM_RENDER:
	stringRender( g, cgp );
	break;
    case GM_GOACTIVE:
        retval = stringGoActive( g, cgp );
	break;
    case GM_GOINACTIVE:
        retval = stringGoInactive( g, cgp );
	break;
    case GM_HANDLEINPUT:
        retval = stringHandleInput( g, cgp );
	break;
    }
    return ( retval );
}
#endif

#if 0
/*
 * this type of thing really should be done at on a one-time
 * basis in a hook called by gadgetInfo() 
 * ZZZ: Per class data.
 */
stringSetupInfo( g, cgp )
struct Gadget		*g;
struct CGPSetupInfo	*cgp;
{
    struct GadgetInfo	*gi = cgp->cgp_GInfo;
    D( printf("setupinfo hook\n") );

    /* set up extension, if there is one	*/
    if ( ( g->Flags & GFLG_STRINGEXTEND ) || ( g->Activation & GACT_STRINGEXTEND ) )
    {
	gi->gi_SEBuffer = *(SI( g )->Extension);
	gi->gi_SExtend = &gi->gi_SEBuffer;
    }
    else
    {
	gi->gi_SExtend = NULL;
    }
    return ( 0 );
}
#endif

#if 0
stringHitTest()
{
    return ( GMR_GADGETHIT ); /* I have no special hit conditions	*/
}
#endif

static
stringRender( g, cgp )
struct Gadget	*g;
struct gpRender	*cgp;
{
    struct GadgetInfo	*gi = cgp->gpr_GInfo;
    struct IBox		gadgetbox;
    struct StringExtend	*getSEx();
    D( printf("gh render hook\n") );

    if ( cgp->gpr_Redraw != GREDRAW_TOGGLE )
    {
	DBOX( dumpGI( "gpr_GInfo", gi ) );

	gadgetBox( g, gi, &gadgetbox );

	DBOX( dumpBox( "gadget box in stringRender", &gadgetbox ));

	commonGadgetRender( cgp->gpr_RPort, g, gi, &gadgetbox, TRUE );

	/* if selected, will highlight */
	displayString( cgp->gpr_RPort, g, gi, &gadgetbox, getSEx(g) );

	commonGadgetTextAndGhost( cgp->gpr_RPort, g, gi, &gadgetbox );
    }
    /* strings do no toggle highlighting  */

    return ( 0 );
}
    
static
stringGoActive( g, cgp )
struct Gadget	*g;
struct gpInput	*cgp;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct GadgetInfo	*gi = cgp->gpi_GInfo;
    struct StringInfo	*si = (struct StringInfo *) g->SpecialInfo;
    BOOL		redisplay;
    struct StringExtend	*getSEx();

    /* this class of gadgets gets to cache some stuff
     * in IBase.
     */

    D( printf("go active hook gadget: %lx, si %lx\n", g, si) );

    DBOX( dumpGI( "goactive GI", gi ) );
    gadgetBox( g, gi, &IBase->ActiveGBox );

    if ( si->UndoBuffer )
    {
	D( printf("copy to undo buffer.b %lx ub %lx mc %ld\n",
		si->Buffer, si->UndoBuffer, si->MaxChars ) );
	CopyMem(si->Buffer, si->UndoBuffer, si->MaxChars );
    }
    si->UndoPos = si->BufferPos;

    D( printf("ga pt2\n"));

    /* I'm allowed to use IBase for my state storage	*/
    IBase->KeyMap = TESTFLAG( g->Activation, ALTKEYMAP )? si->AltKeyMap: NULL;

    /* set up cached string extend,
     * BEFORE using in iMouse below! jimm/bart: 1/9/90
     */
    IBase->ActiveSEx = getSEx( g );

    /* handle initial SELECT event	*/
    if ( cgp->gpi_IEvent ) 
    {
	iMouse( cgp->gpi_IEvent, g, gi, 
		IBase->ActiveSEx,
		&redisplay, &IBase->ActiveGBox );
    }

    D( printf("ga pt2\n"));

    /* my responsibility to set this flag	*/
    SETFLAG( g->Flags, SELECTED );

    D( printf("string goactive display\n") );

    safeDisplayString( g, gi );

    D( printf("string goactive done\n") );

    return ( GMR_MEACTIVE );
}

/*
 * calls displayString with lock on rastport
 * uses ActiveSEx, so watch it.
 */
static
safeDisplayString( g, gi )
struct Gadget 		*g;
struct GadgetIngo	*gi;
{
    struct RastPort	*rp;
    struct IntuitionBase	*IBase = fetchIBase();

    D( printf("safeDisplayString\n") );

    rp = ObtainGIRPort( gi );
    D( printf("rastport: %lx\n", rp) );
    DBOX( dumpBox("safe display string uses ActiveGBox",
		&IBase->ActiveGBox));
    displayString( rp, g, gi, &IBase->ActiveGBox, IBase->ActiveSEx );
    FreeGIRPort( rp );
}

static
stringGoInactive( g, cgp )
struct Gadget	*g;
struct gpGoInactive	*cgp;
{
    D( printf("goinactive\n") );
    /* my job to clear this flag	*/
    CLEARFLAG( g->Flags, SELECTED );
    safeDisplayString( g, cgp->gpgi_GInfo );
}

static
stringHandleInput( g, cgp )
struct Gadget	*g;
struct gpInput	*cgp;
{
    BOOL	redisplay;
    LONG	stringreturn;
    struct GadgetInfo	*gi = cgp->gpi_GInfo;

    stringreturn = stringInput( cgp->gpi_IEvent, g, gi,
	&redisplay, cgp->gpi_Termination);

    if ( redisplay ) safeDisplayString( g, gi );
    
    return ( stringreturn );
}

#if 0
PFU	stringHookTable[] = {
	stringHitTest,	/* GM_HITTEST	*/
	stringRender,	/* GM_RENDER	*/
	stringGoActive,	/* GM_GOACTIVE	*/
	stringGoInactive,	/* GM_GOINACTIVE	*/
	stringHandleInput,	/* GM_HANDLEINPUT	*/
	stringSetupInfo,	/* GM_SETUPINFO	*/
};
#endif

