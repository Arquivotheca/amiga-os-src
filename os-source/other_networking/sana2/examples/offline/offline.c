/*
**  $Id: offline.c,v 1.5 92/01/02 14:45:40 dlarson Exp $
**
**  SANA-II driver utility -- Make online driver go offline.
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


main(argc , argv)
char *argv[];
{
	if (!initdev())
		printf("Open Failed\n");
	else
	{
		printf("Attempting To Go OffLine\n");
		IOB1->ios2_Req.io_Error   = 0;
		IOB1->ios2_Req.io_Command = S2_OFFLINE;
		DoIO(IOB1);
		if (IOB1->ios2_Req.io_Error)
			printf("Error: %ld WireError: %ld\n",
			       (long)IOB1->ios2_Req.io_Error,
			       IOB1->ios2_WireError);
		else
			printf("No Error\n");

	}
bottom:	closedev();
}