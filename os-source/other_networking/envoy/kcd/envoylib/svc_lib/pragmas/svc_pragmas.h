/*
**	$Id: svc_pragmas.h,v 37.1 92/03/22 18:29:23 kcd Exp Locker: kcd $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "generic.service" */
/*--- functions in V37 or higher (Release 2.04) ---*/

#pragma libcall SvcBase RexxReserved 1e 00
#pragma libcall SvcBase StartServiceA 24 801
#ifdef __SASC_60
#pragma tagcall SvcBase StartService 24 801
#endif
#pragma libcall SvcBase GetServiceAttrsA 2a 801
#ifdef __SASC_60
#pragma tagcall SvcBase GetServiceAttrs 2a 801
#endif
