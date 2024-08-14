
#include <exec/types.h>
#include <exec/libraries.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <stdlib.h>

#include "camdlists.h"

struct GfxBase	*GfxBase;
struct Library	*IntuitionBase,
				*GadToolsBase,
				*UtilityBase,
				*CamdBase,
				*RealTimeBase;

void main(int argc, char *argv[])
{	APTR	req;
	LONG	result;
	char	buffer[60] = "Initial String";

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",36L);
	if (GfxBase == NULL) goto bye;
	IntuitionBase = OpenLibrary("intuition.library",36L);
	if (IntuitionBase == NULL) goto bye;
	GadToolsBase = OpenLibrary("gadtools.library",36L);
	if (GadToolsBase == NULL) goto bye;
	UtilityBase = OpenLibrary("utility.library",36L);
	if (UtilityBase == NULL) goto bye;
	CamdBase = OpenLibrary("camd.library",0L);
	if (CamdBase == NULL) goto bye;
	RealTimeBase = OpenLibrary("realtime.library",0L);
	if (RealTimeBase == NULL) goto bye;

	if (req = AllocListRequest(TAG_END))
	{
		result = SelectConductor( req, buffer, sizeof buffer, TAG_END );
		result = SelectCluster( req, buffer, sizeof buffer, TAG_END );

		FreeListRequest(req);
	}

bye:
	if (GfxBase)
	{
		CloseLibrary(RealTimeBase);
		CloseLibrary(CamdBase);
		CloseLibrary(UtilityBase);
		CloseLibrary(GadToolsBase);
		CloseLibrary(IntuitionBase);
		CloseLibrary((struct Library *)GfxBase);
	}

	exit(0);
}
