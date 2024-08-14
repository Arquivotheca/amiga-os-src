#ifndef CXPOOLS_H
#define CXPOOLS_H


/****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>
#include "commoditiesbase.h"
#include "pool.h"


/*****************************************************************************/


struct Message *GetEMsg(LONG size);

#define FreeEMsg(msg)  FreePoolMem(msg)
#define GetCxMsg(size) AllocPoolMem(size)
#define FreeCxMsg(msg) FreePoolMem(msg)
#define GetCxObj(size) AllocPoolMem(size)
#define FreeCxObj(obj) FreePoolMem(obj)
#define GetIE()        AllocPoolMem(sizeof(struct InputEvent))


/****************************************************************************/


#endif /* CXPOOLS_H */
