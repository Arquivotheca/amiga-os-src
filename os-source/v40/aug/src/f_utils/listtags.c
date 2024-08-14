
#include "exec/types.h"
#include "exec/resident.h"

#define START_FMEM  ((UWORD *)0x00f00000)
#define END_FMEM    ((UWORD *)0x00f80000)


void main()
{
UWORD  *wptr;
struct Resident *romtag;

    for(wptr=START_FMEM ; wptr < END_FMEM ; wptr++)
	{
	if (*wptr == RTC_MATCHWORD)
	    {
	    romtag=(struct Resident *)wptr;
	    if (romtag->rt_MatchTag == romtag)
		{
		printf("----ROMTag found at $%lx.  Printing rt_Name and rt_IdString\n");
		printf("%s\n",romtag->rt_Name);
		printf("%s\n\n",romtag->rt_IdString);
		}
	    }
	}
}
