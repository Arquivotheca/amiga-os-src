/*
**	$Id: ramdrive_pragmas.h,v 36.3 90/11/07 15:53:27 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "ramdrive.device" */
/*--- functions in V34 or higher (Release 1.3) ---*/
#pragma libcall RamdriveDevice KillRAD0 2a 00
/*--- functions in V36 or higher (Release 2.0) ---*/
#pragma libcall RamdriveDevice KillRAD 30 001
