#ifndef REXXHOST_H
#define REXXHOST_H


/*****************************************************************************/


#include <exec/types.h>
#include <rexx/storage.h>
#include "localebase.h"


/*****************************************************************************/


ULONG ASM ARexxHostLVO(REG(a0) struct RexxMsg *parameters,
                       REG(a1) STRPTR *result,
                       REG(a6) struct LocaleBase *LocaleBase);


/*****************************************************************************/


#endif /* REXXHOST_H */
