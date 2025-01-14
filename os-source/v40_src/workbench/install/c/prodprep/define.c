/* Define.c - code to handle the drive definition screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <clib/alib_protos.h>

#include "protos.h"

#include "global.h"
#include "refresh.h"

#include "scsidisk.h"
#include "scsi.h"

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

extern struct SysBase *SysBase;
extern struct DOSBase *DOSBase;

/* define globals */
struct DriveDef *NewDef;

char NDManuNameSIBuff[80];
char NDDriveNameSIBuff[80];
char NDRevisionNameSIBuff[80];

int
defineit (w)
	struct Window *w;
{
	/* GODDAMN blank-padded strings! */

	if (strlen(NDManuNameSIBuff)     == 0 &&
	    strlen(NDDriveNameSIBuff)    == 0 &&
	    strlen(NDRevisionNameSIBuff) == 0)
	{
		Notify(w,"Error: Drive has no name!",
		       0);
		return FALSE;
	}

	{
	short i;

	/* these may have changed without a message */

	/* update only if selected */
	NewDef->Initial_RDB->rdb_AutoParkSeconds = 0;

	/* blank pad all the strings */
	strncpy(NewDef->Initial_RDB->rdb_DiskVendor,
		NDManuNameSIBuff,8);
	for (i = strlen(NDManuNameSIBuff); i < 8; i++)
		NewDef->Initial_RDB->rdb_DiskVendor[i] = ' ';

	strncpy(NewDef->Initial_RDB->rdb_DiskProduct,
		NDDriveNameSIBuff,16);
	for (i = strlen(NDDriveNameSIBuff); i < 16; i++)
		NewDef->Initial_RDB->rdb_DiskProduct[i] = ' ';

	strncpy(NewDef->Initial_RDB->rdb_DiskRevision,
		NDRevisionNameSIBuff,4);
	for (i = strlen(NDRevisionNameSIBuff); i < 4; i++)
		NewDef->Initial_RDB->rdb_DiskRevision[i] = ' ';
	}

	return TRUE;
}

int
DefineSetup(w)
	struct Window *w;
{
	if (!(NewDef = AllocNew(DriveDef)))
		return FALSE;

	if (!(NewDef->Initial_RDB = AllocNew(RigidDiskBlock)))
	{
		FreeMem((char *) NewDef,sizeof(*NewDef));
		return FALSE;
	}

	NewDef->Filename[0] = '\0';	/* paranoia */

	NewDef->ControllerType = SCSI;
	*(NewDef->Initial_RDB) = ScsiRDB;

	return TRUE;
}

/* assumes unit number set right */
struct DriveDef *
DoDefine (w)
	struct Window *w;
{
	if (!DefineSetup(w) || !DoReadParms(w))
	{
		Notify(w,"Unable to setup drive",0);
		return FALSE;
	}
	if (!defineit(w))
		return FALSE;

	return NewDef;
}

void
FreeDef (def)
	register struct DriveDef *def;
{
	if (def) {
		FreeRDB(def->Initial_RDB);
		FreeMem((char *) def,sizeof(*def));
	}
}

/* read info from SCSI drive */

extern struct SCSICmd cmdblk;

int
DoReadParms (w)
	struct Window *w;
{
	unsigned char inquiry[10];
	unsigned char data[255];
	long blocks;
	register long cyls,tracksize,i,heads;
	int  opened = FALSE;
	register struct IOStdReq *ior  = NULL;
	struct MsgPort  *port;
	register struct Drive *d;
	LONG zones,alt_sectors,alt_tracks,alt_volume,cylblocks;
	LONG blocks_lost = -1,write_precomp = 0,reduced_write = 0;
	LONG landing_zone = 0,tracksperzone,oldheads,oldtracksize;
	int rc;

	d = SelectedDrive;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;

printf("opened unit %ld\n",d->Addr + d->Lun * 10);
	scsiversion = ior->io_Device->dd_Library.lib_Version;

	/* first we try mode_sense */

	inquiry[0] = S_MODE_SENSE;
	inquiry[1] = 0;
	inquiry[2] = 3;	/* page 3 - we want sectors per track, current values */
	inquiry[3] = 0;
	inquiry[4] = 255;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,255,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		i = 4 + data[3];	/* block descriptor length */
		if (cmdblk.scsi_Actual < i + 12)
{
/*Notify(w,"Sense Actual = %ld",cmdblk.scsi_Actual);*/
			goto try_inquiry;
}
		/* ignore blocks - can get from read_capacity */

		/* page descriptors */


/*
Notify(w,"Page = %ld",data[i] & 0x3f);
Notify(w,"Page len = %ld",data[i+1]);
*/
		if ((data[i] & 0x3f) != 3 || data[i+1] < 10)
			goto try_inquiry;

		tracksize = (data[i+10] << 8) | data[i+11];

/*
Notify(w,"tracksize = %ld",tracksize);
*/
		if (tracksize == 0)
			goto try_inquiry;

		tracksperzone = (data[i+2] << 8) | data[i+3];
		alt_sectors   = (data[i+4] << 8) | data[i+5];
		alt_tracks    = (data[i+6] << 8) | data[i+7];
		alt_volume    = (data[i+8] << 8) | data[i+9];
/*
Notify(w,"tpz = %ld",tracksperzone);
Notify(w,"alt_sectors = %ld",alt_sectors);
Notify(w,"alt_tracks = %ld",alt_tracks);
Notify(w,"alt_volume = %ld",alt_volume);
*/
		/* we have tracksize, now try for heads */
		inquiry[2] = 4;	/* we want cylinders & heads,current values */

		if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,255,
			    SCSIF_READ|SCSIF_AUTOSENSE))
		{
			i = 4 + data[3];	/* block descriptor length */
			if (cmdblk.scsi_Actual < i + 6)
{
/*Notify(w,"Second Sense Actual = %ld",cmdblk.scsi_Actual);*/
				goto try_inquiry;
}
			/* page descriptors */
/*
Notify(w,"Page = %ld",data[i] & 0x3f);
Notify(w,"Page len = %ld",data[i+1]);
*/
			if ((data[i] & 0x3f) != 4 || data[i+1] < 4)
				goto try_inquiry;

			/* cyls are raw, and may include totally invisible   */
			/* cylinders used by the drive for it's own purposes */
			cyls  = (data[i+2] << 16) |
				(data[i+3] << 8)  |
				(data[i+4]);
			heads =  data[i+5];
			if (heads == 0)		/* avoid x/0 problems */
				heads = 1;
			cylblocks = heads * tracksize;


			/* save for later GODDAMN SCSI VENDORS!!! */
			oldheads = heads;
			oldtracksize = tracksize;

/*
Notify(w,"heads = %ld",heads);
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",cylblocks);
*/
			if (data[i+1] >= 15) {
				write_precomp = (data[i+6] << 16) |
						(data[i+7] << 8)  |
						 data[i+8];
				reduced_write = (data[i+9]  << 16) |
						(data[i+10] << 8)  |
						 data[i+11];
				landing_zone  = (data[i+14] << 16) |
						(data[i+15] << 8)  |
						 data[i+16];
			}
/*
Notify(w,"write precomp = %ld",write_precomp);
Notify(w,"reduced write = %ld",reduced_write);
Notify(w,"landing zone  = %ld",landing_zone);
*/
			if (tracksperzone == 0)
				zones = 1;
			else
				/* make sure it rounds up */
				zones = (heads*cyls + tracksperzone-1) /
					tracksperzone;
/*
Notify(w,"Zones = %ld",zones);
*/
			/* now adjust raw numbers... round up (+ heads-1) */
			if (alt_volume || alt_tracks)
			    cyls -= (alt_volume + alt_tracks*zones + heads-1) /
				    heads;

			/* we're going to kodiak's definitions of CylBlocks */
			/* Sectors * Heads does not have to equal CylBlocks */
			if (alt_sectors)
			{
				if (tracksperzone == heads)
					cylblocks -= alt_sectors;
				else if (tracksperzone%heads == 0 &&
					 alt_sectors%(tracksperzone/heads) == 0)
				{
				  /* integer number of sectors/cyl mapped out */
					cylblocks -=
					      alt_sectors%(tracksperzone/heads);
				} else if (alt_sectors % tracksperzone == 0) {
					/* N sectors per track */
					tracksize -= alt_sectors/tracksperzone;
					cylblocks = heads*tracksize;
				} else {
					/* else there's no good multiple! */
					goto try_inquiry;
				}
			}

/*
Notify(w,"heads = %ld",heads);
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",cylblocks);
*/
			/* now sanity check */
			blocks = ReadCapacity(0,0);

/*
Notify(w,"blocks = %ld",blocks);
Notify(w,"cyls*cylblocks = %ld",cyls * cylblocks);
*/
			/* do the numbers add up? */
			if (blocks < cyls * cylblocks)
			{
				/* check if it's an even number of cyls   */
				/* SCSI drives grab a cylinder or two for */
				/* themselves to store private info.      */

				if ((cyls*cylblocks-blocks) % cylblocks == 0)
				{
					/* yes - reduce cylinder count */
					cyls = blocks/cylblocks;
				} else {
					blocks_lost = -1;
					goto try_inquiry;
				}
			} else {
				/* we're right on, or are missing some blocks */
				/* try to use them if we can */
				cyls += (blocks-cyls*cylblocks)/cylblocks;
			}

			blocks_lost = blocks - cyls*cylblocks;

/*
Notify(w,"blocks lost = %ld",blocks_lost);
*/
			/* set geometry no matter how far off */
			SetGeometry(w,NewDef->Initial_RDB,cyls,heads,
				    tracksize,cylblocks,write_precomp,
				    reduced_write,landing_zone);

		} /* else mode_sense for page 4 failed */

	} /* else mode_sense for page 3 failed */

	/* now we try inquiry */
try_inquiry:

	inquiry[0] = S_INQUIRY;
	inquiry[1] =
	inquiry[2] =
	inquiry[3] = 0;
	inquiry[4] = 36;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,36,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		if (cmdblk.scsi_Actual < 36 || data[4] < 31)
		{
			Notify(w,"Insufficient data returned by inquiry",0);
			goto cleanup;
		}
		if (data[0] != 0)
		{
			Notify(w,"Unit is not a disk (type %ld)!",data[0]);
			goto cleanup;
		}
		if ((data[3] & 0x0f) > 2) /* accept 0 (unspecified), 1, or 2 */
		{
			Notify(w,"Don't understand Inquiry data (type 0x%lx)!",
			       data[3]);
			goto cleanup;
		}

		memcpy(NDManuNameSIBuff,&data[8],8);
		NDManuNameSIBuff[8] = '\0';
		strip_trail(NDManuNameSIBuff);

		memcpy(NDDriveNameSIBuff,&data[16],16);
		NDDriveNameSIBuff[16] = '\0';
		strip_trail(NDDriveNameSIBuff);

		memcpy(NDRevisionNameSIBuff,&data[32],4);
		NDRevisionNameSIBuff[4] = '\0';
		strip_trail(NDRevisionNameSIBuff);

		blocks = ReadCapacity(0,0);
		if (blocks != -1)
		{
			heads = 1;

			/* PMI = 1 means next block before next "delay" */
			tracksize = ReadCapacity(1,0);
			if (tracksize == -1 || tracksize == blocks)
			{
				Notify(w,"Cannot find track size, assuming %ld",
				       17 * 4);
				tracksize = 17 * 4;	/* punt */
			}

			if (tracksize == oldheads*oldtracksize)
			{
				heads     = oldheads;
				tracksize = oldtracksize;
			}

			/* make sure cyls < 4096 (?) */
			cyls = blocks/(heads*tracksize);
			while (cyls > 4095)
			{
				tracksize <<= 1;
				cyls = blocks/(heads*tracksize);
			}

/*
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",heads*tracksize);
Notify(w,"block_lost = %ld",blocks-cyls*heads*tracksize);
*/
			/* is this closer than I got via normal means? */
			if (blocks_lost == -1 ||
			    blocks - cyls*heads*tracksize < blocks_lost)
			{
				SetGeometry(w,NewDef->Initial_RDB,cyls,heads,
					    tracksize,heads*tracksize,
					    write_precomp,
					    reduced_write,landing_zone);
			}
		} else
			Notify(w,"Cannot read device size!",NULL);

	} else
		Notify(w,"Drive does not support the SCSI Inquiry command!",
		       NULL);

	rc = TRUE;
	goto cleanup2;

cleanup:
	rc = FALSE;
cleanup2:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return rc;
}

void
SetGeometry (w,rdb,cyls,heads,sectors,cylblocks,precomp,reduced,park)
	struct Window *w;
	register struct RigidDiskBlock *rdb;
	register LONG cyls,heads,sectors,cylblocks,precomp,reduced,park;
{
	if (park == 0)
		park = cyls;
	if (precomp == 0)
		precomp = cyls;
	if (reduced == 0)
		reduced = cyls;

	rdb->rdb_Sectors      = sectors;
	rdb->rdb_Heads        = heads;
	rdb->rdb_Cylinders    = cyls;
	rdb->rdb_CylBlocks    = cylblocks;
	rdb->rdb_LoCylinder   = 2;		/* FIX! */
	rdb->rdb_HiCylinder   = cyls-1;
	rdb->rdb_Park	      = park;
	rdb->rdb_WritePreComp = precomp;
	rdb->rdb_ReducedWrite = reduced;
	rdb->rdb_RDBBlocksLo  = 0;
	rdb->rdb_RDBBlocksHi  = 2 * rdb->rdb_CylBlocks - 1;	/* FIX! */
}

/* strip trailing blanks */

void
strip_trail (str)
	register char *str;
{
	register char *orig = str;

	str += strlen(str) - 1;

	while (*str == ' ' && str >= orig)
		*str-- = '\0';
}

void
Notify (w,s,x)
	struct Window *w;
	char *s;
	LONG x;
{
	printf(s,x);
	printf("\n");
}

/* convert C string to BCPL string */

void
CtoBStr (str)
	register char *str;
{
	register int len,i;

	len = strlen(str);
	for (i = len; i > 0; i--)
		str[i] = str[i-1];

	str[0] = (char) len;
}

