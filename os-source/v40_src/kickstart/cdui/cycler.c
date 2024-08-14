
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <graphics/videocontrol.h>
#include <graphics/sprite.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "cduibase.h"
#include "cycler.h"


/*****************************************************************************/


struct ColorBatch16
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[16];
    UWORD           cb_EndMarker;
};

struct ColorBatch32
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[32];
    UWORD           cb_EndMarker;
};

struct ColorBatch256
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[256];
    UWORD           cb_EndMarker;
};


/*****************************************************************************/


#undef SysBase

/* These are the color registers that the rainbow-colored gradient
 * reflection in the disc and the CD are drawn with:
 */
#define CYCLE_RAINBOW_FIRST	23
#define CYCLE_RAINBOW_LAST	47
#define CYCLE_RAINBOW_COUNT	(CYCLE_RAINBOW_LAST-CYCLE_RAINBOW_FIRST+1)

/* These are the color registers that the rotating perimeter and center
 * of the disc (as well as the twinkling stars) are drawn with:
 */
#define CYCLE_PERIMETER_FIRST	48
#define CYCLE_PERIMETER_LAST	61
#define CYCLE_PERIMETER_COUNT	(CYCLE_PERIMETER_LAST-CYCLE_PERIMETER_FIRST+1)

/* These are the color registers that the face of the disc is drawn with:
 * (Cycling this creates the impression of a reflection from the upper
 * aurora).
 */
#define CYCLE_FACE_FIRST	8
#define CYCLE_FACE_LAST		21
#define CYCLE_FACE_COUNT	(CYCLE_FACE_LAST-CYCLE_FACE_FIRST+1)

/* These are the color registers used to draw the upper aurora: */
#define CYCLE_AURORATOP_FIRST	197
#define CYCLE_AURORATOP_LAST	255
#define CYCLE_AURORATOP_COUNT	(CYCLE_AURORATOP_LAST-CYCLE_AURORATOP_FIRST+1)

/* These are the color registers used to draw the lower aurora: */
#define CYCLE_AURORABOTTOM_FIRST	128
#define CYCLE_AURORABOTTOM_LAST		181
#define CYCLE_AURORABOTTOM_COUNT	(CYCLE_AURORABOTTOM_LAST-CYCLE_AURORABOTTOM_FIRST+1)

/* These are the color registers used to draw the aurora effect in the
 * words "Amiga" and "32":
 */
#define CYCLE_AURORAAMIGA_FIRST	96
#define CYCLE_AURORAAMIGA_LAST	126
#define CYCLE_AURORAAMIGA_COUNT	(CYCLE_AURORAAMIGA_LAST-CYCLE_AURORAAMIGA_FIRST+1)

/* These are the color registers used to draw the border around the
 * Amiga CD 32 logo:
 */
#define CYCLE_AMIGABORDER_COUNT	(CYCLE_AMIGABORDER_LAST-CYCLE_AMIGABORDER_FIRST+1)

/* Rainbow diffraction effect in disc */
static const struct ColorEffect DiscRainbowEffect =
{
    CE_LOOPER,
    4,
    CYCLE_RAINBOW_FIRST, CYCLE_RAINBOW_LAST,
    0,
};

/* Gray perimeter and center of disc, plus some stars */
static const struct ColorEffect DiscPerimeterEffect =
{
    CE_LOOPER,
    5,
    CYCLE_PERIMETER_FIRST, CYCLE_PERIMETER_LAST,
    0,
};

/* Border around "Amiga" and "32" */
static const struct ColorEffect AmigaBorderEffect =
{
    CE_LOOPER,
    6,
    CYCLE_AMIGABORDER_FIRST, CYCLE_AMIGABORDER_LAST,
    0,
};

/* Gray face of disc */
static const struct ColorEffect DiscFaceEffect =
{
    CE_LOOPER,
    4,
    CYCLE_FACE_FIRST, CYCLE_FACE_LAST,
    0,
};

static const struct ColorEffect AuroraEffect =
{
    CE_SHOOTER,
    1,
    0,95,
    CEPALETTE_ALT2,
};

static const struct ColorEffect AmigaAuroraEffect =
{
    CE_SHOOTER,
    2,
    96,165,
    CEPALETTE_ALT2,
};

static const struct ColorEffect BlueAuroraEffect =
{
    CE_SHOOTER,
    1,
    47,79,
    CEPALETTE_ALT1,
};

static const struct ColorEffect MultiAuroraEffect =
{
    CE_SHOOTER,
    1,
    96,255,
    CEPALETTE_ALT1,
};

static const struct ColorEffect DelayEffect =
{
    CE_WAITER,
    60,
    0,0,
    0,
};

static const struct ColorEffect SlowFadeInEffect =
{
    CE_INFADER,
    4,
    10,0,
    0,
};

static const struct ColorEffect FadeOutEffect =
{
    CE_OUTFADER,
    1,
    0,0,
    0,
};


/*****************************************************************************/


static void
initCycle( struct ColorCycle *cycle, const struct ColorEffect *effect,
    UWORD colorbase, UWORD numcolors, UWORD termination )
{
    cycle->cy_Effect = (struct ColorEffect *)effect;
    cycle->cy_Phase = 0;
    cycle->cy_Halted = FALSE;
    cycle->cy_NumColors = numcolors;
    cycle->cy_ColorBase = colorbase;
    cycle->cy_CountDown = effect->ce_Period;
    cycle->cy_Termination = termination;
}

#define NUM_FADE_STEPS 20


/*****************************************************************************/


/* First, operate on all CE_LOOPERs, since they move colors in
 * the source color table, which we'll need to pick up when we
 * make our working copy of the color table.
 */
static VOID processLoopers( struct CDUILib *CDUIBase )
{
    WORD cyclenum;
    struct ColorCycle *cycle;
    struct ColorEffect *effect;

    for ( cyclenum = 0; cyclenum < NUM_CYCLES; cyclenum++ )
    {
	cycle = &CDUIBase->cb_Cycles[ cyclenum ];
	if ( ( effect = cycle->cy_Effect ) && ( effect->ce_Type == CE_LOOPER ) )
	{
	    /* The disc face cycle is special because it only runs when
	     * the top aurora is running.  This is because it's supposed
	     * to be a reflection of the top aurora.  We don't have a great
	     * way of providing for interconnections, so we let a little
	     * special-case stuff affect the purity of the color-effect
	     * engine.
	     */
	    if ( cyclenum == CYCLE_FACE )
	    {
		struct ColorCycle *acycle;
		struct ColorEffect *aeffect;
		acycle = &CDUIBase->cb_Cycles[ CYCLE_AURORATOP ];
		aeffect = acycle->cy_Effect;

		/* If the top aurora exists, and is a shooter, and is not halted,
		 * and is in the most active part of its phase, then we want the
		 * face-cycling effect to be enabled, otherwise we want it off.
		 * (NB: halting a looper takes effect as soon as the phase
		 * returns to normal).
		 * Shooter phases run from zero to the number of colors in
		 * the shoot plus the number in the destination bitmap.  The
		 * most active part of the phase is defined to be any phase
		 * where half or more of the shoot-colors are on-screen.
		 */
		if ( ( aeffect ) && ( aeffect->ce_Type == CE_SHOOTER ) &&
		    ( !acycle->cy_Halted ) && ( acycle->cy_Phase >= acycle->cy_NumColors/2 ) &&
		    ( acycle->cy_Phase <= aeffect->ce_LastColor - aeffect->ce_FirstColor + acycle->cy_NumColors/2 ) )
		{
		    cycle->cy_Halted = FALSE;
		    /* Another kludge that uglies the code but beautifies the
		     * result:  We double the speed of the face-cycle when
		     * the MultiAuroraEffect is in effect.  This is because
		     * the bands are closer together, so it's like a higher
		     * frequency.
		     */
		    if ( aeffect == &MultiAuroraEffect )
		    {
			cycle->cy_CountDown = ( cycle->cy_CountDown+1 ) >> 1;
		    }
		}
		else
		{
		    cycle->cy_Halted = TRUE;
		}
	    }

	    /* Unless the cycle is halted, we count down by one.  Note
	     * that an order to halt only takes effect when the phase
	     * hits zero.  If the countdown we hits zero, then we reset
	     * the countdown value and cycle the colors by one step.
	     */
	    if ( ( ( !cycle->cy_Halted ) || ( cycle->cy_Phase != 0 ) ) &&
		( --cycle->cy_CountDown == 0 ) )
	    {
		struct RGBEntry *dstptr, *srcptr;
		struct RGBEntry temp;
		WORD i;

		cycle->cy_CountDown = effect->ce_Period;

		srcptr = &((struct ColorBatch256 *)CDUIBase->cb_BitMapColors)->cb_Colors[cycle->cy_ColorBase + cycle->cy_NumColors - 1];
		dstptr = srcptr;
		temp = *srcptr--;
		i = cycle->cy_NumColors - 1;
		while ( i-- > 0 )
		{
		    *dstptr-- = *srcptr--;
		}
		*dstptr = temp;
		if ( ++cycle->cy_Phase == cycle->cy_NumColors )
		{
		    cycle->cy_Phase = 0;
		}
	    }

	}
    }
}


/*****************************************************************************/


/* Now do all the shooters, which replace colors not originally
 * in the destination.  Shooters always copy their colors, regardless
 * of whether the CountDown expires, since the original colors in the
 * ctable are not at all the ones from the shooters, whose color palette
 * is really separate from the source picture.
 */
static VOID processShooters( struct CDUILib *CDUIBase, struct ColorBatch256 *ctable )
{
    LONG dst, src, src2, cyclenum;
    struct ColorCycle *cycle;
    struct ColorEffect *effect;
    for ( cyclenum = 0; cyclenum < NUM_CYCLES; cyclenum++ )
    {
	cycle = &CDUIBase->cb_Cycles[ cyclenum ];
	if ( ( effect = cycle->cy_Effect ) && ( effect->ce_Type == CE_SHOOTER ) )
	{
	    struct RGBEntry *dstptr, *srcptr;

	    /* For a shooting color cycle, we want the destination colors
	     * to all start as the first color to shoot, then we load in more
	     * and more of the effect colors, until they've all paraded through,
	     * and finally we trail out to all the final color of the shoot.
	     * Thus, cy_Phase must run from zero through the number of colors in
	     * the effect plus number of colors in the destination, to allow the
	     * fade from/to initial/final color.
	     */
	    src = effect->ce_LastColor + cycle->cy_NumColors - cycle->cy_Phase;
	    dstptr = &ctable->cb_Colors[cycle->cy_ColorBase+cycle->cy_NumColors-1];
	    src2 = src;
	    if ( src > effect->ce_LastColor )
	    {
		src2 = effect->ce_LastColor;
	    }
	    else if ( src < effect->ce_FirstColor )
	    {
		src2 = effect->ce_FirstColor;
	    }

	    srcptr = &((struct ColorBatch256 *)CDUIBase->cb_AuroraColors[effect->ce_Palette])->cb_Colors[src2];

	    for ( dst = cycle->cy_NumColors-1; dst >= 0; dst-- )
	    {
		*dstptr-- = *srcptr;
		if ( ( src > effect->ce_FirstColor ) && ( src <= effect->ce_LastColor ) )
		{
		    srcptr--;
		}
		src--;
	    }

	    if ( ( !cycle->cy_Halted ) && ( --cycle->cy_CountDown == 0 ) )
	    {
		cycle->cy_CountDown = effect->ce_Period;

		cycle->cy_Phase++;
		if ( cycle->cy_Phase > effect->ce_LastColor - effect->ce_FirstColor + 1 + cycle->cy_NumColors)
		{
		    /* We're done this effect.  Let's decide what to do based
		     * on the termination condition.
		     */
		    if ( cycle->cy_Termination == CYTERM_DELAY )
		    {
			initCycle( cycle, &DelayEffect, cycle->cy_ColorBase, cycle->cy_NumColors, 0 );
		    }
		    else /* cycle->cy_Termination == CYTERM_HOLD */
		    {
			cycle->cy_Halted = TRUE;
		    }
		}
	    }
	}
    }
}


/*****************************************************************************/


/* Now we do any post-processing or independent effects such as
 * fading or waiting
 */
static void processFaders( struct CDUILib *CDUIBase, struct ColorBatch256 *ctable )
{
    WORD dst, cyclenum;
    struct ColorCycle *cycle;
    struct ColorEffect *effect;
    struct ColorEffect *AuroraTable[] =
    {
	&AuroraEffect,
	&BlueAuroraEffect,
	&MultiAuroraEffect
    };
    for ( cyclenum = 0; cyclenum < NUM_CYCLES; cyclenum++ )
    {
	cycle = &CDUIBase->cb_Cycles[ cyclenum ];
	if ( ( effect = cycle->cy_Effect ) && ( effect->ce_Type >= CE_WAITER ) )
	{
	    if ( effect->ce_Type == CE_WAITER )
	    {
		if ( ( !cycle->cy_Halted ) && ( --cycle->cy_CountDown == 0 ) )
		{
		    cycle->cy_CountDown = effect->ce_Period;

		    /* We're done this effect, pick the next one */
		    initCycle( cycle, AuroraTable[CDUIBase->cb_AuroraIndex], cycle->cy_ColorBase, cycle->cy_NumColors, CYTERM_DELAY );

		    if ( ++CDUIBase->cb_AuroraIndex == 3 )
		    {
			CDUIBase->cb_AuroraIndex = 0;
		    }
		}
	    }
	    else if ( ( effect->ce_Type == CE_INFADER ) || ( effect->ce_Type == CE_OUTFADER ) )
	    {
		WORD fadelevel;
		WORD phase;
		/* ce_FirstColor is the number of countdowns to wait before
		 * beginning the fade...
		 */
		phase = cycle->cy_Phase - effect->ce_FirstColor;
		if ( phase < 0 )
		{
		    phase = 0;
		}

		/* We know how to fade in or out.  So set the fade level accordingly,
		 * and then scale the colors in the range.
		 */
		if ( effect->ce_Type == CE_INFADER )
		{
		    fadelevel = phase;
		}
		else
		{
		    fadelevel = NUM_FADE_STEPS - phase;
		}

		/* Special case: if fadelevel is 100%, then we don't need to scale,
		 * since the factor is unity.
		 */
		if ( fadelevel != NUM_FADE_STEPS )
		{
		    for ( dst = cycle->cy_ColorBase; dst < cycle->cy_ColorBase + cycle->cy_NumColors; dst++ )
		    {
			ctable->cb_Colors[dst].Red = ( ctable->cb_Colors[dst].Red / NUM_FADE_STEPS ) * fadelevel;
			ctable->cb_Colors[dst].Green = ( ctable->cb_Colors[dst].Green / NUM_FADE_STEPS ) * fadelevel;
			ctable->cb_Colors[dst].Blue = ( ctable->cb_Colors[dst].Blue / NUM_FADE_STEPS ) * fadelevel;
		    }
		}

		/* Our caller needs to know if there were any active
		 * faders, so let's help him out.
		 */
		if ( !cycle->cy_Halted )
		{
		    if ( --cycle->cy_CountDown == 0 )
		    {
			cycle->cy_CountDown = effect->ce_Period;

			/* Effective phase goes from 0 to NUM_FADE_STEPS, inclusive */
			if ( phase >= NUM_FADE_STEPS )
			{
			    cycle->cy_Halted = TRUE;
			}
			else
			{
			    cycle->cy_Phase++;
			}
		    }
		}
	    }
	}
    }
}


/*****************************************************************************/


VOID ColorCycler(VOID)
{
    struct CDUILib         *CDUIBase;
    struct ExecBase        *SysBase;
    struct ColorBatch256 cb;
    BOOL proceed;
    BOOL broken;

    SysBase = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    /* Initialize the regular color cycles */
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_RAINBOW ], &DiscRainbowEffect, CYCLE_RAINBOW_FIRST, CYCLE_RAINBOW_COUNT, 0 );
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_PERIMETER ], &DiscPerimeterEffect, CYCLE_PERIMETER_FIRST, CYCLE_PERIMETER_COUNT, 0 );
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_AMIGABORDER ], &AmigaBorderEffect, CYCLE_AMIGABORDER_FIRST, CYCLE_AMIGABORDER_COUNT, 0 );
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FACE ], &DiscFaceEffect, CYCLE_FACE_FIRST, CYCLE_FACE_COUNT, 0 );

    /* The trim of the word Amiga uses colors 64-77, and we want to keep those
     * black for a while.  So we set up a fader that covers 64-77.  A little
     * cooperation is required to make the fade-in work, because for us
     * to fade, the cb_BitMapColors has to contain the full-scale colors,
     * but the screen opener had better ensure that the colors 64-77 were
     * pre-set to black.
     */
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FADER1 ], &SlowFadeInEffect, CYCLE_AMIGABORDER_FIRST, CYCLE_AMIGABORDER_COUNT, 0 );
    /* Now we have a whole bunch of stuff to kick off.
     * Start the top aurora with the regular multicolor one,
     * Start the bottom aurora with a delay,
     * Start the Amiga-aurora that fills the letters with red,
     * and release the fader which will bring the border around
     * the word Amiga in.
     */
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_AURORATOP ], &AuroraEffect, CYCLE_AURORATOP_FIRST, CYCLE_AURORATOP_COUNT, CYTERM_DELAY );
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_AURORAAMIGA ], &AmigaAuroraEffect, CYCLE_AURORAAMIGA_FIRST, CYCLE_AURORAAMIGA_COUNT, CYTERM_HOLD );
    initCycle( &CDUIBase->cb_Cycles[ CYCLE_AURORABOTTOM ], &DelayEffect, CYCLE_AURORABOTTOM_FIRST, CYCLE_AURORABOTTOM_COUNT, 0 );

    /* We prevent the exit from the cycler until all the outstanding
     * faders have completed
     */
    proceed = TRUE;
    broken = FALSE;
    while ( proceed )
    {
	/* First, operate on all CE_LOOPERs, since they move colors in
	 * the source color table, which we'll need to pick up when we
	 * make our working copy of the color table.
	 */
	processLoopers( CDUIBase );

	/* Now make a working copy of the source color table (after
	 * looper effects are done), so we can fade the values therein.
	 */
	cb = *((struct ColorBatch256 *)CDUIBase->cb_BitMapColors);

	/* Now do all the shooters, which replace colors not originally
	 * in the destination.
	 */
	processShooters( CDUIBase, &cb );

	/* Now we do any post-processing or independent effects such as
	 * fading or waiting.  processFaders() tells us if any faders
	 * were active.
	 */
	processFaders( CDUIBase, &cb );

	WaitTOF();
	LoadRGB32(CDUIBase->cb_ViewPort,(ULONG *)&cb);
	if ( broken )
	{
	    /* We must not exit until all pending effects are
	     * complete.  A pending effect is any one of the
	     * three faders still in action (i.e. not halted)
	     * or if the Amiga-logo border has not returned
	     * to its natural phase (we need that in order
	     * to cleanly transition to the low-cost screen).
	     *
	     * NB: This code assumes you don't use CYCLE_FADER2
	     * or CYCLE_FADER3 for anything except the final
	     * exit, because the "exit to completely black"
	     * code doesn't use them (hence doesn't reset
	     * their cy_Halted flags).
	     */
	    BOOL transitioning = FALSE;
	    LONG i;
	    for ( i = CYCLE_FADER1; i <= CYCLE_FADER3; i++ )
	    {
		if ( ( CDUIBase->cb_Cycles[ i ].cy_Effect ) &&
		    ( !CDUIBase->cb_Cycles[ i ].cy_Halted ) )
		{
		    transitioning = TRUE;
		}
	    }
	    if ( CDUIBase->cb_Cycles[ CYCLE_AMIGABORDER ].cy_Phase )
	    {
		transitioning = TRUE;
	    }
	    if ( !transitioning )
	    {
		proceed = FALSE;
	    }
	}

	if ( !broken )
	{
	    ULONG sigs;
	    WORD cyclenum;
	    struct ColorCycle *cycle;

	    if ( sigs = (SetSignal(0,0) & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)))
	    {
		broken = TRUE;
		/* Waiters are liable to turn into shooters when they expire,
		 * so in preparation for exit, let's halt all waiters in their
		 * tracks
		 */
		for ( cyclenum = 0; cyclenum < NUM_CYCLES; cyclenum++ )
		{
		    cycle = &CDUIBase->cb_Cycles[ cyclenum ];
		    if ( ( cycle->cy_Effect ) && ( cycle->cy_Effect->ce_Type == CE_WAITER ) )
		    {
			cycle->cy_Halted = TRUE;
		    }
		}
		if ( sigs & SIGBREAKF_CTRL_C )
		{
		    /* Fade completely to black, so just use a single fader to
		     * bring all the colors down.  We also halt the Amiga
		     * Border color cycling, because that must arrive in
		     * its original color ordering before exit is permitted.
		     */
		    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FADER1 ], &FadeOutEffect, 0, 256, 0 );
		    CDUIBase->cb_Cycles[ CYCLE_AMIGABORDER ].cy_Halted = TRUE;
		}
		if ( sigs & SIGBREAKF_CTRL_D )
		{
		    /* Fade all colors except those which are used in the low-cost
		     * screen.  Those colors are 64-77, and 80-96, 96-127
		     */
		    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FADER1 ], &FadeOutEffect, 0, 64, 0 );
		    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FADER2 ], &FadeOutEffect, 78, 2, 0 );
		    initCycle( &CDUIBase->cb_Cycles[ CYCLE_FADER3 ], &FadeOutEffect, 128, 128, 0 );
		    CDUIBase->cb_Cycles[ CYCLE_AMIGABORDER ].cy_Halted = TRUE;
		}
	    }
	}
    }

    Forbid();
    Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
    CDUIBase->cb_CyclerTask = NULL;
}
