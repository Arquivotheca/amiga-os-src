/* scale.c
 *
 */

#include "classbase.h"

#define	DB(x)	;

/******************************************************************************
*
* ScaleBM - Copyright (c) 1989,90,91 by MKSoft Development
*
* This code is confidential and property of MKSoft Development
*
******************************************************************************/

void __asm ScaleBitMapLine (register __a0 UWORD *, register __a1 UWORD *, register __d0 UWORD, register __d1 UWORD, register __d7 ULONG);

VOID ScaleBitMap (struct ClassBase *cb, Class *cl, Object *o, struct localData *lod)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    struct BitMap *sbm;
    struct BitMap *dbm;
    ULONG from;
    ULONG to;
    ULONG from_size;
    ULONG to_size;
    ULONG planes;
    ULONG src_count;
    ULONG dst_count;
    ULONG lines;
    UWORD *old;
    UWORD *new;
    WORD error_term;
    WORD error_count;
    WORD full_lines;
    UWORD loop;
    ULONG src_x;
    ULONG bit_start;

    if (lod->lod_SourceBMap)
    {
	sbm = lod->lod_SourceBMap;
	dbm = lod->lod_ZoomBMap;

	from_size = (ULONG)sbm->BytesPerRow;
	to_size   = (ULONG)dbm->BytesPerRow;

	DB (kprintf ("from %ld, to %ld\n", from_size, to_size));

	src_x = si->si_TopHoriz;

	/* Set up the starting x position byte in line */
	bit_start = src_x & 31;
	src_x     = src_x >> 5;
	src_x     = src_x << 2;

	error_count = -(lod->lod_ZoomHeight >> 1);	/* Mid-state error count */
	error_term  = si->si_TotVert % lod->lod_ZoomHeight;
	full_lines  = si->si_TotVert / lod->lod_ZoomHeight;

	for (planes = 0; planes < (ULONG)(MIN (sbm->Depth, dbm->Depth)); planes++)
	{
	    from = (ULONG) (sbm->Planes[planes]);
	    to = (ULONG) (dbm->Planes[planes]);

	    DB (kprintf ("plane %ld, from 0x%lx to 0x%lx\n", planes, from, to));

	    /* Set up starting byte pointer in plane */
	    from += (si->si_TopVert * from_size) + src_x;

	    src_count = lod->lod_ZoomHeight;
	    dst_count = si->si_TotVert;

	    while (src_count--)
	    {
		lines = full_lines;
		error_count += error_term;
		if (error_count >= 0)
		{
		    error_count -= lod->lod_ZoomHeight;
		    lines += 1;
		}

		if (lines--)
		{
		    if (dst_count)
		    {
			dst_count--;
			old = (UWORD *) to;
			ScaleBitMapLine ((UWORD *) from, (UWORD *) to, lod->lod_ZoomWidth, si->si_TotHoriz, bit_start);
			to += to_size;
			new = (UWORD *) to;
		    }


		    while (lines--)
			if (dst_count)
			{
			    dst_count--;
			    for (loop = 0; loop < to_size; loop += 2)
				*new++ = *old++;
			}

		    to = (ULONG) new;
		}

		from += from_size;
	    }
	}
    }
}
