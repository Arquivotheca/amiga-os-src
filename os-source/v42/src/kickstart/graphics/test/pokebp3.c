#include <exec/types.h>
#include "/gfxbase.h"
main(argc,argv)
char **argv;
{
	struct GfxBase *GfxBase=OpenLibrary("graphics.library",0);
	ULONG pokeval;
	pokeval=atoi(argv[1]);
	GfxBase->BP3Bits=pokeval;
	CloseLibrary(GfxBase);
}
