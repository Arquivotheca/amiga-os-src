/*
 * simple test program for amiganet.device
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

#ifndef min
#define min(a, b) ((a) < (b) ? (a):(b))
#endif

struct Library *NetBuffBase;
static char amiganet[] = "amiganet.device";
static UBYTE sendbuf[2048], recvbuf[2048];
static struct NetBuffSegment *snsp, *rnsp;
static struct IOSana2Req *rap, *sap;

static int setup(void);
static void quit(int code);
static void s2err(struct IOSana2Req *, char *);

int main(int argc, char **argv);

main(argc, argv)
	int	argc;
	char	**argv;
{
	struct Sana2PacketTypeStats sstat;
	struct Sana2DeviceQuery sdq;
	struct Sana2PacketType spt;
	unsigned int i, count, size;
	UBYTE type[2], u0, u1;
	ULONG t;

	if(!setup()){
		quit(RETURN_FAIL);
	}
	
	u1 = u0 = 0;

	if(argc > 1 && argv[1][0] != '*'){
		u0 = atoi(argv[1]);
	}

	if(argc > 2 && argv[2][0] != '*'){
		u1 = atoi(argv[2]);
	}

	if(u0 == 0){
		sap->S2io_Command = SANA2CMD_GETSTATIONADDRESS;
		DoIO(sap);
		if(sap->S2io_Error){
			s2err(sap, "xmit getstationaddress failed");
			quit(RETURN_FAIL);
		}
		u0 = sap->S2io_DstAddr[0];
	}
	sap->S2io_SrcAddr[0] = u0;
	sap->S2io_Command = SANA2CMD_CONFIGINTERFACE;
	DoIO(sap);
	if(sap->S2io_Error){
		s2err(sap, "xmit configinterface failed");
		quit(RETURN_FAIL);
	}

	if(u1 == 0){
		rap->S2io_Command = SANA2CMD_GETSTATIONADDRESS;
		DoIO(rap);
		if(rap->S2io_Error){
			s2err(rap, "recv getstationaddress failed");
			quit(RETURN_FAIL);
		}
		u1 = rap->S2io_DstAddr[0];
	}
	rap->S2io_SrcAddr[0] = u1;
	rap->S2io_Command = SANA2CMD_CONFIGINTERFACE;
	DoIO(rap);
	if(rap->S2io_Error){
		s2err(rap, "recv configinterface failed");
		quit(RETURN_FAIL);
	}

	/*
	 * get MTU of device
	 */
	rap->S2io_Command = SANA2CMD_DEVICEQUERY;
	sdq.SizeAvailable = sizeof(sdq);
	rap->S2io_StatData = &sdq;
	DoIO(rap);
	if(rap->S2io_Error){
		s2err(rap, "recv devicequery failed");
		quit(RETURN_FAIL);
	}
	printf("DeviceQuery: supp=%d fmt=%d addrsz=%d MTU=%d bps=%d type=%d\n", 
			sdq.SizeSupplied, sdq.DevQueryFormat, sdq.AddrFieldSize, 
			sdq.MTU, sdq.bps, sdq.HardwareType);

	/*
	 * setup test args
	 */
	size = sdq.MTU;
	if(argc > 3){
		size = atoi(argv[3]);
	}
	if(size < 1 || size > sdq.MTU){
		printf("illegal buffer size %d: must be [1..%d]\n", size, sdq.MTU);
		quit(RETURN_FAIL);
	}

	count = 100;
	if(argc > 4){
		count = atoi(argv[4]);
	}

	/*
	 * setup packet type structure
	 */
	type[0] = 130;
	type[1] = 112;
	spt.CanonicalType = 0;
	spt.Magic = S2MAGIC_ARCNET;
	spt.Length = 2;
	spt.Match = type;
	spt.Mask = 0;

	/*
	 * tell driver to track packet type we're sending
	 */
	sap->S2io_Command = SANA2CMD_TRACKTYPE;
	sap->S2io_PacketType = &spt;
	DoIO(sap);
	if(sap->S2io_Error || sap->S2io_WireError){
		s2err(sap, "could not track type");
	}

	/*
	 * format send packet
	 */
	for(i = 0; i < size; i++){
		sendbuf[i] = i & 0xff;
	}
	sap->S2io_Command = SANA2CMD_BROADCAST;
	NewList((struct List *)&sap->S2io_Body.List);
	snsp->DataCount = size;
	snsp->Buffer = sendbuf;
	snsp->PhysicalSize = 0;
	snsp->DataOffset = 0;
	AddHead((struct List *)&sap->S2io_Body.List, (struct Node *)&snsp->Node);
	sap->S2io_DataLength = snsp->DataCount;

	/*
	 * basic send speed test: broadcast packets require no ack, thus they
	 * go very fast
	 */
	printf("send test: ");
	t = time(0L);
	for(i = 0; i < count; i++){
		DoIO(sap);
	}
	t = time(0L) - t;
	printf("sent %d broadcast packets of len %d in %d seconds (%d pkt/sec)\n", 
		i, sap->S2io_DataLength, t, i/(t==0? 1:t));

	/*
	 * get & print accumulated track stats
	 */
	sap->S2io_Command = SANA2CMD_GETTYPESTATS;
	sap->S2io_StatData = &sstat;
	DoIO(sap);
	if(sap->S2io_Error || sap->S2io_WireError){
		s2err(sap, "could not gettypestats");
	} else {
		printf("send stats: packets=%d bytes=%d\n", 
				sstat.PacketsSent, sstat.BytesSent);
	}

	/*
	 * setup receive packet; use two byte type field
	 */
	rap->S2io_Command = CMD_READ;
	rap->S2io_PacketType = &spt;
	NewList((struct List *)&rap->S2io_Body.List);
	rnsp->Buffer = recvbuf;
	rnsp->PhysicalSize = 0;
	AddHead((struct List *)&rap->S2io_Body.List, (struct Node *)&rnsp->Node);

	sap->S2io_Command = CMD_WRITE;
	sap->S2io_DstAddr[0] = 2;

	/*
	 * send/recv test between boards
	 */
	t = time(0L);
	for(i = 0; i < count; i++){

		rnsp->DataOffset = 0;
		rnsp->DataCount = sizeof(recvbuf);
		SendIO(rap);
		if(CheckIO(rap)){
			s2err(rap, "read failed");
		}

		DoIO(sap);
		if(sap->S2io_Error || sap->S2io_WireError){
			s2err(sap, "send error");
		}

		Wait(SIGBREAKF_CTRL_C | (1L << rap->S2io_Message.mn_ReplyPort->mp_SigBit));
		if(!CheckIO(rap)){
			AbortIO(rap);
			WaitIO(rap);
			printf("receive aborted\n");
		} else {
			if(rap->S2io_Error || rap->S2io_WireError){
				printf("recv error, err=%d, wire error=%d\n", 
						rap->S2io_Error, rap->S2io_WireError);
			} else {
				if(memcmp(sendbuf, recvbuf, snsp->DataCount) != 0){
					printf("data error\n");
				}
			}
		}

		GetMsg(rap->S2io_Message.mn_ReplyPort);
	}
	t = time(0L) - t;
	printf("sent %d packets of len %d in %d seconds (%d pkt/sec)\n", 
		i, sap->S2io_DataLength, t, i/(t==0? 1:t));

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

	rnsp = AllocMem(sizeof(*rnsp), MEMF_PUBLIC | MEMF_CLEAR);
	snsp = AllocMem(sizeof(*rnsp), MEMF_PUBLIC | MEMF_CLEAR);
	if(!rnsp || !snsp){
		printf("Coult not allocate netbuff segments\n");
		return 0;
	}

	rap = CreateExtIO(CreatePort(0, 0), sizeof(*rap));
	if(!rap || !rap->S2io_Message.mn_ReplyPort){
		printf("Could not create reader extio\n");
		return 0;
	}

	sap = CreateExtIO(CreatePort(0, 0), sizeof(*sap));
	if(!sap || !sap->S2io_Message.mn_ReplyPort){
		printf("Could not create reader extio\n");
		return 0;
	}

	if(OpenDevice(amiganet, 1, rap, 0) != 0){
		printf("Could not open %s; error = %d, werr = %d\n", 
			amiganet, rap->S2io_Error, rap->S2io_WireError);
		return 0;
	}

	if(OpenDevice(amiganet, 0, sap, 0) != 0){
		printf("Could not open %s; error = %d, werr = %d\n", 
			amiganet, sap->S2io_Error, sap->S2io_WireError);
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
	if(rap){
		if(rap->S2io_Unit){
			CloseDevice(rap);
		}
		if(rap->S2io_Message.mn_ReplyPort){
			DeletePort(rap->S2io_Message.mn_ReplyPort);
		}
		DeleteExtIO(rap);
	}

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

	if(rnsp){
		FreeMem(rnsp, sizeof(*rnsp));
	}

	if(NetBuffBase){
		CloseLibrary(NetBuffBase);
		NetBuffBase = 0;
	}

	exit(code);
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

	printf("%s: err=%d (%s) werr=%d (%s)\n", str, iob->S2io_Error, err,
						      iob->S2io_WireError, werr);
}

