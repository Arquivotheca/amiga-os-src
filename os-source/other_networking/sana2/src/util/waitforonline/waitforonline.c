/*
**  $Id: waitforonline.c,v 1.3 92/01/02 14:49:33 dlarson Exp $
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

main(argc , argv)
char *argv[];
{
	if(!initdev())
		printf("Open Failed\n");
	else
	{
		printf("Waiting For OnLine\n");
		IOB1->ios2_Req.io_Error     = 0;
		IOB1->ios2_Req.io_Command   = S2_ONEVENT;
		IOB1->ios2_WireError = S2EVENT_ONLINE;
		DoIO(IOB1);
		if(IOB1->ios2_Req.io_Error)
			printf("ERROR CODE: 0x%lx\n",
				(long)IOB1->ios2_Req.io_Error);
		else
			printf("OnLine Occurred - No Error\n");
	}
bottom:	closedev();
}
