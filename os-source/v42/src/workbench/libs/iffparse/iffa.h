#ifndef	IFFA_H
#define	IFFA_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


LONG ASM CallAHook(REG(a0) struct Hook *,
		   REG(a2) VOID *,
		   REG(a1) LONG *,
		   REG(a6) struct IFFParseLib *);

LONG ASM ListAction(REG(a0) struct IFFHandleP *,
		    REG(a1) LONG *,
		    REG(d0) LONG,
		    REG(a2) LONG (*)(),
		    REG(a6) struct IFFParseLib *IFFParseBase);


/*****************************************************************************/


#endif /* IFFA_H */
