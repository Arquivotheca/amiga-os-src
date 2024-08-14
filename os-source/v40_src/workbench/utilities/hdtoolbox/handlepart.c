/* handlepart.c - code to handle the partitioning screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ctype.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"
#include "PartGlob.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

#include "fstype.h"

#include <clib/alib_protos.h>

//
extern UWORD XtraTop;
extern struct Gadget *glist2;
extern struct Gadget *glist3;
extern struct Gadget *glist4;

USHORT BootableFlags = 0;
USHORT BootPriFlags = 0;
USHORT AdvancedFlags = 0;

char FileSysText[18];

/*
  These LongInt value is not just for newgads.c like the other files.
  This window has Advanced options, so that I have to keep this value.
*/
LONG StartCylSInfoLongInt = 0;
LONG EndCylSInfoLongInt   = 0;
LONG TotalCylSInfoLongInt = 0;
LONG BuffersSInfoLongInt  = 0;
LONG BootPriSInfoLongInt  = 0;
LONG HostIDSInfoLongInt   = 0;	/* 39.13 */

extern struct GadToolsBase *GadToolsBase;
//

#define SAME	0

int ForceSize = 0;

int State = 0;
#define INNEW		1

#define PARTLE	14L			/* partition left edge */
#define PARTRE	626L			/* partition right edge */

#define PixelCyl(cyl)	((((cyl) * (PARTRE-PARTLE))/Cylinders)+PARTLE)
#define CylPixel(x)	((((x)-PARTLE) * Cylinders)/(PARTRE-PARTLE))

#define EndCylRefresh(w)   FixString((w),EndCyl)
#define StartCylRefresh(w) FixString((w),StartCyl)
#define TotalCylRefresh(w) FixString((w),TotalCyl)
#define BootPriRefresh(w)  FixString((w),BootPri)
#define HostIDRefresh(w)   FixString((w),HostID)
#define BufferRefresh(w)   FixString((w),Buffers)


char *PartHelp[] = {
"The large rectangle represents your drive.  Each partition is shown",
"according to the key above the rectangle.  To change which partition is",
"the current partition, click on a partition.  To move the current partition,",
"click on it (holding the button down) and drag it.  It will also move if you",
"click in unused area on either side of it.",
" ",
"To change the size of the current partition, use the pointer under its right",
"edge.  Dragging the pointer left and right will change the size of the",
"partition.",
" ",
"To create a new partition, select \"New Partition\", then click in an unused",
"area of the disk.  \"Delete Partition\" will delete the current partition.",
" ",
"Select \"Ok\" to go back and keep any changes you made, or \"Cancel\" if you",
"don't want to keep them.",
NULL,
};


/* Lattice's stcl_d() function appearently does not work on negative
   numbers like it should, so we need to make it work like it should */

void STCL_d(char *Buff, long i)
{
	if (i < 0L)
	{
        	*Buff++ = '-';
        	i = -i;
        }

        stcl_d(Buff,i);
}

// Special refresh rtn for advanced option

void
do_refreshPart (w)
	register struct Window *w;
{
	register struct RastPort *rp;

	rp = w->RPort;

	/* clear everything out */
	SetAPen(rp,0L);
	SetDrMd(rp,JAM1);
	SetAfPt(rp,NULL,0);

	/* Minimize rendering for clear */
	RectFill(rp,StartCyl->LeftEdge - 100,
			StartCyl->TopEdge - 4,
			Buffers->LeftEdge + Buffers->Width + 6,
			Buffers->TopEdge + Buffers->Height + 4);

	RectFill(rp,TotalCyl->LeftEdge + TotalCyl->Width + 4,
			BootPri->TopEdge - 4,
			BootPri->LeftEdge + BootPri->Width + 6,
			BootPri->TopEdge + BootPri->Height + 4);
// 39.13
	RectFill(rp,TotalCyl->LeftEdge + TotalCyl->Width + 4,
			HostID->TopEdge - 4,
			HostID->LeftEdge + HostID->Width + 6,
			HostID->TopEdge + HostID->Height + 4);

	RectFill(rp,ChangeFileSys->LeftEdge,
			ChangeFileSys->TopEdge,
			AddFileSys->LeftEdge + AddFileSys->Width,
			AddFileSys->TopEdge + AddFileSys->Height);
}

int
HandlePart (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct Gadget *gad;
	register int done = FALSE;
	UWORD id;
	register ULONG x;
//	static UWORD dragpos,newpos;

	if (State == INNEW)
	{
		/* fix NewPart */
//		SetAPen(w->RPort,0L);
//		RectFill(w->RPort,NewPart->LeftEdge,NewPart->TopEdge,
//			 NewPart->LeftEdge + NewPart->Width  - 1,
//			 NewPart->TopEdge  + NewPart->Height - 1);

		//AddGList(w,NewPart,newpos,1L,NULL);   /* bring it back */
		//AddGList(w,&DragPart,dragpos,1L,NULL); /* bring it back */

		/* always leave state */
		State = 0;
		SetWindowTitles(w,"Partitioning",(char *) -1);

		/* make new partition if in right area */
		if (!MakeNewPart(w,msg))
			Notify(w,"Click wasn't in an empty area of the disk",0);

		MyOnGadget(w,Bootable);
		MyOnGadget(w,DeletePart);
		MyOnGadget(w,NewPart);
		MyOnGadget(w,QuickSetup);
		MyOnGadget(w,Help);
		MyOnGadget(w,DonePart);
		MyOnGadget(w,CancelPart);
		MyOnGadget(w,Advanced);
		MyOnGadget(w,NamePart);
		SizePart.Flags &= ~GADGDISABLED;
		if (AdvancedFlags)
		{
			MyOnGadget(w,StartCyl);
			MyOnGadget(w,EndCyl);
			MyOnGadget(w,TotalCyl);
			MyOnGadget(w,Buffers);
			MyOnGadget(w,BootPri);
			MyOnGadget(w,ChangeFileSys);
			MyOnGadget(w,AddFileSys);
			MyOnGadget(w,HostID);		/* 39.13 */
		}
		RefreshGList(NewPart,w,NULL,1L);
		RefreshGList(&SizePart,w,NULL,1L);

		if (msg->Class == MOUSEBUTTONS)
			return FALSE;

		/* fall thru - handle other gadgets if clicked on -not likely */
	}

	switch (msg->Class) {
	case GADGETUP:
	case GADGETDOWN:
		gad = (struct Gadget *) (msg->IAddress);

		id  = gad->GadgetID & ~REFRESH_MASK;

		switch (id) {
		case SIZEPART:
			HandleSize(w);
			break;

		case DRAGPART:
			HandleDrag(w);
			break;

		case NEWPART:
			SetWindowTitles(w,"Select empty area for new partition",
					(char *) -1);

			/* so we can click in it */
			//dragpos = RemoveGList(w,&DragPart,1L);
			//newpos  = RemoveGList(w,NewPart,1L);
			MyOffGadget(w,Bootable);
			MyOffGadget(w,DeletePart);
			MyOffGadget(w,NewPart);
			MyOffGadget(w,QuickSetup);
			MyOffGadget(w,Help);
			MyOffGadget(w,DonePart);
			MyOffGadget(w,CancelPart);
			MyOffGadget(w,Advanced);
			MyOffGadget(w,NamePart);
			SizePart.Flags |= GADGDISABLED;
			if (AdvancedFlags)
			{
				MyOffGadget(w,StartCyl);
				MyOffGadget(w,EndCyl);
				MyOffGadget(w,TotalCyl);
				MyOffGadget(w,Buffers);
				MyOffGadget(w,BootPri);
				MyOffGadget(w,ChangeFileSys);
				MyOffGadget(w,AddFileSys);
				MyOffGadget(w,HostID);		/* 39.13 */
			}

			/* Highlight NewPart */
			SetAPen(w->RPort,3L);
			RectFill(w->RPort,NewPart->LeftEdge + 2,NewPart->TopEdge + 1,
				 NewPart->LeftEdge + NewPart->Width  - 3,
				 NewPart->TopEdge  + NewPart->Height - 2);
			PrintIText(w->RPort,NewPart->GadgetText,
				   NewPart->LeftEdge,NewPart->TopEdge);
			SetAPen(w->RPort,2L);
			Move(w->RPort,NewPart->LeftEdge + NewPart->Width - 2,
					NewPart->TopEdge);
			Draw(w->RPort,NewPart->LeftEdge + 1,
					NewPart->TopEdge);
			Draw(w->RPort,NewPart->LeftEdge + 1,
					NewPart->TopEdge + NewPart->Height - 2);
			State = INNEW;
			break;

		case DELETEPART:
			DeleteCurrentPart(w);
			break;

		case QUICKSETUP:
			DoQuickSetup(SelectedDrive);
			PartUpdate |= UPDATE;

			CurrentPart = SelectedDrive->rdb->rdb_PartitionList;

			/* force it to rebuild the size gadget, plus select */
			ForceSize = TRUE;
			SelectPartition(w);
			ForceSize = FALSE;

			break;

		case HELP:
			LongNotify(w,PartHelp);
			break;

		case DONEPART:
			/* Make sure to pick up any changed partition name */
			HandleName(w);

			FreePartitionList(save_part);

			if (PartUpdate)
				SelectedDrive->Flags |= UPDATE;

			done = HANDLE_DONE;
			break;

		case CANCELPART:
			{
			    struct PartitionBlock *temp;

			    temp = SelectedDrive->rdb->rdb_PartitionList;
			    SelectedDrive->rdb->rdb_PartitionList = save_part;
			    FreePartitionList(temp);
			}

			done = HANDLE_CANCEL;
			break;

		case CHANGEFILESYS:
			done = HandleWindowNew(w,NULL,
					&glist3,
					NewWindowStructure3.Title,
					NewWindowStructure3.IDCMPFlags,	// 39.10
					HandleFileSys,
					NULL,
					FSInitialize,
					CreateAllGadgets3,
					&glist2,CreateAllGadgets2);
			RefreshBootable(w);	/* force refresh of bootable */
			SetFileSysName(w);
			break;

		case ADDFILESYS:
			done = HandleWindowNew(w,NULL,
					&glist4,
					NewWindowStructure4.Title,
					NewWindowStructure4.IDCMPFlags,	//39.10
					HandleFSM,
					NULL,//redraw_fsmlist,
					FSMInit,
					CreateAllGadgets4,
					&glist2,CreateAllGadgets2);
			break;

		case STARTCYL:
//			x = min(StartCylSInfo.LongInt,
//				CurrentPart->pb_Environment[DE_UPPERCYL]-1);
			StartCylSInfoLongInt
				 = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			x = min(StartCylSInfoLongInt,
				CurrentPart->pb_Environment[DE_UPPERCYL]-1);
			x = max(x,PrevPMax + 1);

//			if (x != StartCylSInfo.LongInt)	/* out of range? */
			if (x != StartCylSInfoLongInt)
			{
				DisplayBeep(w->WScreen);
				ReCalcStartCyl(w);
			}
			else if (x != CurrentPart->pb_Environment[DE_LOWCYL])
			{
				CurrentPart->pb_Environment[DE_LOWCYL] = x;
				ReCalcTotalCyl(w);
				PropRefresh(w);
				SizeRefresh(w);
				PartUpdate |= UPDATE;
			}
			break;

		case ENDCYL:
//			x = max(EndCylSInfo.LongInt,
//				CurrentPart->pb_Environment[DE_LOWCYL]+1);
			EndCylSInfoLongInt
				= ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			x = max(EndCylSInfoLongInt,
				CurrentPart->pb_Environment[DE_LOWCYL]+1);
			x = min(x,NextPMin - 1);

//			if (x != EndCylSInfo.LongInt)	/* out of range? */
			if (x != EndCylSInfoLongInt)
			{
				DisplayBeep(w->WScreen);
				ReCalcEndCyl(w);
			}
			else if (x != CurrentPart->pb_Environment[DE_UPPERCYL])
			{
				CurrentPart->pb_Environment[DE_UPPERCYL] = x;
				ReCalcTotalCyl(w);
				PropRefresh(w);
				SizeRefresh(w);
				PartUpdate |= UPDATE;
			}
			break;

		case TOTALCYL:
//			x = max(TotalCylSInfo.LongInt +
//				CurrentPart->pb_Environment[DE_LOWCYL]-1,
//				CurrentPart->pb_Environment[DE_LOWCYL]+1);
			TotalCylSInfoLongInt
				= ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			x = max(TotalCylSInfoLongInt +
				CurrentPart->pb_Environment[DE_LOWCYL]-1,
				CurrentPart->pb_Environment[DE_LOWCYL]+1);
			x = min(x,NextPMin - 1);

//			if (x != TotalCylSInfo.LongInt +
//				 CurrentPart->pb_Environment[DE_LOWCYL]-1)
							/* out of range? */
			if (x != TotalCylSInfoLongInt +
				 CurrentPart->pb_Environment[DE_LOWCYL]-1)
			{
				DisplayBeep(w->WScreen);
				ReCalcTotalCyl(w);
			}
			else if (x != CurrentPart->pb_Environment[DE_UPPERCYL])
			{
				CurrentPart->pb_Environment[DE_UPPERCYL] = x;
				ReCalcEndCyl(w);	/* not total! */
				PropRefresh(w);
				SizeRefresh(w);
				PartUpdate |= UPDATE;
			}
			break;

		case BUFFERS:
//			if (BuffersSInfo.LongInt !=
//			    CurrentPart->pb_Environment[DE_NUMBUFFERS])
			BuffersSInfoLongInt
				= ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			if (BuffersSInfoLongInt !=
			    CurrentPart->pb_Environment[DE_NUMBUFFERS])
			{
				CurrentPart->pb_Environment[DE_NUMBUFFERS] =
							   BuffersSInfoLongInt;
				PartUpdate |= UPDATE;
			}
			break;

		case NAMEPART:
			HandleName(w);
			break;

		case BOOTABLE:
//			if (Bootable->Flags & SELECTED)
			if (gad->Flags & SELECTED)
			{
				CurrentPart->pb_Flags |= PBFF_BOOTABLE;
				if (AdvancedFlags & SELECTED)
					MyOnGadget(w,BootPri);	//
				BootableFlags |= SELECTED;	//
				BootPriFlags &= ~GADGDISABLED;	//
			} else {
				CurrentPart->pb_Flags &= ~PBFF_BOOTABLE;
				if (AdvancedFlags & SELECTED)
					MyOffGadget(w,BootPri);	//
				BootableFlags &= ~SELECTED;	//
				BootPriFlags |= GADGDISABLED;	//
			}

			PartUpdate |= UPDATE;
			break;

		case BOOTPRI:
//			if (BootPriSInfo.LongInt !=
//				CurrentPart->pb_Environment[DE_BOOTPRI])
			BootPriSInfoLongInt
				= ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			if (BootPriSInfoLongInt !=
				CurrentPart->pb_Environment[DE_BOOTPRI])
			{
				CurrentPart->pb_Environment[DE_BOOTPRI] =
				  min(127,max(-128,BootPriSInfoLongInt));
				PartUpdate |= UPDATE;
			}
			break;
/* 39.13 */
		case HOSTID:
			HostIDSInfoLongInt = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			if (HostIDSInfoLongInt < 0 || HostIDSInfoLongInt > 7)
			{
				DisplayBeep(w->WScreen);
				ReCalcHostID(w);
			}
			else
			{
				if (HostIDSInfoLongInt !=
					CurrentPart->pb_HostID)
				{
					CurrentPart->pb_HostID = HostIDSInfoLongInt;
					PartUpdate |= UPDATE;
				}
			}
			break;

		case ADVANCED:
			/* turn on/off the "advanced" gadgets */
			/* these are HostID,bootpri, FSM, FS, */
			/* and the cylinder gadgets */
			/* NamePart is the last in the list that */
			/* stays */

			AdvancedFlags = (gad->Flags & SELECTED);
// Remove Old Gadgtes and recreate new gadgets
			{
			    struct Gadget *FirstGad;

			    // Minimize Rendering...
			    if(!AdvancedFlags) do_refreshPart(w);

			    FirstGad = w->FirstGadget;
			    while ((FirstGad) && (FirstGad->GadgetType & SYSGADGET))
				FirstGad = FirstGad->NextGadget;
			    if (FirstGad)
			    {
				RemoveGList(w,FirstGad,-1L);
				FreeGadgets(FirstGad);
				glist2 = NULL;
			    }

			    // I have to clean up these value, because
			    // the FreeGadget() does not clean gadget pointer.
			    // See RefreshBootable().
			    Bootable = 0;
			    BootPri = 0;
			    StartCyl = 0;
			    EndCyl = 0;
			    TotalCyl = 0;
			    Buffers = 0;
			    FileSys = 0;	// 39.8
                            HostID = 0;		/* 39.13 */

			    if (glist2 == NULL)
			    {
				CreateAllGadgets2();
				AddGList(w,glist2,-1,-1,NULL);
			    }
			}
//
			if (AdvancedFlags & SELECTED)
			{
				/* add to end of list */
//				AddGList(w,&StartCyl,0xffff,-1,NULL);
//				BootableIText.NextText = &StartCylIText;
				ReCalcEndCyl(w);
				ReCalcStartCyl(w);
				ReCalcTotalCyl(w);
				ReCalcBootPri(w);
				ReCalcBuffers(w);
				ReCalcHostID(w);	/* 39.13 */
			} else {
//				RemoveGList(w,StartCyl,-1L);
//				BootableIText.NextText = NULL;
			}

			/* redraw entire window to fix gadgets */
//			This is just too much rendering...
//			do_refresh(w,&IntuiTextList2,PartDraw);
			RefreshGList(glist2,w,NULL,-1L);

			break;

		}
		break;

	case RAWKEY:
		/* use '+' and '-' to go up/down 1 partition */
		switch (msg->Code) {

		case 0x4e:	/* cursor forward */

			/* go up one partition */
			if (CurrentPart->pb_Next)
			{
				HandleName(w);	/* make sure it's up to date */

				CurrentPart = CurrentPart->pb_Next;
				SelectPartition(w);
			}
			break;

		case 0x4f:	/* cursor backward */

			/* go down one partition */
			{
				struct PartitionBlock *p;

				for (p = SelectedDrive->rdb->rdb_PartitionList;
	 			     p->pb_Next;
				     p = p->pb_Next)
				{
					if (p->pb_Next == CurrentPart)
					{
						/* make sure it's up to date */
						HandleName(w);

						CurrentPart = p;
						SelectPartition(w);
						break;
					}
				}
			}
			break;
		case 0x5f:	/* 39.13 - help */
			LongNotify(w,PartHelp);
			break;
		}
		break;
//
//	case MENUPICK:			// I don't want to use menu now.
//		HandleMenus(w,msg);
//		break;

	case MOUSEBUTTONS:
		/* Is it in the partition area?? */
		(void) HandlePartClick(w,msg);
		break;

	}

	return done;
}

void
HandleName (w)
	struct Window *w;
{
	register int len,i;

	/* if it has ':'s or '/'s in it, remove them */

	len = strlen(&NamePartSIBuff[0]);
	for (i = 0; i < len; i++)
	{
		if (NamePartSIBuff[i] == ':' || NamePartSIBuff[i] == '/')
		{
			strcpy(&NamePartSIBuff[i],&NamePartSIBuff[i+1]);
			i--;	/* don't skip next character */
		}
	}

	if (strcmp(&CurrentPart->pb_DriveName[0],&NamePartSIBuff[0]) != SAME)
	{
		if (!len)
		{
			DisplayBeep(w->WScreen);
			strcpy(&NamePartSIBuff[0],
			       &CurrentPart->pb_DriveName[0]);
		} else {
			strcpy(&CurrentPart->pb_DriveName[0],
			       &NamePartSIBuff[0]);

			PartUpdate |= UPDATE;
		}
	}
	RefreshGList(NamePart,w,NULL,1L);
}

/* Delete the Current Partition */

void
DeleteCurrentPart (w)
	struct Window *w;
{
	register struct PartitionBlock *p,*lastp;

	/* ask if he REALLY wants to delete the partition */
	/* if not, return */

	/* he wants to delete it */

	lastp = PrevPart;
	p     = CurrentPart;

	if (lastp == NULL && p->pb_Next == NULL)
	{
		/* only one partition, don't delete it */
		DisplayBeep(w->WScreen);
		return;
	}

	/* kill old one */
	if (lastp)
		lastp->pb_Next = p->pb_Next;
	else
		SelectedDrive->rdb->rdb_PartitionList = p->pb_Next;

	FreeMem((char *) p, sizeof(*p));

	/* redo display */
	CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
	SelectPartition(w);

	PartUpdate |= UPDATE;
}

/* make new part if click is in empty region - returns success */

int
MakeNewPart (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct PartitionBlock *p,*lastp,*newp;
	register LONG lastx,maxx,x;

	if (msg->MouseY <  DragPart.TopEdge ||
	    msg->MouseY >= DragPart.TopEdge + DragPart.Height)

		return FALSE;

	lastx = SelectedDrive->rdb->rdb_LoCylinder - 1;
	x     = CylPixel(msg->MouseX);

	lastp = NULL;

	for (p = SelectedDrive->rdb->rdb_PartitionList;
	     p && x > p->pb_Environment[DE_UPPERCYL];
	     lastp = p, p = p->pb_Next)
	{
		lastx = p->pb_Environment[DE_UPPERCYL];
	}

	maxx = p ? p->pb_Environment[DE_LOWCYL] :
		   SelectedDrive->rdb->rdb_HiCylinder + 1;

	if (x > lastx && x < maxx)
	{
		/* Got it! */
		if (newp = AllocNew(PartitionBlock))
		{
			InitPartition(SelectedDrive,newp,lastx + 1, maxx - 1);
			if (lastp)
			{
				newp->pb_Next  = lastp->pb_Next;
				lastp->pb_Next = newp;
			} else {
				newp->pb_Next  =
					SelectedDrive->rdb->rdb_PartitionList;
				SelectedDrive->rdb->rdb_PartitionList = newp;
			}
			HandleName(w);	/* make sure it's up to date */

			CurrentPart = newp;
			SelectPartition(w);

			PartUpdate |= UPDATE;
			return TRUE;
		}
	}

	return FALSE;
}

/* returns success */

int
HandlePartClick (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct PartitionBlock *p;
	register LONG x;

	if (msg->MouseY <  DragPart.TopEdge ||
	    msg->MouseY >= DragPart.TopEdge + DragPart.Height ||
	    msg->MouseX <= PARTLE ||
	    msg->MouseX >= PARTRE)

		return FALSE;

	x = CylPixel(msg->MouseX);

	for (p = SelectedDrive->rdb->rdb_PartitionList;
	     p;
	     p = p->pb_Next)
	{
		if (x >= p->pb_Environment[DE_LOWCYL] &&
		    x <= p->pb_Environment[DE_UPPERCYL])
			break;
	}
	if (p && p != CurrentPart)
	{
		HandleName(w);	/* make sure it's up to date */

		CurrentPart = p;
		SelectPartition(w);
	}

	return (p != NULL);
}

void
HandleSize (w)
	register struct Window *w;
{
	register LONG i;
	register LONG cpmin;	/* current partition min cyl+1 */

	cpmin = CurrentPart->pb_Environment[DE_LOWCYL] + 1;

	i = (SizePartSInfo.HorizPot + 1L) * (NextPMin - cpmin - 1);
	i >>= 16;

	if (cpmin + i >= NextPMin)
		return;

	if (CurrentPart->pb_Environment[DE_UPPERCYL] != cpmin + i)
	{
		CurrentPart->pb_Environment[DE_UPPERCYL] = cpmin + i;
		PartUpdate |= UPDATE;

		ReCalcEndCyl(w);
		ReCalcTotalCyl(w);
		ReCalcSizeText(w);
		PropRefresh(w);
	}
}

void
HandleDrag (w)
	register struct Window *w;
{
	register LONG i,diff;

	/* multiply by 0 to offset to first cyl (i.e. where first cyl would */
	/* be if the uppercyl was up against the next partition.)	    */

	i = (DragPartSInfo.HorizPot + 1L) * (NextPMin - PrevPMax - 1 -
	    (CurrentPart->pb_Environment[DE_UPPERCYL] -
	     CurrentPart->pb_Environment[DE_LOWCYL] + 1));
	i >>= 16;

	diff = (PrevPMax + i + 1) - CurrentPart->pb_Environment[DE_LOWCYL];

	Update(CurrentPart->pb_Environment[DE_LOWCYL], /* start cyl */
	       PrevPMax + i + 1, PartUpdate);

	CurrentPart->pb_Environment[DE_UPPERCYL] += diff;

	ReCalcEndCyl(w);
	ReCalcStartCyl(w);
	SizeRefresh(w);		/* will probably rebuild */
}

/* recalc the partition gadget and redisplay */

void
PropRefresh(w)
	register struct Window *w;
{
	register ULONG pmax,HBody,start;
	LONG freefirst,size;

	freefirst = CurrentPart->pb_Environment[DE_LOWCYL] - PrevPMax - 1;

	size  = CurrentPart->pb_Environment[DE_UPPERCYL] - 	/* max - min */
		CurrentPart->pb_Environment[DE_LOWCYL] + 1;

	pmax  = freefirst + NextPMin -
		CurrentPart->pb_Environment[DE_UPPERCYL] - 1;	/* first+last */


	HBody = (0xFFFFL * size) / (pmax + size);

	start = pmax ? (0xFFFFL * freefirst) / pmax : 0;

	if (!Init)
	{
		NewModifyProp(&DragPart,w,NULL,DragPartSInfo.Flags,
			      start,DragPartSInfo.VertPot,
			      HBody,DragPartSInfo.VertBody,1L);
		RefreshGList(&DragPart,w,NULL,1L);
	} else {
		DragPartSInfo.HorizPot  = start;
		DragPartSInfo.HorizBody = HBody;
	}
}

/* recalc the sizing gadget and redisplay */

void
SizeRefresh(w)
	register struct Window *w;
{
	register ULONG pmax,HBody,start;
	ULONG minx,size;
//	UWORD pos;

	minx = PixelCyl(CurrentPart->pb_Environment[DE_LOWCYL]+1);
	if (minx < 6)
		minx = 6;

	if (Init || ForceSize || minx-6 != SizePart.LeftEdge)
	{
//KK		if (!Init)
//			pos = RemoveGList(w,&SizePart,1L);

		/* erase old one */
		if (!Init)
		{
//			SetAPen(w->RPort,2L);
			SetAPen(w->RPort,0L);	//
			SetDrMd(w->RPort,JAM1);
			RectFill(w->RPort,SizePart.LeftEdge-2,SizePart.TopEdge,
				 SizePart.LeftEdge + SizePart.Width - 1,
				 SizePart.TopEdge  + SizePart.Height - 1);

		}

		SizePart.LeftEdge = minx - 6;
		SizePart.Width    = PixelCyl(NextPMin)-1 + 6-SizePart.LeftEdge;
		if (SizePart.Width < 12)
			SizePart.Width = 12;

		if (!Init)
		{
//KK			AddGList(w,&SizePart,pos,1L,NULL);
//			/* FIX! kludge to work with 2.0 (Beta 5) intuition */
//			/* may cause flashing */
			RefreshGList(&SizePart,w,NULL,1L);
		}
	}

	/* pmax must be at least 1 */
	pmax  = NextPMin - CurrentPart->pb_Environment[DE_LOWCYL] - 1;
	size  = CurrentPart->pb_Environment[DE_UPPERCYL] -
		CurrentPart->pb_Environment[DE_LOWCYL];

	HBody = (0xFFFFL * min(10,pmax)) / pmax;
	start = (0xFFFFL * size) / pmax;

	if (!Init)
	{
		NewModifyProp(&SizePart,w,NULL,SizePartSInfo.Flags,
			      start,SizePartSInfo.VertPot,
			      HBody,SizePartSInfo.VertBody,1L);
		RefreshGList(&SizePart,w,NULL,1L);

		/* draw new size below! */
		ReCalcSizeText(w);
	} else {
		SizePartSInfo.HorizPot  = start;
		SizePartSInfo.HorizBody = HBody;
	}

}

void
FixString (w,gad)
	struct Window *w;
	struct Gadget *gad;
{
	STCL_d(((struct StringInfo *) gad->SpecialInfo)->Buffer,
	      ((struct StringInfo *) gad->SpecialInfo)->LongInt);

	if (!Init)
		RefreshGList(gad,w,NULL,1L);
}

void
SelectPartition (w)
	struct Window *w;
{
	register struct PartitionBlock *p;
//	UWORD pos;

	p = SelectedDrive->rdb->rdb_PartitionList;
	while (p && p->pb_Next != CurrentPart)
		p = p->pb_Next;

	PrevPart = p;	/* if CurrentPart == firstpart, prevpart = NULL */

	PrevPMax = p ? p->pb_Environment[DE_UPPERCYL] :
		       SelectedDrive->rdb->rdb_LoCylinder - 1;

	NextPart = CurrentPart->pb_Next;

	NextPMin = NextPart ?
			CurrentPart->pb_Next->pb_Environment[DE_LOWCYL] :
			SelectedDrive->rdb->rdb_HiCylinder + 1;

//KK	if (!Init)
//		pos = RemoveGList(w,&DragPart,1L);

	DragPart.LeftEdge = PixelCyl(PrevPMax + 1);
	DragPart.Width    = PixelCyl(NextPMin) - 1 - DragPart.LeftEdge;

	/* clip to reasonable bounds */
	DragPart.Width = max(1,min(PARTRE-DragPart.LeftEdge,DragPart.Width));

	if (!Init) {
//KK		AddGList(w,&DragPart,pos,1L,NULL);
//		/* FIX! kludge to work with 2.0 (Beta 5) intuition */
//		/* may cause flashing */
		RefreshGList(&DragPart,w,NULL,1L);
	}

	SetFileSysName (w);

	RefreshEverything(w);
}

/* set the "file system: ssssssssss" string and print */

void
SetFileSysName (w)
	struct Window *w;
{
	register struct RastPort *rp;
	char *name;

	/* set filesystem name - all must be same length */
// 39.11
	switch (CurrentPart->pb_Environment[DE_DOSTYPE]) {
	case ID_DOS_DISK:				// DOS\0
		name = "Old File System";
		break;
	case ID_FFS_DISK:				// DOS\1
		name = "Fast File System";
		break;
	case ID_INTER_DOS_DISK:				// DOS\2
		name = "International(OFS)";
		break;
	case ID_INTER_FFS_DISK:				// DOS\3
		name = "International(FFS)";
		break;
	case ID_DC_DOS_DISK:				// DOS\4
		name = "Dir Cache (OFS)";
		break;
	case ID_DC_FFS_DISK:				// DOS\5
		name = "Dir Cache (FFS)";
		break;
	default:					// other
		if ((CurrentPart->pb_Environment[DE_DOSTYPE] & 0xFFFFFF00)
			== 'UNI\0')
			name = "UNIX File System";
		else
			name = "Custom File System";
		break;
	}
//
/*
	if (CurrentPart->pb_Environment[DE_DOSTYPE] == ID_FFS_DISK)
		name = "Fast File System";
	else if (CurrentPart->pb_Environment[DE_DOSTYPE] == ID_DOS_DISK)
		name = "Old File System ";
	else if (CurrentPart->pb_Environment[DE_DOSTYPE] == 'RESV')
		name = "Reserved        ";
	else if ((CurrentPart->pb_Environment[DE_DOSTYPE] & 0xFFFFFF00) ==
		 'UNI\0')
		name = "UNIX File System  ";
	else
		name = "Custom File System";
*/

	/* write over blanks in IText */
//	strcpy(&(FileSysIText.IText[13]),name);

	strcpy(FileSysText,name);	//

	if (FileSys)	// 39.8
		{
		rp = w->RPort;
		// Kluge! Usually GT_SetGadgetAttrs() clear old field,
		// but after click "advanced options", click another partition,
		// (call HandlePartClick() and call this routine,)
		// somehow it does not clear old string.
		SetAPen(rp,0L);
		SetDrMd(rp,JAM1);
		SetAfPt(rp,NULL,0);
		RectFill(rp,FileSys->LeftEdge,
			FileSys->TopEdge,
			FileSys->LeftEdge + 18*8,
			FileSys->TopEdge + FileSys->Height);

		//
		GT_SetGadgetAttrs(FileSys,w,NULL,GTTX_Text,name,TAG_DONE);
		}

	/* change text */
	PrintIText(w->RPort,&IntuiTextList2,0,0);

}

void
RefreshEverything (w)
	register struct Window *w;
{
	if (!Init) {
		ReCalcEndCyl(w);
		ReCalcStartCyl(w);
		ReCalcTotalCyl(w);
		ReCalcBuffers(w);
		ReCalcHostID(w);	/* 39.13 */
		DrawPartitions(w->RPort);

		PropRefresh(w);
		SizeRefresh(w);
	}

	RefreshBootable(w);

	strcpy(NamePartSIBuff,CurrentPart->pb_DriveName);
	if (!Init)
		RefreshGList(NamePart,w,NULL,1L);
}

/* do refresh of bootable and bootpri */

void
RefreshBootable (w)
	struct Window *w;
{
//	register UWORD pos;

//	if (!Init) {
//	   if (use_advanced)
//		pos = RemoveGList(w,Bootable,2L); /* bootable and BootPri */
//	   else
//		pos = RemoveGList(w,Bootable,1L); /* bootable */
//	}

//
//	if (!(CurrentPart->pb_Flags & PBFF_NOMOUNT))
//	{
//		Bootable->Flags  &= ~GADGDISABLED;
//		if (CurrentPart->pb_Flags & PBFF_BOOTABLE)
//		{
//			Bootable->Flags |= SELECTED;
//			BootPri->Flags  &= ~GADGDISABLED;
//		} else {
//			Bootable->Flags &= ~SELECTED;
//			BootPri->Flags  |= GADGDISABLED;
//		}
//	} else {
//		Bootable->Flags  &= ~SELECTED;
//		Bootable->Flags  |= GADGDISABLED;
//		BootPri->Flags   |= GADGDISABLED;
//		CurrentPart->pb_Flags &= ~PBFF_BOOTABLE;
//	}
//
	// These part is for gadtools type gadgets in place above.
	if (!(CurrentPart->pb_Flags & PBFF_NOMOUNT))
	{
		if (Bootable) MyOnGadget(w,Bootable);
		BootableFlags &= ~GADGDISABLED;
		if (CurrentPart->pb_Flags & PBFF_BOOTABLE)
		{
			if (Bootable) GT_SetGadgetAttrs(Bootable,w,NULL,
						GTCB_Checked,TRUE,TAG_DONE);
			BootableFlags |= SELECTED;

			if (BootPri)  MyOnGadget(w,BootPri);
			BootPriFlags &= ~GADGDISABLED;
		} else {
			if (Bootable) GT_SetGadgetAttrs(Bootable,w,NULL,
						GTCB_Checked,FALSE,TAG_DONE);
			BootableFlags &= ~SELECTED;
			if (BootPri)  MyOffGadget(w,BootPri);
			BootPriFlags |= GADGDISABLED;
		}
	} else {
		if (Bootable)
		{
			GT_SetGadgetAttrs(Bootable,w,NULL,
					GTCB_Checked,FALSE,TAG_DONE);
			MyOffGadget(w,Bootable);
		}
		BootableFlags &= ~SELECTED;
		BootableFlags |= GADGDISABLED;

		if (BootPri) MyOffGadget(w,BootPri);
		BootPriFlags |= GADGDISABLED;

		CurrentPart->pb_Flags &= ~PBFF_BOOTABLE;
	}

	if (!Init)
	{
		if (AdvancedFlags & SELECTED)
//		if (use_advanced)
//		{
//			AddGList(w,Bootable,pos,2L,NULL);
			ReCalcBootPri(w);
//		} else
//			AddGList(w,Bootable,pos,1L,NULL);
//		RefreshGList(Bootable,w,NULL,1L);	/* recalc did bootpri */
	}
}

/* draw all the partitions */

void
DrawPartitions (rp)
	register struct RastPort *rp;
{
	register struct PartitionBlock *p,*lastp;
	register LONG currcyl;

	currcyl = SelectedDrive->rdb->rdb_LoCylinder;

	SetDrMd(rp,JAM2);
	SetAPen(rp,2L);
	SetBPen(rp,1L);
	SetOPen(rp,2L);
	SetAfPt(rp,PartPattern,3L);

	for (lastp = NULL, p = SelectedDrive->rdb->rdb_PartitionList;
	     p;
	     lastp = p, p = p->pb_Next)
	{
		if (p != CurrentPart)
		{
			if (p->pb_Environment[DE_LOWCYL] > currcyl &&
			    lastp != CurrentPart)
			{
				/* fill empty area to left */
				BNDRYOFF(rp);	/* turn off area outline */
				SetDrMd(rp,JAM1);
				SetAPen(rp,0L);
				SetAfPt(rp,NULL,0L);
				RectFill(rp,
				   PixelCyl(currcyl),
				   DragPart.TopEdge,
				   PixelCyl(p->pb_Environment[DE_LOWCYL])-1,
				   DragPart.TopEdge + DragPart.Height -1);

				/* reset to original values */
				SetDrMd(rp,JAM2);
				SetAPen(rp,2L);
				SetOPen(rp,2L);
				SetAfPt(rp,PartPattern,3L);
			}
			/* now draw the partition */
			RectFill(rp,
			       PixelCyl(p->pb_Environment[DE_LOWCYL]),
			       DragPart.TopEdge,
			       min(PixelCyl(p->pb_Environment[DE_UPPERCYL]+1)-1,
			           PARTRE),
			       DragPart.TopEdge + DragPart.Height - 1);

		}
		currcyl = p->pb_Environment[DE_UPPERCYL] + 1;
	}

	BNDRYOFF(rp);	/* turn off area outline */
	SetDrMd(rp,JAM1);
	SetAPen(rp,0L);
	SetAfPt(rp,NULL,0L);

	if (currcyl < SelectedDrive->rdb->rdb_HiCylinder &&
	    lastp != CurrentPart)
	{
		/* fill final empty area */
		RectFill(rp,PixelCyl(currcyl),DragPart.TopEdge,
			 PixelCyl(SelectedDrive->rdb->rdb_HiCylinder+1)-1,
			 DragPart.TopEdge + DragPart.Height -1);
	}

	PrintIText(rp,&PartIText[0],0,XtraTop);	/* draw variable text */
}

int
PartitionSetup (w)
	struct Window *w;
{
	Init = TRUE;	/* Kill RefreshGList, AddGList, RemGList, ModifyProp */

	PartUpdate = 0;
//
	// I have to clean up these value, because the FreeGadget() does not
	// clean gadget pointer.
	// The other gadget pointer is not nessesary, because I don't reffer.
	// See RefreshBootable(), and RecalcXXXXX().
	// These functions may be called before CreateAllGadgets().
	Bootable = 0;
	BootPri = 0;
	StartCyl = 0;
	EndCyl = 0;
	TotalCyl = 0;
	Buffers = 0;
	FileSys = 0;	// 39.8
	HostID = 0;	/* 39.13 */
//
	Cylinders = SelectedDrive->rdb->rdb_HiCylinder -
		    SelectedDrive->rdb->rdb_LoCylinder + 1;

	/* set up the strings for this drive */

	*(PartIText[0].IText) = '0' + SelectedDrive->Addr;
	if (SelectedDrive->Flags & SCSI)
	{
		*(PartIText[1].IText) = '0' + SelectedDrive->Lun;
		PartIText[0].FrontPen = 1;
		PartIText[1].FrontPen = 1;
		PartIText[5].FrontPen = 1;
		PartAddrIText.FrontPen = 1;
	} else {
		/* make the text black */
		PartIText[0].FrontPen = 2;
		PartIText[1].FrontPen = 2;
		PartIText[5].FrontPen = 2;
		PartAddrIText.FrontPen = 2;
	}
	strcpy(PartIText[5].IText, SelectedDrive->Flags & SCSI ? " SCSI " :
								 xt_name);

	if (SelectedDrive->rdb->rdb_HiCylinder > 9999)
		strcpy(PartIText[2].IText,"****");
	else
		stcul_d(PartIText[2].IText,
		       abs(SelectedDrive->rdb->rdb_HiCylinder));

	/* save old partition list for cancel */

	save_part = CopyPartList(SelectedDrive->rdb->rdb_PartitionList);

	if ((CurrentPart = SelectedDrive->rdb->rdb_PartitionList) == NULL)
	{
		/* If no partitions, make some */
		DoQuickSetup(SelectedDrive);
		CurrentPart = SelectedDrive->rdb->rdb_PartitionList;

		/* kludge so size won't show as 4095 Meg */
//		TotalCylSInfo.LongInt = SelectedDrive->rdb->rdb_HiCylinder;
		TotalCylSInfoLongInt = SelectedDrive->rdb->rdb_HiCylinder;

		PartUpdate |= UPDATE;
	}
	SelectPartition(w);

	PropRefresh(w);		/* make sure these always get called */
	SizeRefresh(w);		/* SelectPartition doesn't call them if Init */

	/* make sure buffers agree with internal rep */

	strcpy(NamePartSIBuff,CurrentPart->pb_DriveName);

	/* 39.13 - Now we are supporting Host ID stuff */
	HostIDSInfoLongInt = CurrentPart->pb_HostID;

	ReCalcEndCyl(w);
	ReCalcStartCyl(w);
	ReCalcTotalCyl(w);
	ReCalcBootPri(w);
	ReCalcBuffers(w);
	ReCalcHostID(w);

	ReCalcSizeText(w);

	Init = FALSE;

	return TRUE;
}

void
InitPartition (d,p,low,hi)
	register struct Drive *d;
	register struct PartitionBlock *p;
	LONG low,hi;
{
	p->pb_ID	    = IDNAME_PARTITION;
	p->pb_SummedLongs   = sizeof(*p) >> 2;
	p->pb_HostID	    = HOST_ID;
	p->pb_Next	    = NULL;
	p->pb_Flags	    = 0;	/* default is mounted, not bootable */
	p->pb_DevFlags	    = 0;
	strcpy(&p->pb_DriveName[0],"CHANGE_ME");

	/* handle cylblocks != heads * sectors */

	if (d->rdb->rdb_Heads * d->rdb->rdb_Sectors ==
	    d->rdb->rdb_CylBlocks)
	{
		p->pb_Environment[DE_NUMHEADS]     = d->rdb->rdb_Heads;
		p->pb_Environment[DE_BLKSPERTRACK] = d->rdb->rdb_Sectors;
	} else {
		p->pb_Environment[DE_NUMHEADS]     = 1;
		p->pb_Environment[DE_BLKSPERTRACK] = d->rdb->rdb_CylBlocks;
	}
	p->pb_Environment[DE_LOWCYL]	   = low;
	p->pb_Environment[DE_UPPERCYL]	   = hi;
	p->pb_Environment[DE_BOOTPRI]	   = 0;

	/* Should we InitSFS for the first partition of each drive??? */
	InitFFS(p);	/* sets up most of the rest */
}

void
ReCalcTotalCyl (w)	//
	struct Window *w;
{
	LONG total;

	total = (CurrentPart->pb_Environment[DE_UPPERCYL] -
				CurrentPart->pb_Environment[DE_LOWCYL] + 1);

	if (TotalCyl)
	{
		GT_SetGadgetAttrs(TotalCyl,w,NULL,GTIN_Number,total,TAG_DONE);
//		TotalCylRefresh(w);
	}
	TotalCylSInfoLongInt = total;
}

void
ReCalcEndCyl (w)	//
	struct Window *w;
{
	LONG end;

	end = CurrentPart->pb_Environment[DE_UPPERCYL];

	if (EndCyl)
	{
		GT_SetGadgetAttrs(EndCyl,w,NULL,GTIN_Number,end,TAG_DONE);
//		EndCylRefresh(w);
	}
	EndCylSInfoLongInt = end;

}

void
ReCalcStartCyl (w)	//
	struct Window *w;
{
	LONG start;

	start = CurrentPart->pb_Environment[DE_LOWCYL];

	if (StartCyl)
	{
		GT_SetGadgetAttrs(StartCyl,w,NULL,GTIN_Number,start,TAG_DONE);
//		StartCylRefresh(w);
	}
	StartCylSInfoLongInt = start;
}

void
ReCalcBootPri (w)	//
	struct Window *w;
{
	LONG bootpri;

	bootpri = CurrentPart->pb_Environment[DE_BOOTPRI];

	if (BootPri)
	{
		GT_SetGadgetAttrs(BootPri,w,NULL,GTIN_Number,bootpri,TAG_DONE);
//		BootPriRefresh(w);
	}
	BootPriSInfoLongInt = bootpri;
}

void
ReCalcBuffers (w)	//
	struct Window *w;
{
	LONG buffers;

	buffers = CurrentPart->pb_Environment[DE_NUMBUFFERS];

	if (Buffers)
	{
		GT_SetGadgetAttrs(Buffers,w,NULL,GTIN_Number,buffers,TAG_DONE);
//		BufferRefresh(w);
	}
	BuffersSInfoLongInt = buffers;
}

void
ReCalcHostID (w)	/* 39.13 */
	struct Window *w;
{
	LONG hostid;

	hostid = CurrentPart->pb_HostID;

	if (HostID)
	{
		GT_SetGadgetAttrs(HostID,w,NULL,GTIN_Number,hostid,TAG_DONE);
		HostIDRefresh(w);
	}
	HostIDSInfoLongInt = hostid;
}

/* Drawing routine called by HandleWindow, draws all non-gadget stuff */

void
PartDraw (rp)
	struct RastPort *rp;
{
	DrawPartitions(rp);

	/* outline the partition area */
	SetDrMd(rp,JAM1);
	SetAPen(rp,1L);
	Move(rp,PARTLE-1,DragPart.TopEdge - 1);
	Draw(rp,PARTLE-1,DragPart.TopEdge + DragPart.Height);
	Draw(rp,PARTRE+1,DragPart.TopEdge + DragPart.Height);
	Draw(rp,PARTRE+1,DragPart.TopEdge - 1);
	Draw(rp,PARTLE-1,DragPart.TopEdge - 1);

	/* draw the 'keys' to partitions */
//	SetAPen(rp,0L);
//	RectFill(rp,40,DragPart.TopEdge - 11,79,DragPart.TopEdge - 5);
// The color is gray, so I have to make border with black.
	SetAPen(rp,1L);
	Move(rp,40,DragPart.TopEdge - 11);
	Draw(rp,40,DragPart.TopEdge - 5);
	Draw(rp,79,DragPart.TopEdge - 5);
	Draw(rp,79,DragPart.TopEdge - 11);
	Draw(rp,40,DragPart.TopEdge - 11);
//
	SetAPen(rp,1L);
	RectFill(rp,377,DragPart.TopEdge - 11,416,DragPart.TopEdge - 5);

	SetDrMd(rp,JAM2);
	SetAPen(rp,2L);
	SetBPen(rp,1L);
	SetAfPt(rp,PartPattern,3L);
	RectFill(rp,190,DragPart.TopEdge - 11,229,DragPart.TopEdge - 5);

	SetDrMd(rp,JAM1);
	SetAPen(rp,0L);
	SetAfPt(rp,NULL,0L);
}

void
ReCalcSizeText(w)
	struct Window *w;
{
	register struct RigidDiskBlock *rdb;
	register ULONG x;
	register char *size;

	/* erase old size by writing it in black */
//	PartIText[3].FrontPen = PartIText[4].FrontPen = 2L;
//
	PartIText[3].FrontPen = PartIText[4].FrontPen = 0L;
	if (!Init)
		PrintIText(w->RPort,&PartIText[3],0,XtraTop);
//
	PartIText[3].LeftEdge =
		PixelCyl(CurrentPart->pb_Environment[DE_UPPERCYL]) - 20;
	if (PartIText[3].LeftEdge < 12)
		PartIText[3].LeftEdge = 12;
	else if (PartIText[3].LeftEdge > 597)
		PartIText[3].LeftEdge = 597;
	PartIText[4].LeftEdge = PartIText[3].LeftEdge - 12;
	PartIText[3].FrontPen = PartIText[4].FrontPen = 1L;

	rdb = SelectedDrive->rdb;
//	x = TotalCylSInfo.LongInt * rdb->rdb_BlockBytes * rdb->rdb_CylBlocks -
	x = TotalCylSInfoLongInt * rdb->rdb_BlockBytes * rdb->rdb_CylBlocks -
	    rdb->rdb_BlockBytes *
		(CurrentPart->pb_Environment[DE_RESERVEDBLKS] +
	         CurrentPart->pb_Environment[DE_PREFAC]);
	x /= 1024;


	if (x < 1024) {
		size = "K";
		sprintf(PartIText[4].IText,"%4ld %s",x,size);//
	}
	else {
		x /= 1024;
		size = "Meg";
		if (x < 100)
			sprintf(PartIText[4].IText,"%2ld %s",x,size);//
		else
		{
			sprintf(PartIText[4].IText,"%3ld %s",x,size);//
			PartIText[4].LeftEdge -= 8;	// 39.10
		}
	}
/*
	if (x < 100)
		sprintf(PartIText[4].IText,"%2ld %s",x,size);
	else
		sprintf(PartIText[4].IText,"%4ld %s",x,size);
*/

	if (!Init)
		PrintIText(w->RPort,&PartIText[3],0,XtraTop);	//
}

/* do a quick setup - Two partitions, FFS, names ?DH0 and ?DH1 */
/* may be called by handletype */

int
DoQuickSetup (d)
	struct Drive  *d;
{
	LONG middle;
	struct PartitionBlock *newp,*lastp = NULL;

	if (!d->rdb)
		return FALSE;

	FreePartitionList(d->rdb->rdb_PartitionList);

	middle = (d->rdb->rdb_HiCylinder + d->rdb->rdb_LoCylinder)/2;

	/* if less than 20 meg, use one partition */
	if (((middle << 1) * d->rdb->rdb_CylBlocks * d->rdb->rdb_BlockBytes) <
	    23000000L)
	{
		middle = d->rdb->rdb_HiCylinder;
	}

	/* Make two FFS partitions */
	if (newp = AllocNew(PartitionBlock))
	{
	  InitPartition(d,newp,
			d->rdb->rdb_LoCylinder,
			middle);

	  /* set name to DH0: */
	  if (isalpha(d->rdb->rdb_DiskVendor[0]))
		  sprintf(&(newp->pb_DriveName[0]),"%cDH0",
			  d->rdb->rdb_DiskVendor[0]);
	  else
		  strcpy(&(newp->pb_DriveName[0]),"DH0");

	  d->rdb->rdb_PartitionList = newp;
	  lastp = newp;

	  /* make first partition bootable */
	  newp->pb_Flags |= PBFF_BOOTABLE;

	  if (middle < d->rdb->rdb_HiCylinder &&
	      (newp = AllocNew(PartitionBlock)) != NULL)
	  {
	  	InitPartition(d,newp,
			      middle+1,
			      d->rdb->rdb_HiCylinder);

		  /* set name to DH1: */
		if (isascii(d->rdb->rdb_DiskVendor[0]) &&
		    d->rdb->rdb_DiskVendor[0] != ' ')
		{
			sprintf(&(newp->pb_DriveName[0]),"%cDH1",
				d->rdb->rdb_DiskVendor[0]);
		} else {
			  strcpy(&(newp->pb_DriveName[0]),"DH1");
		}

		lastp->pb_Next = newp;

		/* leave only first partition bootable */
		newp->pb_Flags &= ~PBFF_BOOTABLE;

	  } else {
		/* made one, we'll return TRUE for the hell of it */
		return TRUE;
	  }
	} else
		return FALSE;

	return TRUE;
}
