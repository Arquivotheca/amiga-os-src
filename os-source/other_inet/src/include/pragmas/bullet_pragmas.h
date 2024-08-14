/*
**	$Id: bullet_pragmas.h,v 38.0 92/06/19 11:20:38 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "bullet.library" */
#pragma libcall BulletBase OpenEngine 1e 00
#pragma libcall BulletBase CloseEngine 24 801
#pragma libcall BulletBase SetInfoA 2a 9802
#ifdef __SASC_60
#pragma tagcall BulletBase SetInfo 2a 9802
#endif
#pragma libcall BulletBase ObtainInfoA 30 9802
#ifdef __SASC_60
#pragma tagcall BulletBase ObtainInfo 30 9802
#endif
#pragma libcall BulletBase ReleaseInfoA 36 9802
#ifdef __SASC_60
#pragma tagcall BulletBase ReleaseInfo 36 9802
#endif
/* System-private function: GetGlyphMap */
#pragma libcall BulletBase GetGlyphMap 3c 90803
