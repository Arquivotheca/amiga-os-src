/******************************************************************************
*
*   $Id: great.c,v 42.0 93/06/16 11:16:43 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>

#include "/copper.h"
#include "/macros.h"
#include "/displayinfo_internal.h"
#include "/displayinfo.h"
#include "/gfx.h"
#include "/gfxbase.h"
#include "/sane_names.h"
#include "gfxprivate.h"
#include "c.protos"
#include "/d/d.protos"
#include "/gfxpragmas.h"

/****** graphics.library/SetChipRev *******************************************
*
*   NAME
*	SetChipRev -- turns on the features of a Chip Set  (V39)
*
*   SYNOPSIS
*	chiprevbits = SetChipRev(ChipRev)
*	                           d0
*
*	ULONG SetChipRev(ULONG);
*
*   FUNCTION
*	Enables the features of the requested Chip Set if available,
*	and updates the graphics database accordingly.
*
*   INPUTS
*	ChipRev - Chip Rev that you would like to be enabled.
*
*   RESULT
*	chiprevbits - Actual bits set in GfxBase->ChipRevBits0.
*
*   NOTES
*	This routine should only be called once. It will be called by the system
*	in the startup-sequence, but is included in the autodocs for authors
*	of bootblock-games that wish to take advantage of post-ECS features.
*
*   SEE ALSO
*	<graphics/gfxbase.h>
*
*******************************************************************************/

/*#define DEBUG*/
#ifdef DEBUG
#define D(x) {x};
#else
#define D(x)
#endif

extern struct AmigaRegs custom;

ULONG __asm SetChipRev(register __d0 ULONG want)
{
    struct AmigaRegs *io;
    DisplayInfoHandle handle;
    ULONG ID = INVALID_ID;
    APTR buf;
    BOOL isehb = FALSE;
    UBYTE agnus_chip_id;
    UBYTE denise_chip_id;
    UBYTE have = 0;
    UBYTE set;

    io = &custom;

    agnus_chip_id = (io->vposr>>8) & 0x7f;
    denise_chip_id = get_denise_id();
    D(kprintf("agnus = 0x%lx, denise = 0x%lx\n", agnus_chip_id, denise_chip_id);)

/* denise ID bits according to Raible:
 *	orig	XXXX XXXX
 *	ecs     1111 1100
 *	ecs2	1110 1100	ecs fixed for video toaster
 *	lisa    1111 1000
 *	AAA	???? 0100
 *
 *	B0=unused
 *	B1=does this chip support ECS screen modes?
 *	B2=does this chip support AA screen modes?
 *
 * memory type is defined in bits 8 and 9 of io->deniseid.
 *			Bit9 Bit8
 * NmlCAS, 16BitBus	 0    0		1x
 * NmlCAS, 32BitBus	 0    1		2x
 * DblCAS, 16BitBus	 1    0		2x
 * DblCAS, 32BitBus	 1    1		4x
 *
*/

    /* Let's see what we have... */
    if (!(denise_chip_id & 0x04))
    {
    	/* we have a Lisa */
	have |= (GFXF_AA_LISA | GFXF_AA_MLISA);
    }
    if (!(denise_chip_id & 0x02))
    {
    	have |= GFXF_HR_DENISE;
    }
    if (agnus_chip_id & 0x20)
    {
    	have |= GFXF_HR_AGNUS;
    }
    if (agnus_chip_id & 0x02)	/* alice bit set */
    {
	have |= GFXF_AA_ALICE;
    }

    /* if we want the best chips possible (from setpatch or enlighten), then
     * we should see if a selection was made from bootmenu.
     * No bootmenu selection had been made if GBASE->WantChips == BEST,
     * so read the saved option from BattMem.
     */

    if (want == SETCHIPREV_BEST)
    {
	(want = GBASE->WantChips);
    }
    else
    {
	/* Cannot go from a AA setting down to ECS or A */
	if ((GBASE->ChipRevBits0 & SETCHIPREV_AA) == SETCHIPREV_AA)
	{
		return(SETCHIPREV_AA);
	}
    }

    want |= GFXF_AA_MLISA;


    D(kprintf("have = 0x%lx, want = 0x%lx\n", have, want);)
    D(kprintf("crb = 0x%lx\n", GBASE->ChipRevBits0);)

    set = (have & want);

    if (set != GBASE->ChipRevBits0)
    {
	GBASE->ChipRevBits0 = set;
	if (set & GFXF_AA_LISA)
	{
	    	GBASE->MemType = (((~(io->deniseid)) & 0x0300) >> 8);
		GBASE->ColorMask=0xff000000;
	}
	else GBASE->ColorMask=0xf0000000;

	if (!(set & GFXF_HR_DENISE))
	{
		/* Put in 'A' mode. This requires removing the AA '1-blank-line'
		 * kludge from copinit.
		 */
		extern UWORD copinit_mid_a[];
		CopyWords(copinit_mid_a, &(GBASE->copinit->diagstrt[10]), 16);
	}

	/* We also need to update the database. ALICE determines the maxdepths of
	 * the display modes, LISA the sprite characteristics.
	 */

	if (buf = (APTR)AllocVec(MAX(MAX(sizeof (struct VecInfo), sizeof(struct DimensionInfo)), sizeof(struct DisplayInfo)), MEMF_CLEAR))
	{
		while ((ID = gfx_NextDisplayInfo(ID)) != INVALID_ID)
		{
			if (handle = (DisplayInfoHandle)gfx_FindDisplayInfo(ID))
			{
				D(kprintf("ID = 0x%lx\n", ID);)
				if (ID & MONITOR_ID_MASK)	/* don't do it twice for default */
				{
					/* change the ProgInfo */
					if (gfx_GetDisplayInfoData(handle, buf, sizeof(struct VecInfo), DTAG_VEC, ID))
					{
						correct_driver((struct RawVecInfo *)buf);
						gfx_SetDisplayInfoData(handle, buf, sizeof(struct VecInfo), DTAG_VEC, ID);
					}

					/* change the DisplayInfo */
					if (gfx_GetDisplayInfoData(handle, buf, sizeof(struct DisplayInfo), DTAG_DISP, ID))
					{
						isehb = ((((struct DisplayInfo *)buf)->PropertyFlags) & DIPF_IS_EXTRAHALFBRITE);
						if (set & GFXF_AA_LISA)
						{
							make_aa((struct RawDisplayInfo *)buf);
							gfx_SetDisplayInfoData(handle, buf, sizeof(struct DisplayInfo), DTAG_DISP, ID);
						}
					}
					if (gfx_GetDisplayInfoData(handle, buf, sizeof(struct DimensionInfo), DTAG_DIMS, ID))
					{
						limit_blit((struct RawDimensionInfo *)buf);
						/* change the DimensionInfo for non-EHB modes */
						if (!isehb)
						{
							increase_depth((struct RawDimensionInfo *)buf);
						}
						if ((set & GFXF_AA_LISA) &&

						/* In a 2 cycle mode, there are 8 less pixels per line
						 * than twice the number in a 4 cycle hires mode.
						 * This is reflected in the database when unpacked.
						 * However, on a AA machine, SuperHires will execute
						 * in at least a 4 cycle mode (for 2x), so increment
						 * the range accordingly.
						 */
						    (((ID & (MONITOR_ID_MASK | SUPER_KEY)) == (NTSC_MONITOR_ID | SUPER_KEY)) || 
						     ((ID & (MONITOR_ID_MASK | SUPER_KEY)) == (PAL_MONITOR_ID | SUPER_KEY))))
						{
							((struct DimensionInfo *)buf)->MaxOScan.MaxX = (1447 + ((struct DimensionInfo *)buf)->MaxOScan.MinX);
							((struct DimensionInfo *)buf)->VideoOScan.MaxX = (1471 + ((struct DimensionInfo *)buf)->VideoOScan.MinX);
							D(kprintf(">>> Fudging for SHires,\n");)
							D(kprintf("Max = %ld, Video = %ld\n",((struct DimensionInfo *)buf)->MaxOScan.MaxX, ((struct DimensionInfo *)buf)->VideoOScan.MaxX);)
						}
						gfx_SetDisplayInfoData(handle, buf, sizeof(struct DimensionInfo), DTAG_DIMS, ID);
					}
				}
			}
		}
		all_db_avail();	/* check availability of all modes. */
		FreeVec(buf);
	}
    }

    D(kprintf("setting 0x%lx\n", set);)
    return((ULONG)set);		
}
