/*
**  $Id: waitforonline.c,v 1.2 91/11/13 11:15:04 dlarson Exp $
**
**  SANA-II driver utility -- Wait for a driver to come online.
**			      (No wait if driver already online.)
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/


#define NOBUFFS 1
#include "/skeleton/skeleton.h"

struct MsgPort   *DevPort = NULL;
struct IOSana2Req *IOB1    = NULL;

int    DeviceOpen = 0;
long   DevBits;

void WaitForOnLine();

main(argc , argv)
char *argv[];
{
	if(!initdev())
		printf("Open Failed\n");
	else
	{
		printf("Waiting For OnLine\n");
		WaitForOnLine();
		if(IOB1->ios2_Req.io_Error)
			printf("ERROR CODE: 0x%lx\n",
				(long)IOB1->ios2_Req.io_Error);
		else
			printf("OnLine Occurred - No Error\n");
	}
bottom:	closedev();
}

void WaitForOnLine()
{
	IOB1->ios2_Req.io_Error     = 0;
	IOB1->ios2_Req.io_Flags     = IOF_QUICK;
	IOB1->ios2_Req.io_Command   = S2_ONEVENT;
	IOB1->ios2_WireError = S2EVENT_ONLINE;
	DoIO(IOB1);
}
