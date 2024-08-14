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

struct Library *NetBuffBase;

#define NRAP 32

static char amiganet[] = "amiganet.device";
static struct NetBuffSegment *rnsp;
static struct MsgPort *rport;
static struct IOSana2Req *rap[NRAP], *iob;
static int devnum = 0;

static int setup(void);
static void quit(int code);
static void s2err(struct IOSana2Req *, char *);
static struct {UBYTE sid, did, type[2];} ph;

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

static void primeNB(int numseg, int segsize)
{
	struct NetBuffSegment *np;
	struct NetBuff nb;
	int i;

	NewList((struct List *)&nb.List);
	for(i = 0; i < numseg; i++){
		np = AllocMem(sizeof(*np), MEMF_CLEAR | MEMF_PUBLIC);
		if(!np){
			break;
		}

		np->Buffer = AllocMem(segsize, MEMF_CLEAR | MEMF_PUBLIC);
		if(np->Buffer){
			np->PhysicalSize = segsize;
		}

		AddTail((struct List *)&nb.List, (struct Node *)&np->Node);
	}

	FreeSegments(&nb);

	printf("Primed netbuff.library with %d segs of size %d\n", 
				numseg, segsize);
}


main(int argc, char **argv)
{
	struct Sana2PacketTypeStats sstat;
	struct Sana2DeviceQuery sdq;
	struct Sana2PacketType spt;
	UBYTE type[2], tstr[20], sid;
	struct NetBuff nb;
	unsigned int i;
	ULONG e, m;

	if(argc < 3){
		printf("usage: %s devicenum address [type [type]]\n");
		exit(RETURN_FAIL);
	}

	devnum = atoi(argv[1]);
	if(!setup()){
		quit(RETURN_FAIL);
	}
	
	/* checking that #!@@#??!!! netbuff.library has segments in it */
	NewList((struct List *)&nb.List);
	if(ReadyNetBuff(&nb, 1000)){
		primeNB(256, 64);
	}

	/* set interface address */
	rap[0]->S2io_SrcAddr[0] = atoi(argv[2]);
	rap[0]->S2io_Command = SANA2CMD_CONFIGINTERFACE;
	DoIO(rap[0]);
	if(rap[0]->S2io_Error){
		s2err(rap[0], "configinterface failed");
		quit(RETURN_FAIL);
	}

	/* get MTU of device */
	rap[0]->S2io_Command = SANA2CMD_DEVICEQUERY;
	sdq.SizeAvailable = sizeof(sdq);
	rap[0]->S2io_StatData = &sdq;
	DoIO(rap[0]);
	if(rap[0]->S2io_Error){
		s2err(rap[0], "recv devicequery failed");
		quit(RETURN_FAIL);
	}
	printf("Address size is %d bits, MTU=%d, line rate %d bits/sec, HW type %d\n", 
			sdq.AddrFieldSize, sdq.MTU, sdq.bps, sdq.HardwareType);

	/* setup packet type structure */
	type[0] = 130; 
	type[1] = 112; 
	spt.Length = 2;
	spt.Match = type;
	spt.CanonicalType = 0;
	spt.Magic = S2MAGIC_ARCNET;
	spt.Mask = 0;

	if(argc > 3){
		type[0] = atoi(argv[3]);
		spt.Length = 1;

		if(argc > 4){
			type[1] = atoi(argv[4]);
			spt.Length = 2;
		}
	}

	sprintf(tstr, spt.Length==2? "[%d.%d]":"[%d]", spt.Match[0], spt.Match[1]);
	printf("Using unit %d, %d byte type field %s, board address is %d\n",
		devnum, spt.Length, tstr, rap[0]->S2io_SrcAddr[0]);

	/* tell driver to track packet type we're receiving */
	rap[0]->S2io_Command = SANA2CMD_TRACKTYPE;
	rap[0]->S2io_PacketType = &spt;
	DoIO(rap[0]);
	if(rap[0]->S2io_Error){
		s2err(rap[0], "could not track type");
	}

	/* get & print accumulated track stats */
	rap[0]->S2io_Command = SANA2CMD_GETTYPESTATS;
	rap[0]->S2io_StatData = &sstat;
	DoIO(rap[0]);
	if(rap[0]->S2io_Error){
		s2err(rap[0], "could not gettypestats");
	} else {
		printf("send stats: packets=%d bytes=%d\n", 
				sstat.PacketsSent, sstat.BytesSent);
	}

	/* setup receive packet; use two byte type field */
	for(i = 0; i < NRAP; i++){
		rap[i]->S2io_Flags = 0;
		if(i < NRAP/2){
			rap[i]->S2io_Command = CMD_READ;
		} else {
			rap[i]->S2io_Flags |= SANA2IOF_RAW;
			rap[i]->S2io_Command = SANA2CMD_READORPHAN;
		}
		rap[i]->S2io_PacketType = &spt;
		NewList((struct List *)&rap[i]->S2io_Body.List);
		BeginIO(rap[i]);
	}

	/* loop, catching packets */
	m = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | (1L << rport->mp_SigBit);
	do {
		e = Wait(m);
		while(iob = (struct IOSana2Req *)GetMsg(rport)){
			if(iob->S2io_Error){
				s2err(iob, "bad receive packet");
			} else if(iob->S2io_Command == SANA2CMD_READORPHAN){
				if(!CopyFromNetBuff(&iob->S2io_Body, 0, (UBYTE *)&ph, sizeof(ph))){
					printf("orphaned packet; sid %d did %d type [%d.%d]\n",
						ph.sid, ph.did, ph.type[0], ph.type[1]);
				} else {
					printf("orphaned packet; bummed netbuffs\n");
				}
				sid = -1;
			} else {
				if(sid != iob->S2io_SrcAddr[0]){
					printf("receiving packets from sid = %d\n", 
							iob->S2io_SrcAddr[0]);
					sid = iob->S2io_SrcAddr[0];
				}
			}

			BeginIO(iob);
			FreeSegments(&iob->S2io_Body);
		}

		if(e & SIGBREAKF_CTRL_F){
			globalstats(devnum);
		}

	} while(!(e & SIGBREAKF_CTRL_C));

	for(i = 0; i < NRAP; i++){
		AbortIO(rap[i]);
		WaitIO(rap[i]);
	}

	quit(RETURN_OK);
}

/*
 * allocate & setup resources to run tests
 */
static setup()
{
	char *netbuffname = NETBUFFNAME;
	int i;

	NetBuffBase = OpenLibrary(netbuffname, 0);
	if(!NetBuffBase){
		printf("Could not open %s\n", netbuffname);
		return 0;
	}

	rport = CreatePort(0L, 0L);
	if(!rport){
		return 0;
	}

	for(i = 0; i < NRAP; i++){
		rap[i] = CreateExtIO(rport, sizeof(*rap[i]));
		if(!rap[i]){
			printf("Could not create extio %d\n", i);
			return 0;
		}

		if(OpenDevice(amiganet, devnum, rap[i], 0) != 0){
			s2err(rap[i], "Could not open device");
			return 0;
		}

		NewList((struct List *)&rap[i]->S2io_Body.List);
		AllocSegments(512, (void *)&rap[i]->S2io_Body.List);
	}

	return 1;
}

/*
 * free allocated resources
 */
static void quit(int code)
{
	int i;

	for(i = 0; i < NRAP; i++){
		if(rap[i]){
			if(rap[i]->S2io_Unit){
				CloseDevice(rap[i]);
			}
			rap[i]->S2io_Message.mn_ReplyPort = 0;
			DeleteExtIO(rap[i]);
			rap[i] = 0;
		}

	}

	if(rport){
		DeletePort(rport);
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

