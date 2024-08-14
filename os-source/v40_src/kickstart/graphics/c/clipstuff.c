/******************************************************************************
*
*	$Id: clipstuff.c,v 39.11 92/08/06 11:26:51 spence Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include "/displayinfo.h"
#include "/gfx.h"
#include "/gfxbase.h"
#include "/display.h"
#include "/copper.h"
#include "/view.h"
#include "/monitor.h"
#include "/macros.h"
#include "/vp_internal.h"

#include "c.protos"
#include "/d/d.protos"

/*#define DEBUG*/

#ifdef DEBUG
#define D(x) {{x};}
#else
#define D(x)
#endif

ULONG getclipstuff(struct View *v, struct ViewPort *vp, struct BuildData *bd)
{
    struct ViewPortExtra *vpe = NULL;
    ULONG mode = RealVPModeID(vp, v);
    ULONG err;
    struct VecInfo vinfo;
    UWORD count;
    WORD fmode;

    D(kprintf("In getclipstuff(). mode = 0x%lx\n", mode);)
    D(kprintf("bd at 0x%lx, Flags = 0x%lx, Offset = 0x%lx\n", bd, bd->Flags, bd->Offset);)

    /* we have to call CalcFMode() as ScrollVPort() is a populatr way of
     * double-buffering, and the bitplanes may not be aligned.
     */

    if ((vp->ColorMap) && (vp->ColorMap->Type) && (vpe = vp->ColorMap->cm_vpe))
    {
	if ((fmode = CalcFMode(vp)) != (WORD)-1)
	{
		D(kprintf("Fmode = 0x%lx\n", fmode);)
	}
	else
	{
		return(BAD_CLIPSTUFF);	/* TFB - unaligned. */
	}
    }

    if (GetDisplayInfoData(NULL, (UBYTE *)&vinfo, sizeof(struct VecInfo), DTAG_VEC, mode))
    {
	struct ProgInfo *pinfo = (struct ProgInfo *)vinfo.Data;
	char **makeit;
	int (*nextone)();

	makeit = (GetVecTable(vp, &vinfo))->BuildVP;

	/* first in the list is the Initialisation code, last one
	 * is the cleanup code.
	 */
	nextone = (int (*)())(*makeit++);
	if ((err = ((*nextone)(v, vp, &vpe, pinfo, bd) & 0xFFFF)) == MVP_OK)
	{
		for (count = pinfo->ScrollVPCount; count; count--)
		{
			nextone = (int (*)())(*makeit++);
			(*nextone)(v, vp, vpe);
		}

	}
	/* look for the cleanup routine */

	while (*makeit)
		nextone = (int (*)())(*makeit++);

	(*nextone)(v, vp, vpe);
    }
    else
    {
	D(kprintf("No ProgInfo\n");)
	return(BAD_CLIPSTUFF);
    }

    return(GOOD_CLIPSTUFF);
}
