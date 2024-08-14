#include <exec/types.h>
#include <graphics/view.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

int GfxBase;


main()
{
	int i,c,myvar ;
	struct ColorMap *mycm;
	GfxBase=OpenLibrary("graphics.library",0l);
	mycm=GetColorMap(256);
	for(i=0;i<256;i++)
	{
		printf("i=%ld l=%04x h=%04x\n",i,((UWORD *)(mycm->LowColorBits))[i],((UWORD *)(mycm->ColorTable))[i]);
	}

	FreeColorMap(mycm);

}
