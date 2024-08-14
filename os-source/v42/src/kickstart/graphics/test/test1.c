#include <graphics/gfxbase.h>

struct View * oldview;

struct GfxBase *GfxBase;
ULONG IntuitionBase;
main()
{
	GfxBase=OpenLibrary("graphics.library",0l);
	IntuitionBase=OpenLibrary("intuition.library",0);
	CloseWorkBench();
	oldview=GfxBase->ActiView;
	LoadView(0l);
	WaitTOF();
	WaitTOF();
	LoadView(oldview);
	OpenWorkBench();
}
