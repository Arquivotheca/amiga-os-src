/* handlefsm.c - code to handle the file system maintenance */

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

#include "fstype.h"

WORD FileSysUpdate = 0;

#define DosTypeAddr	(&GetDosTypeSIBuff[0])

//
USHORT FSMDeleteFlags = 0;
USHORT FSMUpdateFlags = 0;
UWORD FSMListPosition = 0;	/* highlight position */

extern UBYTE AddrLunText[44];

extern struct Remember	*RKey;	/* pointer for list allocation */

struct List	lh4;		/* list for file system list */

#define	FSMLIST_LENGTH 80	/* file system list string length */
//

int
HandleFSM (w,msg)
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
		case FSMUPDATE:
			HandleFSMUpdate(w);
			break;

		case FSMDELETE:
			HandleFSMDelete(w);
			break;

		case FSMADD:
			HandleFSMAdd(w);
			break;

		case FSMCANCEL:
			FreeFileSysList(FirstFileSys);
			FirstFileSys = CurrFileSys = NULL;
			done = HANDLE_CANCEL;
			break;

		case FSMOK:
			FreeFileSysList(
				SelectedDrive->rdb->rdb_FileSysHeaderList);
			SelectedDrive->rdb->rdb_FileSysHeaderList =
								   FirstFileSys;
			if (FileSysUpdate)
				SelectedDrive->Flags |= UPDATE;

			done = HANDLE_DONE;
			break;

		case FSMLIST:
			HandleFSMClick(w,msg);
			break;

		}

		break;
//
//	case MENUPICK:			// No menu.
//		HandleMenus(w,msg);
//		break;

	}

	return done;
}

static UWORD gotit;

void
HandleFSMUpdate (w)
	struct Window *w;
{
	register struct FileSysHeaderBlock *newfs,*fs;

	if (CurrFileSys)	/* if one is selected */
	{
		/* replace old file sys on disk */
		GetDosType.Flags |= GADGDISABLED;
		sprintf(DosTypeAddr,"0x%lx",CurrFileSys->fhb_DosType);

		if (newfs = GetFileSys(w))
		{
			newfs->fhb_Next = CurrFileSys->fhb_Next;
			if (CurrFileSys == FirstFileSys)
				FirstFileSys = newfs;
			else {
				/* this MUST succeed, no error checks */
				for (fs = FirstFileSys;
				     fs->fhb_Next;
				     fs = fs->fhb_Next)
				{
					if (fs->fhb_Next == CurrFileSys)
					{
						fs->fhb_Next = newfs;
						break;
					}
				}
			}

			if (DisplayFS == CurrFileSys)
				DisplayFS = newfs;

			FreeFileSys(CurrFileSys);
			CurrFileSys = newfs;

			redraw_fsmlist(w->RPort);
			GT_SetGadgetAttrs(FSMList,w,NULL,
					GTLV_Labels,&lh4,TAG_DONE);	//
//
			if (!gotit)
			{
				MyOffGadget(w,FSMUpdate);
				MyOffGadget(w,FSMDelete);
			}
//
			FileSysUpdate = TRUE;
		}
		GetDosType.Flags &= ~GADGDISABLED;
	} else
		DisplayBeep(w->WScreen);
}

void
HandleFSMDelete (w)
	struct Window *w;
{
	register struct FileSysHeaderBlock *fs,*lastfs;

	if (CurrFileSys)
	{
		for (lastfs = NULL, fs = FirstFileSys;
			fs && fs != CurrFileSys;
			lastfs = fs, fs = fs->fhb_Next)
			; /* NULL */

		if (lastfs == NULL)		/* first one */
			FirstFileSys = fs->fhb_Next;
		else
			// I think this is original bug.
//			lastfs = fs->fhb_Next;
			lastfs->fhb_Next = fs->fhb_Next;

		FreeFileSys(CurrFileSys);
		CurrFileSys = FirstFileSys;

		DisplayFS = CurrFileSys;

		DisplayFSNum = 0;
		NumFS--;

		redraw_fsmlist(w->RPort);
		GT_SetGadgetAttrs(FSMList,w,NULL,
					GTLV_Labels,&lh4,TAG_DONE);	//
//
		if (!gotit)
		{
			MyOffGadget(w,FSMUpdate);
			MyOffGadget(w,FSMDelete);
		}
//
		FileSysUpdate = TRUE;
	} else
		DisplayBeep(w->WScreen);
}

void
HandleFSMAdd (w)
	struct Window *w;
{
	register struct FileSysHeaderBlock *newfs,*fs;

	GetDosType.Flags &= ~GADGDISABLED;

	/* 39.13 - Now default identifier is international fast file system */
	sprintf(DosTypeAddr,"0x%lx",ID_INTER_FFS_DISK);

	if (newfs = GetFileSys(w))
	{
		for (fs = FirstFileSys;
		     fs;
		     fs = fs->fhb_Next)
		{
			if (newfs->fhb_DosType == fs->fhb_DosType)
			{
				Notify(w,
				    "File System of Type 0x%lx already exists!",
				    fs->fhb_DosType);
				FreeFileSys(newfs);
				return;
			}
		}
		newfs->fhb_Next = FirstFileSys;
		CurrFileSys = FirstFileSys = newfs;

		DisplayFS = CurrFileSys;
		DisplayFSNum = 0;
		NumFS++;

		redraw_fsmlist(w->RPort);
//
		GT_SetGadgetAttrs(FSMList,w,NULL,GTLV_Labels,&lh4,TAG_DONE);
		GT_SetGadgetAttrs(FSMList,w,NULL,GTLV_Selected,0,TAG_DONE);
		FSMListPosition = 0;
//
//
		if (gotit)
		{
			MyOnGadget(w,FSMUpdate);
			MyOnGadget(w,FSMDelete);
		}
//
		FileSysUpdate = TRUE;
	}
}

int
GetFileHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	struct IntuiMessage *msg;
{
	int done = 0;

	if (msg->Class == GADGETUP)
	{
		if (msg->IAddress == (APTR) &FsGetNameOk)
			done = HANDLE_DONE;
		else if (msg->IAddress == (APTR) &FsGetNameCancel)
			done = HANDLE_CANCEL;
		else
			;
	} else if (msg->Class == REQSET) {
		/* activate the filename string gadget */
		ActivateGadget(&GetFileName,w,req);
	}

	return done;
}

int
GetTypeHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	struct IntuiMessage *msg;
{
	int done = 0;

	if (msg->Class == GADGETUP)
	{
		if (msg->IAddress == (APTR) &FsGetTypeOk)
			done = HANDLE_DONE;
		else if (msg->IAddress == (APTR) &FsGetTypeCancel)
			done = HANDLE_CANCEL;
		else
			;
	} else if (msg->Class == REQSET) {
		// sould not activate when gadget is disabled.
		if (!(GetDosType.Flags & GADGDISABLED))
			/* activate the filename string gadget */
			ActivateGadget(&GetDosType,w,req);
	}

	return done;
}

/* 39.9 */
struct FileSysHeaderBlock *
GetFileSys (w)
	struct Window *w;
{
	struct FileSysHeaderBlock *fs = NULL;
	char *ptr;
	LONG  dostype;

	/* I think this sentense will bother peple who wants to put different FS. */
	// sprintf(GetFileNameSIBuff,"l:FastFileSystem");
again:
	MakePrettyRequestBorder(FileSysNameRequester.Width,
					FileSysNameRequester.Height);

	if (HandleRequest(w,&FileSysNameRequester,GetFileHandler) == HANDLE_DONE)
	{
		GetVersionSInfo.LongInt = 0;
		GetRevisionSInfo.LongInt = 0;
		strcpy(GetVersionSIBuff,"0");
		strcpy(GetRevisionSIBuff,"0");

		// 39.9
		GetFSVersion(GetFileNameSIBuff,
				    &GetVersionSInfo.LongInt,
				    &GetRevisionSInfo.LongInt);

		sprintf(GetVersionSIBuff,"%ld",GetVersionSInfo.LongInt);
		sprintf(GetRevisionSIBuff,"%ld",GetRevisionSInfo.LongInt);

//		kprintf("name = %s\n",GetFileNameSIBuff);
//		kprintf("DosTypeAddr = %s\n",DosTypeAddr);
//		kprintf("Version = %s,",GetVersionSIBuff);
//		kprintf("Revision = %s\n",GetRevisionSIBuff);

		MakePrettyRequestBorder(FileSysTypeRequester.Width,
					FileSysTypeRequester.Height);

		if (HandleRequest(w,&FileSysTypeRequester,GetTypeHandler) == HANDLE_DONE)
		{
			/* first, check that the dostype is reasonable */
			ptr = DosTypeAddr;
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&dostype) == strlen(ptr))
			{
				/* we have the dostype */
			} else {
				Notify(w,"Invalid hex DosType %s!",
					(LONG) DosTypeAddr);
				Notify(w,
				   "Please enter the DosType in hex with a 0x leading.",
				    NULL);
				/* 39.13 - now deafult is international ffs */
				sprintf(DosTypeAddr,"0x%lx",ID_INTER_FFS_DISK);
				goto again;
			}

			fs = MakeFileSys(w,SelectedDrive->rdb,GetFileNameSIBuff,
				 dostype,
				 GetVersionSInfo.LongInt,GetRevisionSInfo.LongInt);
		}
	}
	return fs;
}

#if 0
struct FileSysHeaderBlock *
GetFileSys (w)
	struct Window *w;
{
	struct FileSysHeaderBlock *fs = NULL;
	char *ptr;
	LONG  dostype;

again:
	MakePrettyRequestBorder(FileSysNRequester.Width,
					FileSysNRequester.Height);	//

	if (HandleRequest(w,&FileSysNRequester,GetFileHandler) == HANDLE_DONE)
	{
		/* first, check that the dostype is reasonable */
		ptr = DosTypeAddr;
		if (*ptr++ == '0' && *ptr++ == 'x' &&
		    stch_l(ptr,&dostype) == strlen(ptr))
		{
			/* we have the dostype */
		} else {
			Notify(w,"Invalid hex DosType %s!",
			       (LONG) DosTypeAddr);
			Notify(w,
			   "Please enter the DosType in hex with a 0x leading.",
			    NULL);
			/* 39.13 - now deafult is international ffs */
			sprintf(DosTypeAddr,"0x%lx",ID_INTER_FFS_DISK);
			goto again;
		}

		fs = MakeFileSys(w,SelectedDrive->rdb,GetFileNameSIBuff,
				 dostype,
				 GetVersionSInfo.LongInt,GetRevisionSInfo.LongInt);
	}
	return fs;
}
#endif

struct FileSysHeaderBlock *
MakeFileSys (w,rdb,name,dostype,version,revision)
	struct Window *w;
	struct RigidDiskBlock *rdb;
	char *name;
	LONG dostype,version,revision;
{
	struct FileSysHeaderBlock *fs = NULL;
	register struct LoadSegBlock *ls,*lastls;
	BPTR fh;
	register LONG len,newlen;
	ULONG temp = 0, done;
	LONG size;

	if (fh = Open(name,MODE_OLDFILE))
	{
		size = Read(fh,(char *) &temp,4);
		if (size != 4)
			goto read_error;
		if (temp == 0x3F3)			/* hunk_header */
		{
			if (Seek(fh,0,OFFSET_BEGINNING) == -1)
				goto read_error;
			if (fs = AllocNew(FileSysHeaderBlock))
			{
			    *fs = defaultfs;	/* structure copy */
			    fs->fhb_DosType = dostype;
//			    fs->fhb_Version = version;
			    fs->fhb_Version = (version << 16)|(revision & 0x0000ffff);

			    lastls = NULL;
			    done = FALSE;
			    while (!done) {
				ls = (struct LoadSegBlock *)
				    AllocMem(rdb->rdb_BlockBytes,MEMF_CLEAR);

				ls->lsb_ID = IDNAME_LOADSEG;
				ls->lsb_SummedLongs =
					rdb->rdb_BlockBytes >> 2;
				ls->lsb_HostID = HOST_ID;

				if (lastls)
					lastls->lsb_Next = ls;
				else
					fs->fhb_SegListBlocks = ls;

				newlen = (ls->lsb_SummedLongs - 5) * 4;
				len = Read(fh,(char *)&ls->lsb_LoadData[0],
					   newlen);
				if (len < 0)
				{
read_error:				if (w)
					  Notify(w,
					   "Error %d while reading filesystem!",
					   IoErr());

					if (fs)
						FreeFileSys(fs);
					fs = NULL;
					done = TRUE;

				} else if (len == 0) {

					/* true EOF - drop last block */
					lastls->lsb_Next = NULL;
					FreeMem((char *)ls,
						ls->lsb_SummedLongs*4);
					done = TRUE;

				} else if (len != newlen) {

					/* EOF is assumed here */
					ls->lsb_SummedLongs = (len >> 2) + 5;
					done = TRUE;

				} /* else not done yet */
				lastls = ls;

			    } /* while */
			}
		} else {
			if (w)
				Notify(w,"File %s is not a filesystem!",
				       (LONG) name);
		}
		Close(fh);
	} else {
		if (w)
			Notify(w,"Can't open filesystem %s!",
			       (LONG) name);
	}

	return fs;
}

#if 0
void
redraw_fsmlist (rp)
	struct RastPort *rp;
{
	register UWORD x,y,i;
	register struct FileSysHeaderBlock *d;
	char buffer[80],name[80];

	gotit = 0;

	x = FSMList.LeftEdge;
	y = FSMList.TopEdge;

	if (DisplayFS == NULL)
		DisplayFS = FirstFileSys;

	/* clear to erase old text */
	SetAPen(rp,0L);
	SetDrMd(rp,JAM1);
	RectFill(rp,x,y,x + FSMList.Width - 1,y + FSMList.Height - 1);

	SetAPen(rp,1L);

	for (d = DisplayFS, i = 0;
	     d && i < MaxDisplayFS;
	     d = d->fhb_Next, i++)
	{
		switch (d->fhb_DosType) {
		case 'DOS\x00':
			strcpy(name,"Old File System");
			break;
		case 'DOS\x01':
			strcpy(name,"Fast File System");
			break;
		default:
			if (d->fhb_DosType & 0xffffff00 == 'UNI\x00')
				sprintf(name,"UNIX File System %d",
					d->fhb_DosType & 0x000000ff);
			else
				strcpy(name,"Custom File System");
			break;
		}
		sprintf(buffer,"0x%lx   %6ld    %6ld   %s",
			d->fhb_DosType,d->fhb_Version,
			LoadSegSize(d->fhb_SegListBlocks),name);

		Move(rp,x,y + rp->TxBaseline + 1);
		Text(rp,buffer,strlen(buffer));

		if (d == CurrFileSys)
		{
			SetAPen(rp,255L);
			SetDrMd(rp,COMPLEMENT);

			RectFill(rp,x,y,
				    x + FSMList.Width - 1,
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
redraw_fsmlist (rp)
	struct RastPort *rp;
{
	register UWORD i;
	register struct FileSysHeaderBlock *d;
	char buffer[FSMLIST_LENGTH],name[FSMLIST_LENGTH];
	char verstr[16];	// 39.9

	struct Node *node,*nodenew;

//	kprintf("redraw\n");
	gotit = 0;

	if (DisplayFS == NULL)
		DisplayFS = FirstFileSys;

	// Set first node in FSM list.
	node = ((struct List *)(&lh4))->lh_Head;

	for (d = DisplayFS, i = 0;
	     d;
	     d = d->fhb_Next, i++)
	{
//		sprintf(tmp,"%lx",d);
//		kprintf("%s:",tmp);

// 39.11
		switch (d->fhb_DosType) {
		case ID_DOS_DISK:				// DOS\0
			strcpy(name,"Old File System");
			break;
		case ID_FFS_DISK:				// DOS\1
			strcpy(name,"Fast File System");
			break;
		case ID_INTER_DOS_DISK:				// DOS\2
			strcpy(name,"International (OFS)");
			break;
		case ID_INTER_FFS_DISK:				// DOS\3
			strcpy(name,"International (FFS)");
			break;
		case ID_DC_DOS_DISK:				// DOS\4
			strcpy(name,"Dir Cache (OFS)");
			break;
		case ID_DC_FFS_DISK:				// DOS\5
			strcpy(name,"Dir Cache (FFS)");
			break;
		default:					// other
			if (d->fhb_DosType & 0xffffff00 == 'UNI\x00')
				sprintf(name,"UNIX File System %d",
					d->fhb_DosType & 0x000000ff);
			else
				strcpy(name,"Custom File System");
			break;
		}
//
		sprintf(verstr,"%d.%d",
			(d->fhb_Version & 0xffff0000) >> 16,
			(d->fhb_Version & 0x0000ffff));		// 39.9
		sprintf(buffer,"0x%08lx  %9s  %6ld   %s",
			d->fhb_DosType,verstr,
			LoadSegSize(d->fhb_SegListBlocks),name);	// 39.9

		// Update FSM list.
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
						FSMLIST_LENGTH + 1,MEMF_CLEAR))
				{
					strcpy(nodenew->ln_Name,buffer);
					AddTail(&lh4,nodenew);
//					sprintf(tmp,"%x",nodenew);
//					kprintf("node %s - ",tmp);
//					kprintf("add %s\n",nodenew->ln_Name);
				}
			}
		}

		if (d == CurrFileSys)
		{
			gotit = 1;
		}
	}
	if (node->ln_Succ)
	{
		// Don't be upset to delete tail of list.
		// Because I already update the content of list above.
		// The deleted content of list does not mean anything.
		node = RemTail(&lh4);	// Delete Tail List
//		sprintf(tmp,"%x",node);
//		kprintf("node %s - deleted\n",tmp);
	}
}


/* Allocate FSM list for CreateGadget(). */

void
AllocFSMList (void)		// allocate rtn for LISTVIEW_KIND Gadgets
{
	register UWORD i;
	register struct FileSysHeaderBlock *d;
	char buffer[FSMLIST_LENGTH],name[FSMLIST_LENGTH];
	char verstr[16];	// 39.9

	struct Node *node;

	if (DisplayFS == NULL)
		DisplayFS = FirstFileSys;

	NewList(&lh4);

	for (d = DisplayFS, i = 0;
	     d;
	     d = d->fhb_Next, i++)
	{
// 39.11
		switch (d->fhb_DosType) {
		case ID_DOS_DISK:				// DOS\0
			strcpy(name,"Old File System");
			break;
		case ID_FFS_DISK:				// DOS\1
			strcpy(name,"Fast File System");
			break;
		case ID_INTER_DOS_DISK:				// DOS\2
			strcpy(name,"International (OFS)");
			break;
		case ID_INTER_FFS_DISK:				// DOS\3
			strcpy(name,"International (FFS)");
			break;
		case ID_DC_DOS_DISK:				// DOS\4
			strcpy(name,"Dir Cache (OFS)");
			break;
		case ID_DC_FFS_DISK:				// DOS\5
			strcpy(name,"Dir Cache (FFS)");
			break;
		default:					// other
			if (d->fhb_DosType & 0xffffff00 == 'UNI\x00')
				sprintf(name,"UNIX File System %d",
					d->fhb_DosType & 0x000000ff);
			else
				strcpy(name,"Custom File System");
			break;
		}
//
/*
		switch (d->fhb_DosType) {
		case ID_DOS_DISK:
			strcpy(name,"Old File System");
			break;
		case ID_FFS_DISK:
			strcpy(name,"Fast File System");
			break;
		default:
			if (d->fhb_DosType & 0xffffff00 == 'UNI\x00')
				sprintf(name,"UNIX File System %d",
					d->fhb_DosType & 0x000000ff);
			else
				strcpy(name,"Custom File System");
			break;
		}
*/
		sprintf(verstr,"%d.%d",
			(d->fhb_Version & 0xffff0000) >> 16,
			(d->fhb_Version & 0x0000ffff));		// 39.9
		sprintf(buffer,"0x%08lx  %9s  %6ld   %s",
			d->fhb_DosType,verstr,
			LoadSegSize(d->fhb_SegListBlocks),name);	// 39.9
/*
		sprintf(buffer,"0x%08lx   %6ld    %6ld   %s",
			d->fhb_DosType,d->fhb_Version,
			LoadSegSize(d->fhb_SegListBlocks),name);
*/
		if (node = (struct Node *)
			AllocRemember(&RKey,sizeof(struct Node),MEMF_CLEAR))
		{
			if(node->ln_Name =
				AllocRemember(&RKey,FSMLIST_LENGTH + 1,MEMF_CLEAR))
			{
			    strcpy(node->ln_Name,buffer);
			    AddTail(&lh4,node);
			}
		}
	}

}



/* return the size of a loadseglist */

LONG
LoadSegSize (ls)
	register struct LoadSegBlock *ls;
{
	register LONG size = 0;

	while (ls) {
		size += (ls->lsb_SummedLongs - 5) * 4;
		ls = ls->lsb_Next;
	}
	return size;
}

void
HandleFSMClick (w,msg)
	register struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct FileSysHeaderBlock *d;

	y = msg->Code;
	FSMListPosition = y;	// Remember the highlight position for CreateGadget().

	for (d = DisplayFS; d && y; d = d->fhb_Next, y--)
		; /* NULL */

	if (d) {
		if (CurrFileSys == d)
			return;
		CurrFileSys = d;
	} else
		DisplayBeep(w->WScreen);
}

#if 0
void
HandleFSMClick (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct FileSysHeaderBlock *d;

	y = (msg->MouseY - FSMList->TopEdge) / (w->RPort->TxHeight + 1);

	for (d = DisplayFS; d && y; d = d->fhb_Next, y--)
		; /* NULL */

	if (d) {
		if (CurrFileSys == d)
			return;
		CurrFileSys = d;

		redraw_fsmlist(w->RPort);
		if (gotit)
		{
			MyOnGadget(w,FSMUpdate);
			MyOnGadget(w,FSMDelete);
		}
	} else
		DisplayBeep(w->WScreen);
}
#endif

int
FSMInit (w)
	struct Window *w;
{
	register struct FileSysHeaderBlock *fsb;
	register int i;

	/* get number of File systems */
	for (i = 0, fsb = SelectedDrive->rdb->rdb_FileSysHeaderList;
	     fsb;
	     i++, fsb = fsb->fhb_Next)
		; /* NULL */

	NumFS        = i;
	FirstFileSys = CopyFileSysList(
				     SelectedDrive->rdb->rdb_FileSysHeaderList);
	CurrFileSys  = FirstFileSys;
	DisplayFS    = FirstFileSys;
	FileSysUpdate = 0;
	DisplayFSNum = 0;

	if (SelectedDrive->Flags & SCSI)
		memcpy(&AddrLunText[16]," SCSI ",6);
	else {
		memset(&AddrLunText[16],' ',6);
		memcpy(&AddrLunText[16],xt_name,strlen(xt_name));
	}

	AddrLunText[31] = SelectedDrive->Addr + '0';
	AddrLunText[38] = SelectedDrive->Lun  + '0';

	if (CurrFileSys == NULL)
	{
		FSMUpdateFlags |= GADGDISABLED;
		FSMDeleteFlags |= GADGDISABLED;
	} else {
		FSMUpdateFlags &= ~GADGDISABLED;
		FSMDeleteFlags &= ~GADGDISABLED;
	}

//	MaxDisplayFS = FSMList.Height / (w->RPort->TxHeight + 1);

	GetVersionSInfo.LongInt = 0;
	GetRevisionSInfo.LongInt = 0;	// 39.9
	strcpy(GetVersionSIBuff,"0");
	strcpy(GetRevisionSIBuff,"0");	// 39.9

	/* 39.13 - bug fix */
	FSMListPosition = 0;

	return TRUE;
}

/* 39.9 - these routines supports version number for file system */

#include "exec/resident.h"

void
CopyVersionStr (UBYTE *data,STRPTR result,UWORD size)
{
	UWORD i,j;

	i = 0;
	while (data[i] == ' ')
		i++;

	j = 0;
	while ((data[i] != '\0') &&
	       (data[i] != '\r') &&
	       (data[i] != '\n') &&
	       (j < size))
		result[j++] = data[i++];

	if (j == size)
		j--;

	result[j] = '\0';
}

struct Resident *
FindRomTag (BPTR segList)
{
	struct Resident *tag;
	UWORD *seg;
	ULONG i;
	ULONG *ptr;

	while (segList)
	{
		ptr     = (ULONG *)((ULONG)segList << 2);
        	seg     = (UWORD *)((ULONG)ptr);
		segList = *ptr;

		for (i = *(ptr-1)>>1;(i > 0);i--)
		{
			if (*(seg++) == RTC_MATCHWORD)
		        {
				tag = (struct Resident *)(--seg);
				if (tag->rt_MatchTag == tag)
				{
					return(tag);
				}
        		}
		}
	}

	return(NULL);
}

#define BUFSIZE		2052
#define VERSTR_LENGTH	256

BOOL
FindVerString (char *name,char *verstr)
{
	UBYTE *buffer;
	BOOL found = FALSE;
	BPTR file;
	LONG i,len;

	/* check for normal $VER: version string */

	if (buffer = AllocVec(BUFSIZE,MEMF_PUBLIC|MEMF_CLEAR))
	{
		if (file = Open(name,MODE_OLDFILE))
		{
			while (!found)
			{
				len = Read(file,buffer,BUFSIZE - 5);
				if (len <= 0)
					break;

				buffer[BUFSIZE - 4] = 0;

				for (i = 0;i < len - 15;i++)
				{
					if (buffer[i] == '$')
					{
						if (!strncmp(&buffer[i],"$VER:",5))
						{
							CopyVersionStr(
								&buffer[i + 5],
								verstr,
								VERSTR_LENGTH);
							found = TRUE;
							break;
						}
					}
				}
				if (len == BUFSIZE - 5)
					Seek(file,-25,OFFSET_CURRENT);
			}
			Close(file);
		}
		FreeVec(buffer);
	}
	return(found);
}

BOOL
GetFSVersion (char *FileSystemName,LONG *version,LONG *revision)
{
	// look for romtag in file
	BPTR seg;
	struct Resident *res;
	char *str;
	BOOL found = TRUE;
	char verstr[VERSTR_LENGTH];
	LONG i,len;

	verstr[0] = '\0';

	seg = LoadSeg(FileSystemName);
	if (seg)
	{
		res = FindRomTag(seg);
		if (res)
		{
			*version = res->rt_Version;

			// we must search for the '.' and get the rev number
			for (str = (char *) res->rt_IdString; *str; str++)
			if (*str == '.')
			{
				*revision = atol(str+1);
				break;
			}
		} else {
			// If we can't find out RomTag, we must search $VER string
			if (found = FindVerString(FileSystemName,verstr))
			{
				i = 0;
				while ((verstr[i] != ' ') && (verstr[i] != '\0'))
					i++;

				if (i > 0)
				{
					len = StrToLong(&verstr[i],version);
					if (len > 0)
					{
						i += len;
						if (verstr[i] == '.')
						{
							i++;
							len = StrToLong(&verstr[i],
									revision);
							if (len > 0)
								i += len;
						}
					}
				}
			}
		}
		UnLoadSeg(seg);
	} else
		found = FALSE;

	return(found);
}
/* 39.9 END */

