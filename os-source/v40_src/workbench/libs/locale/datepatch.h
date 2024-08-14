#ifndef DATEPATCH_H
#define DATEPATCH_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/datetime.h>
#include "localebase.h"


/*****************************************************************************/


BOOL ASM DateToStrPatch(REG(d1) struct DateTime *dt);
BOOL ASM StrToDatePatch(REG(d1) struct DateTime *dt);


/*****************************************************************************/


#endif /* DATEPATCH_H */
