#ifndef BROKER_H
#define BROKER_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


LONG ASM BrokerCommandLVO(REG(a0) STRPTR name, REG(d0) LONG longcommand);
struct CxObj * ASM CxBrokerLVO(REG(a0) struct NewBroker *nb, REG(d0) LONG *errorPtr);
VOID InitBroker(struct CxObj *broker, struct NewBroker *nb);


/*****************************************************************************/


#endif /* BROKER_H */
