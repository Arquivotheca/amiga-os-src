#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netbuff.h"
#include "netbuff_protos.h"

#define SEGSIZE 63
#define NUMSEGS 1024
#define BUFF_SIZE 1024

struct Library *NetBuffBase;

void main( void )
	{
	struct Library *netbufflib;
	struct NetBuffSegment *netbuffsegment;
	struct NetBuff netbuff1;
	struct NetBuff netbuff2;
	int error;
	int i,j,k;
	char *buffer1;
	char *buffer2;
	char *bptr;
	struct MinList list;

	NewList(&list);
	srand(time(NULL));

	if (!(netbufflib = (struct Library *)OpenLibrary(NETBUFFNAME, 0)))
		{
		printf("OpenLibrary failed!\n");
		exit(1);
		}

	printf("NetBuff = %8lx\n", netbufflib);
	NetBuffBase = netbufflib;

	for (i=NUMSEGS; i; i--)
		{
		j = rand() & SEGSIZE;

		if (!(netbuffsegment = (struct NetBuffSegment *)AllocMem(sizeof(struct NetBuffSegment)+j, 0)))
			{
			printf("AllocMem fialed!\n");
			CloseLibrary(netbufflib);
			exit(1);
			}

		netbuffsegment->PhysicalSize = j;
		netbuffsegment->Buffer = ((UBYTE *)netbuffsegment) + sizeof(struct NetBuffSegment);
		netbuffsegment->DataOffset = 0;
		netbuffsegment->DataCount = 0;
		AddHead(&list, netbuffsegment);
		FreeSegments(&list);
		}

	if (!(buffer1 = malloc(BUFF_SIZE)))
		{
		printf("malloc failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	if (!(buffer2 = malloc(BUFF_SIZE)))
		{
		printf("malloc failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	printf("\nbuffer1 = %8lx\n", buffer1);
	printf("\nbuffer2 = %8lx\n", buffer2);

	bptr = buffer1;
	for (i=0; i<BUFF_SIZE-1; i+=2)
		{
		*bptr++ = (i>>6)&0xfe;
		*bptr++ = (i<<1)|1;
		}

	NewList(&netbuff1.List);
	netbuff1.Count = 0;

	NewList(&netbuff2.List);
	netbuff2.Count = 0;

	for (;;)
		{
		i = rand() % BUFF_SIZE;
		j = rand() % (BUFF_SIZE-i);
		k = rand() % (BUFF_SIZE-i);

		printf("%5d %5d %5d\n", i, j, k);

		if (error = ReadyNetBuff(&netbuff1, i))
			{
			printf("ReadyNetBuff1 = %8ld\n", error);
			}

		if (error = CopyToNetBuff(&netbuff1, 0, buffer1+j, i))
			{
			printf("CopyToNetBuff = %8ld\n", error);
			}

		if (error = CopyNetBuff(&netbuff1, &netbuff2))
			{
			printf("CopyNetBuff = %8ld\n", error);
			}

		if (error = CopyFromNetBuff(&netbuff2, 0, buffer2+k, i))
			{
			printf("CopyFromNetBuff = %8ld\n", error);
			}

		if (error = memcmp(buffer1+j, buffer2+k, i))
			{
			printf("memcmp = %8ld\n", error);
			}

		FreeSegments(&netbuff1.List);
		FreeSegments(&netbuff2.List);
		}

	CloseLibrary(netbufflib);
	}
