/* handletype.c - code to handle the set type screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

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

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "refresh.h"
#include "protos.h"
#include <clib/alib_protos.h>

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

//
extern struct Gagdget *glist5;
extern struct Gagdget *glist12;

USHORT SetTypeOKFlags  = 0;
USHORT DeleteTypeFlags = 0;
USHORT EditTypeFlags   = 0;

extern struct Remember	*RKey;
struct List	lh12;		/* List for DriveList */
UWORD TypeListPosition = 0;	/* highlight position */
UWORD SCSITypeActive   = 0;	/* active cycle position */

#define	TYPELIST_LENGTH 60	/* DriveList string length */
#define	SCSI_CYCLE	0
#define	ST506_CYCLE	1

#define SAME 0

char *ReallyDeleteType[] = {
	"Are you certain you wish to delete",
	"the current Drive Type?",
	NULL,
};

char *WriteError[] = {
	"A disk error may have destroyed your list of drives.",
	"A hopefully intact version is in T:prep_temp.",
	"You should copy it to somewhere safe, or restore your",
	"drive definitions list from the original Prep: disk.",
	NULL,
};

char *FSReadError[] = {
	"The file L:FastFileSystem cannot be found or has errors.",
	"The setup for the drive will not have a filesystem.  This",
	"may stop you from using the drive under AmigaDos 1.3 and before.",
	"You are advised to exit the program and install the file",
	"L:FastFileSystem, then try again.",
	NULL,
};

char *ChangeDriveText[] = {
	"Are you sure you want to change",
	"the drive type for the current drive?",
	" ",
	"(All old partitions on the drive will",
	"be lost if you save these changes.)",
	NULL,
};

char *TooManyBlocks[] = {
	"WARNING!  The drive type you selected has",
	"           ",
	"blocks, while the drive itself has only",
	"           ",
	"blocks.  Are you SURE you want to continue?",
	NULL,
};

int
HandleSetType (w,msg)
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
		case TYPELIST:
			/* figure out which one he clicked on */
			HandleTypeClick(w,msg);
			break;

		case DEFINEDRIVE:
			done = HandleWindowNew(w,NULL,
					&glist5,
					NewWindowStructure5.Title,
					NewWindowStructure5.IDCMPFlags,	// 39.10
					HandleDefine,
					NULL,//DefineDraw,
					DefineSetup,
					CreateAllGadgets5,
					&glist12,CreateAllGadgets12);

			/* forces first in list to be displayed */
			DisplayType = NULL;
			TypeListPosition = 0;
			HandleTypeListNew(w);	/* recalc the gadget, etc */
			break;

		case EDITTYPE:
			if (!CurrType)
			{
				Notify(w,"No Type selected",0);
				break;
			}
			done = HandleWindowNew(w,NULL,
					&glist5,
					NewWindowStructure5.Title,
					NewWindowStructure5.IDCMPFlags,	// 39.10
					HandleDefine,
					NULL,//DefineDraw,
					EditSetup,
					CreateAllGadgets5,
					&glist12,CreateAllGadgets12);

			/* CurrType should still be set */
			HandleTypeListNew(w);	/* recalc the gadget, etc */
			break;

		case DELETETYPE:
			if (!CurrType)
			{
				Notify(w,"No Type selected",0);
				break;
			}
			if (AskSure(w,&ReallyDeleteType[0]))
				DeleteDiskType(w,CurrType);
			break;

		case SCSITYPE:
			if (msg->Code == SCSI_CYCLE)
			{
				if (TypeType != SCSI)
				{
					TypeType    = SCSI;
					SCSITypeActive = SCSI_CYCLE;
					DisplayType = CurrType = NULL;
//					GadMXSet(w,&SCSIType);
//					GadMXClr(w,&St506Type);
					TypeListPosition = 0;
					HandleTypeListNew(w); /* rebuilds type list */
				}
			} else
			{
				if (TypeType != ST506)
				{
					TypeType    = ST506;
					SCSITypeActive = ST506_CYCLE;
					DisplayType = CurrType = NULL;
//					GadMXSet(w,&St506Type);
//					GadMXClr(w,&SCSIType);
					TypeListPosition = 0;
					HandleTypeListNew(w); /* rebuilds type list */
				}
			}
			break;

		case SETTYPEOK:
			if (ChangeDriveType(w,CurrType))
				done = HANDLE_DONE;
			break;

		case SETTYPECANCEL:
			done = HANDLE_CANCEL;
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

/* drawing routine for type, draws all non-text/gadget stuff on prep screen */

#if 0
void
TypeDraw (rp)
	struct RastPort *rp;
{
	redraw_drivetype(rp);
}
#endif

/* redraw the drive type list, highlight selected item if visible.  */
static int gottype;

void
redraw_drivetype (rp)
	register struct RastPort *rp;
{
	register UWORD i;
	register struct DriveDef *def;
	char buffer[TYPELIST_LENGTH];

	struct Node *node;

//	kprintf("redraw\n");
	gottype = 0;

	if (DisplayType == NULL)
		DisplayType = FirstType;

	if (RKey) FreeRemember(&RKey,TRUE);
	NewList(&lh12);

	for (def = DisplayType, i = 0;
	     def /*&& i < MaxDisplayTypes*/;
	     def = def->Succ,i++)
	{
		sprintf(buffer,"%-8.8s",
					def->Initial_RDB->rdb_DiskVendor);
		sprintf(buffer + strlen(buffer),"%-16.16s",
					def->Initial_RDB->rdb_DiskProduct);
		sprintf(buffer + strlen(buffer)," %-4.4s",
					def->Initial_RDB->rdb_DiskRevision);

//		kprintf("%s\n",buffer);

		// I have to allocate every time because switch scsi and xt.
		if (node = (struct Node *)
			AllocRemember(&RKey,sizeof(struct Node),MEMF_CLEAR))
		{
			if(node->ln_Name =
				AllocRemember(&RKey,TYPELIST_LENGTH + 1,MEMF_CLEAR))
			{
			    strcpy(node->ln_Name,buffer);
			    AddTail(&lh12,node);
			}
		}

		if (def == CurrType)
			gottype = 1;
	}
}


#if 0
void
redraw_drivetype (rp)
	register struct RastPort *rp;
{
	register UWORD x,y,i;
	register struct DriveDef *def;

	gottype = 0;

	x = TypeList.LeftEdge;
	y = TypeList.TopEdge;

	if (DisplayType == NULL)
		DisplayType = FirstType;

	/* clear to erase old text */
	SetAPen(rp,0L);
	SetDrMd(rp,JAM1);
	RectFill(rp, x, y, x + TypeList.Width  - 1,
			   y + TypeList.Height - 1);

	SetAPen(rp,1L);

	for (def = DisplayType, i = 0;
	     def && i < MaxDisplayTypes;
	     def = def->Succ,i++)
	{
		Move(rp,x,y + rp->TxBaseline + 1);
		WriteDiskName(rp,def->Initial_RDB);

		if (def == CurrType)
		{
			SetAPen(rp,255L);
			SetDrMd(rp,COMPLEMENT);

			RectFill(rp,x,y,
				    x + TypeList.Width - 1,
				    y + rp->TxHeight);
			SetAPen(rp,1L);
			SetDrMd(rp,JAM1);

			gottype = 1;
		}
		y += rp->TxHeight + 1;
	}
}
#endif

/* Allocate Type list for CreateGadget(). */

void AllocTypeList(void)		// Allocate rtn for LISTVIEW_KIND Gadgets
{
	register UWORD i;
	register struct DriveDef *def;
	char buffer[TYPELIST_LENGTH];
	struct Node *node;

	gottype = 0;

	if (DisplayType == NULL)
		DisplayType = FirstType;

	NewList(&lh12);

	for (def = DisplayType, i = 0;
	     def /*&& i < MaxDisplayTypes*/;
	     def = def->Succ,i++)
	{
		sprintf(buffer,"%-8.8s",
					def->Initial_RDB->rdb_DiskVendor);
		sprintf(buffer + strlen(buffer),"%-16.16s",
					def->Initial_RDB->rdb_DiskProduct);
		sprintf(buffer + strlen(buffer)," %-4.4s",
					def->Initial_RDB->rdb_DiskRevision);

		if (node = (struct Node *)
			AllocRemember(&RKey,sizeof(struct Node),MEMF_CLEAR))
		{
			if(node->ln_Name =
				AllocRemember(&RKey,TYPELIST_LENGTH + 1,MEMF_CLEAR))
			{
			    strcpy(node->ln_Name,buffer);
			    AddTail(&lh12,node);
			}
		}

	}
}

void
HandleTypeClick (w,msg)
	register struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct DriveDef *def;

	y = msg->Code;
	TypeListPosition = y;	// Remember the highlight position for CreateGadget().

	/* find def */
	for (def = DisplayType; def && y; def = def->Succ, y--)
		; /* NULL */

	if (def) {
		if (CurrType == def)
			return;
		CurrType = def;
	} else
		DisplayBeep(w->WScreen);
}

#if 0
void
HandleTypeClick (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register UWORD y;
	register struct DriveDef *def;

	y = (msg->MouseY - TypeList.TopEdge) / (w->RPort->TxHeight + 1);

	/* find def */
	for (def = DisplayType; def && y; def = def->Succ, y--)
		; /* NULL */
	if (!def)
		return;

	CurrType = def;

	MyOnGadget(w,&EditType);
	MyOnGadget(w,&DeleteType);
	if (SelectedDrive && ((TypeType & SelectedDrive->Flags) != 0))
	{
		MyOnGadget(w,&SetTypeOK);
	}

	redraw_drivetype(w->RPort);
}
#endif


/* Change the current drive type to def */

int
ChangeDriveType (w,def)
	struct Window *w;
	register struct DriveDef *def;
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	struct RigidDiskBlock *rdb = NULL;
	LONG i,opened = FALSE;
	LONG version = 0,revision = 0;	// 39.9

	if (!(def->ControllerType & (SelectedDrive->Flags & (SCSI|ST506))))
	{
		/* not the same type! */
		if (def->ControllerType & SCSI)
			Notify(w,"Wrong type for drive, use SCSI types",0);
		else
			Notify(w,"Wrong type for drive, use %s types",
			       (LONG) xt_name);
		return FALSE;
	}

	/* check if he used too many blocks */

	if (SelectedDrive->Flags & SCSI)
	{
		LONG blocks;

		blocks = ReadCapacity(w,SelectedDrive,0,0);
		if (blocks < def->Initial_RDB->rdb_Cylinders *
			     def->Initial_RDB->rdb_CylBlocks)
		{
			sprintf(TooManyBlocks[1],"%ld",
					def->Initial_RDB->rdb_Cylinders *
				 	def->Initial_RDB->rdb_CylBlocks);
			sprintf(TooManyBlocks[3],"%ld",blocks);

			if (!AskSure(w,TooManyBlocks))
				return FALSE;
		}
	}

	/* if we don't know what it is, don't ask */
	if ((!SelectedDrive->oldrdb && !SelectedDrive->rdb) ||
	    AskSure(w,ChangeDriveText))
	{
/* note: duplicate of this code is in readwrite.c */

		/* let's change the type */
		FreeRDB(SelectedDrive->rdb);
		FreeBad(SelectedDrive->bad);

		SelectedDrive->bad   = NULL;
		SelectedDrive->rdb   = CopyRDB(def->Initial_RDB);
		if (!SelectedDrive->rdb)
			return FALSE;

		SelectedDrive->Flags = def->ControllerType;
		SelectedDrive->Block = 0;

		DoQuickSetup(SelectedDrive);

		GetFSVersion("l:FastFileSystem",&version,&revision);	// 39.9

		/* add FFS to the drive */
		SelectedDrive->rdb->rdb_FileSysHeaderList =
		      MakeFileSys(NULL,SelectedDrive->rdb,
				  "L:FastFileSystem",'DOS\x01',version,revision);
		if (!SelectedDrive->rdb->rdb_FileSysHeaderList)
			LongNotify(w,FSReadError);

		SelectedDrive->Flags |= UPDATE;

#define STUPID_WD_XT_DRIVE

#ifdef STUPID_WD_XT_DRIVE
		/* STUPID wd xt drives run in non-translate mode which means  */
		/* I have to write the definition and tell the driver to read */
		/* it before I can use verify/lowlevelformat, etc on the drive*/

		if ((SelectedDrive->Flags & ST506) &&
		    !SelectedDrive->oldrdb)
		{
			if (!(port = CreatePort(0L,0L)))
				goto cleanup;
			if (!(ior = CreateStdIO(port)))
				goto cleanup;
			if (i = OpenDevice(SelectedDrive->DeviceName,
				           SelectedDrive->Addr +
				           SelectedDrive->Lun * 10,
				           (struct IORequest *) ior,0L))
			{
				Notify(w,"Error %ld on open of device!",i);
				goto cleanup;
			}
			opened = TRUE;

			rdb = (struct RigidDiskBlock *)
			      AllocMem(SelectedDrive->rdb->rdb_BlockBytes,
				       MEMF_PUBLIC|MEMF_CLEAR);
			if (!rdb)
				goto cleanup;

			/* make a rigid disk block I can write out */
			*rdb = *(SelectedDrive->rdb);

			/* nothing but bare RDSK */
			rdb->rdb_BadBlockList  = (struct BadBlockBlock *) -1;
			rdb->rdb_PartitionList = (struct PartitionBlock *) -1;
			rdb->rdb_FileSysHeaderList =
					       (struct FileSysHeaderBlock *) -1;
			rdb->rdb_DriveInit     = (struct LoadSegBlock *) -1;
			for (i = 0; i < 6; i++)
				rdb->rdb_Reserved1[i] = -1;

			CheckSumBlock(rdb);

			ior->io_Command = CMD_WRITE;
			ior->io_Data    = (APTR) rdb;
			ior->io_Length  = rdb->rdb_BlockBytes;
			ior->io_Offset  = 0;

			DoIO((struct IORequest *) ior);

			CloseDevice((struct IORequest *) ior);
			opened = FALSE;

			/* make the driver re-read the disk */

			/* flags of 1 makes A590 drive reread disk */
			if (!OpenDevice(SelectedDrive->DeviceName,
				       SelectedDrive->Addr +
				       SelectedDrive->Lun * 10,
				       (struct IORequest *) ior,1L))
				opened = TRUE;

cleanup:
			if (rdb)
				FreeMem((char *) rdb,
					SelectedDrive->rdb->rdb_BlockBytes);
			if (opened)
				CloseDevice((struct IORequest *) ior);
			if (ior)
				DeleteStdIO(ior);
			if (port)
				DeletePort(port);
		}
#endif
		return TRUE;
	}

	return FALSE;
}

/* delete a type from the drive, if possible */
/* I WANT A GODDAMN TRUNCATE() DOS CALL!!!!! */

void
DeleteDiskType (w,def)
	struct Window   *w;
	struct DriveDef *def;
{
	struct DriveDef *def2;
	struct DiskDrive dd;
	BPTR fh,ram;
	ULONG offset;
	long len;

	/* they have confirmed they wish to delete */
	if (fh = Open(def->Filename,MODE_OLDFILE))
	{
	   if (ram = Open("t:prep_temp",MODE_NEWFILE))
	   {
		while (1) {
			offset = Seek(fh,0L,OFFSET_CURRENT);
			if ((len =
			     Read(fh,(char *) &dd,sizeof(dd))) != sizeof(dd))
			{
				if (len == 0)	/* EOF */
					break;
				ReadError(w,len);
				goto closeit;
			}
			if (dd.Ident != IDNAME_DRIVE)
			{
				Notify(w,"Not a drive type file!",0);
				goto closeit;
			}

			/* don't copy the one being deleted */

			if (offset != def->Offset &&
			    Write(ram,(char *) &dd,sizeof(dd)) != sizeof(dd))
			{
				Notify(w,"Error %ld on write to temp file!",
				       IoErr());
				goto closeit;
			}
		}

		/* file copied to t:, now write over original */
		Seek(ram,0L,OFFSET_BEGINNING);

		Close(fh);
		if (!(fh = Open(&def->Filename[0],MODE_NEWFILE)))
		{
			/* really bad error */
			Notify(w,"Couldn't reopen %s!",(LONG) def->Filename);
			goto writeerror;
		}

		while (1) {
			if ((len =
			     Read(ram,(char *) &dd,sizeof(dd))) != sizeof(dd))
			{
				if (len == 0)	/* EOF */
					break;
				ReadError(w,len);
				goto writeerror;
			}
			if (Write(fh,(char *) &dd,sizeof(dd)) != sizeof(dd))
			{
			       Notify(w,"Error on write to drive type file!",0);
			       goto writeerror;
			}
		}

		/* success! Delete it from list! */
		if (FirstType == def)
			FirstType = def->Succ;

		/* unlink */
		if (def->Prev)
			def->Prev->Succ = def->Succ;
		if (def->Succ)
			def->Succ->Prev = def->Prev;

		if (DisplayType == def)
		{
			DisplayType    = NULL;
			DisplayTypeNum = 0;
		}

		/* reduce count, fix lists */
		if (def->ControllerType == ST506)
		{
			if (--St506_DefNum == 0)
				St506_Defs = NULL;
			if (St506_Defs == def)
				St506_Defs = def->Succ;
		} else {
			if (--SCSI_DefNum == 0)
				SCSI_Defs  = NULL;
			if (SCSI_Defs == def)
				SCSI_Defs = def->Succ;
		}

		/* fix all offsets */
		for (def2 = St506_Defs; def2; def2 = def2->Succ)
		{
			if (def2->Offset > def->Offset) /* note: not equal */
				def2->Offset -= sizeof(dd);
		}
		for (def2 = SCSI_Defs; def2; def2 = def2->Succ)
		{
			if (def2->Offset > def->Offset) /* note: not equal */
				def2->Offset -= sizeof(dd);
		}

		/* none selected */
//		CurrType = NULL;	// Bug Fix!

		if (def->Succ)	// Next is not NULL, we put next pointer to current.
			CurrType = def->Succ;
		else		// Next is NULL, put previous data to current.
			CurrType = def->Prev;
//
//		sprintf(tmp,"%x",def);
//		kprintf("def = %s\n",tmp);
//		sprintf(tmp,"%x",CurrType);
//		kprintf("CurrType = %s\n",tmp);

		/* rebuilds drag bar, redisplays, resets NumTypes */
		HandleTypeListNew(w);

		/* free the def */
		FreeRDB(def->Initial_RDB);
		FreeMem((char *) def, sizeof(*def));

closeit:
		Close(ram);
		DeleteFile("t:prep_temp");
	    } else
		Notify(w,"Can't open temp file \"t:prep_temp\"!",0);

	    Close(fh);

	} else
		Notify(w,"Can't open file %s!",(LONG) def->Filename);
	return;

writeerror:	/* error that may have killed the save file */
	LongNotify(w,WriteError);

	if (fh)
		Close(fh);
	Close(ram);		/* NO DeleteFile! */
}


/* handle the list of drive types known */

void
HandleTypeListNew (w)
	struct Window *w;
{
	struct DriveDef *old = DisplayType;

	if (!old)
	{
//	I don't know why CurrType = NULL ? I think this is original bug.
//	Change drive type. We can see disable information is not correct.
//	See redraw_drivetype(), gotit is never set 1.
//		CurrType  = NULL;
//		FirstType = DisplayType =
		CurrType = FirstType = DisplayType =
				(TypeType == SCSI ? SCSI_Defs : St506_Defs);
		DisplayTypeNum = 0;
		NumTypes = (TypeType == SCSI ? SCSI_DefNum : St506_DefNum);
	}

	redraw_drivetype(w->RPort);

	GT_SetGadgetAttrs(TypeList,w,NULL,GTLV_Labels,&lh12,TAG_DONE);
	GT_SetGadgetAttrs(TypeList,w,NULL,GTLV_Selected,TypeListPosition,TAG_DONE);

	if (gottype) {
		MyOnGadget(w,EditType);
		MyOnGadget(w,DeleteType);
		MyOnGadget(w,SetTypeOK);
	} else {
		MyOffGadget(w,EditType);
		MyOffGadget(w,DeleteType);
		MyOffGadget(w,SetTypeOK);
	}

}

#if 0
void
HandleTypeList (w,id)
	struct Window *w;
	LONG id;
{
	register struct DriveDef *def;
		 struct DriveDef *old = DisplayType;
	register ULONG i,j;
	LONG range;

	if (!old)
	{
		CurrType  = NULL;
		FirstType = DisplayType =
				(TypeType == SCSI ? SCSI_Defs : St506_Defs);
		DisplayTypeNum = 0;
		NumTypes = (TypeType == SCSI ? SCSI_DefNum : St506_DefNum);
	}

	/* if rebuilding, don't do this code */
	if (id != -1 && old && NumTypes <= MaxDisplayTypes)
	{
		if (DisplayTypeNum != 0)
		{
			DisplayTypeNum = 0;
			DisplayType    = FirstType;
			redraw_drivetype(w->RPort);
		}
		return;
	}

	switch (id) {
	case TYPEUP:
		if (old->Prev)
		{
			DisplayType = old->Prev;
			DisplayTypeNum--;
		}
		break;

	case TYPEDOWN:
		if (old->Succ && DisplayTypeNum < NumTypes-MaxDisplayTypes)
		{
			DisplayType = old->Succ;
			DisplayTypeNum++;
		}
		break;

	case TYPEDRAG:
		i = (TypeDragSInfo.VertPot + 1L) *
		    max(((LONG) NumTypes - MaxDisplayTypes),0);
		i >>= 16;

		for (def = FirstType, j = 0;
		     def->Succ && j < i;
		     def = def->Succ, j++)
			; /* NULL */

		if (def) {
			DisplayType    = def;
			DisplayTypeNum = j;
		}
		break;
	case -1:
		break;
	}

	if (id == -1 || old != DisplayType)
	{
		redraw_drivetype(w->RPort);
		if (!gottype)
		{
			OffGadget(&EditType,w,NULL);
			OffGadget(&DeleteType,w,NULL);
			OffGadget(&SetTypeOK,w,NULL);
		}

		if (id != TYPEDRAG)
		{
		    range = (LONG) NumTypes - (LONG) MaxDisplayTypes;

		    if (range <= 0)
			NewModifyProp(&TypeDrag,w,NULL,TypeDragSInfo.Flags,
			      MAXPOT,0,
			      TypeDragSInfo.HorizBody,0xFFFF,1L);
		    else
			NewModifyProp(&TypeDrag,w,NULL,TypeDragSInfo.Flags,
			      MAXPOT,
			      (0xFFFF * min(DisplayTypeNum,range)) / range,
			      TypeDragSInfo.HorizBody,
			      range ? (0xFFFF * MaxDisplayTypes) / NumTypes :
				      0xFFFF,
			      1L);
		    RefreshGList(&TypeDrag,w,NULL,1L);
		}
	}
}
#endif


int
TypeSetup (w)
	struct Window *w;
{
	/* which list of drives */
	TypeType  = SelectedDrive->Flags & (SCSI|ST506);
	FirstType = DisplayType = (TypeType == SCSI ? SCSI_Defs : St506_Defs);
	NumTypes  = (TypeType == SCSI ? SCSI_DefNum : St506_DefNum);
//	CurrType  = NULL;
	CurrType = FirstType;	// Bug fix! No longer ghosted gagdtes
	DisplayTypeNum = 0;
	TypeListPosition = 0;	//

	if (TypeType == SCSI)
		SCSITypeActive = SCSI_CYCLE;
	else
		SCSITypeActive = ST506_CYCLE;

// Bug fix! No longer ghosted gagdtes
	if (CurrType == NULL) {
		SetTypeOKFlags  |= GADGDISABLED;
		EditTypeFlags   |= GADGDISABLED;
		DeleteTypeFlags |= GADGDISABLED;
	} else {
		SetTypeOKFlags  &= ~GADGDISABLED;
		EditTypeFlags   &= ~GADGDISABLED;
		DeleteTypeFlags &= ~GADGDISABLED;
	}

	return TRUE;
}

#if 0
int
TypeSetup (w)
	struct Window *w;
{
	/* which list of drives */
	TypeType  = SelectedDrive->Flags & (SCSI|ST506);
	FirstType = DisplayType = (TypeType == SCSI ? SCSI_Defs : St506_Defs);
	NumTypes  = (TypeType == SCSI ? SCSI_DefNum : St506_DefNum);
	CurrType  = NULL;
	DisplayTypeNum = 0;

	SCSIType.Flags  &= ~SELECTED;
	St506Type.Flags &= ~SELECTED;

	if (TypeType == SCSI)
		SCSIType.Flags |= SELECTED;
	else
		St506Type.Flags |= SELECTED;

	SetTypeOK.Flags  |= GADGDISABLED;
	EditType.Flags   |= GADGDISABLED;
	DeleteType.Flags |= GADGDISABLED;

	MaxDisplayTypes  = TypeList.Height  / (w->RPort->TxHeight + 1);

	/* set up the prop gadget!! */
	/* equivalent to the NewModifyProp in HandleTypeList */

	TypeDragSInfo.HorizPot  = MAXPOT;
	TypeDragSInfo.VertPot   = NumTypes > MaxDisplayTypes ?
				  (0xFFFF * min(0,NumTypes-MaxDisplayTypes)) /
				  (NumTypes - MaxDisplayTypes) : 0;
	TypeDragSInfo.VertBody  = NumTypes ? (0xFFFF *
					      min(MaxDisplayTypes,NumTypes)) /
					     NumTypes : 0xFFFF;

	return TRUE;
}
#endif


/* writes the rdb disk name at the current position */

#if 0
void
WriteDiskName (rp,rdb)
	register struct RastPort *rp;
	register struct RigidDiskBlock *rdb;
{
	if (rdb)
	{
		Text(rp,rdb->rdb_DiskVendor,8);
		Move(rp,rp->cp_x + rp->TxWidth,rp->cp_y);
		Text(rp,rdb->rdb_DiskProduct,16);
		Move(rp,rp->cp_x + rp->TxWidth,rp->cp_y);
		Text(rp,rdb->rdb_DiskRevision,4);
	}
}
#endif

/* two identical rdb disk names? */

int
SameDrive (rdb1, rdb2)
	register struct RigidDiskBlock *rdb1,*rdb2;
{
	if (!rdb1 || !rdb2)
		return FALSE;

	if (memcmp(rdb1->rdb_DiskVendor,rdb2->rdb_DiskVendor,8) == SAME &&
	    memcmp(rdb1->rdb_DiskProduct,rdb2->rdb_DiskProduct,16) == SAME &&
	    memcmp(rdb1->rdb_DiskRevision,rdb2->rdb_DiskRevision,4) == SAME)
		return TRUE;

	return FALSE;
}

/* report a read error to the user */

void
ReadError (w,len)
	struct Window *w;
	LONG len;
{
	if (len == -1)
		Notify(w,"Error %ld on read!",IoErr());
	else
		Notify(w,"File is the wrong length!",0);
}
