/*
**	$Id: jcc_pragmas.h,v 39.1 92/12/12 15:14:44 kaori Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "jcc.library" */

/*--- functions in V39 or higher (Release 3) ---*/
#pragma libcall JCCBase AllocConversionHandleA 1e 801
#ifdef __SASC_60
#pragma tagcall JCCBase AllocConversionHandle 1e 801
#endif
#pragma libcall JCCBase FreeConversionHandle 24 801
#pragma libcall JCCBase GetJConversionAttrsA 2a 9802
#ifdef __SASC_60
#pragma tagcall JCCBase GetJConversionAttrs 2a 9802
#endif
#pragma libcall JCCBase SetJConversionAttrsA 30 9802
#ifdef __SASC_60
#pragma tagcall JCCBase SetJConversionAttrs 30 9802
#endif
#pragma libcall JCCBase IdentifyCodeSet 36 09803
#pragma libcall JCCBase JStringConvertA 3c B210A9807
#ifdef __SASC_60
#pragma tagcall JCCBase JStringConvert 3c B210A9807
#endif
#pragma libcall JCCBase HanToZen 42 210A9806
#pragma libcall JCCBase ZenToHan 48 210A9806
#pragma libcall JCCBase EightToSeven 4e 9802
#pragma libcall JCCBase SevenToEight 54 9802
