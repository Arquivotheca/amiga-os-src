/****************************************************************************
*
*	$Id: aaFormatFunctions.h,v 39.1 92/11/18 10:32:27 Jim2 Exp $
*
*****************************************************************************
*
*	$Log:	aaFormatFunctions.h,v $
 * Revision 39.1  92/11/18  10:32:27  Jim2
 * First Release - works with remote wack.
 * 
*
*****************************************************************************/

#ifndef AAFORMATFUNCTIONS_H
#define AAFORMATFUNCTIONS_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifndef VERSION
	#include "CopDis_rev.h"
#endif

extern BOOL LocState (UWORD Field);
extern VOID ResetAADisplay (VOID);
extern UWORD SetField (UWORD Field);

extern VOID AADisplayStart (UWORD This);		/* 08E */
extern VOID AADisplayStop (UWORD This);                 /* 090 */
extern VOID AADataFetch (UWORD This);			/* 092, 094 */
extern VOID AABPMiscControl (UWORD This);               /* 100 */
extern VOID AAHScroll (UWORD This);			/* 102 */
extern VOID AABPPriority (UWORD This);			/* 104 */
extern VOID AABPEnhance (UWORD This);	                /* 106 */
extern VOID AABPModulo (UWORD This);			/* 108, 10A */
extern VOID AASpriteStart (UWORD This);			/* 140, 148, 150, 158, 160, 168, 170, 178 */
extern VOID AAXDisplay (UWORD This);			/* 1E4 */
extern VOID AAFetch (UWORD This);			/* 1FC */

#endif
