#include <exec/types.h>
#include <stdio.h>
#include "netbuff.h"
#include "netbuff_protos.h"

struct Library *NetBuffBase;

void main( void )
	{
	struct Library *netbufflib;
	struct NetBuffSegment *netbuffsegment;
	struct NetBuff netbuff;
	int error;
	struct MinList list;

	NewList(&list);

	if (!(netbufflib = (struct Library *)OpenLibrary(NETBUFFNAME, 0)))
		{
		printf("OpenLibrary failed!\n");
		exit(1);
		}

	printf("NetBuff = %8lx\n", netbufflib);
	NetBuffBase = netbufflib;

	NewList(&netbuff.List);
	netbuff.Count = 0;

	if (error = ReadyNetBuff(&netbuff, 1))
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

	if (error = ReadyNetBuff(&netbuff, 2049))
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

	FreeSegments(&netbuff.List);

	CloseLibrary(netbufflib);
	}
