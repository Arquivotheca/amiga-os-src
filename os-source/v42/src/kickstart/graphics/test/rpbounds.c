/* continuosly print the rp_bounds of the active window */

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <pragmas/graphics_pragmas.h>
#include "/rpattr.h"

struct IntuitionBase *IntuitionBase;
struct Library *GfxBase;

struct Rectangle myrect;

ULONG tags[]={RPTAG_DrawBounds,&myrect,0};

main()
{
	IntuitionBase=OpenLibrary("intuition.library",39);
	GfxBase=OpenLibrary("graphics.library",39);
	for(;;)
	{
		struct Window *mywindow=IntuitionBase->ActiveWindow;
		if (mywindow)
		{
			GetRPAttrA(mywindow->RPort,tags);
			kprintf("bounds=(%ld,%ld)-(%ld,%ld)\n",myrect.MinX,myrect.MinY,myrect.MaxX,myrect.MaxY);
		}
		Delay(50);
	}
}
