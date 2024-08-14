#ifndef ENGLISH_H
#define ENGLISH_H


/*****************************************************************************/


#include <exec/types.h>
#include "locale.h"
#include "localebase.h"


/*****************************************************************************/


ULONG ASM EnglishConvToLower(REG(a0) struct ExtLocale *locale,
                             REG(d0) ULONG character,
                             REG(a6) struct LocaleLib *LocaleBase);

ULONG ASM EnglishConvToUpper(REG(a0) struct ExtLocale *locale,
                             REG(d0) ULONG character,
                             REG(a6) struct LocaleLib *LocaleBase);

ULONG ASM EnglishGetCodeSet(REG(a6) struct LocaleLib *LocaleBase);

ULONG ASM EnglishGetDriverInfo(REG(a6) struct LocaleLib *LocaleBase);

STRPTR ASM EnglishGetLocaleStr(REG(a0) struct ExtLocale *locale,
                               REG(d0) ULONG stringNum,
                               REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsAlNum(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsAlpha(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsCntrl(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsDigit(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsGraph(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsLower(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsPrint(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsPunct(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsSpace(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsUpper(REG(a0) struct ExtLocale *locale,
                        REG(d0) ULONG character,
                        REG(a6) struct LocaleLib *LocaleBase);

BOOL ASM EnglishIsXDigit(REG(a0) struct ExtLocale *locale,
                         REG(d0) ULONG character,
                         REG(a6) struct LocaleLib *LocaleBase);

LONG ASM EnglishStrnCmp(REG(a0) struct ExtLocale *locale,
                        REG(a1) STRPTR string1,
                        REG(a2) STRPTR string2,
                        REG(d0) LONG length,
                        REG(d1) ULONG type,
                        REG(a6) struct LocaleLib *LocaleBase);

ULONG ASM EnglishStrConvert(REG(a0) struct ExtLocale *locale,
                            REG(a1) STRPTR string,
                            REG(a2) APTR buffer,
                            REG(d0) ULONG bufferSize,
                            REG(d1) ULONG type,
                            REG(a6) struct LocaleLib *LocaleBase);


/*****************************************************************************/


#endif /* ENGLISH_H */
