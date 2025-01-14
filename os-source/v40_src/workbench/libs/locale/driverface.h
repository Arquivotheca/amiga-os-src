#ifndef DRIVERFACE_H
#define DRIVERFACE_H


/*****************************************************************************/


#include <exec/types.h>
#include "locale.h"
#include "localebase.h"


/*****************************************************************************/


ULONG ASM ConvToLowerLVO(REG(a0) struct ExtLocale *locale,
                         REG(d0) ULONG character);

ULONG ASM ConvToUpperLVO(REG(a0) struct ExtLocale *locale,
                         REG(d0) ULONG character);

ULONG ASM GetCodeSet(REG(a0) struct ExtLocale *locale);

ULONG ASM GetDriverInfo(REG(a0) struct ExtLocale *locale);

STRPTR ASM GetLocaleStrLVO(REG(a0) struct ExtLocale *locale,
                           REG(d0) ULONG stringNum);

BOOL ASM IsAlNumLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsAlphaLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsCntrlLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsDigitLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsGraphLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsLowerLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsPrintLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsPunctLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsSpaceLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsUpperLVO(REG(a0) struct ExtLocale *locale,
                    REG(d0) ULONG character);

BOOL ASM IsXDigitLVO(REG(a0) struct ExtLocale *locale,
                     REG(d0) ULONG character);

ULONG ASM StrConvertLVO(REG(a0) struct ExtLocale *locale,
                        REG(a1) STRPTR string,
                        REG(a2) APTR buffer,
                        REG(d0) ULONG bufferSize,
                        REG(d1) ULONG type);

LONG ASM StrnCmpLVO(REG(a0) struct ExtLocale *locale,
                    REG(a1) STRPTR string1,
                    REG(a2) STRPTR string2,
                    REG(d0) LONG length,
                    REG(d1) ULONG type);


/*****************************************************************************/


#endif /* DRIVERFACE_H */
