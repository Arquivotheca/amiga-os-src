/*
**	$Id: videocd_pragmas.h,v 40.0 93/09/14 15:21:53 davidj Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "videocd.library" */
/*--- functions in V37 or higher (Release 2.04) ---*/
/* ARexx function host entry point */
#ifdef VIDEOCD_PRIVATE_PRAGMAS
#pragma libcall VideoCDBase REXXHost 1e 9802
#endif

/* Public entries */

#pragma libcall VideoCDBase GetCDTypeA 24 9802
#pragma libcall VideoCDBase ObtainCDHandleA 2a 9802
#ifdef __SASC_60
#pragma tagcall VideoCDBase ObtainCDHandle 2a 9802
#endif
#pragma libcall VideoCDBase ReleaseCDHandle 30 801
#pragma libcall VideoCDBase GetVideoCDInfoA 36 90803
#ifdef __SASC_60
#pragma tagcall VideoCDBase GetVideoCDInfo 36 90803
#endif
#pragma libcall VideoCDBase FreeVideoCDInfo 3c 801
