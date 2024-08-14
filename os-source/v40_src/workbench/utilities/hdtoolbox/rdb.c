/* rdb.c */
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "protos.h"
#include "global.h"

extern struct SysBase *SysBase;

void
FreePartitionList (p)
	register struct PartitionBlock *p;
{
	register struct PartitionBlock *nextp;

	while (p) {
		nextp = p->pb_Next;

		FreeMem((char *) p,sizeof(*p));

		p = nextp;
	}
}

void
FreeLoadSegList (ls)
	register struct LoadSegBlock *ls;
{
	register struct LoadSegBlock *nextls = NULL;

	long size = 0;

	for (; ls; ls = nextls)	{
	  if (!size) size = ls->lsb_SummedLongs * 4; /* assume first block is full */
	  nextls = ls->lsb_Next;
	  FreeMem((char *) ls, size);
	}
}

void
FreeFileSys (fsb)
	struct FileSysHeaderBlock *fsb;
{
	if (fsb) {
		FreeLoadSegList(fsb->fhb_SegListBlocks);
		FreeMem((char *) fsb, sizeof(*fsb));
	}
}

/* replace file sys 1 with file sys 2 in rdb's list, free file sys 1 */

void
ReplaceFileSys (rdb,fsb1,fsb2)
	struct RigidDiskBlock *rdb;
	struct FileSysHeaderBlock *fsb1,*fsb2;
{
	register struct FileSysHeaderBlock *temp;

	/* first replace pointer to fsb1 */

	if (rdb->rdb_FileSysHeaderList == fsb1)
	{
		rdb->rdb_FileSysHeaderList = fsb2;
	} else {
		for (temp = rdb->rdb_FileSysHeaderList;
		     temp && temp->fhb_Next != fsb1;
		     temp = temp->fhb_Next)
			; /* NULL */
		if (!temp)
		{
			FreeFileSys(fsb2);	/* failure!!! */
			return;
		}
		temp->fhb_Next = fsb2;
	}

	fsb2->fhb_Next = fsb1->fhb_Next;

	FreeFileSys(fsb1);
}

void
FreeFileSysList (fsb)
	register struct FileSysHeaderBlock *fsb;
{
	register struct FileSysHeaderBlock *next;

	while (fsb) {
		next = fsb->fhb_Next;
		FreeFileSys(fsb);
		fsb = next;
	}
}

void
FreeBadBlockList (bb)
	register struct BadBlockBlock *bb;
{
	register struct BadBlockBlock *nextbb = NULL;

	for (;
	     bb;
	     bb = nextbb)
	{
		nextbb = bb->bbb_Next;
		FreeMem((char *) bb, bb->bbb_SummedLongs * 4);
	}
}

void
FreeRDB (rdb)
	register struct RigidDiskBlock *rdb;
{
	if (rdb)
	{
		FreePartitionList(rdb->rdb_PartitionList);
		FreeFileSysList(rdb->rdb_FileSysHeaderList);
		FreeBadBlockList(rdb->rdb_BadBlockList);
/*FIX*/		/* FreeDriveInit(rdb->rdb_DriveInit); */

		FreeMem((char *) rdb, sizeof(*rdb));
	}
}

struct FileSysHeaderBlock *
CopyFileSys (fsb)
	register struct FileSysHeaderBlock *fsb;
{
	register struct FileSysHeaderBlock *new = NULL;

	if (fsb) {
		if (new = AllocNew(FileSysHeaderBlock))
		{
			*new = *fsb;
			new->fhb_Next = NULL;
			if (!(new->fhb_SegListBlocks =
			      CopyLoadSegList(fsb->fhb_SegListBlocks)))
			{
				FreeFileSys(new);
				new = NULL;
			}
		}
	}
	return new;
}

struct LoadSegBlock *
CopyLoadSeg (ls, bs)
	register struct LoadSegBlock *ls;
        register ULONG bs;
{
	register struct LoadSegBlock *new = NULL;

	if (ls) {
		if (new = (struct LoadSegBlock *)
			  AllocMem(bs, MEMF_CLEAR))
		{
			memcpy((char *) new, (char *) ls, bs);
			new->lsb_Next = NULL;
		}
	}
	return new;
}

struct BadBlockBlock *
CopyBadBlock (bb)
	register struct BadBlockBlock *bb;
{
	register struct BadBlockBlock *new = NULL;

	if (bb) {
		if (new = (struct BadBlockBlock *)
			  AllocMem(bb->bbb_SummedLongs * 4,MEMF_CLEAR))
		{
			memcpy((char *) new, (char *) bb,
			       bb->bbb_SummedLongs * 4);
			new->bbb_Next = NULL;
		}
	}
	return new;
}

struct FileSysHeaderBlock *
CopyFileSysList (fsb)
	register struct FileSysHeaderBlock *fsb;
{
		 struct FileSysHeaderBlock *head = NULL;
	register struct FileSysHeaderBlock *curr;

	if (fsb) {
		curr = head = CopyFileSys(fsb);
		while (fsb->fhb_Next && curr) {
			fsb = fsb->fhb_Next;
			if (curr->fhb_Next = CopyFileSys(fsb))
			{
				curr = curr->fhb_Next;
				curr->fhb_Next = NULL;
			}
		}
		if (!curr)
		{
			FreeFileSysList(head);
			head = NULL;
		}
	}
	return head;
}

struct LoadSegBlock *
CopyLoadSegList (ls)
	register struct LoadSegBlock *ls;
{
		 struct LoadSegBlock *head = NULL;
	register struct LoadSegBlock *curr;

	ULONG bs = 0;

	if (ls) {
		bs = ls->lsb_SummedLongs * 4; /* assume first block is full */
		curr = head = CopyLoadSeg(ls, bs);
		while (ls->lsb_Next && curr) {
			ls = ls->lsb_Next;
			if (curr->lsb_Next = CopyLoadSeg(ls, bs))
			{
				curr = curr->lsb_Next;
				curr->lsb_Next = NULL;
			}
		}
		if (!curr)
		{
			FreeLoadSegList(head);
			head = NULL;
		}
	}
	return head;
}

struct BadBlockBlock *
CopyBadBlockList (bb)
	register struct BadBlockBlock *bb;
{
		 struct BadBlockBlock *head = NULL;
	register struct BadBlockBlock *curr;

	if (bb) {
		curr = head = CopyBadBlock(bb);
		while (bb->bbb_Next && curr) {
			bb = bb->bbb_Next;
			if (curr->bbb_Next = CopyBadBlock(bb))
			{
				curr = curr->bbb_Next;
				curr->bbb_Next = NULL;
			}
		}
		if (!curr)
		{
			FreeBadBlockList(head);
			head = NULL;
		}
	}
	return head;
}

struct PartitionBlock *
CopyPartList (p)
		 struct PartitionBlock *p;
{
		 struct PartitionBlock *headp;
	register struct PartitionBlock *newp,*tailp;

	tailp = headp = NULL;

	while (p)
	{
		if (newp = AllocNew(PartitionBlock)) {
			if (headp == NULL)
				headp = newp;
			if (tailp != NULL)
				tailp->pb_Next = newp;
			tailp = newp;

			*newp = *p;	/* structure copy */

			newp->pb_Next = NULL;
		}
		p = p->pb_Next;
	}
	return headp;
}

struct RigidDiskBlock *
CopyRDB (rdb)
	register struct RigidDiskBlock *rdb;
{
	register struct RigidDiskBlock *newrdb;

	if (!rdb || !(newrdb = AllocNew(RigidDiskBlock)))
		return NULL;

	*newrdb = *rdb;

	newrdb->rdb_PartitionList = CopyPartList(rdb->rdb_PartitionList);
	newrdb->rdb_FileSysHeaderList = CopyFileSysList(rdb->rdb_FileSysHeaderList);
	newrdb->rdb_BadBlockList = CopyBadBlockList(rdb->rdb_BadBlockList);
/* FIX */

	return newrdb;
}
