/* scsi.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <devices/scsidisk.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <stddef.h>
#include <stdlib.h>

#include "scsi.h"
#include "scsidirect.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

extern char *device;
extern int  unit;

/* do any SCSI command using the supplied iorequest, ret 0=success or error */
/* data must be word aligned and dmaable */
/* ditto for command */

unsigned char sensedata[255];
size_t SCSI_Actual, SCSI_CmdLength, SCSI_SenseActual, SCSI_Length;
static struct SCSICmd cmdblk;

static struct MsgPort *port = NULL;

SCSIDEVICE *InitSCSI(char *interface, int addr, int lun)
{
	int unit = addr + 10 * lun;
	struct IORequest *ior;

	if (port)			/* Busy */
		return 0;

	if (!(port = CreatePort(0L,0L)))
		return 0;
	if (!(ior = CreateStdIO(port)))
	{
		DeletePort(port);
		return 0;
	}
	if (OpenDevice(interface, unit, ior, 0L))
	{
		DeleteStdIO(ior);
		DeletePort(port);
		return 0;
	}

	return ior;
}

void QuitSCSI(SCSIDEVICE *ior)
{
	CloseDevice(ior);
	DeleteStdIO(ior);
	DeletePort(port);
	port = 0;
}


static void SendSCSI(struct IORequest *ior,
		     UWORD *command,
		     ULONG clen,
		     UWORD *data,
		     ULONG dlen,
		     ULONG flags);

int
DoSCSI (SCSIDEVICE *ior,
	void *command,
	size_t clen,
	void *data,
	size_t dlen,
	int flags)			/* SCSI_READ or SCSI_WRITE */
{

	SendSCSI(ior,command,clen,data,dlen,flags);
/*		 (flags&SCSI_WRITE) ?
		 (SCSIF_WRITE|SCSIF_AUTOSENSE) :
		 (SCSIF_READ|SCSIF_AUTOSENSE));*/
	WaitIO((struct IORequest *) ior);

#ifdef TEST_SCSI
printf("direct scsi return error %d, status %d\n",ior->io_Error,cmdblk.scsi_Status);
if (cmdblk.scsi_SenseActual)
{
int i;
printf("Sense data (length %d) = 0x",cmdblk.scsi_SenseActual);
for(i = 0; i < cmdblk.scsi_SenseActual; i++)
printf("%02.2x",sensedata[i]);
printf("\n");
}
#endif

	SCSI_Actual	 = cmdblk.scsi_Actual;
	SCSI_CmdLength	 = cmdblk.scsi_CmdLength;
	SCSI_SenseActual = cmdblk.scsi_SenseActual;
	SCSI_Length	 = cmdblk.scsi_Length;

	if (cmdblk.scsi_Status)
		return (int) cmdblk.scsi_Status;

	return (int) ior->io_Error;	/* see scsidisk.h for errors */
}

static void
SendSCSI (struct IOStdReq *ior,
		     UWORD *command,
		     ULONG clen,
		     UWORD *data,
		     ULONG dlen,
		     ULONG flags)/* only a ubyte used for actual xfer, rest for this rtn */
{
	/* first set up ior */
	ior->io_Data    = (APTR) &cmdblk;
	ior->io_Length  = sizeof(cmdblk);
	ior->io_Actual  = ior->io_Offset = 0;
	ior->io_Command = HD_SCSICMD;

	/* now set up cmdblk */
	cmdblk.scsi_Data      = data;
	cmdblk.scsi_Length    = dlen;
	cmdblk.scsi_Actual    = 0;
	cmdblk.scsi_Command   = (UBYTE *) command;
	cmdblk.scsi_CmdLength = clen;
	cmdblk.scsi_CmdActual = 0;
	cmdblk.scsi_Flags     = flags & 0xff; /* probably not needed */
	cmdblk.scsi_Status    = 0;
	cmdblk.scsi_SenseData = sensedata;
	cmdblk.scsi_SenseLength = sizeof(sensedata);
	cmdblk.scsi_SenseActual = 0;
/*	cmdblk.scsi_NextLinked  = NULL;*/

	SendIO((struct IORequest *) ior);
}

