#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/scale.h>

#include <stdio.h>

struct Screen *myscreen;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

main()
{
	int d;
	struct BitMap *srcbm;
	IntuitionBase=(struct IntuitionBase *) OpenLibrary("intuition.library",0l);
	GfxBase=(struct GfxBase *) OpenLibrary("graphics.library",0l);
	srcbm=&(IntuitionBase->FirstScreen->BitMap);
	myscreen=OpenScreenTags(0l,SA_DisplayID,SUPER_KEY,SA_Width,1500);
	if (myscreen)
	{
		struct BitScaleArgs b;
		for(d=1;d<1499;d+=25)
		{
			b.bsa_SrcX = b.bsa_SrcY = b.bsa_DestX = b.bsa_DestY = 0;
			b.bsa_SrcWidth = b.bsa_XSrcFactor = 1008;
			b.bsa_SrcHeight = b.bsa_YSrcFactor = 512;
			b.bsa_DestWidth= b.bsa_XDestFactor= d; b.bsa_DestHeight=b.bsa_YDestFactor = 200;
			b.bsa_SrcBitMap=srcbm;
			b.bsa_DestBitMap=&(myscreen->BitMap);
			b.bsa_Flags=0;
			BitMapScale(&b);
		}
			

		CloseScreen(myscreen);
	}
	CloseLibrary(GfxBase);
	CloseLibrary(IntuitionBase);
}
