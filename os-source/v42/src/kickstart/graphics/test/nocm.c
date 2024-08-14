#include <exec/types.h>
#include <graphics/view.h>
#include <intuition/intuition.h>

ULONG GfxBase;
ULONG IntuitionBase;

UWORD mycolors[1];

main()
{
	int i,j;
	struct ColorMap *oldcm;
	struct Screen *wbscreen;
	GfxBase=OpenLibrary("graphics.library",0l);
	IntuitionBase=OpenLibrary("intuition.library",0);
	wbscreen=LockPubScreen(0);
	oldcm=wbscreen->ViewPort.ColorMap;
	wbscreen->ViewPort.ColorMap=0;
	for(i=0;i<150;i++)
	{
		for(j=0;j<60;j++);
			WaitTOF();
		mycolors[0]=i & 15;
		LoadRGB4(&(wbscreen->ViewPort),mycolors,1);
	}
	wbscreen->ViewPort.ColorMap=oldcm;
	UnlockPubScreen(wbscreen);
}
