/******************************************************************************
*
*	$Id: setrast.c,v 42.0 93/06/16 11:15:19 chrisg Exp $
*
******************************************************************************/

#define CHRIS_DOWNCODE 1

#ifndef CHRIS_DOWNCODE

/* graphics  kernel routines */
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <hardware/dmabits.h>
#include "/sane_names.h"
#include "/macros.h"
#include <graphics/gfxbase.h>
#include "c.protos"

/*#define DEBUG*/
/*#define QWE*/
#define NO_BIG_BLIT

extern struct AmigaRegs custom;

#ifdef QWE
setrast1(cr,color,w)
register struct RastPort *w;
short color;
struct ClipRect *cr;
{
	struct BitMap *bm;
	UWORD	mask = 1;
    USHORT BltSize;
    long dstoffset,bltbytes;
    UBYTE **p;
    int i;
    register struct AmigaRegs *io = &custom;

	bm = w->BitMap;
	bltbytes = BLTBYTES(cr->bounds.MinX,cr->bounds.MaxX);
	BltSize = ((cr->bounds.MaxY-cr->bounds.MinY+1)<<6) | (bltbytes >> 1);
#ifdef DEBUG
   	printf("cr=%lx\n",cr);
   	printf("Bltbyte = %lx\n",bltbytes);
   	printf("BltSize = %lx\n",BltSize);
   	Debug();
#endif
   	p = bm->Planes;
   	dstoffset = umuls(cr->bounds.MinY,bm->BytesPerRow) + (cr->bounds.MinX>>3);
	i = cr->bounds.MaxX & 0xf;
   	OwnBlitter();
	waitblitdone();
   	io->bltmdd = bm->BytesPerRow - bltbytes;
   	io->bltmdc = bm->BytesPerRow - bltbytes;
	io->bltcon1 = 0;
   	io->bltcon0 = 0;    /* set shift to 0 for adata put*/
   	io->adata = 0xffff;
	io->fwmask = (USHORT)0xffff >> (cr->bounds.MinX&0xF);
	io->lwmask = (USHORT)0xffff << ( 15 - i);
	for(i=0;i<bm->Depth;i++)
	{
   		if (w->Mask  & mask)
   		{
			waitblitdone();
			io->bltcon0 = DEST|SRCC | NANBC|NABC |
 				(color & mask ? ABC|ABNC|ANBC|ANBNC  : 0 );
			io->bltptd = *p + dstoffset;
			io->bltptc = *p + dstoffset;
			io->bltsize = BltSize;
   		}
		mask <<= 1;
   		p++;
	}
	DisownBlitter();
}

bigsetrast1(cr,color,w)
register struct RastPort *w;
short color;
register struct ClipRect *cr;
{
	struct BitMap *bm;
	UWORD	mask = 1;
    USHORT BltHSize,BltVSize;
    long dstoffset,bltbytes;
    UBYTE **p;
    int i;
    register struct AmigaRegs *io = &custom;
    USHORT BltSize;
	struct ClipRect newcr;

	bm = w->BitMap;
	bltbytes = BLTBYTES(cr->bounds.MinX,cr->bounds.MaxX);
	BltHSize = bltbytes >> 1;
	BltVSize = cr->bounds.MaxY-cr->bounds.MinY+1;
	if ( (GBASE->ChipRevBits0 & GFXF_BIG_BLITS) == 0) /* old blitter? */
	{
		if ( (BltVSize > 1024) | (BltHSize & 0xFFC0) )
		{	/* too big! */
			/* subdivide and recurs */
			newcr = *cr;	/* get all goodies */
			do
			{
				unsigned short saverowsize;
				unsigned short rows_to_do;
				rows_to_do = SHORTMIN(1024,BltVSize);
				saverowsize = cr->bounds.MaxX - cr->bounds.MinX + 1;
				/* recover original x size */
				newcr.bounds.MinX = cr->bounds.MinX;
				newcr.bounds.MaxX = cr->bounds.MaxX;
				newcr.bounds.MaxY = newcr.bounds.MinY + rows_to_do - 1;
				do
				{
					unsigned short columns_to_do;
					columns_to_do = SHORTMIN(1008,saverowsize);
					newcr.bounds.MaxX = newcr.bounds.MinX+columns_to_do-1;
					bigsetrast1(&newcr,color,w);
					saverowsize -= columns_to_do;
					newcr.bounds.MinX += columns_to_do;
				} while (saverowsize);
				newcr.bounds.MinY += rows_to_do;
				BltVSize -= rows_to_do;
			}	while (BltVSize);
			return;
		}
		BltSize = (BltVSize<<6)|(BltHSize);
	}
#ifdef DEBUG
   	printf("cr=%lx\n",cr);
   	printf("Bltbyte = %lx\n",bltbytes);
   	printf("BltHSize = %lx\n",BltHSize);
   	printf("BltVSize = %lx\n",BltVSize);
   	Debug();
#endif
   	p = bm->Planes;
   	dstoffset = umuls(cr->bounds.MinY,bm->BytesPerRow) + (cr->bounds.MinX>>3);
	i = cr->bounds.MaxX & 0xf;
   	OwnBlitter();
	waitblitdone();
   	io->bltmdd = bm->BytesPerRow - bltbytes;
   	io->bltmdc = bm->BytesPerRow - bltbytes;
	io->bltcon1 = 0;
   	io->bltcon0 = 0;    /* set shift to 0 for adata put*/
   	io->adata = 0xffff;
	io->fwmask = (USHORT)0xffff >> (cr->bounds.MinX&0xF);
	io->lwmask = (USHORT)0xffff << ( 15 - i);
	io->bltsizv = (BltVSize & 0x7fff);
	for(i=0;i<bm->Depth;i++)
	{
   		if (w->Mask  & mask)
   		{
			waitblitdone();
			io->bltcon0 = DEST|SRCC | NANBC|NABC |
 				(color & mask ? ABC|ABNC|ANBC|ANBNC  : 0 );
			io->bltptd = *p + dstoffset;
			io->bltptc = *p + dstoffset;
			if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
				io->bltsizh = (BltHSize & 0x07ff);
			else	io->bltsize = BltSize;
   		}
		mask <<= 1;
   		p++;
	}
	DisownBlitter();
}

#else

bigsetrast1(cr,color,w)
register struct RastPort *w;
short color;
register struct ClipRect *cr;
{
	struct BitMap *bm;
	UWORD	mask = 1;
    USHORT BltHSize,BltVSize;
    long dstoffset,bltbytes;
    UBYTE **p;
    int i;
    register struct AmigaRegs *io = &custom;
    USHORT BltSize;

	bm = w->BitMap;
	bltbytes = BLTBYTES(cr->bounds.MinX,cr->bounds.MaxX);
	BltHSize = bltbytes >> 1;
	BltVSize = cr->bounds.MaxY-cr->bounds.MinY+1;
#ifdef DEBUG
   	printf("cr=%lx\n",cr);
   	printf("Bltbyte = %lx\n",bltbytes);
   	printf("BltHSize = %lx\n",BltHSize);
   	printf("BltVSize = %lx\n",BltVSize);
   	Debug();
#endif
   	p = bm->Planes;
   	dstoffset = umuls(cr->bounds.MinY,bm->BytesPerRow) + (cr->bounds.MinX>>3);
	i = cr->bounds.MaxX & 0xf;
   	OwnBlitter();
	waitblitdone();
   	io->bltmdd = bm->BytesPerRow - bltbytes;
   	io->bltmdc = bm->BytesPerRow - bltbytes;
	io->bltcon1 = 0;
   	io->bltcon0 = 0;    /* set shift to 0 for adata put*/
   	io->adata = 0xffff;
	io->fwmask = (USHORT)0xffff >> (cr->bounds.MinX&0xF);
	io->lwmask = (USHORT)0xffff << ( 15 - i);
	if ( (GBASE->ChipRevBits0 & GFXF_BIG_BLITS) == 0) /* old blitter? */
	{
		BltSize = (BltVSize<<6)|(BltHSize & 63);
	}
	else
	{
		io->bltsizv = (BltVSize & 0x7fff);
	}
	for(i=0;i<bm->Depth;i++)
	{
   		if (w->Mask  & mask)
   		{
			waitblitdone();
			io->bltcon0 = DEST|SRCC | NANBC|NABC |
 				(color & mask ? ABC|ABNC|ANBC|ANBNC  : 0 );
			io->bltptd = *p + dstoffset;
			io->bltptc = *p + dstoffset;
			if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
			{
				io->bltsizh = (BltHSize & 0x07ff);
			}
			else	io->bltsize = BltSize;
   		}
		mask <<= 1;
   		p++;
	}
	DisownBlitter();
}

#endif


#endif
