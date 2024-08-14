/*
**	$Id: disk_pragmas.h,v 36.1 91/02/19 03:51:46 jesup Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "disk.resource" */
#pragma libcall DiskBase AllocUnit 6 001
#pragma libcall DiskBase FreeUnit c 001
#pragma libcall DiskBase GetUnit 12 901
#pragma libcall DiskBase GiveUnit 18 00
#pragma libcall DiskBase GetUnitID 1e 001
/*------ new for V37 ------*/
#pragma libcall DiskBase ReadUnitID 24 001
