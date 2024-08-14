#ifndef	IFFW_H
#define	IFFW_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


VOID FreeBufferedStream(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase);
LONG PushChunkW(struct IFFHandleP *iffp, LONG type, LONG id, LONG size, struct IFFParseLib *IFFParseBase);
LONG PopChunkW(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase);


/*****************************************************************************/


#endif /* IFFW_H */
