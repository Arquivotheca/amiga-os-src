/* test bug report #15055 */

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <pragmas/graphics_pragmas.h>

struct IntuitionBase *IntuitionBase;
struct Library *GfxBase;


main()
{
	struct Window *mywindow;
	IntuitionBase=OpenLibrary("intuition.library",39);
	GfxBase=OpenLibrary("graphics.library",39);
	mywindow=IntuitionBase->ActiveWindow;
	ScrollRaster(mywindow->RPort,-5,-10,1000,1000,1120,1120);
}
