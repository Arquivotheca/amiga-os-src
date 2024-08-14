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

static printNB(struct NetBuff *nb)
{
	struct NetBuffSegment *np;

	printf("NetBuff.Count=%d, chain:\n", nb->Count);
	for(np = nb->List.mlh_Head; np->Node.mln_Succ; np = np->Node.mln_Succ){
		printf("\tPhySize=%d, Off=%d, Count=%d, Buf=%x\n",
			np->PhysicalSize, np->DataOffset, 
			np->DataCount, np->Buffer);
	}
	printf("\n");
}

main(int argc, char **argv)
{
	struct NetBuff nb;
	int rtn;

	NetBuffBase = OpenLibrary(nbname, 0);
	if(!NetBuffBase){
		printf("could not open %s\n", nbname);
		exit(RETURN_FAIL);
	}

/*	primeNB(100, 128);*/

	NewList((struct List *)&nb.List);
	
	AllocSegments(1024, &nb);
	printNB(&nb);

	rtn = ReadyNetBuff(&nb, 1000);
	printf("ReadyNetBuff returns %d\n", rtn);
	printNB(&nb);
}

