
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

/* application includes */
#include "pe_custom.h"


/*****************************************************************************/


VOID RenderSample(EdDataPtr ed, UWORD *pens, BOOL complete);
APTR NewPrefsObject(EdDataPtr ed,struct IClass *cl, STRPTR name, ULONG data,...);
