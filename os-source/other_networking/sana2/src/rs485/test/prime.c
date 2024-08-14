/*
 * netbuff test program
 */

#include <exec/memory.h>

#include <proto/exec.h>
#include <pragmas/exec.h>

#include <libraries/netbuff.h>
#include <pragmas/netbuff.h>
#include <proto/netbuff.h>

#include <libraries/dos.h>

struct Library *NetBuffBase;

static char nbname[] = "netbuff.library";

static primeNB(int numseg, int segsize)
{
	struct NetBuffSegment *np;
	struct NetBuff nb;
	int i;

	NewList((struct List *)&nb.List);
	for(i = 0; i < numseg; i++){
		np = AllocMem(sizeof(*np), MEMF_CLEAR | MEMF_PUBLIC);
		if(!np){
			break;
		}

		np->Buffer = AllocMem(segsize, MEMF_CLEAR | MEMF_PUBLIC);
		if(np->Buffer){
			np->PhysicalSize = segsize;
		}

		AddTail((struct List *)&nb.List, (struct Node *)&np->Node);
	}

	printf("Primed %s with %d segs of size %d\n", nbname, numseg, segsize);
	FreeSegments(&nb);
}

main(int argc, char **argv)
{
	NetBuffBase = OpenLibrary(nbname, 0);
	if(!NetBuffBase){
		printf("could not open %s\n", nbname);
		exit(RETURN_FAIL);
	}

	primeNB(atoi(argv[1]), atoi(argv[2]));

	CloseLibrary(NetBuffBase);
}

