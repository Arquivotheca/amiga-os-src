/* handleprep.c - code to handle the prep main screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"
#include "scsidisk.h"
#include "scsi.h"

//
extern struct Gadget	*glist1;
extern struct Gadget	*glist2;
extern struct Gadget	*glist10;
extern struct Gadget	*glist12;

USHORT SetTypeFlags	   = 0;
USHORT LowFormatFlags	   = 0;
USHORT BadBlockFlags	   = 0;
USHORT PartitionDriveFlags = 0;
USHORT FormatDriveFlags    = 0;
USHORT SurfCheckFlags	   = 0;

UWORD  DriveListPosition   = 0;	/* highlight position */

extern struct Remember	*RKey;

struct List	lh;		/* List for DriveList */

#define	DRIVELIST_LENGTH 80	/* DriveList string length */
//

extern struct TextFont *tfont;	/* 40.3 */
extern UWORD XtraTop;	/* 40.3 */

char *DoYouWantToQuit[] = {
	"Some disks have changes that have not been saved!",
	" ",					/* must not be ""! */
	"Do you really want to exit the program?",
	"(Select \"Continue\" to exit the program)",
	"(Select \"Cancel\" if you don't want to exit)",
	NULL,
};

char *DoYouWantToReboot[] = {
	"A reboot is required to make your changes take effect.",
	" ",
	"Select \"Continue\" to reboot.",
	"Select \"Cancel\" to go back to the program.",
	NULL,
};

char *PrepHelp[] = {
	"To set up a drive quickly:",
	" ",
	"If your drive is shown as being \"Unknown\",",
	"use \"Change Drive Type\" to tell HDToolBox what your drive is.",
	" ",
	"Once the correct type is shown, use \"Low Level Format\"",
	"to format the disk.  After using Format, use \"Verify Data on Drive\"",
	"to search for and map out any bad blocks on the drive.",
	" ",
	"When the drive is done verifying, use \"Save Changes to Drive\".",
	" ",
	"Exit, then format your partitions as you would a floppy disk.",
	"Exiting may require you to reboot; if so the program",
	"will tell you what to do.",
	" ",
	"For more detailed information, see your manual.",
	NULL,
};

char *FlagsChanged[] = {
	"Drives have been added or removed from the system.",
	" ",
	"This information needs to be recorded on some other",
	"drives in the system.  These drives will be marked as",
	"'Changed' under 'Status'.",
	" ",
	"Please do a 'Save Changes to Drive' on each of these",
	"drives.  If you do not do so, new drives might not show",
	"up, or booting might take much more time.",
	NULL,
};

int LowLevelConfirm;

#ifdef INTERLEAVE_REQ
LONG interleave;
#endif

int
HandlePrep (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct Gadget *gad;
		 struct Drive  *d;
	register int done = FALSE;
	UWORD id;

	switch (msg->Class) {
	case GADGETUP:
	case GADGETDOWN:
		gad = (struct Gadget *) (msg->IAddress);

		id  = gad->GadgetID & ~REFRESH_MASK;

		switch (id) {
		case EXITPREP:
			/* ask if any are changed but not written out */
			for (d = FirstDrive; d; d = d->NextDrive)
				if (d->Flags & UPDATE)
					break;
			if (!d || AskSure(w,&DoYouWantToQuit[0]))
				done = HANDLE_DONE;

			if (ForceReboot)
			{
				if (AskSure(w,&DoYouWantToReboot[0]))
//					DoReboot();
					ColdReboot();	//
				else
					done = FALSE;
			} else {
				/* no reboot required, make driver pick up */
				/* changes from modified drives.	   */
				ReopenDrives();
			}

			break;

		case PARTITIONDRIVE:
			if (!SelectedDrive->rdb)
			{
				DisplayBeep(w->WScreen);
				return done;
			}

			done = HandleWindowNew(w,&IntuiTextList2,
					&glist2,
					NewWindowStructure2.Title,
					NewWindowStructure2.IDCMPFlags,	// 39.10
					HandlePart,PartDraw,PartitionSetup,
					CreateAllGadgets2,
					&glist1,CreateAllGadgets1);

			/* do we need to enable the save changes gadget? */
			if (SelectedDrive->Flags & UPDATE)
				{
				MyOnGadget(w,FormatDrive);
				FormatDriveFlags &= ~GADGDISABLED;	//P
				}
			else
				{
				MyOffGadget(w,FormatDrive);
				FormatDriveFlags |= GADGDISABLED;	//P
				}
			break;

		case DRIVELIST:
			/* figure out which one he clicked on */
			HandleDriveClick(w,msg);
			break;

		case FORMATDRIVE:	/* really "commit to changes" */
			if (!SelectedDrive->rdb)
			{
				DisplayBeep(w->WScreen);
				return done;
			}

			/* this is duplicated in handletype.c */
			if (CommitChanges(w,FALSE))
			{
				SelectedDrive->Flags &= ~UPDATE;
				FreeRDB(SelectedDrive->oldrdb);
				SelectedDrive->oldrdb =
						CopyRDB(SelectedDrive->rdb);
				redraw_drivelist(w->RPort);
				GT_SetGadgetAttrs(DriveList,w,NULL,
					GTLV_Labels,&lh,TAG_DONE);	//
				MyOffGadget(w,FormatDrive);
				FormatDriveFlags |= GADGDISABLED;	//P
			}
			break;

		case LOWFORMAT:
			if (!SelectedDrive->rdb)
			{
				DisplayBeep(w->WScreen);
				break;
			}
			DoLowLevel(w);
			break;

		case SURFCHECK:
			/* Verify the drive */
			if (!SelectedDrive->rdb)
			{
				DisplayBeep(w->WScreen);
				break;
			}

			DoVerify(w);

			/* do we need to enable the save changes gadget? */
			if (SelectedDrive->Flags & UPDATE)
				{
				MyOnGadget(w,FormatDrive);
				FormatDriveFlags &= ~GADGDISABLED;	//P
				}
			else
				{
				MyOffGadget(w,FormatDrive);
				FormatDriveFlags |= GADGDISABLED;	//P
				}
			break;

		case BADBLOCK:
			done = HandleWindowNew(w,NULL,
						&glist10,
						BadWindow.Title,
						BadWindow.IDCMPFlags,	// 39.10
						HandleBadBlock,
						NULL,//BadDraw,
						BadSetup,
						CreateAllGadgets10,
						&glist1,CreateAllGadgets1);

			/* do we need to enable the save changes gadget? */
			if (SelectedDrive->Flags & UPDATE)
				{
				MyOnGadget(w,FormatDrive);
				FormatDriveFlags &= ~GADGDISABLED;	//P
				}
			else
				{
				MyOffGadget(w,FormatDrive);
				FormatDriveFlags |= GADGDISABLED;	//P
				}
			break;

		case SETTYPE:
			if (!SelectedDrive)
			{
				Notify(w,"No drive selected",0);
				break;
			}

			done = HandleWindowNew(w,NULL,
						&glist12,
						TypeWindow.Title,
						TypeWindow.IDCMPFlags,	// 39.10
						HandleSetType,
						NULL,//TypeDraw,
						TypeSetup,
						CreateAllGadgets12,
						&glist1,CreateAllGadgets1);

			/* do we need to enable the save changes gadget? */
			if (SelectedDrive->Flags & UPDATE)
			{
				MyOnGadget(w,FormatDrive);	//
				FormatDriveFlags &= ~GADGDISABLED;	//P
				if (SelectedDrive->rdb)
				{
				    MyOnGadget(w,PartitionDrive);	//
				    MyOnGadget(w,SurfCheck);		//
				    MyOnGadget(w,LowFormat);		//
				    MyOnGadget(w,BadBlock);		//
				    PartitionDriveFlags &= ~GADGDISABLED;	//P
				    SurfCheckFlags	&= ~GADGDISABLED;	//P
				    LowFormatFlags	&= ~GADGDISABLED;	//P
				    BadBlockFlags	&= ~GADGDISABLED;	//P
				}
			} else
				{
				MyOffGadget(w,FormatDrive);	//
				FormatDriveFlags |= GADGDISABLED;	//P
				}

//			redraw_drivelist(w->RPort);
			break;

		case HELPPREP:
			LongNotify(w,PrepHelp);
			break;

		}
		break;

	case RAWKEY:
		switch (msg->Code) {

		case 0x5f:	/* 39.13 - help */
			LongNotify(w,PrepHelp);
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

/* redraw the list of drives in the system, highlight 1 */
/* if selected drive isn't visible, changes selecteddrive to NULL */

static UWORD gotit;

#if 0
void
redraw_drivelist (rp)
	register struct RastPort *rp;
{
	register UWORD x,y,i;
	register struct Drive *d;
	char buffer[80];

	gotit = 0;

	x = DriveList.LeftEdge;
	y = DriveList.TopEdge;

	if (DisplayDrive == NULL)
		DisplayDrive = FirstDrive;

	/* clear to erase old text */
	SetAPen(rp,0L);
	SetDrMd(rp,JAM1);
	RectFill(rp,x,y,x + DriveList.Width - 1,y + DriveList.Height - 1);

	SetAPen(rp,1L);

	/* if no drives, say so */
	if (!FirstDrive)
	{
		Move(rp,x + 30,y + (DriveList.Height >> 1));
		if (found_driver)
			Text(rp,"No drives attached to system",28L);
		else
			Text(rp,"Driver not installed",20L);
	}

	for (d = DisplayDrive, i = 0;
	     d && i < MaxDisplayDrives;
	     d = d->NextDrive, i++)
	{
		if (d->rdb)
		{
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   sprintf(buffer+strlen(buffer),
		        "%s  %-8.8s %-16.16s %-4.4s",
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
					      (d->oldrdb ? "  Not Changed  " :
						           "     Empty     ")),
			d->rdb->rdb_DiskVendor,
			d->rdb->rdb_DiskProduct,
			d->rdb->rdb_DiskRevision);
		} else {
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   strcat(buffer,
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
							   "    Unknown    "));
		}

		Move(rp,x,y + rp->TxBaseline + 1);
		Text(rp,buffer,strlen(buffer));

		if (d == SelectedDrive)
		{
			SetAPen(rp,255L);
			SetDrMd(rp,COMPLEMENT);

			RectFill(rp,x,y,
				    x + DriveList.Width - 1,
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
redraw_drivelist (rp)
	register struct RastPort *rp;
{
	register UWORD i;
	register struct Drive *d;
	char buffer[DRIVELIST_LENGTH];
	struct Node *node;

	gotit = 0;

	if (DisplayDrive == NULL)
		DisplayDrive = FirstDrive;

	// Set first node in drive list.
	node = ((struct List *)(&lh))->lh_Head;

	for (d = DisplayDrive, i = 0;
	     d;
	     d = d->NextDrive, i++, node = node->ln_Succ)
	{
		if (d->rdb)
		{
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   sprintf(buffer+strlen(buffer),
		        "%s  %-8.8s %-16.16s %-4.4s",
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
					      (d->oldrdb ? "  Not Changed  " :
						           "     Empty     ")),
			d->rdb->rdb_DiskVendor,
			d->rdb->rdb_DiskProduct,
			d->rdb->rdb_DiskRevision);
		} else {
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   strcat(buffer,
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
							   "    Unknown    "));
		}

		// Update drive list.
		if (node->ln_Succ)
		{
			if (strcmp(node->ln_Name,buffer) != 0)
				strcpy(node->ln_Name,buffer);
//			kprintf("%s\n",node->ln_Name);
		}

		if (d == SelectedDrive)
		{
			gotit = 1;
		}
	}
}

/* Allocate drive list for CreateGadget(). */

void AllocDriveList(void)	// Allocate rtn for LISTVIEW_KIND Gadgets
{
	register UWORD i;
	register struct Drive *d;
	char buffer[DRIVELIST_LENGTH];

	struct Node *node;

	if (DisplayDrive == NULL)
		DisplayDrive = FirstDrive;

	NewList(&lh);

	for (d = DisplayDrive, i = 0;
	     d && i < NumDrives;
	     d = d->NextDrive, i++)
	{

		if (d->rdb)
		{
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   sprintf(buffer+strlen(buffer),
		        "%s  %-8.8s %-16.16s %-4.4s",
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
					      (d->oldrdb ? "  Not Changed  " :
						           "     Empty     ")),
			d->rdb->rdb_DiskVendor,
			d->rdb->rdb_DiskProduct,
			d->rdb->rdb_DiskRevision);
		} else {
		   strcpy(buffer,d->Flags & SCSI ? "SCSI  " : xt_name);
		   strcat(buffer,"       ");
		   sprintf(buffer+strlen(buffer),"%d      ",d->Addr);
		   if (d->Flags & SCSI)
		   {
			   sprintf(buffer+strlen(buffer),"%d  ",d->Lun);
		   } else {
			   strcat(buffer,"   ");
		   }
		   strcat(buffer,
			d->Flags & UPDATE ?		   "    Changed    " :
				(d->Flags & MINOR_UPDATE ? " Minor Changes " :
							   "    Unknown    "));
		}


		if (node = (struct Node *)
			AllocRemember(&RKey,sizeof(struct Node),MEMF_CLEAR))
		{
			if(node->ln_Name =
				AllocRemember(&RKey,DRIVELIST_LENGTH + 1,MEMF_CLEAR))
			{
			    strcpy(node->ln_Name,buffer);
			    AddTail(&lh,node);
			}
		}
	}

}


void
HandleDriveClick (w,msg)
	register struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct Drive *d;

	y = msg->Code;
	DriveListPosition = y;	// Remember the highlight position for CreateGadget().

	for (d = DisplayDrive; d && y; d = d->NextDrive, y--)
		; /* NULL */

	if (d) {
		if (SelectedDrive == d)
			return;

		SelectedDrive = d;

		redraw_drivelist(w->RPort);

		if (!gotit || !d->rdb)
		{
			/* never displayed selected, so de-select */
			if (!gotit)
			{
				SelectedDrive = NULL;
				MyOffGadget(w,SetType);
				SetTypeFlags |= GADGDISABLED;	//P
			} else {
				MyOnGadget(w,SetType);
				SetTypeFlags &= ~GADGDISABLED;	//P
			}

			MyOffGadget(w,FormatDrive);
			MyOffGadget(w,PartitionDrive);
			MyOffGadget(w,SurfCheck);
			MyOffGadget(w,LowFormat);
			MyOffGadget(w,BadBlock);
			FormatDriveFlags    |= GADGDISABLED;	//P
			PartitionDriveFlags |= GADGDISABLED;	//P
			SurfCheckFlags	    |= GADGDISABLED;	//P
			LowFormatFlags	    |= GADGDISABLED;	//P
			BadBlockFlags	    |= GADGDISABLED;	//P
		} else {
			MyOnGadget(w,PartitionDrive);
			MyOnGadget(w,SurfCheck);
			MyOnGadget(w,LowFormat);
			MyOnGadget(w,SetType);
			MyOnGadget(w,BadBlock);
			PartitionDriveFlags &= ~GADGDISABLED;	//P
			SurfCheckFlags	    &= ~GADGDISABLED;	//P
			LowFormatFlags	    &= ~GADGDISABLED;	//P
			SetTypeFlags	    &= ~GADGDISABLED;	//P
			BadBlockFlags	    &= ~GADGDISABLED;	//P

			if (SelectedDrive->Flags & UPDATE)
			{
				MyOnGadget(w,FormatDrive);
				FormatDriveFlags &= ~GADGDISABLED;	//P
			} else {
				MyOffGadget(w,FormatDrive);
				FormatDriveFlags |= GADGDISABLED;	//P
			}
		}
	} else
		DisplayBeep(w->WScreen);
}


#if 0
void
HandleDriveClick (w,msg)
	register struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct Drive *d;

	y = (msg->MouseY - DriveList->TopEdge) / (w->RPort->TxHeight + 1);

	for (d = DisplayDrive; d && y; d = d->NextDrive, y--)
		; /* NULL */

	if (d) {
		if (SelectedDrive == d)
			return;

		SelectedDrive = d;

		redraw_drivelist(w->RPort);

		if (!gotit || !d->rdb)
		{
			/* never displayed selected, so de-select */
			if (!gotit)
			{
				SelectedDrive = NULL;
				MyOffGadget(w,SetType);
			} else
				MyOnGadget(w,SetType);

			MyOffGadget(w,FormatDrive);
			MyOffGadget(w,PartitionDrive);
			MyOffGadget(w,SurfCheck);
			MyOffGadget(w,LowFormat);
			MyOffGadget(w,BadBlock);
		} else {
			MyOnGadget(w,PartitionDrive);
			MyOnGadget(w,SurfCheck);
			MyOnGadget(w,LowFormat);
			MyOnGadget(w,SetType);
			MyOnGadget(w,BadBlock);

			if (SelectedDrive->Flags & UPDATE)
				MyOnGadget(w,FormatDrive);
			else
				MyOffGadget(w,FormatDrive);
		}
	} else
		DisplayBeep(w->WScreen);
}
#endif

void
UpdateDrive(w)
	struct Window *w;
{
	if (!(SelectedDrive->Flags & UPDATE))
	{
		MyOnGadget(w,FormatDrive);
		FormatDriveFlags &= ~GADGDISABLED;	//P
		redraw_drivelist(w->RPort);
		GT_SetGadgetAttrs(DriveList,w,NULL,
				GTLV_Labels,&lh,TAG_DONE);	// 39.9
	}

	SelectedDrive->Flags |= UPDATE;
}

int
PrepInit (w)
	struct Window *w;
{
	register struct RastPort *rp;
	register struct Drive *d;

	struct Process *proc;

	/* search for drives out there */
	rp = w->RPort;

	SetAPen(rp,1L);
	SetBPen(rp,0L);
	SetDrMd(rp,JAM2);
	SetFont(rp,tfont);	/* 40.3 */
	Move(rp,216,100+XtraTop);
	Text(rp,"Reading Definitions...",22L);

	SCSI_Defs  = GetDefs("Drive definitions",SCSI);
	St506_Defs = GetDefs("Drive definitions",ST506);
	St506_DefNum = CountDefs(St506_Defs);
	SCSI_DefNum  = CountDefs(SCSI_Defs);

	proc = (struct Process *) FindTask(0);
	proc->pr_WindowPtr = (struct Window *)-1;
	NumDrives  = 0;
	FirstDrive = NULL;

	GetDrives(rp,"xt.device",ST506,2,1);
	GetDrives(rp,scsi_device,SCSI,scsi_addrs,scsi_luns);
	proc->pr_WindowPtr = 0;

	/* Tell user if any drives need saving for changed flags */
	for (d = FirstDrive; d; d = d->NextDrive)
	{
		if (d->Flags & UPDATE)
		{
			/* tell the user */
			Delay(25);		/* refresh problems - KLUDGE */
			LongNotify(w,FlagsChanged);
			break;				/* only once */
		}
	}

	SelectedDrive = DisplayDrive = FirstDrive;
	DisplayNum = 0;

	if (SelectedDrive)
	{
		if (!(SelectedDrive->Flags & UPDATE))
			FormatDriveFlags |= GADGDISABLED;   /* isn't changed yet */
	}
	if (!SelectedDrive || !(SelectedDrive->rdb))
	{
		PartitionDriveFlags |= GADGDISABLED;
		SurfCheckFlags      |= GADGDISABLED;
		LowFormatFlags	    |= GADGDISABLED;
		BadBlockFlags	    |= GADGDISABLED;
		if (!SelectedDrive)
		{
			FormatDriveFlags |= GADGDISABLED;
			SetTypeFlags     |= GADGDISABLED;
		}
	}


	if (!AllocCommit())	/* allocate memory for commit text */

	{
		FreeEverything();
		return FALSE;
	}


//	MaxDisplayDrives = DriveList.Height / (rp->TxHeight + 1);

	return TRUE;
}



int
FormatHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	register struct IntuiMessage *msg;
{
	register int done = 0;

	if (msg->Class == GADGETUP)
		if (msg->IAddress == (APTR) &NoLLFormat)
		{
			LowLevelConfirm = FALSE;
			done = HANDLE_DONE;
		} else if (msg->IAddress == (APTR) &LLFormat)
		{
			LowLevelConfirm = TRUE;
			done = HANDLE_DONE;
		} else
			; /* Huh? */
	return done;
}

#ifdef INTERLEAVE_REQ
int
InterleaveHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	register struct IntuiMessage *msg;
{
	register int done = 0;

	if (msg->Class == GADGETUP)
	{
		if (msg->IAddress == (APTR) &InterleaveOK)
			done = HANDLE_DONE;
		else if (msg->IAddress == (APTR) &InterleaveCancel)
			done = HANDLE_CANCEL;
		else
			; /* Huh? */
	} else if (msg->Class == REQSET) {
		/* activate the interleave int gadget */
		ActivateGadget(&Interleave,w,req);
	}
	return done;
}
#endif

void
DoLowLevel (w)
	struct Window *w;
{
	struct Drive *d = SelectedDrive;
	int  error;
#ifdef INTERLEAVE_REQ
	LONG old_interleave;
#endif

	LowLevelConfirm = FALSE;

	//Make new border for 1.3 type requester
	MakePrettyRequestBorder(LLFormatRequester.Width,
					LLFormatRequester.Height);

	/* does the user wish to low level format? */
	(void) HandleRequest(w,&LLFormatRequester,FormatHandler);
	if (LowLevelConfirm == FALSE)
		return;

#ifdef INTERLEAVE_REQ
	/* ask what interleave he wants (can still cancel here) */
	/* default is 1 */

	InterleaveSInfo.LongInt = d->rdb->rdb_Interleave;
	sprintf(&InterleaveSIBuff[0],"%ld",d->rdb->rdb_Interleave);

	if (HandleRequest(w,&InterleaveRequester,InterleaveHandler) ==
	    HANDLE_CANCEL)
	{
		/* He cancelled, get out of here */
		return;
	}
	old_interleave = d->rdb->rdb_Interleave;
	interleave = InterleaveSInfo.LongInt;	/* global */
	d->rdb->rdb_Interleave = interleave;
#endif

	/* Write out the rigid disk block for the driver. */
	/* This is so the driver can map out bad blocks   */
	/* if needed.					  */
	/* Also warns user of lost partitions.		  */
	/* May set ForceReboot.				  */
	/* Don't fail if error on write.		  */

	ForceReboot = TRUE;

	if (!CommitChanges(w,TRUE))
	{
#ifdef INTERLEAVE_REQ
		/* restore the old interleave value */
		d->rdb->rdb_Interleave = old_interleave;
#endif
		return;
	}

	error = DoLongIO(w,d,"Formatting, please wait",LowLevelSendIO,
//			 &IntuiTextList1,prep_draw,NULL);
			 NULL,redraw_drivelist,NULL);		//

	if (error)
	{
		Notify(w,"Failed Low-level Format, error %d !",
			(LONG) error);
	}
	/* else successful format of drive */

	/* Formatted: no rigid disk block exists on it anymore.     */
	/* bad block list stays */

	/* Even if failure, it probably wiped the rigid disk block. */
	/* In any case, dump the old rdb.			    */
	if (d->oldrdb)
	{
		FreeRDB(d->oldrdb);
		d->oldrdb = NULL;
	}

	/* the rigid disk block was wiped, set changed status */
	UpdateDrive(w);
}

/* routine called by DoLongIO to send the IO request */
/* return TRUE if ok, FALSE if error		     */

int
LowLevelSendIO (w,d,ior)
	struct Window *w;
	register struct Drive *d;
	struct IOStdReq *ior;
{
	char modeselect[6];
	char modes[22];

	if (d->Flags & ADAPTEC) {
/* I HATE ADAPTEC! */
		modeselect[0] = S_MODE_SELECT;
		modeselect[4] = 22;	/* soft sectored 4000/4070 */
		modeselect[1] = 	/* driver will fill in lun */
		modeselect[2] =
		modeselect[3] =
		modeselect[5] = 0;

		/* clear to 0, then set up mode select */
		memset(modes,'\0',sizeof(modes));

		modes[3]  = 8;
		modes[9]  = (d->rdb->rdb_BlockBytes & 0x00ff0000) >> 16;
		modes[10] = (d->rdb->rdb_BlockBytes & 0x0000ff00) >> 8;
		modes[11] = (d->rdb->rdb_BlockBytes & 0x000000ff);
		modes[12] = 1;	/* soft sectored */
		modes[13] = (d->rdb->rdb_Cylinders & 0x0000ff00) >> 8;
		modes[14] = (d->rdb->rdb_Cylinders & 0x000000ff);
		modes[15] = d->rdb->rdb_Heads;
		modes[16] = (d->rdb->rdb_ReducedWrite & 0x00ff00) >> 8;
		modes[17] = (d->rdb->rdb_ReducedWrite & 0x0000ff);
		modes[18] = (d->rdb->rdb_WritePreComp & 0x00ff00) >> 8;
		modes[19] = (d->rdb->rdb_WritePreComp & 0x0000ff);

		if (d->rdb->rdb_Park <= 0)
			modes[20] = (-(d->rdb->rdb_Park)) & 0x00ff;
		if (d->rdb->rdb_Park >= d->rdb->rdb_Cylinders - 1)
			modes[20] = d->rdb->rdb_Park -
				    (d->rdb->rdb_Cylinders - 1);

/* FIX! */	modes[21] = 2; /* d->rdb->rdb_StepRate; */

		if (DoSCSI(ior,(UWORD *) modeselect, 6,
			       (UWORD *) modes, 22,
			       SCSIF_WRITE | SCSIF_AUTOSENSE))
		{
			/* ERROR */
			Notify(w,"Failed Mode_Select, error %d!",
			       (LONG) ior->io_Error);
			return FALSE;
		}

	}

	/* now do the format unit */
	ior->io_Command = TD_FORMAT;
	ior->io_Offset  = 0;
	ior->io_Data	= NULL; 	/* dangerous! */
	ior->io_Length	= d->rdb->rdb_BlockBytes *
			  d->rdb->rdb_CylBlocks;
	SendIO((struct IORequest *) ior);

	return TRUE;
}

/* silly little routine for counting definitions */

WORD
CountDefs (def)
	register struct DriveDef *def;
{
	WORD num = 0;

	while (def) {
		num++;
		def = def->Succ;
	}

	return num;
}

/* open drives that were written out to - use flags = 1 - this causes the */
/* driver to read the rigid disk block. */

void
ReopenDrives ()
{
	register struct Drive *d;
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	for (d = FirstDrive; d; d = d->NextDrive)
	{
		if (d->Flags & WRITTEN)
		{
			if (!OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
				   (struct IORequest *) ior,1L))
				CloseDevice((struct IORequest *) ior);
		}
	}
cleanup:
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
}
