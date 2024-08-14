/*
**  $Id: typestats.c,v 1.6 92/01/02 14:28:02 dlarson Exp $
**
**  SANA-II driver utility -- Gather and print stats for one type of packet.
**			      (Over a specified period of time.)
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/

#define NOBUFFS 1
#include "/skeleton/skeleton.h"
#include <stdio.h>

struct MsgPort   *DevPort = NULL;
struct IOSana2Req *IOB1    = NULL;
struct Sana2PacketTypeStats Stats;

int    DeviceOpen = 0;
long   DevBits;

USHORT PacketType = 0x6003;

void GetTypeStats();
void PrintStats();


main(argc , argv)
char *argv[];
{
int ticks = 500;

	if(!initdev())
	{
		printf("Open Failed\n");
		goto bottom;
	}
	if (argc > 1)
		PacketType = atoi(argv[1]);
	if (argc > 2)
		ticks = atoi(argv[2]);
	printf("Tracking Packet Type: 0x%lx\n" , PacketType);
	IOB1->ios2_PacketType = PacketType;
	if (TrackType())
	{
		printf("could not track type: 0x%lx\n" , PacketType);
		goto bottom;
	}
	printf("Waiting for %ld seconds.\n\n", ticks/50);
	Delay(ticks);
	GetTypeStats();
	if (!IOB1->ios2_Req.io_Error)
		PrintStats();
	else
		printf("Error Returned: %ld WireError: %ld\n" , IOB1->ios2_Req.io_Error , IOB1->ios2_WireError);
bottom:	UnTrackType();
	closedev();
}

TrackType()
{
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Command = S2_TRACKTYPE;
	DoIO(IOB1);
	if (IOB1->ios2_Req.io_Error) {
		printf("TRACKTYPE Error: %ld WireError: %ld\n" , (long) IOB1->ios2_Req.io_Error , IOB1->ios2_WireError);
		return(1);
	}
	return(0);
}

UnTrackType()
{
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Command = S2_UNTRACKTYPE;
	DoIO(IOB1);
	if (IOB1->ios2_Req.io_Error) {
		printf("UNTRACKTYPE Error: %ld WireError: %ld\n" , (long) IOB1->ios2_Req.io_Error , IOB1->ios2_WireError);
		return(1);
	}
	return(0);
}

void GetTypeStats()
{
	IOB1->ios2_Req.io_Error   = 0;
	IOB1->ios2_Req.io_Command = S2_GETTYPESTATS;
	IOB1->ios2_StatData = &Stats;
	DoIO(IOB1);
}

void PrintStats()
{
	printf("Packet Type:       0x%lx\n", (long) PacketType);
	printf("Packets Received:    %d\n" , Stats.PacketsReceived);
	printf("Packets Transmitted: %d\n" , Stats.PacketsSent);
	printf("Bytes Received:      %d\n" , Stats.BytesReceived);
	printf("Bytes Transmitted:   %d\n" , Stats.BytesSent);
	printf("Packets Dropped:     %d\n" , Stats.PacketsDropped);
}
