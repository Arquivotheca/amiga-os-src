/* spr.h -- sprite rastport declarations
 * 
 * Copyright (c) 1988, I and I Computing and Commodore-Amiga, Inc.
 *
 * Executables based on this information may be used in software
 * for Commodore Amiga computers.  All other rights reserved.
 *
 * This information is provided "as is"; no warranties are made.
 * All use is at your own risk, and no liability or responsibility is assumed.
 */

/* wanted it to have 'spr' as an acronym.
 * the setup is twisted, so why not the name?
 */
struct SpritePortRast {
	struct SimpleSprite	spr_SSprite;
	struct RastPort		spr_RPort;		/* only use leftmost 16 pixels	*/
	struct BitMap		spr_BMap;
	UWORD			*spr_Data;		/* chip ram sprite data			*/
	int			spr_Size;		/* safe and easy free			*/
	int			spr_SNum;		/* sprite number				*/
	UWORD			spr_Flags;		/* see below					*/
};

/* I didn't legally GetSprite, so don't FreeSprite() */
#define SPRF_FORCED	(1)

