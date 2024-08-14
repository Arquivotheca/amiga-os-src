#ifndef ENGLISH_H
#define ENGLISH_H


/*****************************************************************************/


#include <exec/types.h>
#include "locale.h"
#include "localebase.h"


/*****************************************************************************/


ULONG ASM EnglishConvToLower(REG(a0) struct ExtLocale *locale,
                             REG(d0) ULONG character);

ULONG ASM EnglishConvToUpper(REG(a0) struct ExtLocale *locale,
                             REG(d0) ULONG character);

ULONG ASM EnglishGetCodeSet(VOID);

ULONG ASM EnglishGetDriverInfo(VOID);

STRPTR ASM EnglishGetLocaleStr(REG(a0) struct ExtLocale *locale,
                               REG(d0) ULONG stringNum);

BOOL ASM EnglishIsAlNum(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsAlpha(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsCntrl(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsDigit(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsGraph(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsLower(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsPrint(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsPunct(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsSpace(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsUpper(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character);

BOOL ASM EnglishIsXDigit(REG(a0) struct ExtLocale *locale,
                         REG(d0) ULONG character);

LONG ASM EnglishStrnCmp(REG(a0) struct ExtLocale *locale,
                        REG(a1) STRPTR string1,
                        REG(a2) STRPTR string2,
                        REG(d0) LONG length,
                        REG(d1) ULONG type);

ULONG ASM EnglishStrConvert(REG(a0) struct ExtLocale *locale,
                            REG(a1) STRPTR string,
                            REG(a2) APTR buffer,
                            REG(d0) ULONG bufferSize,
                            REG(d1) ULONG type);


/*****************************************************************************/


#endif /* ENGLISH_H */
