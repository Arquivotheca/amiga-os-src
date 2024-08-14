#include "/gfxbase.h"
main()
{
	struct GfxBase * GfxBase=OpenLibrary("graphics.library",0);
	GfxBase->BoardMemType=BUS_32 | DBL_CAS;
	GfxBase->MemType=BUS_32 | DBL_CAS;
}
