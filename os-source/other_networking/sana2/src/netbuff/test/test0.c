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
	printf("List = %8lx\n", &list);

	if (!(netbuffsegment = (struct NetBuffSegment *)AllocMem(sizeof(struct NetBuffSegment)+1024, 0)))
		{
		printf("AllocMem failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	printf("NetBuffSegment = %8lx\n", netbuffsegment);

	netbuffsegment->PhysicalSize = 1024;
	netbuffsegment->Buffer = ((UBYTE *)netbuffsegment) + sizeof(struct NetBuffSegment);
	netbuffsegment->DataOffset = 0;
	netbuffsegment->DataCount = 0;
	AddHead(&list, netbuffsegment);

	FreeSegments(&list);

	CloseLibrary(netbufflib);
	}
