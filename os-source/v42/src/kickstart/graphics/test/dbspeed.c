#include <exec/types.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

main()
{
	struct Screen *myscreen;
	struct DBufInfo *mydbi;
	void *temp;
	int i;
	ULONG secs,micros,oldsecs;
	myscreen=OpenScreenTags(0,SA_DisplayID,0,SA_Depth,8,0);
	mydbi=AllocDBufInfo(&(myscreen->ViewPort));
	CurrentTime(&oldsecs,&micros);
	for(i=0;i<60000;i++)
	{
		temp=myscreen->ViewPort.DspIns->CopIns; /* save intermed ptr */
		myscreen->ViewPort.DspIns->CopIns=0;    /* zero it */
		ChangeVPBitMap(&(myscreen->ViewPort),myscreen->RastPort.BitMap,mydbi);
		myscreen->ViewPort.DspIns->CopIns=temp;
	}
	CurrentTime(&secs,&micros);
	printf("#secs=%d\n",secs-oldsecs);
	FreeDBufInfo(mydbi);
	CloseScreen(myscreen);
}
