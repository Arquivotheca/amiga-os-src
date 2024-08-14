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
	p->id_DiskState		= ID_WRITE_PROTECTED;
	p->id_NumBlocks		=
	p->id_NumBlocksUsed	= 1 + (spaceused >> 9);
	p->id_BytesPerBlock	= 512;
	p->id_DiskType		= ID_DOS_DISK;
	p->id_VolumeNode	= ((LONG) volumenode) >> 2;
	p->id_InUse		= NULL;       /*FIX!!! lock_list;*/

	return DOS_TRUE;
}
