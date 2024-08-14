/* test bug report #15055 */

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <pragmas/graphics_pragmas.h>
#include <graphics/rastport.h>

struct IntuitionBase *IntuitionBase;
struct Library *GfxBase;

struct RastPort myrp;

main()
{
	struct Window *mywindow;
	IntuitionBase=OpenLibrary("intuition.library",39);
	GfxBase=OpenLibrary("graphics.library",39);
	myrp=IntuitionBase->ActiveScreen->RastPort;
	Move(&myrp,0,0);
	ClearScreen(&myrp);
}
