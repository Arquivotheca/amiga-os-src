#ifndef BROKER_H
#define BROKER_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


LONG ASM BrokerCommandLVO(REG(a0) STRPTR name,
                          REG(d0) LONG longcommand,
                          REG(a6) struct CxLib *CxBase);

struct CxObj * ASM CxBrokerLVO(REG(a0) struct NewBroker *nb,
                               REG(d0) LONG *errorPtr,
                               REG(a6) struct CxLib *CxBase);

VOID InitBroker(struct CxLib *CxBase, struct CxObj *broker, struct NewBroker *nb);


/*****************************************************************************/


#endif /* BROKER_H */
