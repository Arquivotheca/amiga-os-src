#include <exec/types.h>
#include <graphics/view.h>

ULONG GfxBase;

main()
{
	int i,j;
	GfxBase=OpenLibrary("graphics.library",0l);
	for(i=0;i<150;i++)
	{
		for(j=0;j<60;j++);
			WaitTOF();
		SetRGB4(0,0,i & 15,0,0);
	}
}
