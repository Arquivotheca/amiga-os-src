#include <exec/types.h>
#include <stdio.h>
#include "netbuff.h"
#include "netbuff_protos.h"

struct Library *NetBuffBase;

void main( void )
	{
	struct Library *netbufflib;
	struct NetBuffSegment *netbuffsegment;
	struct MinList list;

	NewList(&list);

	if (!(netbufflib = (struct Library *)OpenLibrary(NETBUFFNAME, 0)))
		{
		printf("OpenLibrary failed!\n");
		exit(1);
		}

	printf("NetBuff = %8lx\n", netbufflib);
	NetBuffBase = netbufflib;

	AllocSegments(1, &list);

	while (netbuffsegment = RemHead(&list))
		{
		printf("\nNetBuffSegment = %8lx\n", netbuffsegment);
		printf("\tPSize = %8ld\n", netbuffsegment->PhysicalSize);
		printf("\tDOff  = %8ld\n", netbuffsegment->DataOffset);
		printf("\tDSize = %8ld\n", netbuffsegment->DataCount);
		printf("\tBuff  = %8lx\n", netbuffsegment->Buffer);
		FreeMem(netbuffsegment->Buffer, netbuffsegment->PhysicalSize);
		FreeMem(netbuffsegment, sizeof(struct NetBuffSegment));
		AllocSegments(1, &list);
		}

	CloseLibrary(netbufflib);
	}
