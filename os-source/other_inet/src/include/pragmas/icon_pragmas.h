/*
**	$Id: icon_pragmas.h,v 38.1 91/06/24 19:01:42 mks Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "icon.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/
/*	Use DiskObjects instead of obsolete WBObjects */
/* System-private function: OBSOLETEGetWBObject */
#pragma libcall IconBase OBSOLETEGetWBObject 1e 801
/* System-private function: OBSOLETEPutWBObject */
#pragma libcall IconBase OBSOLETEPutWBObject 24 9802
#pragma libcall IconBase GetIcon 2a A9803
#pragma libcall IconBase PutIcon 30 9802
#pragma libcall IconBase FreeFreeList 36 801
/* System-private function: OBSOLETEFreeWBObject */
#pragma libcall IconBase OBSOLETEFreeWBObject 3c 801
/* System-private function: OBSOLETEAllocWBObject */
#pragma libcall IconBase OBSOLETEAllocWBObject 42 00
#pragma libcall IconBase AddFreeList 48 A9803
#pragma libcall IconBase GetDiskObject 4e 801
#pragma libcall IconBase PutDiskObject 54 9802
#pragma libcall IconBase FreeDiskObject 5a 801
#pragma libcall IconBase FindToolType 60 9802
#pragma libcall IconBase MatchToolValue 66 9802
#pragma libcall IconBase BumpRevision 6c 9802
/* System-private function: FreeAlloc */
#pragma libcall IconBase FreeAlloc 72 A9803
#pragma libcall IconBase GetDefDiskObject 78 001
#pragma libcall IconBase PutDefDiskObject 7e 801
#pragma libcall IconBase GetDiskObjectNew 84 801
#pragma libcall IconBase DeleteDiskObject 8a 801
/*--- (4 function slots reserved here) ---*/
