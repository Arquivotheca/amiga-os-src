/*
**  $Id: online.c,v 1.4 91/11/13 11:10:51 dlarson Exp $
**
**  SANA-II driver utility -- Make offline driver come online.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/

#define NOBUFFS
#include "/skeleton/skeleton.h"

struct MsgPort   *DevPort = NULL;
struct IOSana2Req *IOB1    = NULL;

int    DeviceOpen = 0;
long   DevBits;

void DoOnLine();


main(argc , argv)
char *argv[];
{
	if(!initdev())
		printf("Open Failed\n");
	else
	{
		printf("Attempting To Go OnLine\n");
		DoOnLine();
		if(IOB1->ios2_Req.io_Error)
			printf("Error: %ld WireError: %ld\n",
				(long)IOB1->ios2_Req.io_Error,
				IOB1->ios2_WireError);
		else
			printf("No Error\n");

	}
bottom:	closedev();
}

void DoOnLine()
{
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Flags   = IOF_QUICK;
	IOB1->ios2_Req.io_Command = S2_ONLINE;
	DoIO(IOB1);
}
