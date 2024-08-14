/* Badblock.c - code for the bad block entry screen. */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"
#include "scsidisk.h"
#include "scsi.h"

//
extern struct Gadget *glist10;
USHORT BadDeleteFlags = 0;

extern struct Remember	*RKey;
struct List	lh10;		/* List for DriveList */
UWORD BadListPosition = 0;	/* highlight position */
#define	BADLIST_LENGTH 80	/* BadList string length */

char BadBlockText[10];
//

WORD BadBlockUpdate = 0;

//struct Bad *Bad = NULL;

int
HandleBadBlock (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct Gadget *gad;
	register int done = FALSE;
	UWORD id;

	switch (msg->Class) {
	case GADGETUP:
	case GADGETDOWN:
		gad = (struct Gadget *) (msg->IAddress);

		id  = gad->GadgetID & ~REFRESH_MASK;

		switch (id) {
		case BADOK:
			if (BadBlockUpdate)
			{
			    FreeBadBlockList(SelectedDrive->rdb->rdb_BadBlockList);
			    SelectedDrive->rdb->rdb_BadBlockList =
						     MakeBadBlockList(FirstBad);
			    if (SelectedDrive->bad)
				 FreeBad(SelectedDrive->bad);

			    SelectedDrive->bad    = FirstBad;
			    SelectedDrive->Flags |= UPDATE;
			}
			done = HANDLE_DONE;
			break;

		case BADCANCEL:
			FreeBad(FirstBad);
			done = HANDLE_CANCEL;
			break;

		case BADDELETE:
			DeleteBad(w);
			break;

		case BADADD:
			AddBad(w);
			break;

		case BADLIST:
			HandleBadClick(w,msg);
			break;
		}
		break;
//
//	case MENUPICK:			// I don't want to use menu now.
//		HandleMenus(w,msg);
//		break;

	}

	return done;
}

/* WARNING! called from verify.c to add bad blocks! */

int
BadSetup (w)
	struct Window *w;
{
	register struct Bad *b;
	register struct BadBlockBlock *bb;
	struct RigidDiskBlock *rdb;
	register int j;

	rdb = SelectedDrive->rdb;

	FirstBad = NULL;
	NumBad   = 0;

/*
	Build a list of bad blocks from the Rigid Disk Block.  When exiting,
	if changed, convert the list back to BadBlockBlocks.  This allows
	much easier handling of insertions/deletions.

	Open question: should he be able to delete an old bad block? FIX!
*/
	/* modify badblocks mapped out IText */
	sprintf(BadBlockText,"%ld",SelectedDrive->SCSIBadBlocks);

	/* get bad blocks */
	if (SelectedDrive->bad)
	{
		/* copy old list */
		b = SelectedDrive->bad;
		CurrBad = DisplayBad = FirstBad = CopyBad(b);
		if (!CurrBad)
			return FALSE;

		NumBad++;
		for (b = b->Next; b; b = b->Next)
		{
			CurrBad->Next = CopyBad(b);
			if (!CurrBad->Next)
			{
				FreeBad(FirstBad);
				return FALSE;
			}
			NumBad++;
			CurrBad = CurrBad->Next;
		}

	} else {

	    /* Make list from badblockblocks */
	    for (bb = rdb->rdb_BadBlockList;
		    bb;
		    bb = bb->bbb_Next)
	    {
		for (j = 0;
		     j < ((bb->bbb_SummedLongs - 6) * 4) /
			 sizeof(struct BadBlockEntry);
		     j++)
		{
			b = AllocNew(Bad);
			if (!b)
			{
				FreeBad(FirstBad);
				return FALSE;
			}

			b->Next	       = NULL;
			b->flags       = OLDBADBLOCK;
			b->block       = bb->bbb_BlockPairs[j].bbe_BadBlock;
			b->replacement = bb->bbb_BlockPairs[j].bbe_GoodBlock;

			/* this use of sectors is OK */
			b->cyl    = b->block / rdb->rdb_CylBlocks;
			b->head	  = (b->block - b->cyl * rdb->rdb_CylBlocks) /
				    rdb->rdb_Sectors;
			b->sector = b->block -
				    b->cyl  * rdb->rdb_CylBlocks -
				    b->head * rdb->rdb_Sectors;

			/* bumps NumBad */
			InsertBad(NULL,b);
		}
	    }
	}

	CurrBad = DisplayBad = FirstBad;
	BadBlockUpdate = 0;
	DisplayBadNum = 0;

//	MaxDisplayBad = BadList->Height / (w->RPort->TxHeight + 1);

	if (CurrBad == NULL)
		BadDeleteFlags |= GADGDISABLED;
	else
		BadDeleteFlags &= ~GADGDISABLED;


	return TRUE;
}

void
DeleteBad (w)
	struct Window *w;
{
	register struct Bad *b,*lastb;

	if (CurrBad)
	{
		for (lastb = NULL, b = FirstBad;
		     b && b != CurrBad;
		     lastb = b, b = b->Next)
			; /* NULL */
		if (!b)
			return;	/* ERROR */

		if (lastb == NULL)
		{
			FirstBad   = FirstBad->Next;	/* delete first one */
			DisplayBad = CurrBad = FirstBad;
			DisplayBadNum = 0;
		} else {
			lastb->Next = CurrBad->Next;
			if (DisplayBad == CurrBad)
			{
				DisplayBad = lastb;
				DisplayBadNum--;
			}
			b = CurrBad; /* safety */
			CurrBad = lastb;
			if (NumBad-1 <= MaxDisplayBad)
			{
				DisplayBad    = FirstBad;
				DisplayBadNum = 0;
			}
		}

		NumBad--;
		BadBlockUpdate = TRUE;

		FreeMem((char *) b, sizeof(*b));

		BadDraw(w->RPort);
		GT_SetGadgetAttrs(BadList,w,NULL,GTLV_Labels,&lh10,TAG_DONE);//

		if (NumBad == 0)
			MyOffGadget(w,BadDelete);	//

	} else
		Notify(w,"No bad block selected!",(APTR) 0);
}

void
AddBad (w)
	struct Window *w;
{
	register struct Bad *newb;

	if (newb = GetBad(w))
	{
		MaxDisplayBad = BadList->Height / (w->RPort->TxHeight + 1);	//
		InsertBad(w,newb);

		BadDraw(w->RPort);
		GT_SetGadgetAttrs(BadList,w,NULL,
				GTLV_Labels,&lh10,TAG_DONE);	//
		GT_SetGadgetAttrs(BadList,w,NULL,
				GTLV_Selected,BadListPosition,TAG_DONE); //
		GT_SetGadgetAttrs(BadList,w,NULL,
				GTLV_Top,DisplayBadNum,TAG_DONE); //
		BadBlockUpdate = TRUE;

		MyOnGadget(w,BadDelete);	//
	}
}

/* insert a bad block in sorted order.  Ignore if duplicate. */
/* called from init with NULL window ptr */
/* WARNING! called from verify to add bad blocks */

void
InsertBad (w,newb)
	struct Window *w;
	register struct Bad *newb;
{
	register struct Bad *b;
	register int i,ord;

	/* insert in sorted order */
	if (!FirstBad || (ord = BadLessEqual(newb,FirstBad)) != 1)
	{
		if (FirstBad && ord == 0)
		{
			/* same as an old one: drop */
			FreeMem((char *) newb,sizeof(struct Bad));
			return;
		}
		/* must go first */
		newb->Next = FirstBad;
		FirstBad = DisplayBad = CurrBad = newb;
		DisplayBadNum = 0;
		BadListPosition = 0;	//
	} else
		for (b = FirstBad, i = 1; b; b = b->Next, i++)
		{
			if (b->Next) {
				ord = BadLessEqual(newb,b->Next);
				if (ord == 1)
					continue;	/* not yet */
				if (ord == 0)
				{
					/* same as an old one: drop */
					FreeMem((char *) newb,
						sizeof(struct Bad));
					return;
				}
			} /* else put it on the end */

			/* found place to add it! */
			newb->Next = b->Next;
			CurrBad = b->Next = newb;

			BadListPosition = i;	//

			if (i <= DisplayBadNum ||
			    i >= DisplayBadNum + MaxDisplayBad)
			{
				/* Can't be seen: make visible */
//				if (i < MaxDisplayBad/2 ||
				if (i < MaxDisplayBad ||
				    NumBad < MaxDisplayBad)
				{
					DisplayBadNum = 0;
					DisplayBad = FirstBad;
				} else {
					/* put in middle of screen */
//					DisplayBadNum = i - MaxDisplayBad/2;
					// We can't put middle of screen of
					// GadTools library.
					DisplayBadNum = i + 1 - MaxDisplayBad;
					DisplayBad = FirstBad;
					for (i = 0; i < DisplayBadNum; i++)
						DisplayBad = DisplayBad->Next;
				}
			}

			break;
		}
	NumBad++;
}

/* return -1 if b1 comes before b2, 0 for equal, 1 for b2 before b1 */

int
BadLessEqual (b1,b2)
	register struct Bad *b1,*b2;
{

	if (b1->cyl < b2->cyl)
		return -1;
	if (b1->cyl > b2->cyl)
		return 1;

	if (b1->head < b2->head)	/* cyls == same */
		return -1;
	if (b1->head > b2->head)
		return 1;

	if (b1->sector < b2->sector)	/* heads == same */
		return -1;
	if (b1->sector > b2->sector)
		return 1;

	return 0;
}

int badsector = 0;

int
GetBadHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	struct IntuiMessage *msg;
{
	int done = 0;
	WORD pos;

	if (msg->Class == GADGETUP)
	{
		if (msg->IAddress == (APTR) &BadGetOK)
		{
			done = HANDLE_DONE;

		} else if (msg->IAddress == (APTR) &BadGetCancel)
		{
			done = HANDLE_CANCEL;

		} else if (msg->IAddress == (APTR) &BadSector)
		{
			pos = RemoveGList(w,&BadBfi,1L);
			BadBfiSInfo.LongInt = 0;
			BadBfiSIBuff[0] = '\0';
			AddGList(w,&BadBfi,pos,1L,req);
			RefreshGList(&BadBfi,w,req,1L);
			badsector = 0;

		} else if (msg->IAddress == (APTR) &BadBfi)
		{
			pos = RemoveGList(w,&BadSector,1L);
			BadSectorSInfo.LongInt =
					       BfiToSector(BadBfiSInfo.LongInt);
			sprintf(BadSectorSIBuff,"%ld",BadSectorSInfo.LongInt);
			AddGList(w,&BadSector,pos,1L,req);
			RefreshGList(&BadSector,w,req,1L);
			badsector = 1;

		} else if (msg->IAddress == (APTR) &BadCylinder)
		{
			/* activate the head string gadget */
			ActivateGadget(&BadHead,w,req);

		} else if (msg->IAddress == (APTR) &BadHead)
		{
			/* activate the sector string gadget */
			ActivateGadget(&BadSector,w,req);

		} else
			;

	} else if (msg->Class == REQSET) {
		/* activate the filename string gadget */
		ActivateGadget(&BadCylinder,w,req);
	}

	return done;
}

struct Bad *
GetBad (w)
	struct Window *w;
{
	register struct RigidDiskBlock *rdb;
	register struct Bad *b = NULL;
	register long cyl,head,bfi,sector;

	badsector = 0;

again:
	MakePrettyRequestBorder(BadRequester.Width,BadRequester.Height); //

	if (HandleRequest(w,&BadRequester,GetBadHandler) == HANDLE_DONE)
	{
		cyl  = BadCylinderSInfo.LongInt;
		head = BadHeadSInfo.LongInt;
		bfi  = BadBfiSInfo.LongInt;
		sector = BadSectorSInfo.LongInt;

		if (cyl < 0 || cyl > SelectedDrive->rdb->rdb_HiCylinder)
		{
			Notify(w,"Cylinder was out of range 0 to %ld!",
				SelectedDrive->rdb->rdb_HiCylinder);
			goto again;
		}
		if (head < 0 || head >= SelectedDrive->rdb->rdb_Heads)
		{
			Notify(w,"Head was out of range 0 to %ld!",
				SelectedDrive->rdb->rdb_Heads - 1);
			goto again;
		}
		if (bfi < 0)
		{
			Notify(w,"Bytes From Index must be positive!",0);
			goto again;
		}

/* FIX! - maybe this should be checked for a max! */

		/* this use of sectors is OK */
		if (sector > SelectedDrive->rdb->rdb_Sectors - 1)
		{
			Notify(w,"Sector too large for size of track!",0);
			goto again;
		}

		/* check for cylblocks != heads * sectors */

		if ((head == SelectedDrive->rdb->rdb_Heads) &&
		    (SelectedDrive->rdb->rdb_CylBlocks !=
				SelectedDrive->rdb->rdb_Heads *
				SelectedDrive->rdb->rdb_Sectors) &&
		    (sector + head*SelectedDrive->rdb->rdb_Sectors >=
					SelectedDrive->rdb->rdb_CylBlocks))
		{
			/* he tried to map out a sector used by the drive */
			Notify(w,"That sector is not a user data sector!",0);
			goto again;
		}

		if (b = AllocNew(Bad))
		{
			rdb = SelectedDrive->rdb;

			b->Next   = NULL;
			b->flags  = badsector ? BADSECTOR : 0;
			b->cyl    = cyl;
			b->head   = head;
			b->bfi    = bfi;
			b->sector = sector;
			/* this use of sectors is OK */
			b->block  = cyl  * rdb->rdb_CylBlocks +
				    head * rdb->rdb_Sectors +
				    sector;
			b->replacement = GetReplacement(rdb,FirstBad);

			if (b->replacement == 0)
			{
				FreeMem((char *) b,sizeof(*b));
				b = NULL;
			}
		}
	}
	return b;
}

/*
  from Western Digital: for their 20 meg XT drives, the BFI calculation is:

	575 + sector * 575

  It is assumed that sector == 512 bytes.
*/

LONG
BfiToSector (bfi)
	register LONG bfi;
{
	register LONG sector;

	/* this is based on the Western digital calculations! */
	/* assumes all 575 bytes are used by the sector! */

	sector = max(bfi-575,0) / (SelectedDrive->rdb->rdb_BlockBytes + 63);

#ifdef A2090BADBLOCKS
	sector = bfi / (SelectedDrive->rdb->rdb_BlockBytes + 85); /* FIX! */
	if (bfi % (SelectedDrive->rdb->rdb_BlockBytes + 85) >
	    (SelectedDrive->rdb->rdb_BlockBytes + 64))
	{
		sector++;
	}
#endif

	/* sector is raw sector number.  To determine actual sector, use     */
	/* interleave. 							     */
	/* we don't need to do this, since all internal bad block numbers    */
	/* assume an interleave of 1 (we xlate before writing/after reading) */
	/* see readwrite.c for the translations.			     */

	return sector;
}

LONG
MinBfi (sector)
	LONG sector;
{
	if (sector == 0)
		return 0;

	return (long)(575 + (sector * (SelectedDrive->rdb->rdb_BlockBytes + 63)));

#ifdef A2090BADBLOCKS
	return ((sector - 1) * (SelectedDrive->rdb->rdb_BlockBytes + 85) +
		SelectedDrive->rdb->rdb_BlockBytes + 65); /* fix */
#endif
}

LONG
MaxBfi (sector)
	LONG sector;
{
	return (long)(575 + ((sector+1)*(SelectedDrive->rdb->rdb_BlockBytes + 63)) - 1);

#ifdef A2090BADBLOCKS
	return (sector * (SelectedDrive->rdb->rdb_BlockBytes + 85) +
		SelectedDrive->rdb->rdb_BlockBytes + 64); /* fix */
#endif
}

static UWORD gotit;

#if 0
void
BadDraw (rp)
	struct RastPort *rp;
{
	register UWORD x,y,i;
	register struct Bad *b;
	char buffer[80];

	gotit = FALSE;

	x = BadList.LeftEdge;
	y = BadList.TopEdge;

	if (DisplayBad == NULL)		/* Can't update prop gadget here */
		DisplayBad = FirstBad;

	/* clear to erase old text */
	SetAPen(rp,0L);
	SetDrMd(rp,JAM1);
	RectFill(rp,x,y,x + BadList.Width - 1,y + BadList.Height - 1);

	SetAPen(rp,1L);

	for (b = DisplayBad, i = 0;
	     b && i < MaxDisplayBad;
	     b = b->Next, i++)
	{
		if (b->flags & (OLDBADBLOCK|BADSECTOR))
		   sprintf(buffer,
			"%6ld  %6ld     %6ld-%-6ld   %6ld",
			b->cyl,b->head,MinBfi(b->sector),MaxBfi(b->sector),
			b->sector);
		else
		   sprintf(buffer,
			"%6ld  %6ld       %6ld        %6ld",
			b->cyl,b->head,b->bfi,b->sector);

		Move(rp,x,y + rp->TxBaseline + 1);
		Text(rp,buffer,strlen(buffer));

		if (b == CurrBad)
		{
			SetAPen(rp,255L);
			SetDrMd(rp,COMPLEMENT);

			RectFill(rp,x,y,
				    x + BadList.Width - 1,
				    y + rp->TxHeight);
			SetAPen(rp,1L);
			SetDrMd(rp,JAM1);

			gotit = 1;
		}
		y += rp->TxHeight + 1;
	}
}
#endif

void
BadDraw (rp)
	struct RastPort *rp;
{
	register i;
	register struct Bad *b;
	char buffer[BADLIST_LENGTH];

	struct Node *node,*nodenew;

//	kprintf("redraw\n");
	gotit = FALSE;

	// Set first node in Bad list.
	node = ((struct List *)(&lh10))->lh_Head;

	for (b = FirstBad, i = 0;
	     b;
	     b = b->Next, i++)
	{
		if (b->flags & (OLDBADBLOCK|BADSECTOR))
		   sprintf(buffer,
			"%6ld  %6ld     %6ld-%-6ld   %6ld",
			b->cyl,b->head,MinBfi(b->sector),MaxBfi(b->sector),
			b->sector);
		else
		   sprintf(buffer,
			"%6ld  %6ld       %6ld        %6ld",
			b->cyl,b->head,b->bfi,b->sector);

//		kprintf("buffer %s\n",buffer);

		// Update Bad list.
		if (node->ln_Succ)	// Update
		{
			if (strcmp(node->ln_Name,buffer) != 0)
				strcpy(node->ln_Name,buffer);
//			sprintf(tmp,"%x",node);
//			kprintf("node %s - ",tmp);
//			kprintf("update %s\n",node->ln_Name);

			/* We don't need worry about add case,
			   because list is added only once at a time. */
			node = node->ln_Succ;
		} else {		// Add Tail list
			if (nodenew = (struct Node *)
				AllocRemember(&RKey,
					sizeof(struct Node),MEMF_CLEAR))
			{
				if(nodenew->ln_Name = AllocRemember(&RKey,
						BADLIST_LENGTH + 1,MEMF_CLEAR))
				{
					strcpy(nodenew->ln_Name,buffer);
					AddTail(&lh10,nodenew);
//					sprintf(tmp,"%x",nodenew);
//					kprintf("node %s - ",tmp);
//					kprintf("add %s\n",nodenew->ln_Name);
				}
			}
		}

		if (b == CurrBad)
		{
			gotit = 1;
		}
	}

	if (node->ln_Succ)
	{
		// Don't be upset to delete tail of list.
		// Because I already update the content of list above.
		// The deleted content of list does not mean anything.
		node = RemTail(&lh10);	// Delete Tail List
//		sprintf(tmp,"%x",node);
//		kprintf("node %s - deleted\n",tmp);
	}
}

// Allocate rtn for LISTVIEW_KIND Gadgets

void AllocBadList(void)
{
	register UWORD i;
	register struct Bad *b;
	char buffer[BADLIST_LENGTH];

	struct Node *node;

	NewList(&lh10);

	for (b = FirstBad, i = 0;
	     b;
	     b = b->Next, i++)
	{
		if (b->flags & (OLDBADBLOCK|BADSECTOR))
		   sprintf(buffer,
			"%6ld  %6ld     %6ld-%-6ld   %6ld",
			b->cyl,b->head,MinBfi(b->sector),MaxBfi(b->sector),
			b->sector);
		else
		   sprintf(buffer,
			"%6ld  %6ld       %6ld        %6ld",
			b->cyl,b->head,b->bfi,b->sector);

		if (node = (struct Node *)
			AllocRemember(&RKey,sizeof(struct Node),MEMF_CLEAR))
		{
			if(node->ln_Name =
				AllocRemember(&RKey,BADLIST_LENGTH + 1,MEMF_CLEAR))
			{
			    strcpy(node->ln_Name,buffer);
			    AddTail(&lh10,node);
			}
		}
	}

}

void
HandleBadClick (w,msg)
	register struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct Bad *b;

	y = msg->Code;

	// Remember the highlight position for CreateGadget().
	BadListPosition = y;

	for (b = FirstBad; b && y; b = b->Next, y--)
		; /* NULL */

	if (b) {
		if (CurrBad == b)
			return;
		CurrBad = b;
	} else
		DisplayBeep(w->WScreen);
}

#if 0
void
HandleBadClick (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct Bad *b;

	y = (msg->MouseY - BadList.TopEdge) / (w->RPort->TxHeight + 1);

	for (b = DisplayBad; b && y; b = b->Next, y--)
		; /* NULL */

	if (b) {
		if (CurrBad == b)
			return;
		CurrBad = b;

		BadDraw(w->RPort);
		if (gotit)
			MyOnGadget(w,BadDelete);	//

	} else
		DisplayBeep(w->WScreen);
}
#endif

#if 0
void
HandleBadList (w,id)
	struct Window *w;
	UWORD id;
{
	register struct Bad *b;
	register struct Bad *old = DisplayBad;
	register ULONG i,j;

	if (NumBad <= MaxDisplayBad || !old)
		return;

	switch (id) {
	case BADUP:
		if (FirstBad != DisplayBad)
		{
			for (b = FirstBad;
			     b && b->Next != old;
			     b = b->Next)
				; /* NULL */

			DisplayBad = b;
			DisplayBadNum--;
		}
		break;

	case BADDOWN:
		if (old->Next && DisplayBadNum < NumBad-MaxDisplayBad) {
			DisplayBad = old->Next;
			DisplayBadNum++;
		}
		break;

	case BADDRAG:
		i = ((LONG) BadDragSInfo.VertPot + 1L) *
		    max(((LONG) NumBad - (LONG) MaxDisplayBad),0);
		i >>= 16;

		for (b = FirstBad, j = 0;
		     b->Next && j < i;
		     b = b->Next, j++)
			; /* NULL */

		DisplayBad = b;
		DisplayBadNum = j;
		break;
	}

	if (old != DisplayBad)
	{
		BadDraw(w->RPort);

		/* if you can't see it, deselect it. */
		if (!gotit)
		{
			CurrBad = NULL;
			MyOffGadget(w,BadDelete);	//
		}
	}
}
#endif

void
FreeBad (b)
	register struct Bad *b;
{
	register struct Bad *nextb;

	for(; b; b = nextb)
	{
		nextb = b->Next;
		FreeMem((char *) b, sizeof(*b));
	}
}

/* convert a list of struct Bad into struct BadBlockBlock's */
/* called from ReassignAll as well!!!! */


// Bug fixed !
// If bbb_SummedLongs < SelectedDrive->rdb->rdb_BlockBytes,
// the memory for the BadBlockBlock will be never freed perfectly.
// Try to add bad block, select OK, and exit, you can see losing memory.

#if 0
struct BadBlockBlock *
MakeBadBlockList (b)
	register struct Bad *b;
{
	struct BadBlockBlock *firstbb = NULL,*oldbb = NULL;
	register struct BadBlockBlock *bb;
	register int i;
	while (b)
	{
		if (!firstbb)	{
			/* first bad block */
			firstbb = bb = (struct BadBlockBlock *)
			       AllocMem(SelectedDrive->rdb->rdb_BlockBytes,
					MEMF_CLEAR);
			if (!firstbb)
				return NULL;

			bb->bbb_ID          = IDNAME_BADBLOCK;
			bb->bbb_SummedLongs = 6;
			bb->bbb_HostID	    = HOST_ID; /* FIX! */
			bb->bbb_Next	    = NULL;
		}
		if (bb->bbb_SummedLongs >= SelectedDrive->rdb->rdb_BlockBytes/4)
		{
			/* allocate new bad block */
			oldbb = bb;
			bb = (struct BadBlockBlock *)
			     AllocMem(SelectedDrive->rdb->rdb_BlockBytes,
					MEMF_CLEAR);
			if (!bb)
				return (firstbb);

			oldbb->bbb_Next = bb;

			bb->bbb_ID          = IDNAME_BADBLOCK;
			bb->bbb_SummedLongs = 6;
			bb->bbb_HostID	    = HOST_ID; /* FIX! */
			bb->bbb_Next	    = NULL;
		}

		i = ((bb->bbb_SummedLongs - 6) * 4) /
		    sizeof(struct BadBlockEntry);
		bb->bbb_BlockPairs[i].bbe_BadBlock  = b->block;
		bb->bbb_BlockPairs[i].bbe_GoodBlock = b->replacement;
		bb->bbb_SummedLongs += sizeof(struct BadBlockEntry) >> 2;
/*printf("Adding bbe #%d, block %d, replacement %d\n",i,b->block,b->replacement);*/

		b = b->Next;
	}

	return firstbb;
}
#endif


struct BadBlockBlock *
MakeBadBlockList (b)
	register struct Bad *b;
{
	struct BadBlockBlock *firstbb = NULL,*oldbb = NULL;
	struct BadBlockBlock *firstbb_tmp = NULL,*bbb = NULL;
	register struct BadBlockBlock *bb;
	register int i;

	while (b)
	{
		if (!firstbb_tmp)
		{
			/* first bad block */
			firstbb_tmp = bb = (struct BadBlockBlock *)
			       AllocMem(SelectedDrive->rdb->rdb_BlockBytes,
					MEMF_CLEAR);
//sprintf(tmp,"%x:%d bytes alloc",bb,SelectedDrive->rdb->rdb_BlockBytes);
//kprintf("first: %s\n",tmp);
			if (!firstbb_tmp)
				return NULL;

			bb->bbb_ID          = IDNAME_BADBLOCK;
			bb->bbb_SummedLongs = 6;
			bb->bbb_HostID	    = HOST_ID; /* FIX! */
			bb->bbb_Next	    = NULL;
		}
		if (bb->bbb_SummedLongs >= SelectedDrive->rdb->rdb_BlockBytes/4)
		{
			if (!firstbb)	// back up first one
				firstbb = firstbb_tmp;

			/* allocate new bad block */
			oldbb = bb;	// back up before one
			bb = (struct BadBlockBlock *)
			     AllocMem(SelectedDrive->rdb->rdb_BlockBytes,
					MEMF_CLEAR);
//sprintf(tmp,"%x:%d bytes alloc",bb,SelectedDrive->rdb->rdb_BlockBytes);
//kprintf("new: %s\n",tmp);
			if (!bb)
				return (firstbb);

			oldbb->bbb_Next = bb;

			bb->bbb_ID          = IDNAME_BADBLOCK;
			bb->bbb_SummedLongs = 6;
			bb->bbb_HostID	    = HOST_ID; /* FIX! */
			bb->bbb_Next	    = NULL;
		}

		i = ((bb->bbb_SummedLongs - 6) * 4) /
		    sizeof(struct BadBlockEntry);
		bb->bbb_BlockPairs[i].bbe_BadBlock  = b->block;
		bb->bbb_BlockPairs[i].bbe_GoodBlock = b->replacement;
		bb->bbb_SummedLongs += sizeof(struct BadBlockEntry) >> 2;

/*printf("Adding bbe #%d, block %d, replacement %d\n",i,b->block,b->replacement);*/

		b = b->Next;
	}

	if (!firstbb)
	{
//sprintf(tmp,"%d",firstbb_tmp->bbb_SummedLongs);
//kprintf("SummedLongs:%s\n",tmp);
		firstbb = (struct BadBlockBlock *)
		       AllocMem(firstbb_tmp->bbb_SummedLongs * 4,MEMF_CLEAR);

		memcpy(firstbb,firstbb_tmp,firstbb_tmp->bbb_SummedLongs * 4);
		if (firstbb_tmp)
			FreeMem(firstbb_tmp,SelectedDrive->rdb->rdb_BlockBytes);
	}
	else if	(bb->bbb_SummedLongs < SelectedDrive->rdb->rdb_BlockBytes/4)
	{
		bbb = (struct BadBlockBlock *)
		       AllocMem(bb->bbb_SummedLongs * 4,MEMF_CLEAR);

		memcpy(bbb,bb,bb->bbb_SummedLongs * 4);
		if(bb)
			FreeMem(bb,SelectedDrive->rdb->rdb_BlockBytes);

		oldbb->bbb_Next = bbb;
	}

	return firstbb;
}

/* Copy a struct Bad, NOT the whole list */

struct Bad *
CopyBad (b)
	struct Bad *b;
{
	register struct Bad *newb;

	if (newb = AllocNew(Bad))
	{
		*newb = *b;
		newb->Next = NULL;
	}

	return newb;
}

/* find a free empty rigid disk block */

ULONG
GetReplacement (rdb,bad)
		 struct RigidDiskBlock *rdb;
		 struct Bad *bad;
{
	register LONG replacement;
	register struct Bad *b;

	for (replacement = rdb->rdb_RDBBlocksHi;
	     replacement > 3;	/* arbitrary */
	     replacement--)
	{
	    b = bad;
	    while (b)
	    {
		if (b->replacement == replacement)
			goto next_replacement;

		b = b->Next;
	    }
	    /* replacement block not used!!!! */
	    return (ULONG)replacement;

next_replacement:
	    ;
	}

	/* failed - almost impossible */
	return 0;
}

/* check if a block is already mapped out - if so, return TRUE */

int
isbad (d,block)
	struct Drive *d;
	LONG block;
{
	register struct BadBlockBlock *bb;
	register int j;

	for (bb = d->rdb->rdb_BadBlockList;
	     bb;
	     bb = bb->bbb_Next)
	{
		for (j = 0;
		     j < ((bb->bbb_SummedLongs - 6) * 4) /
			 sizeof(struct BadBlockEntry);
		     j++)
		{
			if (bb->bbb_BlockPairs[j].bbe_BadBlock == block)
				return TRUE;
		}
	}

	return FALSE;
}

/* read the defect list from the drive */
extern struct SCSICmd cmdblk;

LONG
ReadDefects (ior)
	struct IOStdReq *ior;
{
	unsigned char comm[10];
	unsigned char *data = NULL;
	LONG buffsize = 512,defect_length,size = 0;


	/* set up for SCSI commands */
	/* memory must be clear because of STUPID scsi disks that screw */
	/* up the data phase - scsi_actual says they xfered, but they didn't */

	if (!(data = AllocMem(buffsize,MEMF_PUBLIC|MEMF_CLEAR)))
		goto cleanup;

	/* try to read the defect list */
	if (DoReadDefect(ior,comm,data,buffsize))
		goto cleanup;

	defect_length = (data[2] << 8) | data[3];

	if (defect_length+4 > cmdblk.scsi_Actual)
	{
		FreeMem(data,buffsize);

		buffsize = defect_length + 8;	/* try again */
		if (!(data = AllocMem(buffsize,MEMF_PUBLIC|MEMF_CLEAR)))
			goto cleanup;

		if (DoReadDefect(ior,comm,data,buffsize))
			goto cleanup;

		defect_length = (data[2] << 8) | data[3];

		if (defect_length+4 > cmdblk.scsi_Actual)
			goto cleanup;		/* very wierd... */
	}

	/* we've read it in, now decipher it */

	/* check the type of defect list returned.. */
	/* %0xx = logical block format */
	/* %100 = BFI */
	/* %101 = physical sector format(?) */

	/* run the lists backwards because of the way MakeBadSCSI adds them */
	if (data[1] & 4)
	{
		/* must be BFI or physical sector */
		if ((data[1] & 3) == 0)
		{
			/* BFI */
			size = defect_length/8;

		} else if ((data[1] & 3) == 1) {
			/* physical sector */
			size = defect_length/4;

		} else {
			/* unknown!!! */
			goto cleanup;
		}
	} else {
		/* must be logical blocks (I like this) */
		size = defect_length/4;
	}

cleanup:
	if (data)
		FreeMem(data,buffsize);

	return size;
}

LONG
DoReadDefect (ior,comm,data,buffsize)
	struct IOStdReq *ior;
	char *comm;
	char *data;
	LONG buffsize;
{
	LONG error;

	comm[0] = S_READ_DEFECT_DATA;
	comm[1] = 0;
	comm[2] = 0x08 | 0x0; /* grown defects only, logical block format */
	comm[3] = 0;
	comm[4] = 0;
	comm[5] = 0;
	comm[6] = 0;
	comm[7] = (buffsize & 0xff00) >> 8;
	comm[8] = (buffsize & 0x00ff);
	comm[9] = 0;

	error = DoSCSI(ior,(UWORD *) comm,10,(UWORD *) data,buffsize,
		       SCSIF_READ|SCSIF_AUTOSENSE);

	/* if it wont give it to me in logical block format, try phys and BFI */
	if (error)
	{
		comm[2] = 0x08 | 0x05;		 /* try physical sector next */
		error = DoSCSI(ior,(UWORD *) comm,10,(UWORD *) data,buffsize,
			       SCSIF_READ|SCSIF_AUTOSENSE);

		if (error)
		{
			comm[2] = 0x08 | 0x04;	 	     /* try BFI last */
			error = DoSCSI(ior,(UWORD *) comm,10,(UWORD *) data,
				       buffsize,
				       SCSIF_READ|SCSIF_AUTOSENSE);
		}
	}

	return error;
}

/* reassign all bad blocks I can from BadBlockList */

void
ReassignAll (d,ior)
	struct Drive *d;
	struct IOStdReq *ior;
{
	struct BadBlockBlock *bb;
	struct Bad *b,*b_list = NULL,*b_last = NULL;
	char read[6];
	char *data;
	LONG j,good,bad;

	if (!(d->Flags & SCSI))
		return;

	if (!(data = AllocMem(d->rdb->rdb_BlockBytes,MEMF_PUBLIC)))
		return;

	for (bb = d->rdb->rdb_BadBlockList; bb; bb = bb->bbb_Next)
	{
		for (j = 0;
		     j < ((bb->bbb_SummedLongs - 6) * 4) /
			 sizeof(struct BadBlockEntry);
		     j++)
		{
			good = bb->bbb_BlockPairs[j].bbe_GoodBlock;
			bad  = bb->bbb_BlockPairs[j].bbe_BadBlock;
			if (good > 0)
			{
				/* read in any data from replacement */
				read[0] = S_READ;
				read[1] = (good & 0x001f0000) >> 16;
				read[2] = (good & 0x0000ff00) >> 8;
				read[3] = (good & 0x000000ff);
				read[4] = 1;
				read[5] = 0;

				DoSCSI (ior,(UWORD *) read, 6, (UWORD *) data,
					d->rdb->rdb_BlockBytes,
					SCSIF_READ|SCSIF_AUTOSENSE);
			}
			if (ReassignBlock(ior,bad))
			{
				d->SCSIBadBlocks++;

				/* try to preserve data in the mapped sector */
				read[0] = S_WRITE;
				read[1] = (bad & 0x001f0000) >> 16;
				read[2] = (bad & 0x0000ff00) >> 8;
				read[3] = (bad & 0x000000ff);
				read[4] = 1;
				read[5] = 0;

				DoSCSI (ior,(UWORD *) read, 6, (UWORD *) data,
					d->rdb->rdb_BlockBytes,
					SCSIF_WRITE|SCSIF_AUTOSENSE);

			} else {

				/* can't map it out! */
				/* add to list to be passed to MakeBBL */
				b = AllocNew(Bad);
				if (b)
				{
					b->block       = bad;
					b->replacement = good;
					b->Next        = NULL;

					if (!b_last)
					{
						b_list = b;
						b_last = b;
					} else {
						b_last->Next = b;
                                                b_last = b;
					}
				}
			}
		}
	}

	/* safe to call on NULL */
	FreeBadBlockList(d->rdb->rdb_BadBlockList);
	FreeBad(d->bad);

	d->bad = NULL;
	d->rdb->rdb_BadBlockList = NULL;

	if (b_list)
	{
		d->rdb->rdb_BadBlockList = MakeBadBlockList(b_list);
		FreeBad(b_list);
	}

	FreeMem(data,d->rdb->rdb_BlockBytes);
}
