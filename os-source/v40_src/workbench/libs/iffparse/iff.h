#ifndef	IFF_H
#define	IFF_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


VOID FreeContextNode(struct IFFHandleP *iffp, struct ContextNodeP *cnp);
#define IsGenericID(id) ((id) == ID_FORM || (id) == ID_CAT || (id) == ID_LIST || (id) == ID_PROP)


/*****************************************************************************/


#endif /* IFF_H */
