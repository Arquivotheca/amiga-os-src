/*
**	$Id: services_pragmas.h,v 37.3 92/06/11 14:08:56 kcd Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "services.library" */
/*--- functions in V37 or higher (Release 2.04) ---*/

#pragma libcall ServicesBase FindServiceA 1e BA9804
#ifdef __SASC_60
#pragma tagcall ServicesBase FindService 1e BA9804
#endif
#pragma libcall ServicesBase LoseService 24 801
