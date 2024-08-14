#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

main()
{
	struct Screen *myscreen;
	void *temp;
	int i;
	ULONG secs,micros,oldsecs;
	myscreen=OpenScreenTags(0,SA_DisplayID,0,SA_Depth,8,0);
	CurrentTime(&oldsecs,&micros);
	for(i=0;i<6000;i++)
	{
		myscreen->ViewPort.RasInfo->RxOffset=(i % 153);
		myscreen->ViewPort.RasInfo->RyOffset=(i % 187);
		temp=myscreen->ViewPort.DspIns->CopIns; /* save intermed ptr */
		myscreen->ViewPort.DspIns->CopIns=0;    /* zero it */
		myscreen->ViewPort.DspIns->CopLStart+=256*2*2; /* skip colors */
		ScrollVPort(&(myscreen->ViewPort));
		myscreen->ViewPort.DspIns->CopLStart-=256*2*2; /* correct back */
		myscreen->ViewPort.DspIns->CopIns=temp;
	}
	CurrentTime(&secs,&micros);
	printf("#secs=%d\n",secs-oldsecs);
	CloseScreen(myscreen);
}
