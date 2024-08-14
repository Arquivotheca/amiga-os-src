#include <exec/types.h>
#include <stdio.h>
#include "netbuff.h"
#include "netbuff_protos.h"

struct Library *NetBuffBase;

void main( void )
	{
	struct Library *netbufflib;
	struct NetBuffSegment *netbuffsegment;
	struct NetBuffSegment *netbuffsegment1;
	struct NetBuff netbuff;
	int error;

	if (!(netbufflib = (struct Library *)OpenLibrary(NETBUFFNAME, 0)))
		{
		printf("OpenLibrary failed!\n");
		exit(1);
		}

	printf("NetBuff = %8lx\n", netbufflib);
	NetBuffBase = netbufflib;

	NewList(&netbuff.List);
	netbuff.Count = 0;

	if (error = ReadyNetBuff(&netbuff, 1070))
		{
		printf("ReadyNetBuff = %8ld\n", error);
		}

	printf("\nNetBuff = %8lx\n", &netbuff);
	printf("\tHead  = %8lx\n", netbuff.List.mlh_Head);
	printf("\tTail  = %8lx\n", netbuff.List.mlh_TailPred);
	printf("\tSize  = %8ld\n", netbuff.Count);

	for (netbuffsegment = netbuff.List.mlh_Head; netbuffsegment->Node.mln_Succ; netbuffsegment = netbuffsegment->Node.mln_Succ)
		{
		printf("\nNetBuffSegment = %8lx\n", netbuffsegment);
		printf("\tNext  = %8lx\n", netbuffsegment->Node.mln_Succ);
		printf("\tPrev  = %8lx\n", netbuffsegment->Node.mln_Pred);
		printf("\tPSize = %8ld\n", netbuffsegment->PhysicalSize);
		printf("\tDOff  = %8ld\n", netbuffsegment->DataOffset);
		printf("\tDSize = %8ld\n", netbuffsegment->DataCount);
		printf("\tBuff  = %8lx\n", netbuffsegment->Buffer);
		}

#define CHECKISCONT(a,b) printf("IsContiguous: %6ld %6ld = %1ld\n", (a), (b), IsContiguous(&netbuff, (a), (b)))

	CHECKISCONT(   0,   0);
	CHECKISCONT(   1,   0);
	CHECKISCONT(   2,   0);
	CHECKISCONT(1023,   0);
	CHECKISCONT(1024,   0);
	CHECKISCONT(1025,   0);
	CHECKISCONT(2045,   0);

	CHECKISCONT(   0,   1);
	CHECKISCONT(   1,   1);
	CHECKISCONT(   2,   1);
	CHECKISCONT(1023,   1);
	CHECKISCONT(1024,   1);
	CHECKISCONT(1025,   1);
	CHECKISCONT(2045,   1);

	CHECKISCONT(   0,   2);
	CHECKISCONT(   1,   2);
	CHECKISCONT(   2,   2);
	CHECKISCONT(1023,   2);
	CHECKISCONT(1024,   2);
	CHECKISCONT(1025,   2);
	CHECKISCONT(2045,   2);

	CHECKISCONT(   0,  20);
	CHECKISCONT(   1,  20);
	CHECKISCONT(   2,  20);
	CHECKISCONT(1023,  20);
	CHECKISCONT(1024,  20);
	CHECKISCONT(1025,  20);
	CHECKISCONT(2045,  20);

	CHECKISCONT(   0,   5);
	CHECKISCONT(   1,   5);
	CHECKISCONT(   2,   5);
	CHECKISCONT(1023,   5);
	CHECKISCONT(1024,   5);
	CHECKISCONT(1025,   5);
	CHECKISCONT(2045,   5);

	CHECKISCONT(   0,  90);
	CHECKISCONT(   1,  90);
	CHECKISCONT(   2,  90);
	CHECKISCONT(1023,  90);
	CHECKISCONT(1024,  90);
	CHECKISCONT(1025,  90);
	CHECKISCONT(2045,  90);

	FreeSegments(&netbuff.List);

	CloseLibrary(netbufflib);
	}
