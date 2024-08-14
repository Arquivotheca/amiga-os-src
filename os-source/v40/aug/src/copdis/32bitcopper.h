/****************************************************************************
*
*	$Id $
*
*****************************************************************************
*
*	$Log $
*
*****************************************************************************/

#ifndef THIRTYTWOBITCOPPER_H
#define THIRTYTWOBITCOPPER_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifndef VERSION
    #include "copdisAAA_rev.h"
#endif

struct CopInstruction
{
	ULONG ir1;
	ULONG ir2;
};


extern VOID ICopDis32(struct CopIns * Walking, struct ViewPort *CurrentVP, BOOL Comments, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst);

extern VOID CopDis32(struct CopInstruction * CopList, BOOL Comments, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst);
#endif