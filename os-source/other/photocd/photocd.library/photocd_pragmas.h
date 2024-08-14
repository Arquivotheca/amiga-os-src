/*
**	$Id: PhotoCD_pragmas.h,v 40.1 93/11/18 17:39:42 jjszucs Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "photocd.library" */
/*--- functions in V40 or higher (Release 3.1) ---*/

/*   Public entries */

#pragma libcall PhotoCDBase AllocPCDImageBufferA 1e 8002
#ifdef __SASC_60
#pragma tagcall PhotoCDBase AllocPCDImageBuffer 1e 8002
#endif
#pragma libcall PhotoCDBase ClosePhotoCD 24 801
#pragma libcall PhotoCDBase FreePCDImageBuffer 2a 801
#pragma libcall PhotoCDBase GetPCDImageDataA 30 A9803
#ifdef __SASC_60
#pragma tagcall PhotoCDBase GetPCDImageData 30 A9803
#endif
#pragma libcall PhotoCDBase GetPCDResolution 36 98003
#pragma libcall PhotoCDBase IsPhotoCD 3c 00
#pragma libcall PhotoCDBase ObtainPhotoCDInfoA 42 9802
#ifdef __SASC_60
#pragma tagcall PhotoCDBase ObtainPhotoCDInfo 42 9802
#endif
#pragma libcall PhotoCDBase OpenPhotoCDA 48 801
#ifdef __SASC_60
#pragma tagcall PhotoCDBase OpenPhotoCD 48 801
#endif
#pragma libcall PhotoCDBase ReleasePhotoCDInfo 4e 801
