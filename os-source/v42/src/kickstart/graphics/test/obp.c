/* test obtainbestpen possible bug by asking for rgb=0,0,0 */

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
main()
{
	ULONG gotpen;
	IntuitionBase=OpenLibrary("intuition.library",39l);
	GfxBase=OpenLibrary("graphics.library",39l);
	gotpen=ObtainBestPen(IntuitionBase->ActiveScreen->ViewPort.ColorMap,0,0,0,0);
	printf("gotpen=%d\n",gotpen);
	Delay(50*20);
	ReleasePen(IntuitionBase->ActiveScreen->ViewPort.ColorMap,gotpen);
}
