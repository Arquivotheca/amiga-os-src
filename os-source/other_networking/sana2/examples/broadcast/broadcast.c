/*
**  $Id: broadcast.c,v 1.1 92/01/02 13:58:22 dlarson Exp Locker: dlarson $
**
**  SANA-II driver utility -- Broadcast packets.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/

#include "/skeleton/skeleton.h"

struct MsgPort   *DevPort  = NULL;
struct IOSana2Req *IOB1    = NULL;

int    DeviceOpen = 0;
long   DevBits;

BOOL __saveds __asm CopyToBuff(register __a0 VOID *to,
				register __a1 VOID *from,
				register __d0 ULONG n)
{
	memcpy(to, from, n);
	return TRUE;
}

BOOL __saveds __asm CopyFromBuff(register __a0 VOID *to,
				register __a1 VOID *from,
				register __d0 ULONG n)
{
	memcpy(to, from, n);
	return TRUE;
}


main(argc , argv)
char *argv[];
{
char test[] =
"abcdefghijklmnopqrstuvwxyz\0";

	if(!initdev())
		printf("Open Failed\n");
	else
	{
		printf("Please enter a hexidecimal packet type to broadcast:  ");
		scanf("%lx", &IOB1->ios2_PacketType);
		IOB1->ios2_Data = test;
		IOB1->ios2_DataLength = 27L;
		printf("Attempting To Broadcast.\n");
		IOB1->ios2_Req.io_Error   = 0;
		IOB1->ios2_Req.io_Command = S2_BROADCAST;
		DoIO(IOB1);
		if(IOB1->ios2_Req.io_Error)
			printf("Error: %ld WireError: %ld\n",
				(long)IOB1->ios2_Req.io_Error,
				IOB1->ios2_WireError);
		else
			printf("No Error\n");

	}
bottom:	closedev();
}

