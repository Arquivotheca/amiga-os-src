#ifndef EXCHANGE_H
#define EXCHANGE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/lists.h>


/*****************************************************************************/


LONG ASM CopyBrokerListLVO(REG(a0) struct List *list, REG(a6) struct CxLib *CxBase);
VOID ASM FreeBrokerListLVO(REG(a0) struct List *list, REG(a6) struct CxLib *CxBase);


/*****************************************************************************/


#endif /* EXCHANGE_H */
