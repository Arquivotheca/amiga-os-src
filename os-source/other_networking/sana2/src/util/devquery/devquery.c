/*
**  $Id: devquery.c,v 1.2 92/01/02 13:46:31 dlarson Exp $
**
**  SANA-II driver device query example.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/

#define NOBUFFS
#include "/skeleton/skeleton.h"

struct MsgPort   *DevPort = NULL;
struct IOSana2Req *IOB1   = NULL;

int    DeviceOpen = 0;
long   DevBits;


main(int argc, char *argv[])
{
struct Sana2DeviceQuery query;

	if(!initdev())
	{
		printf("Open Failed\n");
		goto bottom;
	}
        printf("About to Query the device.\n");
	query.SizeAvailable = sizeof(query);
        IOB1->ios2_Req.io_Error   = 0;
        IOB1->ios2_Req.io_Command = S2_DEVICEQUERY;
        IOB1->ios2_StatData = &query;
        DoIO((struct IORequest *)IOB1);
        IOB1->ios2_StatData = NULL;
	if(IOB1->ios2_Req.io_Error)
	{
            	printf("ERROR RETURNED:%ld  WERR:%ld\n",
            	       (long)IOB1->ios2_Req.io_Error,
            	       (long)IOB1->ios2_WireError);
		goto bottom;
     	}
	printf("Query results:\n");
	printf("Size available:%ld  Size Supplied:%ld  DevQueryFormat:%ld\n",
	       query.SizeAvailable, query.SizeSupplied, query.DevQueryFormat);
	printf("DeviceLevel:%ld\n", query.DeviceLevel);
	printf("Number of bits in address:%ld\n", (long)query.AddrFieldSize);
	printf("Maximum Transmition Unit: %ld\n", query.MTU);
	printf("Bits Per Second:          %ld\n", query.BPS);
	printf("HardwareType:             %ld\n", query.HardwareType);
bottom:	closedev();
}
