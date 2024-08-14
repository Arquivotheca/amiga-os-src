#ifndef FORMAT_H
#define FORMAT_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/dos.h>
#include <utility/hooks.h>

#include "localebase.h"


/*****************************************************************************/


VOID LongToStr(ULONG num, STRPTR buffer, WORD digits);


VOID ASM FormatDateLVO(REG(a0) struct ExtLocale *locale,
                       REG(a1) STRPTR string,
                       REG(a2) struct DateStamp *date,
                       REG(a3) struct Hook *putCharFunc);


APTR ASM FormatStringLVO(REG(a0) struct ExtLocale *locale,
                         REG(a1) STRPTR string,
                         REG(a2) UBYTE *dataStream,
                         REG(a3) struct Hook *putCharFunc);


BOOL ASM ParseDateLVO(REG(a0) struct ExtLocale *locale,
                      REG(a1) struct DateStamp *date,
                      REG(a2) STRPTR template,
                      REG(a3) struct Hook *getCharFunc);


/*****************************************************************************/


#endif /* FORMAT_H */
