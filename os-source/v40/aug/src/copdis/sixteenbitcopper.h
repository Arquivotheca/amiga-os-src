/****************************************************************************
*
*	$Id: SixteenBitCopper.h,v 39.2 92/11/20 17:41:19 Jim2 Exp $
*
*****************************************************************************
*
*	$Log:	SixteenBitCopper.h,v $
 * Revision 39.2  92/11/20  17:41:19  Jim2
 * Supports AAA sixteen bit lists.
 * 
 * Revision 39.1  92/11/18  10:36:20  Jim2
 * First Release - works with remote wack.
 *
*
*****************************************************************************/

#ifndef SIXTEENBITCOPPER_H
#define SIXTEENBITCOPPER_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifndef VERSION
    #ifdef AA_CHIPS
	#include "copdis_rev.h"
    #endif
    #ifdef AAA_CHIPS
	#include "copdisAAA_rev.h"
    #endif
#endif

struct copinstruction
{
	UWORD ir1;
	UWORD ir2;
};


extern VOID ICopDis16(struct CopIns * Walking, struct ViewPort *CurrentVP, BOOL Comments, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst);

extern VOID copdis16(struct copinstruction * coplist, BOOL comm, BOOL data, BOOL nocolours, BOOL ShowOpcode, BOOL ShowConst);
#endif