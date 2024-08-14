#ifndef EXCHANGE_H
#define EXCHANGE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/lists.h>


/*****************************************************************************/


LONG ASM CopyBrokerListLVO(REG(a0) struct List *list);
VOID ASM FreeBrokerListLVO(REG(a0) struct List *list);


/*****************************************************************************/


#endif /* EXCHANGE_H */
