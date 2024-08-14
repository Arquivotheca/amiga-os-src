#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netbuff.h"
#include "netbuff_protos.h"

#define SEGSIZE 15
#define NUMSEGS 32
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
	FILE *file;
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

	for (i=BUFF_SIZE; i>0; i-=j)
		{
		j = rand() & SEGSIZE;

		if (!(netbuffsegment = (struct NetBuffSegment *)AllocMem(sizeof(struct NetBuffSegment)+j, 0)))
			{
			printf("AllocMem failed!\n");
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

	if (!(file = fopen("test.txt","r")))
		{
		printf("fopen failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	if (!(i = fread(buffer1, 1, BUFF_SIZE-1, file)))
		{
		printf("fread failed!\n");
		CloseLibrary(netbufflib);
		exit(1);
		}

	printf("fread = %d\n", i);
	buffer1[i] = 0;

	NewList(&netbuff1.List);
	netbuff1.Count = 0;

	NewList(&netbuff2.List);
	netbuff2.Count = 0;


	for (;;)
		{
		k = (rand() % (i+1));
		printf("%8ld  %8ld\n", i, k);

		if (error = ReadyNetBuff(&netbuff1, i))
			{
			printf("ReadyNetBuff1 = %8ld\n", error);
			}

		if (error = CopyToNetBuff(&netbuff1, 0, buffer1, i))
			{
			printf("CopyToNetBuff = %8ld\n", error);
			}

		if (error = SplitNetBuff(&netbuff1, k, &netbuff2))
			{
			printf("SplitNetBuff = %8ld\n", error);
			}

		printf("\t%8ld  %8ld\n", netbuff1.Count, netbuff2.Count);

		if (k >= 0)
			{
			if (error = PrependNetBuff(&netbuff1, &netbuff2))
				{
				printf("PrependNetBuff = %8ld\n", error);
				}
			}
		else
			{
			if (error = NetBuffAppend(&netbuff1, &netbuff2))
				{
				printf("NetBuffAppend = %8ld\n", error);
				}
			}

		printf("\t%8ld  %8ld\n", netbuff1.Count, netbuff2.Count);

		if (error = CopyFromNetBuff(&netbuff1, 0, buffer2, netbuff1.Count))
			{
			printf("CopyFromNetBuff1 = %8ld\n", error);
			}

		if (error = memcmp(buffer1, buffer2, i))
			{
			printf("memcmp = %8ld\n", error);
			}

		FreeSegments(&netbuff1.List);
		FreeSegments(&netbuff2.List);
		}


	CloseLibrary(netbufflib);
	}
