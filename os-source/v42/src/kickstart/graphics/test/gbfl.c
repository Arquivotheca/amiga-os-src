#include <graphics/gfxbase.h>
struct GfxBase *GfxBase;
main()
{
	GfxBase=OpenLibrary("graphics.library",0);
	printf("dflags=%04x\n",GfxBase->DisplayFlags);
}
