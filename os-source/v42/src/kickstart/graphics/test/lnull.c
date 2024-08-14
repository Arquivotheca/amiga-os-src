#include <graphics/gfxbase.h>
struct GfxBase *GfxBase;
main()
{
	GfxBase=OpenLibrary("graphics.library",0);
	LoadView(0l);
	printf("copadr=%08lx\n",GfxBase->LOFlist);
}
