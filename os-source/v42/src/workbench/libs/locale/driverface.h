#ifndef DRIVERFACE_H
#define DRIVERFACE_H


/*****************************************************************************/


#include <exec/types.h>
#include "locale.h"
#include "localebase.h"


/*****************************************************************************/


ULONG ASM GetCodeSet(REG(a0) struct ExtLocale *locale,
                     REG(a6) struct LocaleLib *LocaleBase);

ULONG ASM GetDriverInfo(REG(a0) struct ExtLocale *locale,
			REG(a6) struct LocaleLib *LocaleBase);


/*****************************************************************************/


#endif /* DRIVERFACE_H */
