/*
 * simple sender test program for amiganet.device
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>

#include <sana/sana2device.h>
#include <sana/sana2packetmagic.h>
#include <proto/netbuff.h>
#include <pragmas/netbuff.h>
#include <libraries/dos.h>

#include <string.h>
#include <time.h>

struct Library *NetBuffBase;

static char amiganet[] = "amiganet.device";
static struct NetBuffSegment *snsp;
static struct IOSana2Req *sap;
static UBYTE sendbuf[2048];
static int devnum = 0;

static int setup(void);
static void quit(int code);
static void s2err(struct IOSana2Req *, char *);
static void globalstats(int device);

int main(int argc, char **argv)
{
	struct Sana2PacketTypeStats sstat;
	struct Sana2DeviceQuery sdq;
	unsigned int i, count, size;
	struct Sana2PacketType spt;
	UBYTE type[2], remoteaddr;
	ULONG t, e, m;

	if(argc < 4){
		printf("usage: %s devnum my_address remote_address [pkt_size [pkt_count [type0 [type1]]]]\n",
			argv[0]);
		exit(RETURN_FAIL);
	}

	devnum = atoi(argv[1]);
	if(!setup()){
		quit(RETURN_FAIL);
	}
	
	/* set address of interface */
	sap->S2io_SrcAddr[0] = atoi(argv[2]);
	remoteaddr = atoi(argv[3]);
	sap->S2io_Command = SANA2CMD_CONFIGINTERFACE;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "xmit configinterface failed");
		quit(RETURN_FAIL);
	}

	/* get MTU of device */
	sap->S2io_Command = SANA2CMD_DEVICEQUERY;
	sdq.SizeAvailable = sizeof(sdq);
	sap->S2io_StatData = &sdq;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "devicequery failed");
		quit(RETURN_FAIL);
	}
	printf("DeviceQuery: Address size is %d bits, MTU is %d, %d bits/sec, type %d\n", 
			sdq.AddrFieldSize, sdq.MTU, sdq.bps, sdq.HardwareType);

	/* setup test args */
	size = sdq.MTU;
	if(argc > 4){
		size = atoi(argv[4]);
	}
	if(size < 1 || size > sdq.MTU){
		printf("illegal buffer size %d: must be [1..%d]\n", size, sdq.MTU);
		quit(RETURN_FAIL);
	}
	count = 5000;
	if(argc > 5){
		count = atoi(argv[5]);
	}
	type[0] = 130;
	type[1] = 112;
	spt.Length = 2;
	if(argc > 6){
		type[0] = atoi(argv[6]);
		spt.Length = 1;

		if(argc > 7){
			type[1] = atoi(argv[7]);
			spt.Length++;
		}
	}

	/* setup packet type structure */
	spt.CanonicalType = 0;
	spt.Magic = S2MAGIC_ARCNET;
	spt.Match = type;
	spt.Mask = 0;

	/* tell driver to track packet type we're sending */
	sap->S2io_Command = SANA2CMD_TRACKTYPE;
	sap->S2io_PacketType = &spt;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "could not track type");
	}

	/* format send packet */
	for(i = 0; i < size; i++){
		sendbuf[i] = i & 0xff;
	}

	/* format packet itself */
	sap->S2io_Command = CMD_WRITE;
	sap->S2io_DstAddr[0] = remoteaddr;
	sap->S2io_Flags = 0;
	NewList((struct List *)&sap->S2io_Body.List);
	snsp->DataCount = size;
	snsp->Buffer = sendbuf;
	snsp->PhysicalSize = 0;
	snsp->DataOffset = 0;
	AddHead((struct List *)&sap->S2io_Body.List, (struct Node *)&snsp->Node);
	sap->S2io_DataLength = snsp->DataCount;

	/* send packets */
	m = SIGBREAKF_CTRL_C | (1L << sap->S2io_Message.mn_ReplyPort->mp_SigBit);
	t = time(0L);
	for(i = 0; i < count; i++){
		SendIO(sap);
		e = Wait(m);
		if(CheckIO(sap)){
			if(sap->S2io_Error != S2ERR_NO_ERROR){
				s2err(sap, "send error");
			}
			WaitIO(sap);
		}

		if(e & SIGBREAKF_CTRL_C){
			if(!CheckIO(sap)){
				AbortIO(sap);
				WaitIO(sap);
				printf("send aborted\n");
			}

			printf("Break; sent %d out of %d packets\n", i, count);
			break;
		}
	}
	t = time(0L) - t;

	printf("sent %d packets of len %d in %d seconds (%d pkt/sec)\n", 
				i, sap->S2io_DataLength, t, i/(t==0? 1:t));

	/* get & print accumulated track stats */
	sap->S2io_Command = SANA2CMD_GETTYPESTATS;
	sap->S2io_StatData = &sstat;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "could not gettypestats");
	} else {
		printf("Accumulated statistics: sent %d packets; %d bytes\n", 
				sstat.PacketsSent, sstat.BytesSent);
	}

	globalstats(devnum);

	/* do offline/online to zero stats */
	sap->S2io_Command = SANA2CMD_OFFLINE;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "could not gettypestats");
	}

	sap->S2io_Command = SANA2CMD_ONLINE;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "could not gettypestats");
	}

	/* print stats again */
	printf("\n\nAfter offline/online cycle, stats are:\n");
	sap->S2io_Command = SANA2CMD_GETTYPESTATS;
	sap->S2io_StatData = &sstat;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "could not gettypestats");
	} else {
		printf("Accumulated statistics: sent %d packets; %d bytes\n", 
				sstat.PacketsSent, sstat.BytesSent);
	}
	globalstats(devnum);

	quit(RETURN_OK);
}

/*
 * allocate & setup resources to run tests
 */
static setup()
{
	char *netbuffname = NETBUFFNAME;

	NetBuffBase = OpenLibrary(netbuffname, 0);
	if(!NetBuffBase){
		printf("Could not open %s\n", netbuffname);
		return 0;
	}

	snsp = AllocMem(sizeof(*snsp), MEMF_PUBLIC | MEMF_CLEAR);
	if(!snsp){
		printf("Could not allocate netbuff segment\n");
		return 0;
	}

	sap = CreateExtIO(CreatePort(0, 0), sizeof(*sap));
	if(!sap || !sap->S2io_Message.mn_ReplyPort){
		printf("Could not create reader extio\n");
		return 0;
	}

	if(OpenDevice(amiganet, devnum, sap, 0) != 0){
		s2err(sap, "Could not open device");
		return 0;
	}


	return 1;
}

/*
 * free allocated resources
 */
static void quit(code)
	int code;
{
	if(sap){
		if(sap->S2io_Unit){
			CloseDevice(sap);
		}
		if(sap->S2io_Message.mn_ReplyPort){
			DeletePort(sap->S2io_Message.mn_ReplyPort);
		}
		DeleteExtIO(sap);
	}

	if(snsp){
		FreeMem(snsp, sizeof(*snsp));
	}

	if(NetBuffBase){
		CloseLibrary(NetBuffBase);
		NetBuffBase = 0;
	}

	exit(code);
}

/*
 * get/print global stats
 */
static void globalstats(int device)
{
	struct Sana2DeviceStats stat;
	struct IOSana2Req sa;

	sa.S2io_Message.mn_ReplyPort = CreatePort(0, 0);
	if(!sa.S2io_Message.mn_ReplyPort){
		return;
	}

	if(OpenDevice(amiganet, device, &sa, 0) != 0){
		DeletePort(sa.S2io_Message.mn_ReplyPort);
		return;
	}

	sa.S2io_Command = SANA2CMD_GETGLOBALSTATS;
	sa.S2io_StatData = &stat;
	DoIO(&sa);

	printf("Global statistics for unit %d at timestamp [%d.%d]\n", 
			device, stat.last_start.tv_secs, stat.last_start.tv_micro);
	printf("pkts rcvd = %d, send = %d\n", stat.packets_received,
			stat.packets_sent);
	printf("framing errors = %d, bad data = %d\n", stat.framing_errors,
				stat.bad_data);
	printf("misses, hard = %d, soft = %d\n", stat.hard_misses, stat.soft_misses);
	printf("unknown types = %d\n", stat.unknown_types_received);
	printf("fifo: underruns = %d, overruns = %d\n", stat.fifo_underruns,
			stat.fifo_overruns);
	printf("reconfigurations = %d\n", stat.reconfigurations);

	CloseDevice(&sa);
	DeletePort(sa.S2io_Message.mn_ReplyPort);
}

/*
 * print error codes
 */
#define NUMERR 	10
#define NUMWERR	19

static char *errname[NUMERR] = {
	"S2ERR_NO_ERROR",
	"S2ERR_NO_RESOURCES",
	"S2ERR_UNKNOWN_ENTITY",
	"S2ERR_BAD_ARGUMENT",
	"S2ERR_BAD_STATE",
	"S2ERR_BAD_ADDRESS",
	"S2ERR_MTU_EXCEEDED",
	"S2ERR_BAD_PROTOCOL",
	"S2ERR_NOT_SUPPORTED",
	"S2ERR_SOFTWARE"
};
static char *werrname[NUMWERR] = {
	"S2WERR_GENERIC_ERROR",
	"S2WERR_NOT_CONFIGURED",
	"S2WERR_UNIT_ONLINE",
	"S2WERR_UNIT_OFFLINE",
	"S2WERR_ALREADY_TRACKED",
	"S2WERR_NOT_TRACKED",
	"S2WERR_NETBUFF_ERROR",
	"S2WERR_SRC_ADDRESS",
	"S2WERR_DST_ADDRESS",
	"S2WERR_BAD_BROADCAST",
	"S2WERR_BAD_MULTICAST",
	"S2WERR_ALIAS_LIST_FULL",
	"S2WERR_BAD_ALIAS",
	"S2WERR_MULTICAST_FULL",
	"S2WERR_BAD_EVENT",
	"S2WERR_BAD_STATDATA",
	"S2WERR_PROTOCOL_UNKNOWN",
	"S2WERR_IS_CONFIGURED",
	"S2WERR_NULL_POINTER"
};

static void s2err(iob, str)
	struct IOSana2Req *iob;
	char *str;
{
	static char werr[32], err[32], *unk = "unknown";

	strcpy(werr, unk);
	strcpy(err, unk);
	if(iob->S2io_Error >= 0 || iob->S2io_Error < NUMERR){
		strcpy(err, errname[iob->S2io_Error]);
	}
	if(iob->S2io_WireError < NUMWERR){
		strcpy(werr, werrname[iob->S2io_WireError]);
	}

	printf("%s: err %d (%s) werr %d (%s)\n", str, iob->S2io_Error, err,
						      iob->S2io_WireError, werr);
}

