#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <stdio.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

main()
{
	int count,i;
	IntuitionBase=OpenLibrary("intuition.library",0);
	GfxBase=OpenLibrary("graphics.library",0);
	count=GfxBase->VBCounter;
	for(i=0;i<1000;i++)
	{
//		MakeScreen(IntuitionBase->ActiveScreen);
		MakeVPort(GfxBase->ActiView,&(IntuitionBase->ActiveScreen->ViewPort));
	}
	printf("count=%ld\n",GfxBase->VBCounter-count);
}
