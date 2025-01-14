/*****************************************************************************
*
*	$Id: AAAFormatFunctions.h,v 41.0 92/11/23 11:00:22 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	AAAFormatFunctions.h,v $
 * Revision 41.0  92/11/23  11:00:22  Jim2
 * Initial release.
 * 
*
******************************************************************************/
#ifndef REVSION
    #include "copdisAAA_rev.h"
#endif

#ifndef AAAFORMATFUNCTIONS_H
#define AAAFORMATFUNCTIONS_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

extern VOID AAABeamHigh (ULONG This);			/* 02A */
extern VOID AAABeamLow (ULONG This);			/* 02C */
extern VOID AAACopperControl (ULONG This);		/* 02E */
extern VOID AAABlitterControl0 (ULONG This);		/* 040 */
extern VOID AAABlitterControl1 (ULONG This);		/* 042 */
extern VOID AAABlitterSizeSm (ULONG This);		/* 058 */
extern VOID AAABlitterControl0Short (ULONG This);	/* 05A */
extern VOID AAABlitterSizeMedV (ULONG This);		/* 05C */
extern VOID AAABlitterSizeMedH (ULONG This);		/* 05E */
extern VOID AAADisplayStart (ULONG This);		/* 08E */
extern VOID AAADisplayStop (ULONG This);		/* 090 */
extern VOID AAADataFetch (ULONG This);			/* 092, 094 */
extern VOID AAADMA16 (ULONG This);			/* 096 */
extern VOID AAACollision (ULONG This);			/* 098 */
extern VOID AAAInterrupt (ULONG This);			/* 09A, 09C */
extern VOID AAAADUControl (ULONG This);			/* 09E */
extern VOID AAAAudioLength16 (ULONG This);		/* 0A4, 0B4, 0C4, 0D4 */
extern VOID AAAAudioCoursePeriod (ULONG This);		/* 0A6, 0B6, 0C6, 0D6 */
extern VOID AAAAudioVolumeUnsigned (ULONG This);	/* 0A6, 0B8, 0C8, 0D8 */
extern VOID AAABPMiscControl (ULONG This);		/* 100 */
extern VOID AAAHScroll (ULONG This);			/* 102 */
extern VOID AAABPPriority (ULONG This);			/* 104 */
extern VOID AAABPEnhance (ULONG This);			/* 106 */
extern VOID AAABPModulo (ULONG This);			/* 108, 10A */
extern VOID AAABPSpriteMagic (ULONG This);		/* 10C */
extern VOID AAASpriteStart (ULONG This);		/* 140, 148, 150, 158, 160, 168, 170, 178 */
extern VOID AAASpriteControl (ULONG This);		/* 142, 14A, 152, 15A, 162, 16A, 172, 17A */
extern VOID LowResColorClk (ULONG This);		/* 1C0, 1C2, 1C4, 1C6, 1D8, 1DA, 1DE */
extern VOID LineCount (ULONG This);			/* 1C8, 1CA, 1CC, 1CE, 1E0 */
extern VOID AAABeamCounter (ULONG This);		/* 1DC */
extern VOID AAAXDisplay (ULONG This);			/* 1E4 */
extern VOID AAABeamPos (ULONG This);			/* 22C */
extern VOID AAABitmapWidth (ULONG This);		/* 25C, 260, 264, 458 */
extern VOID AAABlitterSizeLar (ULONG This);		/* 268 */
extern VOID AAABlitterFunction (ULONG This);		/* 278 */
extern VOID AAADiskControl (ULONG This);		/* 28C */
extern VOID AAADMA32 (ULONG This);			/* 290 */
extern VOID AAAInterruptXtend (ULONG This);		/* 294, 29C */
extern VOID AAACollisionXtend (ULONG This);		/* 298 */
extern VOID AAAAudioLength (ULONG This);		/* 2A4, 2BC, 2D4, 2EC, 4A4, 4BC, 4D4, 4EC */
extern VOID AAAAudioPeriod (ULONG This);		/* 2A8, 2C0, 2D8, 2F0, 4A8, 4C0, 4D8, 4F0 */
extern VOID AAAAudioVolumeSigned (ULONG This);		/* 2AC, 2C4, 2DC, 2F4, 4AC, 4C4, 4DC, 4F4 */
extern VOID AAAAudioControl (ULONG This);		/* 2B4, 2CC, 2E4, 2FC, 4B4, 4BC, 4E4, 4FC */
extern VOID AAASpriteStartXtend (ULONG This);		/* 380, 388, 390, 398, 3A0, 3A8, 3B0, 3B8 */
extern VOID AAASpriteControlXtend (ULONG This);		/* 384, 38C, 394, 39C, 3A4, 3AC, 3B4, 3BC */
extern VOID HighResColorClk (ULONG This);		/* 3C0, 3CC, 3D0, 3D4, 3D8, 3DC, 568, 56C, 570, 574, 578, 57C, 584, 588 */
extern VOID AAADisplayNew (ULONG This);			/* 3C4, 3C8 */
extern VOID AAABlitterEnable (ULONG This);		/* 434 */
extern VOID AAABlitterPosition (ULONG This);		/* 43C, 440, 45C, 460 */
extern VOID AAABlitterFunctionLine (ULONG This);	/* 464 */
extern VOID AAACollisionCHUNKYEnable (ULONG This);	/* 494 */
extern VOID AAACollisionCHUNKYMatch (ULONG This);	/* 498 */
extern VOID MonicaTest (ULONG This);			/* 49C */
extern VOID AAALUTOffset (ULONG This);			/* 504 */
extern VOID HighWordLineCount (ULONG This);		/* 580 */
extern UWORD SetField (UWORD Field);
extern VOID ResetAAADisplay (VOID);

#endif
