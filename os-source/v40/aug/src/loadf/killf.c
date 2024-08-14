
/* Don't use the Lattice 5.04 optimizer until volatile is fixed! */

#include "exec/types.h"
#include "exec/resident.h"

#define START_FMEM  ((UWORD *)0x00f00000)
#define END_FMEM    ((UWORD *)0x00f80000)
#define F_MAGIC     ((ULONG *)0x00f7ffe4)


void killname(name)
char *name;
{
UWORD  *wptr;
struct Resident * volatile romtag;
BOOL	lastmodule=1;	      /* If set, we are killing the last module */


    if(! strcmp(name,"exec.library") )
	{
	printf("Magically eliminating %s\n",name);
	*START_FMEM=0;	/* Kill Exec cookie */
	if(*START_FMEM)
	    printf("Error writing to $F00000 memory");
	}

    printf("Trying to kill %s\n",name);
    for(wptr=END_FMEM-2 ; wptr > START_FMEM ; wptr--)
	{
	if (*wptr == RTC_MATCHWORD)
	    {
	    romtag=(struct Resident *)wptr;
	    if (romtag->rt_MatchTag == romtag)
		if (! strcmp(romtag->rt_Name,name) )
		    {
		    romtag->rt_MatchTag = 0;
		    if( romtag->rt_MatchTag )
			{
			printf("Error writing to $F00000 memory\n");
			exit(10);
			}
		    printf("Killed %s (%lx)\n",romtag->rt_Name,romtag);

		    if(lastmodule)
			{
			printf("LoadF end marker bumped down to %lx\n",romtag);
			*F_MAGIC=(ULONG)romtag;
			}

		    /* If this is the last ROMTag in the F-memory */
		    /* if((ULONG)romtag->rt_EndSkip == *F_MAGIC)
			{
			printf("LoadF end marker bumped down to %lx\n",romtag);
			*F_MAGIC=(ULONG)romtag;
			} */
		    }
		else
		    {
		    lastmodule=0;
		    }
	    }
	}
}


void killall()
{
UWORD  *wptr;
struct Resident * volatile romtag;

    for(wptr=START_FMEM ; wptr < END_FMEM ; wptr++)
	{
	if (*wptr == RTC_MATCHWORD)
	    {
	    romtag=(struct Resident *)wptr;
	    if (romtag->rt_MatchTag == romtag)
		{
		romtag->rt_MatchTag=0;
		if( romtag->rt_MatchTag )
		    {
		    printf("Error writing to $F00000 memory\n");
		    exit(10);
		    }
		printf("Killed %s (%lx)\n",romtag->rt_Name,romtag);
		}
	    }
	}

    *START_FMEM=0;  /* Kill Exec cookie */
    *F_MAGIC=0;     /* Kill LoadF cookie */
    if(*START_FMEM)
	    {
	    printf("Error writing to $F00000 memory\n");
	    exit(10);
	    }
}


main(argc,argv)
int argc;
char *argv[];
{
int argindex;

    if(argc==1)
	killall();

    if(argc==2 && *argv[1]=='?')
	{
	printf("KillF 3.0b - Invalidate ROMTags in $F00000 memory\n");
	printf("USAGE: KillF <module> <module> <...>\n");
	exit(5);
	}

    for(argindex=(argc-1) ; argindex > 0  ; argindex--)
	killname(argv[argindex]);
}
