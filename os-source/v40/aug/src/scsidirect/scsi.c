/* scsi.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <devices/trackdisk.h>
#include <devices/scsidisk.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <stdio.h>

#include "scsi.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

extern char *device;
extern int  unit;

struct IOStdReq *ior = NULL;
struct MsgPort *port = NULL;
int opened=FALSE;

void closedevice(void);

void closedevice ()
{
	if (opened)
		CloseDevice(ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	opened = 0;
	ior = NULL;
	port = NULL;
}

int
opendevice (char *device, int unit)
{
	int i = 0;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	printf("Opening device %s, unit %ld\n",device,unit);

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		printf("error %ld on open\n",i);
		goto cleanup;
	}
	return 0;

cleanup:
	closedevice();
	return i;
}

/* do any SCSI command using the supplied iorequest, ret 0=success or error */
/* data must be word aligned and dmaable */
/* ditto for command */

UBYTE  sensedata[255];
struct SCSICmd cmdblk;

int DoSCSI (struct IOStdReq *ior,
		   UWORD *command,
		   ULONG clen,
		   UWORD *data,
		   ULONG dlen,
		   ULONG flags);

void SendSCSI (struct IOStdReq *ior,
		     UWORD *command,
		     ULONG clen,
		     UWORD *data,
		     ULONG dlen,
		     ULONG flags);

int
DoSCSI (struct IOStdReq *ior,
		   UWORD *command,
		   ULONG clen,
		   UWORD *data,
		   ULONG dlen,
		   ULONG flags)/* only a ubyte used for actual xfer, rest for this rtn */
{

	SendSCSI(ior,command,clen,data,dlen,flags);
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
	if (cmdblk.scsi_Status)
		return (int) cmdblk.scsi_Status;

	return (int) ior->io_Error;	/* see scsidisk.h for errors */
}

void
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

