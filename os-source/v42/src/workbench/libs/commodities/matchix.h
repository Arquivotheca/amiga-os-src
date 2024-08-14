#ifndef MATCHIX_H
#define MATCHIX_H


/*****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>
#include "commoditiesbase.h"


/*****************************************************************************/


BOOL ASM MatchIXLVO(REG(a0) struct InputEvent *ie,
                    REG(a1) struct InputXpression *ix,
                    REG(a6) struct CxLib *CxBase)


/*****************************************************************************/


#endif /* MATCHIX_H */
