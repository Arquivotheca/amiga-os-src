#ifndef DATEPATCH_H
#define DATEPATCH_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/datetime.h>
#include "localebase.h"


/*****************************************************************************/


BOOL ASM DateToStrPatch(REG(d1) struct DateTime *dt, REG(a6) struct LocaleLib *LocaleBase);
BOOL ASM StrToDatePatch(REG(d1) struct DateTime *dt, REG(a6) struct LocaleLib *LocaleBase);


/*****************************************************************************/


#endif /* DATEPATCH_H */
