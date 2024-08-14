/* scsi.c */

#include <sys/types.h>
#include "sd.h"

#include <fcntl.h>
#include <osfcn.h>
#include <stdlib.h>
#include <memory.h>

#include "scsi.h"
#include "scsidirect.h"



SCSIDEVICE *InitSCSI(char *interface, int addr, int lun)
{
	SCSIDEVICE *ior;

#ifndef LUNS_WORK
	if (lun)
		return 0;
#endif

	ior = malloc(sizeof *ior);
	if (!ior)
		return 0;

	ior->fd = open("/dev/scsi", O_RDWR);
	if (ior->fd < 0)
	{
		free(ior);
		return 0;
	}

	ior->interface = atoi(interface);
	ior->addr = addr;
	ior->lun = lun;

	return ior;
}

void QuitSCSI(SCSIDEVICE *ior)
{
	(void)close(ior->fd);
	free(ior);
}


int gsio(SCSIDEVICE *ior, struct sdcom *cp)
{
	if (ioctl(ior->fd, 'GSIO', cp))
		return 1;
	if (!cp->okay)
		return 1;
	return 0;
}


unsigned char sensedata[255];
size_t SCSI_Actual, SCSI_CmdLength, SCSI_SenseActual, SCSI_Length;


/* Do any SCSI command using the supplied SCSIDEVICE, ret 0=success or error */

int DoSCSI(SCSIDEVICE *ior,
	   void *command,
	   size_t clen,
	   void *data,
	   size_t dlen,
	   int flags)
{
	struct sdcom cmdblk;

	cmdblk.card           = ior->interface;
	cmdblk.unit           = ior->addr;
#ifdef LUNS_WORK
	cmdblk.lun            = ior->lun;
#endif
	cmdblk.reading        = !(flags&SCSI_WRITE);
	cmdblk.addr           = data;
	cmdblk.nbyte          = dlen;
	memcpy(cmdblk.cdb, command, sizeof cmdblk.cdb);

	SCSI_CmdLength = clen;
	SCSI_Actual = dlen;
	SCSI_SenseActual = sizeof sensedata;
	SCSI_Length = dlen;

	if (gsio(ior, &cmdblk))
		return -1;

	if (cmdblk.status == 2)
	{
		cmdblk.reading = 1;
		memset(cmdblk.cdb, 0, sizeof cmdblk.cdb);
		cmdblk.cdb[0] = 3;
		cmdblk.cdb[4] = sizeof sensedata;
		cmdblk.addr = sensedata;
		cmdblk.nbyte = sizeof sensedata;
		if (gsio(ior, &cmdblk))
			return -1;
		if (cmdblk.status == 0)
			return 0;
		return -1;
	}

	return cmdblk.status;
}
