
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


#define SAMPLE_HEIGHT 115


VOID RenderSample(EdDataPtr ed, UWORD *pens, BOOL complete);
APTR NewPrefsObject(EdDataPtr ed,struct IClass *cl, STRPTR name, ULONG data,...);
ULONG BestModeIDP(EdDataPtr ed, ULONG tag, ...);


VOID CreateClickMap(EdDataPtr ed);
VOID DisposeClickMap(EdDataPtr ed);
WORD WhichPen(EdDataPtr ed, WORD x, WORD y);
