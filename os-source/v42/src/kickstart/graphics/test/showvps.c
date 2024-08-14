#include <graphics/gfxbase.h>
#include <graphics/view.h>

struct GfxBase *GfxBase;

main()
{
	struct ViewPort *vp;
	GfxBase=(struct GfxBase *) OpenLibrary("graphics.library",0);
	printf("Active view : %08lx dyoffset=%ld\n",GfxBase->ActiView,GfxBase->ActiView->DyOffset);
	printf("viewports:\n");
	for(vp=GfxBase->ActiView->ViewPort; vp ; vp=vp->Next)
	{
		printf("viewport at %08lx modes=%04x DyOffset=%ld DHeight=%ld\n",vp,vp->Modes,vp->DyOffset,vp->DHeight);
	}
}
