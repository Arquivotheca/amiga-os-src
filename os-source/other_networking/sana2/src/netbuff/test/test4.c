#include <exec/types.h>
#include <stdio.h>
#include "netbuff.h"
#include "netbuff_protos.h"

#define SEGSIZE 16
#define NUMSEGS 32

struct Library *NetBuffBase;

void main( void )
	{
	struct Library *netbufflib;
	struct NetBuffSegment *netbuffsegment;
	struct NetBuff netbuff1;
	struct NetBuff netbuff2;
	int error;
	int i,j;
	char *buffer;
	char *bptr;
	struct MinList list;

	NewList(&list);

	if (!(netbufflib = (struct Library *)OpenLibrary(NETBUFFNAME, 0)))
		{
		printf("OpenLibrary failed!\n");
		exit(1);
		}

	printf("NetBuff = %8lx\n", netbufflib);
	NetBuffBase = netbufflib;

	for (i=NUMSEGS; i; i--)
		{
		if (!(netbuffsegment = (struct NetBuffSegment *)AllocMem(sizeof(struct NetBuffSegment)+SEGSIZE, 0)))
			{
			printf("AllocMem fialed!\n");
			CloseLibrary(netbufflib);
			exit(1);
			}

		netbuffsegment->PhysicalSize = SEGSIZE;
		netbuffsegment->Buffer = ((UBYTE *)netbuffsegment) + sizeof(struct NetBuffSegment);
		netbuffsegment->DataOffset = 0;
		netbuffsegment->DataCount = 0;
		AddHead(&list, netbuffsegment);
		FreeSegments(&list);
		}

	if (!(buffer = malloc(1024)))
		{
		printf("malloc failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	printf("\nbuffer = %8lx\n", buffer);

	bptr = buffer;
	for (i=0; i<16; i+=2)
		{
		for (j=1; j<256; j+=2)
			{
			*bptr++ = i;
			*bptr++ = j;
			}
		}

	NewList(&netbuff1.List);
	netbuff1.Count = 0;

	NewList(&netbuff2.List);
	netbuff2.Count = 0;

	if (error = ReadyNetBuff(&netbuff1, 20))
		{
		printf("ReadyNetBuff1 = %8ld\n", error);
		CloseLibrary(netbufflib);
		exit(1);
		}

	if (error = ReadyNetBuff(&netbuff2, 0))
		{
		printf("ReadyNetBuff2 = %8ld\n", error);
		CloseLibrary(netbufflib);
		exit(1);
		}

	printf("\nNetBuff1 = %8lx\n", &netbuff1);
	printf("\tHead  = %8lx\n", netbuff1.List.mlh_Head);
	printf("\tTail  = %8lx\n", netbuff1.List.mlh_TailPred);
	printf("\tSize  = %8ld\n", netbuff1.Count);

	printf("\nNetBuff2 = %8lx\n", &netbuff2);
	printf("\tHead  = %8lx\n", netbuff2.List.mlh_Head);
	printf("\tTail  = %8lx\n", netbuff2.List.mlh_TailPred);
	printf("\tSize  = %8ld\n", netbuff2.Count);


	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer,  0);
	printf("CopyToNetBuff1 = %8ld\n", error);

	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer,  1);
	printf("CopyToNetBuff1 = %8ld\n", error);

	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer,  8);
	printf("CopyToNetBuff1 = %8ld\n", error);

	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer, 12);
	printf("CopyToNetBuff1 = %8ld\n", error);

	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer, 16);
	printf("CopyToNetBuff1 = %8ld\n", error);

	error = CopyToNetBuff(&netbuff1, -(netbuff1.Count-1), buffer, 19);
	printf("CopyToNetBuff1 = %8ld\n", error);

	FreeSegments(&netbuff1.List);
	FreeSegments(&netbuff2.List);

	CloseLibrary(netbufflib);
	}
