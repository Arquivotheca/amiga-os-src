/* :ts=4
*
*	decompDLTAX.c
*
*	William A. Ware						CC24
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <proto/exec.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>

#include "title.h"

VOID __asm DecompDLTAXPlane( 	register __a0 UBYTE *indata,
				register __a1 UBYTE *outdata,
				register __a2 UWORD *ytable,
				register __d0 WORD  col,
				register __d1 WORD  bpr,
				register __d2 WORD  rows );

#define DECOMP_SHUTDOWN		0
#define DECOMP_CD			1
#define DECOMP_BM			2

UWORD *AllocYTable( WORD bpr, WORD rows )
{
	UWORD *table,offset = 0;
	UWORD *tptr;
	
	if (table = AllocMem( rows * sizeof( UWORD ),MEMF_PUBLIC ))
	{
		for( tptr = table; --rows > 0; offset += bpr ) *tptr++ = offset;
	}
	return table;
}


void FreeYTable( UWORD *tbl,WORD rows )
{
	FreeMem( tbl, rows+rows );
}


LONG DecompDLTAX(	UBYTE *data,
					WORD bpr,
					struct BitMap *bm,
					UWORD *ytable )
{
	register LONG	*ddata, offset;
	register int	i;
	register UWORD	*dest;
	UWORD			*tempytable = NULL;

	if (!bpr) bpr = bm->BytesPerRow;

	if (!ytable) 
	{
		if (!(tempytable = AllocYTable( bm->BytesPerRow, bm->Rows )))
			return 1;
		ytable = tempytable;
	}

	ddata = (LONG *)data;
	for (i = bm -> Depth;  i--; ) 
	{
		if (offset = ddata[i]) 
		{
			dest = (UWORD *) bm -> Planes[i];
			
			
			DecompDLTAXPlane(((UBYTE *)ddata)+offset, (UBYTE *)dest, 
								ytable, bpr, bm->BytesPerRow, bm->Rows );
		}
	}
	if(tempytable) FreeYTable( tempytable,bm->Rows );
	
	return 0;
}


//===========================================================================

//----------------------------------------------------------------------------
#define	GfxBase		(td->GfxBase)
#define DeBoxBase	(td->DeBoxBase)


WORD __far CDCoords[] =
{
	589,418, 580,415, 570,411, 561,408, 551,405, 542,401, 532,398, 523,395,
	513,391, 504,388, 495,385, 485,381, 476,378, 466,375, 457,371, 447,368,
	438,365, 428,361, 419,358, 410,355, 400,351, 391,348, 381,345, 372,341,
	362,338, 353,335, 343,331, 334,328, 325,325, 317,322, 309,319, 302,317,
	295,314, 289,312, 283,310, 278,308, 273,306, 268,305, 264,303, 261,302,
	257,301, 255,300, 253,299, 251,299, 250,298, 249,298, 249,298, 249,298,
	249,298, 249,298,
};


void AnimCD( register struct TData *td )
{
	WORD temp;

	if (((td->CDFrame == 0) && (td->CDState == CDSTATE_STARTWAIT)) ||
		((td->CDFrame == 47) && (td->CDState == CDSTATE_WAIT)))
	{
		if (td->CDState == td->NextCDState) return;
		td->CDState = td->NextCDState;
//		if (td->CDState == CDSTATE_ROTATE) td->CDFrame++;
	}

	if ((td->CDState == CDSTATE_ROTATE) || 
		(td->CDState == CDSTATE_LEAVE))
	{
		td->CDFrame--;
		if ((td->CDFrame == 45) && (td->CDState == CDSTATE_ROTATE))
			td->CDFrame = 57;
		if (td->CDFrame == 0) 
			td->CDState = td->NextCDState = CDSTATE_STARTWAIT;
	}

	temp = td->DecompFrame ^ 1;
	WaitBOVP( &td->Vp );
	DecompDLTAX( ((UBYTE *)&SAnim)+SAnim.CDAnim[td->CDFrame][temp],0,
					&td->SpriteBm[temp],td->YTable );
	temp ^= 1;
	WaitBOVP( &td->Vp );
	DecompDLTAX( ((UBYTE *)&SAnim)+SAnim.CDAnim[td->CDFrame][temp],0,
					&td->SpriteBm[temp],td->YTable );

	if (td->CDState == CDSTATE_START)
	{
		SetSpriteGroup( td, SAnim.CD,CD_SECCOUNT,
						CDCoords[td->CDFrame*2]/2-14,CDCoords[td->CDFrame*2+1]+5);
		SetSpriteGroup( td, SAnim.Number,2,235,CDCoords[td->CDFrame*2+1]+7 );
		td->CDFrame++;
	}
	
	if (td->CDFrame == 47)
	{
		td->CDState = td->NextCDState;
	}
}

BackDecompBitMap( struct TData *td, UBYTE *data, struct BitMap **bm )
{
	struct BMInfo	*bmi;

	if (*bm) return 0;

	if (!(bmi = DecompBMInfo( NULL,NULL,data ))) TERR(TERR_MEMORY);
	if (!(*bm = AllocBitMap(	bmi->bmi_Width,bmi->bmi_Height,
								bmi->bmi_Depth,
						 		NULL, NULL )))
		goto err;
	if (DecompBitMap( NULL, data, *bm, NULL ))
	FreeBMInfo( bmi );
	return 0;
err:
	if (*bm) FreeBitMap( *bm );
	*bm = NULL;
	FreeBMInfo( bmi );
	TERR( TERR_MEMORY );
}
