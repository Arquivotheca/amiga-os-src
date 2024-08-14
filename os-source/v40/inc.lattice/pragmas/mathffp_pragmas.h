/*
**	$Id: mathffp_pragmas.h,v 1.4 90/05/03 11:51:45 rsbx Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "mathffp.library" */
#pragma libcall MathBase SPFix 1e 001
#pragma libcall MathBase SPFlt 24 001
#pragma libcall MathBase SPCmp 2a 0102
#pragma libcall MathBase SPTst 30 101
#pragma libcall MathBase SPAbs 36 001
#pragma libcall MathBase SPNeg 3c 001
#pragma libcall MathBase SPAdd 42 0102
#pragma libcall MathBase SPSub 48 0102
#pragma libcall MathBase SPMul 4e 0102
#pragma libcall MathBase SPDiv 54 0102
/*--- functions in V33 or higher (Release 1.2) ---*/
#pragma libcall MathBase SPFloor 5a 001
#pragma libcall MathBase SPCeil 60 001
