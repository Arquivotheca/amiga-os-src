/*** newedges.c *************************************************************
 *
 *  File
 *
 *  $Id: prop.c,v 38.4 93/01/14 14:29:30 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"
#include "newlook.h"

#define DPS(x)	;
#define DPGX(x)	;
#define DXY(x)	;

/* this turns into a structure copy somewhere ... */
#define setImageBox(T, F) getImageBox(F, T)

/* ZZZ: should get special info consistently, from
 * ActivePropInfo or GadgetEnv
 */
containerBumpPots( g, gi, coord, knobbox )
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct Point coord;
struct IBox		*knobbox;
{
    struct PropInfo *pi;
    LONG mul;
    UWORD	containerIncrement();

    pi = (struct PropInfo *) g->SpecialInfo;


    if (pi->Flags & FREEHORIZ)
    {
	mul = 0;
	if ( coord.X < 0 )
	{
	    mul--;
	}
	else if (coord.X >= knobbox->Width )
	{
	    mul++;
	}
	pi->HorizPot = potInRange((long)pi->HorizPot +
			    mul * containerIncrement(pi->HorizBody));
    }
    if (pi->Flags & FREEVERT)
    {
	mul = 0;
	if ( coord.Y < 0 )
	{
	    mul--;
	}
	else if (coord.Y >= knobbox->Height )
	{
	    mul++;
	}
	pi->VertPot = potInRange((long)pi->VertPot +
			    mul * containerIncrement(pi->VertBody));
    }
}

/* wherein 1/4 becomes 1/3 -- portion visible converts to fractional
 * amount that must added to reach top of next 'page'.
 * note that if displaying a document, the last top of page is 
 * at point 3/4, but Pot must read 1/1 (0xFFFF).
 */
UWORD
containerIncrement(body)
USHORT body;
{
    USHORT inc = 0xFFFF;
    if (body < 0x8000)	/* for big bodies, max out at 0xFFFF */
    inc = ((long)body << 16)/(0xFFFF - body);
    return (inc);
}

/* for prop gadget, calculate image dims.
 * be sure gi is for an AUTOKNOB propgadget
 */
dimensionNewKnobForBodys( g, gi, pgx )
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct PGX		*pgx;
{
    struct PropInfo *pi;
    struct IBox	*newknob = &pgx->pgx_NewKnob;
    struct IBox		container;

    pi = (struct PropInfo *) g->SpecialInfo;
    container = pgx->pgx_Container;

    if (pi->Flags & FREEHORIZ)
    {
	newknob->Width =
	    MIN(container.Width,
	     MAX(KNOBHMIN,((container.Width*pi->HorizBody)>>16)+1));
	newknob->Width = MAX(newknob->Width, 1);	/* jimm: 7/28/86: */
    }
    else
    {
	newknob->Width = container.Width;
	newknob->Left = 0;	/* jimm: 6/12/86: can't ignore */
    }

    if (pi->Flags & FREEVERT)
    {
	newknob->Height =
	    MIN(container.Height,
	     MAX(KNOBVMIN,((container.Height*pi->VertBody)>>16)+1));
	newknob->Height = MAX(newknob->Height, 1);	/* jimm: 7/28/86: */
    }
    else
    {
	newknob->Height = container.Height;
	newknob->Top = 0; 	/* jimm: 6/12/86: can't ignore */
    }
}

#if 0	/* should be user program */
/* for scrolling 6 pages, on last page pot shows 0xFFFF but
 * want top of page to be at (5/6)*length.  use this function.
 */
USHORT
potToTop(pot, body)
USHORT pot;
USHORT body;
{
    (((long)(0xFFFF - body) * pot) + 0xFFFF) >> 16;
}
#endif

/* calculate knob position for current pot settings.
 * note that this doesn' change dimensions of new knob,
 * which it expects to be valid.  They are set up
 * in gadgetInfo, and updated by autoknob.
 */
positionNewKnobForPots( g, gi, pgx )
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct PGX		*pgx;
{
    struct PropInfo *pi;
    struct IBox *newknob = &pgx->pgx_NewKnob;

    pi = (struct PropInfo *) g->SpecialInfo;

    if (pi->Flags & FREEHORIZ)
    {
	newknob->Left = ((ULONG)((pi->HorizPot  + 1) *
			(pgx->pgx_Container.Width - newknob->Width))) >> 16;
    }

    if (pi->Flags & FREEVERT)
    {
	newknob->Top = ((ULONG)((pi->VertPot + 1) * 
			(pgx->pgx_Container.Height - newknob->Height))) >> 16;
    }
}

USHORT
potInRange(potval)
LONG potval;
{
    if (potval < 0) potval = 0;
    if (potval > 0xFFFF) potval = 0xFFFF;
    return ((USHORT) potval & 0xFFFF);
}


/* given a new mouse value in window coordinates, adjusts the
 * pots for the appropriate new knob position
 *
 * pass it point in window coords, get back the new valid
 * location for the knob image.
 * 
 * WARNING: assume we're working on the active gadget, uses ActivePGX
 */
BOOL	/* returns false if knob doesn't move */
dragNewKnob( g, gi, mousept, gbox )
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct Point mousept;
struct IBox		*gbox;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct IBox	oldknob;
    struct IBox	newknob;
    struct IBox	constraint;
    struct PropInfo *pi = (struct PropInfo *) g->SpecialInfo;

    /* method: calculate proposed new position of knob,
     * from mouse position and offset that the pointer is supposed to
     * stay in the knob.  fit this knob legally into container.
     * based on left/top of new knob positions, get new pots.
     */

    getImageBox( g->GadgetRender, &oldknob);
    newknob = oldknob;

    DPS( dumpBox( "pSlide oldknob", &oldknob ) );

    /* get mouse position, relative to container */
    gadgetPoint( g, gi, mousept, &newknob.Left, gbox);
    transfPoint( &newknob.Left, *((struct Point * ) &pi->LeftBorder) );

    /* get new proposed image left/top from this */
    transfPoint(&newknob.Left,  IBase->KnobOffset );

    DPS( dumpBox( "pSlide unconstrained newknob", &newknob ) );

    /* fit proposed knob location into container.  define the container
     * small and expand to allow FREEHORIZ/FREEVERT
     */
    constraint = oldknob;
    if ( pi->Flags & FREEHORIZ )
    {
	constraint.Left = 0;
	constraint.Width = IBase->ActivePGX.pgx_Container.Width;
    }
    if ( pi->Flags & FREEVERT )
    {
	constraint.Top = 0;
	constraint.Height = IBase->ActivePGX.pgx_Container.Height;
    }

    boxFit(&newknob, &constraint, &newknob);

    DPS( dumpBox( "pSlide constrained newknob", &newknob ) );

    /* get new pots from left/top of new knob */
    setPotsForKnobPosition( g, gi, upperLeft(&newknob), &IBase->ActivePGX );

    IBase->ActivePGX.pgx_NewKnob = newknob;
}

/* sets pots for gadget (pointed to by info) given x/y position
 * for knob left/top.  assumes x/y are valid positions within container
 * relative to container
 *
 * LIMITATION: works only for intuition's selected prop gadget.
 * will not change a pot if there is no displacement
 * from current gadget knob image position.
 *
 * Peter 12-Dec-91:  Now changes a pot if the knob is already at the
 * extreme left (or top).  That's because if the current pot is very
 * small but non-zero (so that the knob is in its leftmost (or topmost)
 * position, you need to be able to drag it to pot=0.
 *
 * Since this calculation rounds down absolutely (you have to have
 * pot exactly equal to 65535 for the knob to be in the last spot),
 * we only have to worry about the leftmost (or topmost) position.
 */
setPotsForKnobPosition( g, gi, pt, pgx )
struct Gadget 		*g;
struct GadgetInfo  	*gi;
struct Point		pt;
struct PGX		*pgx;
{
    struct IBox imbox;
    int freedom;
    USHORT pot;
    struct PropInfo *pi = (struct PropInfo *) g->SpecialInfo;

    getImageBox( g->GadgetRender, &imbox);

    /* Recalculate pot if knob moved or is at zero (see note above) */
    if ( ( !imbox.Left ) || ( imbox.Left != pt.X ) )
    {
	DXY( printf("xyP: calc new horiz\n") );
	pot = 0xFFFF;

	if (freedom = ( pgx->pgx_Container.Width - imbox.Width ))
	{
	    pot =  potInRange(((long) pt.X << 16)/freedom);
	}
	pi->HorizPot = pot;
    }

    /* Recalculate pot if knob moved or is at zero (see note above) */
    if ( ( !imbox.Top ) || ( imbox.Top != pt.Y ) )
    {
	pot = 0xFFFF;
	if (freedom = ( pgx->pgx_Container.Height - imbox.Height ))
	{
	    pot =  potInRange(((long) pt.Y << 16)/freedom);
	}
	pi->VertPot =  pot;
    }
}



/*
 * copies NewKnob dimensions back to propgadget images
 */
syncKnobs( g, gi, pgx )
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct PGX		*pgx;
{
    setImageBox( g->GadgetRender, &pgx->pgx_NewKnob );

    if ( ( ( g->Flags & GADGHIGHBITS ) == GADGHIMAGE ) && ( g->SelectRender ) )
    {
	setImageBox( g->SelectRender, &pgx->pgx_NewKnob );
    }
}

borderlessProp( g )
struct Gadget	*g;
{
    return ( TESTFLAG( ((struct PropInfo *)g->SpecialInfo)->Flags, PROPBORDERLESS ) );
}

setupPGX( g, gi, pgx )
struct Gadget		*g;
struct GadgetInfo	*gi;
struct PGX		*pgx;
{
    struct PropInfo	*pi = (struct PropInfo *) g->SpecialInfo;

    gadgetBox( g, gi, &pgx->pgx_Container );

    /* unused compatibilty fields: entire gadget width */
    pi->CWidth = pgx->pgx_Container.Width;
    pi->CHeight = pgx->pgx_Container.Height;

    if ( borderlessProp( g ) )
    {
	pi->LeftBorder = 0;
	pi->TopBorder = 0;
    }
    else
    {
	if ( newlookProp( g ) )
	{
	    /* newlook prop gets skinnier borders, and 
	     * container that runs right up against them.
	     */
	    DPGX( printf("newlook prop gets skinnier borders\n"));

	    /*pi->LeftBorder = gi->gi_Screen->WBorLeft/2;*/
	    /*pi->TopBorder = gi->gi_Screen->WBorTop/2;*/

	    /*	jimm: 5/27/90
	     * The container inset is Left/TopBorder, but
	     * the thickness of the drawn border might
	     * be half that.
	     *
	     * Left/TopBorder cannot change meaning.
	     */

	    pi->LeftBorder = FRAMEHTHICK;
	    pi->TopBorder = FRAMEVTHICK;

	    DPGX( printf("%lx/%lx\n", pi->LeftBorder, pi->TopBorder));
	}
	else
	{
	    /* compatible prop borders: inset to container is
	     * WBorLeft, but border drawn is half that
	     */
	    pi->LeftBorder = gi->gi_Screen->WBorLeft;
	    pi->TopBorder = gi->gi_Screen->WBorTop;
	}
	pgx->pgx_Container.Left	+= pi->LeftBorder;
	pgx->pgx_Container.Top	+= pi->TopBorder;
	pgx->pgx_Container.Width	-= (pi->LeftBorder << 1);
	pgx->pgx_Container.Height	-= (pi->TopBorder << 1);
    }

    /* copy image box to newknob	*/
    /* set get dimensions for knob	*/
    if (pi->Flags & AUTOKNOB)
    {
	dimensionNewKnobForBodys( g, gi, pgx );
    }
    else
    {
	getImageBox( g->GadgetRender, &pgx->pgx_NewKnob );
    }

    /* set knob images in correct position */
    positionNewKnobForPots( g, gi, pgx );	/* sets pgx_NewKnob */

    /* for compatibility */
    pi->HPotRes = containerIncrement(pi->HorizBody);
    pi->VPotRes = containerIncrement(pi->VertBody);
}
