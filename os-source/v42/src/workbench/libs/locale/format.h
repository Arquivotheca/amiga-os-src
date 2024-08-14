#ifndef FORMAT_H
#define FORMAT_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/dos.h>
#include <utility/hooks.h>

#include "localebase.h"


/*****************************************************************************/


VOID LongToStr(ULONG num, STRPTR buffer, WORD digits);


/*****************************************************************************/


#endif /* FORMAT_H */
