/*
**	$Id: mpeg_pragmas.h,v 1.6 91/01/25 15:46:51 rsbx Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "cd32mpeg.device" */
#pragma libcall CD32Base SetSCR 2a 0802
#pragma libcall CD32Base GetSCR 30 801
/* System-private function: GetSector */
#pragma libcall CD32Base GetSector 36 801
