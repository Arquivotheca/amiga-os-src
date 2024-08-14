#include <exec/types.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>

ULONG GfxBase;
ULONG IntuitionBase;

ULONG must_remake;

ULONG tags[]={
	VTAG_IMMEDIATE,&must_remake,
	VTAG_BORDERBLANK_SET,-1,TAG_END };

main()
{
	int i,j;
	struct ColorMap *oldcm;
	struct Screen *wbscreen;
	GfxBase=OpenLibrary("graphics.library",0l);
	IntuitionBase=OpenLibrary("intuition.library",0);
	wbscreen=LockPubScreen(0);
	UnlockPubScreen(wbscreen);
	VideoControl(wbscreen->ViewPort.ColorMap,tags);
	printf("must_remake=%ld\n",must_remake);
}
