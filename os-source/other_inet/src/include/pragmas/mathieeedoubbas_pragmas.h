/*
**	$Id: mathieeedoubbas_pragmas.h,v 1.3 90/11/07 15:58:12 mks Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "mathieeedoubbas.library" */
#pragma libcall MathIeeeDoubBasBase IEEEDPFix 1e 001
#pragma libcall MathIeeeDoubBasBase IEEEDPFlt 24 001
#pragma libcall MathIeeeDoubBasBase IEEEDPCmp 2a 2002
#pragma libcall MathIeeeDoubBasBase IEEEDPTst 30 001
#pragma libcall MathIeeeDoubBasBase IEEEDPAbs 36 001
#pragma libcall MathIeeeDoubBasBase IEEEDPNeg 3c 001
#pragma libcall MathIeeeDoubBasBase IEEEDPAdd 42 2002
#pragma libcall MathIeeeDoubBasBase IEEEDPSub 48 2002
#pragma libcall MathIeeeDoubBasBase IEEEDPMul 4e 2002
#pragma libcall MathIeeeDoubBasBase IEEEDPDiv 54 2002
/*--- functions in V33 or higher (Release 1.2) ---*/
#pragma libcall MathIeeeDoubBasBase IEEEDPFloor 5a 001
#pragma libcall MathIeeeDoubBasBase IEEEDPCeil 60 001
