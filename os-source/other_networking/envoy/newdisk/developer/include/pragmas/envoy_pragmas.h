/*
**	$Id: envoy_pragmas.h,v 1.1 92/10/13 14:12:37 kcd Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "envoy.library" */
/*--- functions in V39 or higher (Release 3) ---*/
#pragma libcall EnvoyBase HostRequestA 1e 801
#ifdef __SASC_60
#pragma tagcall EnvoyBase HostRequest 1e 801
#endif
#pragma libcall EnvoyBase LoginRequestA 24 801
#ifdef __SASC_60
#pragma tagcall EnvoyBase LoginRequest 24 801
#endif
#pragma libcall EnvoyBase UserRequestA 2a 801
#ifdef __SASC_60
#pragma tagcall EnvoyBase UserRequest 2a 801
#endif
