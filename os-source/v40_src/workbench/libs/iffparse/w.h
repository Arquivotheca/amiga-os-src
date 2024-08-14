#ifndef	IFFW_H
#define	IFFW_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


VOID FreeBufferedStream(struct IFFHandleP *iffp);
LONG PushChunkW(struct IFFHandleP *iffp, LONG type, LONG id, LONG size);
LONG PopChunkW(struct IFFHandleP *iffp);


/*****************************************************************************/


#endif /* IFFW_H */
