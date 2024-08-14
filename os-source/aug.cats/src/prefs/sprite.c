/* spr.c -- easy sprites, sprite rastports
 *
 * Copyright (c) 1988, I and I Computing and Commodore-Amiga, Inc.
 * 
 * Executables based on this information may be used in software
 * for Commodore Amiga computers.  All other rights reserved.
 *
 * This information is provided "as is"; no warranties are made.
 * All use is at your own risk, and no liability or responsibility is assumed.
 */

#include "sysall.h"
#include "spr.h"

#define D( x )	;

struct SpritePortRast
{
    struct SimpleSprite spr_SSprite;
    struct BitMap spr_BMap;
    struct RastPort spr_RPort;
    SHORT *spr_Data;
    SHORT spr_Size;
    SHORT spr_SNum;
    LONG spr_Flags;
};

/* extra memory (in bytes ) allocated for control info and terminator	*/
#define SPMEM_EXTRA		( 2 * sizeof (short int) + 2 *  sizeof (short int))
#define SPMEM_BYTES( height )	(SPMEM_EXTRA+2*sizeof (short int) *(height))

/* word index to reserved at bottom	*/
#define SPMEM_RSRVD( height ) 	(2 + 2 * (height))

/*
 * Allocates sprite requested, creates bitmap and
 * rastport that can be used to render into sprite using
 * standard graphics calls (Dale's trick).
 *
 * passed an already allocated SpritePortRast structure
 * do not render in any but the leftmost 16 pixel columns
 *
 * returns sprite number
 * if force, than unless you asked for "any sprite" (spritepick == -1),
 * we'll steal your choice.
 */
initSPR( spr, height, spritepick, force )
struct SpritePortRast	*spr;
{
	int 		spnum;
	UWORD		*sprdata;

	D( printf("iSPR: spr: %lx\n", spr) );
	D( printf("iSPR: spritepick: %d\n", spritepick) );
	D( printf( "height: %d, spmem_bytes: %d\n", height, SPMEM_BYTES(height)));

	if ( ((spnum = GetSprite( &spr->spr_SSprite, (LONG) spritepick )) != -1 )
		 || ((spritepick != -1) && force) )
	{
		/* I'm ignoring GetSprite	*/
		if ( spnum == -1 )
		{
			D( printf("force sprite.\n") );
			spr->spr_SSprite.num = spnum = spritepick;
			spr->spr_Flags |= SPRF_FORCED;
		}
		D( else printf("got sprite: spnum: %d\n", spnum ) );

		/* allocate chip mem	*/
		sprdata = (UWORD *)
			AllocMem( (LONG) SPMEM_BYTES( height ), (LONG)MEMF_CHIP|MEMF_CLEAR );

		D( printf( "sprdata: %lx\n", sprdata ) );
		if ( spr->spr_Data = sprdata )
		{
			/* setup posctrl terminator	*/
			sprdata[ SPMEM_RSRVD( height ) ] =		0;
			sprdata[ SPMEM_RSRVD( height ) + 1 ] =	0;

			/* set up weird bitmap	*/
			InitBitMap( &spr->spr_BMap, 2L,  32L, (LONG) height );

			spr->spr_BMap.Planes[ 0 ] =	(PLANEPTR) (sprdata + 2);
			spr->spr_BMap.Planes[ 1 ] =	(PLANEPTR) (sprdata + 3);	/* trick */

			/* and rastport	*/
			InitRastPort( &spr->spr_RPort );
			spr->spr_RPort.BitMap = &spr->spr_BMap;

			/* finish setup	*/
			spr->spr_Size = SPMEM_BYTES( height );
			spr->spr_SSprite.height = height;
		}
		else
		{
			D( printf("calling freeSPR\n") );
			freeSPR( spr );
			spnum = -1;
		}
	}
	D( printf("iSPR: pt2: spnum: %d\n", spnum) );

	if ( spnum == -1 ) spr->spr_Data = NULL;
	spr->spr_SNum = spnum;
	return ( spnum );
}

/* note: doesn't free spr structure, just the sprite data	*/
freeSPR( spr )
struct SpritePortRast	*spr;
{
	if ( ( spr->spr_SNum != -1 ) && !( spr->spr_Flags & SPRF_FORCED ) )
	{
		FreeSprite( (LONG) spr->spr_SNum );
	}
	if ( spr->spr_Data != NULL )
	{
		FreeMem( spr->spr_Data, (ULONG) spr->spr_Size );
	}
	spr->spr_Data = NULL;

	spr->spr_SNum = -1;
	spr->spr_Size = 0;
}
