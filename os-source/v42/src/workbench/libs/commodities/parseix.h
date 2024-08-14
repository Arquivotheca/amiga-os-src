#ifndef PARSEIX_H
#define PARSEIX_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


LONG ASM ParseIXLVO(REG(a0) STRPTR string,
                    REG(a1) struct InputXpression *ix,
                    REG(a6) struct CxLib *CxBase);


/*****************************************************************************/


#endif /* PARSEIX_H */
