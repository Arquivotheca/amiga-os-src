/*
 * simple test program for amiganet.device
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>

#include <sana/sana2device.h>
#include <sana/sana2packetmagic.h>

#include "amiganet.h"

static struct IOSana2Req sa, *sap = &sa;
static char amiganet[] = "amiganet.device";

int main(int argc, char **argv);
static notimp(void);
static nullpointer(void);
static badcommand(void);
static badtype(void);

main(argc, argv)
	int	argc;
	char	**argv;
{
	struct Sana2DeviceQuery sdq;

	sap->S2io_Message.mn_ReplyPort = CreatePort(0, 0);
	if(sap->S2io_Message.mn_ReplyPort == 0){
		printf("Could not create replyport\n");
		exit(0);
	}

	if(OpenDevice(amiganet, 0, sap, 0) != 0){
		DeletePort(sap->S2io_Message.mn_ReplyPort);
		printf("Could not open %s; error = %d, werr = %d\n", 
			amiganet, sap->S2io_Error, sap->S2io_WireError);
		exit(0);
	}

	sap->S2io_Command = SANA2CMD_DEVICEQUERY;
	sdq.SizeAvailable = sizeof(sdq);
	sap->S2io_StatData = &sdq;
	DoIO(sap);
	printf("DeviceQuery: supp=%d fmt=%d addrsz=%d MTU=%d bps=%d type=%d\n", 
			sdq.SizeSupplied, sdq.DevQueryFormat, sdq.AddrFieldSize, 
			sdq.MTU, sdq.bps, sdq.HardwareType);
	    


	badcommand();
	nullpointer();
	notimp();
	badtype();

	DeletePort(sap->S2io_Message.mn_ReplyPort);
	CloseDevice(sap);
}

/*
 * check bad command codes
 */
static badcommand()
{
	int fail = 0;

	printf("Check bad command processing\n");
	sap->S2io_Command = -1;
	DoIO(sap);
	if(sap->S2io_Error != S2ERR_NOT_SUPPORTED){
		printf("\5failed on cmd %d, wanted %d, got %d\n",
			sap->S2io_Command, S2ERR_NOT_SUPPORTED, sap->S2io_Error);
		fail++;
	}
	sap->S2io_Command = 0;
	DoIO(sap);
	if(sap->S2io_Error != S2ERR_NOT_SUPPORTED){
		printf("\5failed on cmd %d, wanted %d, got %d\n",
			sap->S2io_Command, S2ERR_NOT_SUPPORTED, sap->S2io_Error);
		fail++;
	}
	sap->S2io_Command = 10000;
	DoIO(sap);
	if(sap->S2io_Error != S2ERR_NOT_SUPPORTED){
		printf("\5failed on cmd %d, wanted %d, got %d\n",
			sap->S2io_Command, S2ERR_NOT_SUPPORTED, sap->S2io_Error);
		fail++;
	}
	printf("....test %s\n", fail ? "fails":"passes");
}

/*
 * Check null unit pointer
 */
static nullpointer()
{
	struct AmiganetUnit *up;
	int i, fail=0;

	printf("Check null unit pointer\n");
	up = UNIT(sap);
	UNIT(sap) = 0;
	for(i = 0; i < SANA2_CMD_END; i++){
		sap->S2io_Command = i;
		DoIO(sap);
		if(sap->S2io_Error == S2ERR_NOT_SUPPORTED){
			continue;
		}
		if(sap->S2io_WireError != S2WERR_NULL_POINTER){
			printf("\tfail on command %d; wanted %d got %d\n", i,
				S2WERR_NULL_POINTER, sap->S2io_WireError);
			fail++;
		}
	}
	UNIT(sap) = up;
	printf("....test %s\n", fail ? "fails":"passes");
}

/*
 * Check not implemented codes
 */
#define NUMNOTIMP 12
static int notimpl[NUMNOTIMP] = {
	CMD_INVALID,	
	CMD_RESET,	
	CMD_UPDATE,	
	CMD_CLEAR,	
	CMD_STOP,
	CMD_START,	
	CMD_STOP,	
	CMD_FLUSH,	
	SANA2CMD_ADDSTATIONALIAS,
	SANA2CMD_DELSTATIONALIAS,	
	SANA2CMD_ADDMULTICASTADDRESS,
	SANA2CMD_DELMULTICASTADDRESS
};

static notimp()
{
	int i;

	printf("Check notimpl implemented commands\n");
	for(i = 0; i < NUMNOTIMP; i++){
		sap->S2io_Command = notimpl[i];
		DoIO(sap);
		if(sap->S2io_Error != S2ERR_NOT_SUPPORTED){
			printf("\tcommand %d should return %d, got %d\n", 
				sap->S2io_Command, 
				S2ERR_NOT_SUPPORTED,
				sap->S2io_Error);
			break;
		}
	}
	printf("....test %s\n", (i != NUMNOTIMP) ? "fails":"passes");
}

/*
 * Check bad packet types
 */
#define NUMTYPE 7
static int badtypear[NUMTYPE] = {
	CMD_READ,
	CMD_WRITE,
	SANA2CMD_BROADCAST,
	SANA2CMD_GETTYPESTATS,
	SANA2CMD_READORPHAN,
	SANA2CMD_TRACKTYPE,
	SANA2CMD_UNTRACKTYPE
};

static badtype()
{
	struct Sana2PacketTypeStats stat;
	struct Sana2PacketType spt;
	UBYTE type[2];
	int i;

	sap->S2io_Flags &= ~SANA2IOF_RAW;
	sap->S2io_StatData = &stat;
	sap->S2io_PacketType = &spt;

	spt.CanonicalType = 0;

	/*
	 * null pointer in Match/Mask
	 */
	spt.Magic = S2MAGIC_ARCNET;
	spt.Length = 1;
	spt.Match = (UBYTE *)0;
	spt.Mask = (UBYTE *)0;

	printf("Check bad packet type (null match/mask) processing\n");
	for(i = 0; i < NUMTYPE; i++){
		sap->S2io_Command = badtypear[i];
		DoIO(sap);
		if(sap->S2io_WireError != S2WERR_NULL_POINTER){
			printf("\tcommand %d should return %d, got %d\n", 
				sap->S2io_Command, 
				S2WERR_NULL_POINTER,
				sap->S2io_WireError);
			break;
		}
	}
	printf("....test %s\n", (i != NUMTYPE) ? "fails":"passes");

	spt.Match = spt.Mask = type;
	spt.Length = 1;

	/*
	 * bad type
	 */
	spt.Magic = 1000;
	printf("Check bad packet magic processing\n");
	for(i = 0; i < NUMTYPE; i++){
		sap->S2io_Command = badtypear[i];
		DoIO(sap);
		if(sap->S2io_Error != S2ERR_BAD_PROTOCOL){
			printf("\tcommand %d should return %d, got %d\n", 
				sap->S2io_Command, 
				S2ERR_BAD_PROTOCOL,
				sap->S2io_Error);
			break;
		}
	}
	printf("....test %s\n", (i != NUMTYPE) ? "fails":"passes");

	spt.Magic = S2MAGIC_ARCNET;

	/*
	 * bad type length
	 */
	spt.Length = 10;
	printf("Check bad packet type length processing\n");
	for(i = 0; i < NUMTYPE; i++){
		sap->S2io_Command = badtypear[i];
		DoIO(sap);
		if(sap->S2io_Error != S2ERR_BAD_PROTOCOL){
			printf("\tcommand %d should return %d, got %d\n", 
				sap->S2io_Command, 
				S2ERR_BAD_PROTOCOL,
				sap->S2io_Error);
			break;
		}
	}
	printf("....test %s\n", (i != NUMTYPE) ? "fails":"passes");

	spt.Length = -1;
	for(i = 0; i < NUMTYPE; i++){
		sap->S2io_Command = badtypear[i];
		DoIO(sap);
		if(sap->S2io_Error != S2ERR_BAD_PROTOCOL){
			printf("\tcommand %d should return %d, got %d\n", 
				sap->S2io_Command, 
				S2ERR_BAD_PROTOCOL,
				sap->S2io_Error);
			break;
		}
	}
	printf("....test %s\n", (i != NUMTYPE) ? "fails":"passes");
}

