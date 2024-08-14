/*
**	$Id: icon_pragmas.h,v 38.2 93/06/16 12:11:44 vertex Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "icon.library" */
/*	Use DiskObjects instead of obsolete WBObjects */
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase OBSOLETEGetWBObject 1e 801
#endif
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase OBSOLETEPutWBObject 24 9802
#endif
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase GetIcon 2a A9803
#endif
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase PutIcon 30 9802
#endif
#pragma libcall IconBase FreeFreeList 36 801
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase OBSOLETEFreeWBObject 3c 801
#endif
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase OBSOLETEAllocWBObject 42 00
#endif
#pragma libcall IconBase AddFreeList 48 A9803
#pragma libcall IconBase GetDiskObject 4e 801
#pragma libcall IconBase PutDiskObject 54 9802
#pragma libcall IconBase FreeDiskObject 5a 801
#pragma libcall IconBase FindToolType 60 9802
#pragma libcall IconBase MatchToolValue 66 9802
#pragma libcall IconBase BumpRevision 6c 9802
#ifdef ICON_PRIVATE_PRAGMAS
#pragma libcall IconBase FreeAlloc 72 A9803
#endif
/*--- functions in V36 or higher (Release 2.0) ---*/
#pragma libcall IconBase GetDefDiskObject 78 001
#pragma libcall IconBase PutDefDiskObject 7e 801
#pragma libcall IconBase GetDiskObjectNew 84 801
/*--- functions in V37 or higher (Release 2.04) ---*/
#pragma libcall IconBase DeleteDiskObject 8a 801
/*--- (4 function slots reserved here) ---*/
