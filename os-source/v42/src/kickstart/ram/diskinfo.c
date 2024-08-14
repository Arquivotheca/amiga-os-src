/* diskinfo.c */

#include "ram_includes.h"
#include "ram.h"

LONG
diskinfo (pkt)
	struct DosPacket *pkt;
{
	struct InfoData *p;

	p = (struct InfoData *) ((pkt->dp_Action == ACTION_DISK_INFO ?
				  pkt->dp_Arg1 : pkt->dp_Arg2) << 2);

	p->id_NumSoftErrors	= 0;
	p->id_UnitNumber	= -1;
	p->id_DiskState		= ID_VALIDATED;
	p->id_NumBlocks		= spaceused;
	p->id_NumBlocksUsed	= spaceused;
	p->id_BytesPerBlock	= MIN_BLKSIZE;
	p->id_DiskType		= ID_DOS_DISK;
	p->id_VolumeNode	= ((LONG) volumenode) >> 2;
	p->id_InUse		= NULL;       /*FIX!!! lock_list;*/

	return DOS_TRUE;
}
