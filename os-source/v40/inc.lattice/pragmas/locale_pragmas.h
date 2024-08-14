/*
**	$Id: locale_pragmas.h,v 38.5 93/06/18 09:19:02 vertex Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "locale.library" */
/*--- functions in V38 or higher (Release 2.1) ---*/
/* ARexx function host entry point */
/* This function also returns a (struct RexxMsg *) in A1 */
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocaleARexxHost 1e 801
#endif
#pragma libcall LocaleBase CloseCatalog 24 801
#pragma libcall LocaleBase CloseLocale 2a 801
#pragma libcall LocaleBase ConvToLower 30 0802
#pragma libcall LocaleBase ConvToUpper 36 0802
#pragma libcall LocaleBase FormatDate 3c BA9804
#pragma libcall LocaleBase FormatString 42 BA9804
#pragma libcall LocaleBase GetCatalogStr 48 90803
#pragma libcall LocaleBase GetLocaleStr 4e 0802
#pragma libcall LocaleBase IsAlNum 54 0802
#pragma libcall LocaleBase IsAlpha 5a 0802
#pragma libcall LocaleBase IsCntrl 60 0802
#pragma libcall LocaleBase IsDigit 66 0802
#pragma libcall LocaleBase IsGraph 6c 0802
#pragma libcall LocaleBase IsLower 72 0802
#pragma libcall LocaleBase IsPrint 78 0802
#pragma libcall LocaleBase IsPunct 7e 0802
#pragma libcall LocaleBase IsSpace 84 0802
#pragma libcall LocaleBase IsUpper 8a 0802
#pragma libcall LocaleBase IsXDigit 90 0802
#pragma libcall LocaleBase OpenCatalogA 96 A9803
#ifdef __SASC_60
#pragma tagcall LocaleBase OpenCatalog 96 A9803
#endif
#pragma libcall LocaleBase OpenLocale 9c 801
#pragma libcall LocaleBase ParseDate a2 BA9804
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase SetDefaultLocale a8 801
#endif
#pragma libcall LocaleBase StrConvert ae 10A9805
#pragma libcall LocaleBase StrnCmp b4 10A9805

/* Reserved entries */

#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved0 ba 00
#endif
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved1 c0 00
#endif
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved2 c6 00
#endif
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved3 cc 00
#endif
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved4 d2 00
#endif
#ifdef LOCALE_PRIVATE_PRAGMAS
#pragma libcall LocaleBase LocReserved5 d8 00
#endif
