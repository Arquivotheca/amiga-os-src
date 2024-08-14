#ifndef	IFFR_H
#define	IFFR_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


LONG PushChunkR(struct IFFHandleP *iffp, LONG override, struct IFFParseLib *IFFParseBase);
LONG PopChunkR(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase);


/*****************************************************************************/


#endif /* IFFR_H */
