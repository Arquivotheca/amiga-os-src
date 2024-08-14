/************************************************************************/
/* Test out the virtual memory handler. Compile by:			*/
/*									*/
/*	lc -d3 -L vtest.c						*/
/************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <proto/exec.h>

#define MEMF_VIRTUAL	(1<<3)
#define REGION		(1024*1024*4)

ULONG BASE, END;

struct ExecBase *ExecBase = NULL;

int CXBRK()
{
	FreeMem(BASE,REGION);
	return(1);
}

void main ()
{
    ULONG	*here, temp;
    long t;
    ExecBase = (struct ExecBase *) OpenLibrary ("exec.library", 0);



    if (!(BASE = AllocMem(REGION,MEMF_VIRTUAL)))
	exit(20);
    END = BASE+REGION;


/*	BASE = 0x2100000;
	END = 0x2500000;
*/
    while(TRUE)
      {

		/* Pass 1 */

	time(&t); printf("Current time: %s\n",ctime(&t));
	for(here = (ULONG *)BASE; here < (ULONG *)END; here++)
	    {
		here[0] = (ULONG)here;
	    }
	printf("Finished Writing, pass 1\n");
	time(&t); printf("Current time: %s\n",ctime(&t));

	for(here = (ULONG *)(END); here > (ULONG *)BASE; here--)
	    if(here[0] != (ULONG)here)
		{
		    printf("Wrong numbers address:%lx value:%lx\n",here[0],(ULONG)here);
		}
	printf ("Finished Reading, pass 1\n");




		/* Pass 2 */

	for(here = (ULONG *)BASE; here < (ULONG *)END; here++)
	    {
		here[0] =(ULONG)here+3L;
	    }
	printf("Finished Writing, pass 2\n");

	for(here = (ULONG *)BASE; here < (ULONG *)END; here++)
	    if(here[0] != (ULONG)here+3L)
		{
		    printf("Wrong numbers address:%lx value:%lx\n",here[0],(ULONG)here);
		}
	printf("Finished Reading, pass 2\n");




		/* Pass 3 */

	for(here = (ULONG *)END; here > (ULONG *)BASE; here--)
	    {
		here[0] =(ULONG)here+5L;
		temp = here[0];
	    }
	printf("Finished Writing, pass 3\n");

	for(here = (ULONG *)BASE; here < (ULONG *)END; here++)
	    if(here[0] != (ULONG)here+5L)
		{
		    printf("Wrong numbers address:%lx value:%lx\n",here[0],(ULONG)here);
		}
	printf("Finished Reading, pass 3\n");

	/* End */

      }

	FreeMem(BASE,REGION);
	CloseLibrary((struct Library *)ExecBase);
}
