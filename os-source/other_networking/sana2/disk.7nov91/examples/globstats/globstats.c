/*
**  $Id: globstats.c,v 1.4 91/11/13 11:18:52 dlarson Exp $
**
**  SANA-II driver utility -- Print statistics for device.
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
struct Sana2DeviceStats Stats;

int    DeviceOpen = 0;
long   DevBits;

void PrintStats(void);
void GetGlobalStats(void);


main(argc , argv)
char *argv[];
{
	if(!initdev())
		printf("Open Failed.\n");
	else
	{
		GetGlobalStats();
		if (!IOB1->ios2_Req.io_Error)
			PrintStats();
		else
			printf("Error Returned: %ld WireError: %ld\n" , IOB1->ios2_Req.io_Error , IOB1->ios2_WireError);
	}
bottom:	closedev();
	return(0);
}

void GetGlobalStats(void)
{
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Flags   = IOF_QUICK;
	IOB1->ios2_Req.io_Command = S2_GETGLOBALSTATS;
	IOB1->ios2_StatData = &Stats;
	DoIO(IOB1);
}

void PrintStats(void)
{
	printf("Packets Received:    %d\n" , Stats.PacketsReceived);
	printf("Packets Transmitted: %d\n" , Stats.PacketsSent);
	printf("CRC Errors:          %d\n" , Stats.BadData);
	printf("Unknown Packets:     %d\n" , Stats.UnknownTypesReceived);
	printf("Fifo Overruns:       %d\n" , Stats.Overruns);
	printf("Reconfigurations:    %d\n" , Stats.Reconfigurations);
}
