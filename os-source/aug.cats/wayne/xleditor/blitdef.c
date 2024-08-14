/****************

	BlitDef.c

	W.D.L	920509

	Added GetBitMapAttr() stuff 930401

****************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/graphics_protos.h>

#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "cdxl/blitdef.h"
#include "cdxl/debugsoff.h"

/*	// Uncomment to get debug output turned on
#define KPRINTF
#include "cdxl/debugson.h"
*/

IMPORT	struct GfxBase	* GfxBase;

InitBDef( BLITDEF * BDef, struct BitMap * Srcbm, int Srcx, int Srcy,
 struct BitMap * Dstbm, int Dstx, int Dsty,	int inwidth, int inheight,
 UBYTE * mask)
{
	register ULONG		 SrcOffset;
	register ULONG		 DstOffset;

	UWORD		SrcWordStart;
	UWORD		DstWordStart;
	UWORD		newwidth;
	UWORD		SrcWordEnd;
	UWORD		DstWordEnd;
	UWORD		SrcBitStart;
	UWORD		DstBitStart;
	UWORD		SrcBitEnd;
	UWORD		DstBitEnd;
	UWORD		Srcmod;
	UWORD		Dstmod;
	UWORD		shift;
	UWORD		bltshift;
	UWORD		SrcByteWidth;
	UWORD		DstByteWidth;
	UWORD		bltwidth;
	UWORD		size;
	UWORD		firstmask;
	UWORD		lastmask;
	UWORD		othermask;

/* --------------------------------------------------------------------- */
	int width  = inwidth,bmw; 
	int height = inheight,bmh;

	setmem(BDef, sizeof(BLITDEF), 0);

	bmw = GetBitMapAttr( Dstbm, BMA_WIDTH );
	bmh = GetBitMapAttr( Dstbm, BMA_HEIGHT );

	if (Dstx < 0) {
		width	  = inwidth + Dstx;
		Dstx	  = 0;
		Srcx += inwidth - width;
	} else if (Dstx > (bmw - inwidth) ) {
		width	  = bmw - Dstx;
	}


	if (width <= 0) {
		BDef->TotalClip = TRUE;
		return( FALSE );
	}

	if (Dsty < 0) {
		height	 = inheight + Dsty;
		Dsty	 = 0;
		Srcy += inheight - height;
	} else  if (Dsty > bmh - inheight) {
		height	 = bmh - Dsty;
	}

	if (height <= 0) {
		BDef->TotalClip = TRUE;
		return( FALSE );
	}

/* --------------------------------------------------------------------- */

	othermask = 0xFFFF;

	SrcWordStart = Srcx >> 4;
	DstWordStart = Dstx   >> 4;

	newwidth = width - 1;
	SrcWordEnd   = Srcx + newwidth;
	DstWordEnd   = Dstx   + newwidth;

	SrcOffset = Srcy * Srcbm->BytesPerRow + (SrcWordStart << 1);
	DstOffset   = Dsty * Dstbm->BytesPerRow + (DstWordStart << 1);

	SrcBitStart	= Srcx & 0x0F;
	DstBitStart	= Dstx   & 0x0F;

	SrcBitEnd	= SrcWordEnd & 0x0F;
	DstBitEnd	= DstWordEnd & 0x0F;

	SrcByteWidth = (SrcWordEnd >> 4) - SrcWordStart + 1;
	DstByteWidth = (DstWordEnd >> 4) - DstWordStart + 1;

	shift = (DstBitStart - SrcBitStart);

	D(PRINTF("\nSrcByteWidth= %ld, DstByteWidth= %ld, width= %ld, shift= %ld\n",
		SrcByteWidth,DstByteWidth,width,shift);)

	if (shift > 15) {
//		bltwidth += 1;
		shift &= 0x0F;
		DstOffset -= 2;
		othermask = 0x0000;
//		lastmask  = 0xFFFF << (15 - SrcBitEnd);
		D(PRINTF("New shift= %ld\n",shift);)
	} else {
		if (DstByteWidth > SrcByteWidth) {
			lastmask = 0x0;
		} else {
			lastmask  = 0xFFFF << (15 - SrcBitEnd);
		}
	}

	firstmask = 0xFFFF >> SrcBitStart;

	bltwidth  = max( SrcByteWidth, DstByteWidth );

	Srcmod = Srcbm->BytesPerRow - (bltwidth << 1);
	Dstmod   = Dstbm->BytesPerRow   - (bltwidth << 1);

	D(PRINTF("AFWM= 0x%lx, ALWM= 0x%lx, Srcmod= %ld, Dstmod= %ld\n",
		firstmask,lastmask,Srcmod,Dstmod);)

	D(PRINTF("SrcBitStart= %ld, DstBitStart= %ld, SrcBitEnd= %ld, DstBitEnd= %ld\n",
		SrcBitStart,DstBitStart,SrcBitEnd,DstBitEnd);)

	bltshift = shift << 12;
	size = (height << 6) | bltwidth;

//	mask += SrcOffset;

	BDef->bltcon0 = bltshift | SRCB|SRCC|DEST|ABC|ABNC|NABC|NANBC;
	BDef->bltcon1 = bltshift;
	BDef->bltafwm = firstmask;
	BDef->bltalwm = lastmask & othermask;
	BDef->bltbmod = Srcmod;
	BDef->bltcmod = Dstmod;
	BDef->bltdmod = Dstmod;
	BDef->bltsize = size;
	BDef->srceoff = SrcOffset;
	BDef->destoff = DstOffset;

	if ( mask ) {
	    BDef->bltcon0 |= SRCA;
	    BDef->bltamod = Srcmod;
	    mask += SrcOffset;
	} else {
	    BDef->bltadat = (USHORT)~0;
	}

	return( TRUE );

} // InitBDef()


/****************

	BlitBDef

	W.D.L	920509

****************/

BlitBDef( BLITDEF * BDef, struct BitMap *source, struct BitMap *dest,
 UBYTE * Mask )
{
	int		plane, destoff, srceoff, depth;
	REGISTER	struct Custom *Custom = (struct Custom *)0x00dff000;

	if ( BDef->TotalClip )
	    return( FALSE );

	OwnBlitter();

	srceoff = BDef->srceoff;
	destoff = BDef->destoff;

	if ( Mask )
	    Mask += srceoff;

//	depth = min( GetBitMapAttr( source, BMA_DEPTH ), GetBitMapAttr( dest, BMA_DEPTH ) );
	depth = min( source->Depth, dest->Depth );

	WaitBlit();

	Custom->bltamod = BDef->bltamod;
	Custom->bltbmod = BDef->bltbmod;
	Custom->bltcmod = BDef->bltcmod;
	Custom->bltdmod = BDef->bltdmod;
	Custom->bltafwm = BDef->bltafwm;
	Custom->bltalwm = BDef->bltalwm;
	Custom->bltadat = BDef->bltadat;
	Custom->bltcon0 = BDef->bltcon0;
	Custom->bltcon1 = BDef->bltcon1;

	for( plane = 0; plane < depth; plane++ ) {
		WaitBlit();
		Custom->bltapt = (APTR)Mask;
		Custom->bltbpt = (APTR)(source->Planes[plane] + srceoff);
		Custom->bltdpt = Custom->bltcpt = (APTR)(dest->Planes[plane] + destoff);
		Custom->bltsize = BDef->bltsize;
	}

	DisownBlitter();

	return( TRUE );

} // BlitBDef()



