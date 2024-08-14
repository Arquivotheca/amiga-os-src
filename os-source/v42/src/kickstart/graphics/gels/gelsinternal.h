/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gelsinternal.h,v 42.0 93/06/16 11:19:34 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	gelsinternal.h,v $
 * Revision 42.0  93/06/16  11:19:34  chrisg
 * initial
 * 
 * Revision 42.0  1993/06/01  07:21:18  chrisg
 * *** empty log message ***
 *
*   Revision 39.0  91/08/21  17:34:50  chrisg
*   Bumped
*   
*   Revision 37.1  91/01/30  11:12:00  spence
*   appended struct PosCtlData for new MoveSprite() behaviour
*   
*   Revision 37.0  91/01/07  15:30:42  spence
*   initial switchover from V36
*   
*   Revision 33.4  90/07/27  16:39:02  bart
*   id
*   
*   Revision 33.3  90/03/28  09:27:28  bart
*   *** empty log message ***
*   
*   Revision 33.2  88/11/16  09:44:37  bart
*   *** empty log message ***
*   
*   Revision 33.1  88/11/16  09:02:09  bart
*   blitSizV , blitSizH
*   ,
*   
*   Revision 33.0  86/05/17  15:24:05  bart
*   added to rcs for updating
*   
*
******************************************************************************/


/******************************************************************************
 *
 * include file for AMIGA GELS (Graphics Elements)
 *
 *  Confidential Information: Amiga Computer, Inc.
 *  Copyright (c) Amiga Computer, Inc.
 *
 *                  Modification History
 *  date    :   author :    Comments
 *  -------     ------      ---------------------------------------
 *  8-24-84     Dale        added this header file
 *  9-28-84     -=RJ=-      for GELS16 added Bob.h to this file
 *                          made name and declaration changes
 *
 *****************************************************************************/


struct gelRect
/* GEL-blit source/destination rectangles */
{
    WORD rX, rY;        /* starting positions */
    WORD rW, rH;        /* width & height of source/destination */
    WORD rRealWW;       /* real word width of block this rect points to */
    char *rAddr;        /* pointer to source image or dest address */
};

#include <hardware/blit.h>


#define FILLSHADOW (ABC|ABNC|ANBC|ANBNC | NABC|NANBC | SRCA | SRCC | DEST)
#define CLEARSHADOW (NABC|NANBC | SRCA | SRCC | DEST)
#define BLOCKCUT (ABC|ABNC | NABC|NANBC | SRCB | SRCC | DEST)
#define SHADOWCUT (ABC|ABNC | NABC|NANBC | SRCA | SRCB | SRCC | DEST)

struct blissObj
{
    struct bltnode *nextBlit;
    WORD (*blitRoutine)();
    BYTE blitStat;
    BYTE blitCnt;
    WORD blitSize;
    WORD beamSync;
    WORD (*cleanUp)();
/* that's all for the system-required memory locations */

    WORD pPick, pOnOff;
    BYTE blissIndex;
    BYTE srcIndex;
    WORD fwm, lwm, pbcn0, minterm, bcn1;
    WORD bmdsrc, bmddst;
    WORD blitType;
    BYTE writeMask;
    BYTE deBug;
    BYTE *asrc;
    BYTE *srcPtr[8];
    BYTE *destPtr[8];
    WORD *shadow;
    ULONG shadowSize;		/* bart - 04.03.86 was SHORT shadowSize; */
    struct Task *whoSentMe;
    struct blissObj	*Next;
    WORD blitSizV;
    WORD blitSizH;
};

#define BLISS_BLOCK_MAXCOUNT 8

struct blissBlock
{
    struct blissObj	blissObj[BLISS_BLOCK_MAXCOUNT];
    UBYTE		CurCount;
    UBYTE 		MaxCount;
    struct blissBlock  *NextBlock; 
};

struct PosCtlData
{
	APTR	Address;		/* address of ss->posctldata */
	ULONG	Data;			/* data to poke */
};
