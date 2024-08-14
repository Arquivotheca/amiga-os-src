/******************************************************************************
*
*	$Id: drawvect.c,v 39.2 92/10/06 13:51:01 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <hardware/dmabits.h>
#include "/sane_names.h"
#include "c.protos"

/*#define DEBUG*/
/*#define DEBUGALL*/
#include "cdrawinfo.h"

extern struct AmigaRegs custom;

void __regargs draw_vect(rp,cdi,x0,y0,dot_count,e,fudge)
struct cdraw_info *cdi;
struct RastPort *rp;
int x0,y0,dot_count;
int e;
WORD fudge;
{
    int i=0;
    UWORD bit_position;
    struct BitMap *bm;
    int offset;
    UWORD con0,con1;
    struct AmigaRegs *io;
    ULONG   BltSize;

#ifdef DEBUG
#ifndef DEBUGALL
    if ( (y0 < 0) || (x0 < 0))
#endif
    {
	printf("entering drawvect\n");
	printf("rp=%lx,x0=%ld,y0=%ld,dotcount=%ld,e=%ld\n",
	    rp,x0,y0,dot_count,e);
	printf("CALL DALE\n");
	printf("to continue just hit control d\n");
	Debug();
    }
#endif

    bm = rp->BitMap;
    bit_position = x0 & 15;
    offset = IMulU(y0,bm->BytesPerRow) + (x0>>3);

    con1 = cdi->con1|LINEMODE;
    if (e < 0)  con1 |= SIGNFLAG;
    e = e + e;
    /* not worrying about one dot for now */
    io = &custom;
	
    /* con1 |= rp->linpatcnt<<12; */
	/* bart - 07.01.86 - adjust linpatcnt*/

/*    con1 |=
	(rp->linpatcnt-((cdi->xmajor)?(x0 - rp->cp_x):( (y0 + fudge) - rp->cp_y)))<<12;*/

	i = ((cdi->xmajor)?(x0 - rp->cp_x):(y0 - rp->cp_y)) + fudge;
	if (i < 0)
		i = -i;
	con1 |= ((rp->linpatcnt - i) << 12);	/* spence - Dec  5 1990 */

    dot_count++;
    BltSize = (dot_count<<6)|2;     /* truncate this vector */
    con0 = DEST|SRCA|SRCC|(bit_position<<12);

    OwnBlitter();
    waitblitdone();

    io->bltmdb = (cdi->absdy<<1)<<1;        /* inc 2 */
    io->bltmda = ((cdi->absdy-cdi->absdx)<<1)<<1; /*inc1;*/
    io->fwmask = -1;
    io->lwmask = -1;
    io->adata = 0x8000;
    io->bltmdc = bm->BytesPerRow;

    /* not worrying about first dot */

    for(i = 0;i < bm->Depth; i++)
    {
		ULONG local_BltSize = BltSize;
		if ( (rp->Mask>>i) & 1)
		{
			waitblitdone();
			io->bltcon0 = con0|rp->minterms[i];
			io->bltcon1 = con1;
			io->bltpta = (UBYTE *)e;
			io->bltptc = bm->Planes[i] + offset;
			io->bltptd = bm->Planes[i] + offset;
			io->bdata = rp->LinePtrn;
			while (local_BltSize > 0x10002)
			{
				io->bltsize = 2;
				local_BltSize -= 0x10000;
				waitblitdone();
			}
			if(local_BltSize >= 0x42)
			{
			    io->bltsize = local_BltSize;
			}
			/* else exact multiple of 1024 -- bart */
		}
    }
    DisownBlitter();
}
