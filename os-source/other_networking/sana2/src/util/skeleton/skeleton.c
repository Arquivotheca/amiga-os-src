/*
**  $Id: skeleton.c,v 1.2 91/11/08 10:47:58 dlarson Exp $
**
**  SANA-II driver utility skeleton functions.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/


#include "skeleton.h"

int initdev(void)
{
char device[80];
char buff[80];
LONG unit;
struct TagItem MyTags[2];

	printf("Device name to open:  ");
	if(GetVar("SANA2.Device", device, 80, 0) == -1)
		scanf("%s", device);
	else
		printf("%s\n", device);
	printf("Unit number to open:  ");
	if(GetVar("SANA2.Unit", buff, 80, 0) == -1)
		scanf("%d", &unit);
	else
	{
		sscanf(buff, "%ld", &unit);
		printf("%ld\n", unit);
	}
	printf("\n");

	if ( !(DevPort = CreateMsgPort()) )
	{
		printf("Could not allocate port.\n");
		return(0);
	}

	if (!(IOB1 = CreateIORequest(DevPort, sizeof(struct IOSana2Req))))
	{
		printf("Could not allocate extio block.\n");
		return(0);
	}

	MyTags[0].ti_Tag = S2_CopyToBuff;
	MyTags[0].ti_Data = (ULONG) CopyToBuff;
	MyTags[1].ti_Tag = S2_CopyFromBuff;
	MyTags[1].ti_Data = (ULONG) CopyFromBuff;
	IOB1->ios2_BufferManagement = MyTags;

	if (OpenDevice(device, unit, IOB1, 0))
	{
		printf("Could not open device.\n");
		return(0);
	}

	DeviceOpen = 1;
	DevBits = 1L << DevPort->mp_SigBit;
	return(1);
}


void closedev(void)
{
	if(DeviceOpen)
	{
		printf("\nAttempting To Close Unit.\n");
		CloseDevice(IOB1);
		printf("Closed.\n");
	}
	if(IOB1)
		DeleteIORequest(IOB1);
	if(DevPort)
		DeleteMsgPort(DevPort);
}
