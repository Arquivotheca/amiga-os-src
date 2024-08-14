/*
**  $Id: catch.c,v 1.1 91/12/13 12:14:17 dlarson Exp Locker: dlarson $
**
**  SANA-II driver utility -- Catch packets sent to us (or broadcast).
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


main(int argc , char *argv[])
{
char test[512];

	if(!initdev())
	{
		printf("Open Failed\n");
		goto bottom;
	}
	IOB1->ios2_Data = test;
	IOB1->ios2_DataLength = 512L;
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Command = S2_READORPHAN;
	printf("Attempting To Catch.\n");
	DoIO(IOB1);
	if(IOB1->ios2_Req.io_Error)
	{
		printf("Error: %ld WireError: %ld\n",
			(long)IOB1->ios2_Req.io_Error,
			IOB1->ios2_WireError);
		goto bottom;
	}
	printf("No Error\n");
	printf("data: %s", IOB1->ios2_Data);
bottom:	closedev();
}

