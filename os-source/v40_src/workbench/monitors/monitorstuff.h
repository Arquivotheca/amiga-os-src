/******************************************************************************
*
*	$Id: monitorstuff.h,v 40.1 93/02/19 13:40:43 spence Exp $
*
******************************************************************************/

#include <internal/vp_internal.h>

#define KILLEHB 0x200
#define PLFD_PRIO 0x24
#define BPLCON2 KILLEHB
#define VGAPLFD_PRIO 0x24
#define VGABPLCON2 KILLEHB

#define SDBLED 0
#define NON_LACE 1
#define IS_LACE 2

#define RESN_35 0
#define RESN_70 1
#define RESN_140 2

#define IDNORM 0
#define IDDPF 1
#define IDDPF2 2
#define IDHAM 3
#define IDEHB 4
#define UNIQUEIDS 5
#define AAIDS UNIQUEIDS
#define NONAAIDS (IDDPF2 + 1)
/* ensure that non-AA machines do not have the AA info taking up RAM */
#define LASTID (((XDim < RESN_140) && !IsAA) ? (IDDPF2 + 1) : UNIQUEIDS)

/* FETCH1 is the colour clock number the first possible data fetch can end.
 * 
 * For ECS, this is 0x1b.
 *
 * For AA without the BUG_ALICE_LHS_PROG set (those that run VGAOnly), this is 
 * 0x20 for 35ns, 0x28 for 70ns, 0x38 for 140ns. These are too
 * big a difference, so we limit this to 0x20 (it would be limited anyway when we
 * check later that the dimension is not too small).
 *
 * For AA with the BUG_ALICE_LHS_PROG set, we are pushed 8 colour clocks over to the
 * right, so just subtract another 8 from the other AA values.
 */

#define FETCH1 ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? 0x28 : \
                (((GfxBase->Bugs & BUG_ALICE_LHS) || IsAA) ? 0x20 : 0x1b))

#define NOM_MINX 0
#define NOM_MINY 0

/* MaxOscan width is the max number of pixels that scrolling still works for, up
 * to a certain limit.
 *
 * We also min this out to the Nominal width.
 */

#define MAX_WIDTH max(((((TOTCLKS + 1) - (IsAA ? FETCH1 : 0x1d)) << 3) - (RomBug ? 16 : 0)), NOM_WIDTH)

/* MaxOScan height is much simpler - basically the difference between TOTROWS and
 * MINROW.
 *
 * Remember - there are (TOTROWS + 2) rows every Long Frame, and
 * (TOTROWS + 1) every short frame, so we should be able to add 2 to TOTROWS
 * in our calculations. BUT (surprise surprise) there is a chip bug! If we make
 * the MAX_HEIGHT go all the way to the bottom of the screen, then the sprite
 * data is not fetched on the bottom line (it is a repeat of the previous line).
 * For the default pointer, this means the pointer vanishes on the last line.
 */
#define MAX_HEIGHT (TOTROWS - MINROW)

#define MAX_MINX -((MAX_WIDTH - NOM_WIDTH) >> 1)
#define MAX_MINY -((MAX_HEIGHT - NOM_HEIGHT) >> 1)
#define MAX_MAXX (MAX_MINX + MAX_WIDTH)
#define MAX_MAXY (MAX_MINY + MAX_HEIGHT)

/* VideoOscan width is the maximum number of pixels that can be displayed, scrolling
 * not included.
 */

#define VIDEO_WIDTH ((MAX_DENISE - MIN_DENISE) << 2)
#define VIDEO_HEIGHT MAX_HEIGHT

#define VIDEO_MINX (MAX_MINX - (VIDEO_WIDTH - MAX_WIDTH))
#define VIDEO_MINY (MAX_MINY)
#define VIDEO_MAXX (VIDEO_MINX + VIDEO_WIDTH)
#define VIDEO_MAXY (VIDEO_MINY + VIDEO_HEIGHT)

/* ViewPositionRange */
#define VPRANGEX ((MAX_WIDTH - NOM_WIDTH) >> 2)
#define VPRANGEY (MAX_HEIGHT - NOM_HEIGHT)

/* VPMINX is the first 140ns pixel that can be visible. This is (FETCH1 * 2) for
 * colour-clock to 140ns conversion, + 1 for the .5 colour clock delay that is 
 * always around.
 */
#define VPMINX (((IsAA ? FETCH1 : 0x1d) << 1) + 1)
#define VPMINY MINROW
#define VPMAXX (VPMINX + VPRANGEX)
#define VPMAXY (VPMINY + VPRANGEY)
#define VPX (VPMINX + (VPRANGEX >> 1))
#define VPY (VPMINY + (VPRANGEY >> 1))


/* MonitorSpec stuff */
#define MAX_DENISE (((TOTCLKS + 1) << 1) + 1)
#define MIN_DENISE (((IsAA ? 0x20 : 0x1d) << 1) + 1)

#define HBSTRT 1
#define HBSTOP ((MIN_DENISE + 1) >> 1)
#define HSSTRT (((HBSTOP - HBSTRT) / 3) + HBSTRT)
#define HSSTOP ((((HBSTOP - HBSTRT) / 3) << 1) + HBSTRT)
#define VBSTRT 0
#define VBSTOP (MINROW * TOTCLKS)
#define VSSTRT ((MINROW / 3) * TOTCLKS)
#define VSSTOP (((MINROW / 3) << 1) * TOTCLKS)
