#ifndef	IFFR_H
#define	IFFR_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


LONG PushChunkR(struct IFFHandleP *iffp, LONG override);
LONG PopChunkR(struct IFFHandleP *iffp);


/*****************************************************************************/


#endif /* IFFR_H */
