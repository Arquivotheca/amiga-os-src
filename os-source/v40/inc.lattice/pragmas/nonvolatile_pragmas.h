/*
**	$Id: nonvolatile_pragmas.h,v 40.5 93/07/30 17:13:06 vertex Exp Locker: vertex $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "nonvolatile.library" */
/*--- functions in V40 or higher (Release 3.1) ---*/
#pragma libcall NVBase GetCopyNV 1e 19803
#pragma libcall NVBase FreeNVData 24 801
#pragma libcall NVBase StoreNV 2a 10A9805
#pragma libcall NVBase DeleteNV 30 19803
#pragma libcall NVBase GetNVInfo 36 101
#pragma libcall NVBase GetNVList 3c 1802
#pragma libcall NVBase SetNVProtection 42 129804
