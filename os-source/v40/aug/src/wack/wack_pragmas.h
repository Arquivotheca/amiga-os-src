/*
**	$Id: wack_pragmas.h,v 39.5 93/11/05 15:12:04 jesup Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "wack" */

/* WackBase is a pointer to the Wack ARexx port obtained via FindPort(). */

#pragma libcall WackBase Wack_VPrintf 24 9802
#ifdef __SASC_60
#pragma tagcall WackBase Wack_Printf 24 801
#endif
#pragma libcall WackBase Wack_ReadCurrAddr 2a 00
#pragma libcall WackBase Wack_WriteCurrAddr 30 801
#pragma libcall WackBase Wack_ReadSpareAddr 36 00
#pragma libcall WackBase Wack_WriteSpareAddr 3c 801
#pragma libcall WackBase Wack_ReadByte 42 801
#pragma libcall WackBase Wack_ReadWord 48 801
#pragma libcall WackBase Wack_ReadLong 4e 801
#pragma libcall WackBase Wack_ReadPointer 4e 801
#pragma libcall WackBase Wack_ReadBlock 54 09803
#pragma libcall WackBase Wack_WriteByte 5a 0802
#pragma libcall WackBase Wack_WriteWord 60 0802
#pragma libcall WackBase Wack_WriteLong 66 0802
#pragma libcall WackBase Wack_WritePointer 66 0802
#pragma libcall WackBase Wack_FindLibrary 6c 801
#pragma libcall WackBase Wack_ReadString 72 09803
#pragma libcall WackBase Wack_ReadBSTR 78 09803
#pragma libcall WackBase Wack_ReadContextAddr 7e 00
#pragma libcall WackBase Wack_WriteBlock 84 09803
#pragma libcall WackBase Wack_Call 8a 801
