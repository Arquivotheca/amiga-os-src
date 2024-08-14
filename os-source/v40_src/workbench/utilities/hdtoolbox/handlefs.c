/* handleFS.c - code to handle the file system screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>
#include <resources/filesysres.h>

#include <dos.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"
#include "partglob.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

#include "fstype.h"

USHORT BuffersFlags           = 0; // maybe no need this here, transfer to handlepart.c
USHORT FSBlockSizeFlags       = 0;
USHORT FSIdentifierFlags      = 0;
USHORT MaskFlags              = 0;
USHORT MaxTransferFlags       = 0;
USHORT CustomNumFlags         = 0;
USHORT ReservedEndFlags       = 0;
USHORT ReservedBeginFlags     = 0;
USHORT FFSCheckFlags          = 0;
USHORT IntlModeFlags          = 0;
USHORT DirCacheFlags          = 0;
USHORT AutoMountFlags         = 0;
USHORT CustomBootFlags        = 0;

LONG ReservedBeginSInfoLongInt = 0;
LONG ReservedEndSInfoLongInt   = 0;
LONG CustomNumSInfoLongInt     = 0;

char PartNameText[44];

UWORD FSTypeActive      = 0;	/* fstype active cycle position      */
UWORD FSTypeSave        = 0;	/* save original fs position         */
UWORD FSBlockSizeActive = 0;	/* block size active cycle position  */

struct FSTypeLabels FSTypeLabels;
STRPTR FSBlockSizeLabels[] =			/* block size labels */
{
	"512",
        "1024",
	"2048",
	"4096",
	"8192",
	"16384",
	"32768",
	NULL,
};

#define VALID		0x8000

UWORD NumFSType;		/* File system total number */

/* 39.14 - Determine user select "Intenational" or "Dir Cache" select. */
LONG IntlModeUserCheck = FALSE;

/* 39.18 - for updating fs PatchFlag. This stuff had been in "handlefsm.c",
 now I put this stuff here for avoiding user confusion. */
extern WORD FileSysUpdate;

int
HandleFileSys (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct Gadget *gad;
	register int done = FALSE;
	UWORD id;
	char *ptr;
	LONG x;

	switch (msg->Class)
	{
	case GADGETUP:
	case GADGETDOWN:
		gad = (struct Gadget *) (msg->IAddress);

		id  = gad->GadgetID & ~REFRESH_MASK;

		switch (id)
		{
		case FSTYPE:
			FSTypeActive = msg->Code;
			switch(msg->Code)
			{
				case STANDARD_CYCLE:
					DeselectFS(w,DefaultSDFS);
					break;
				case CUSTOM_CYCLE:
					DeselectFS(w,DefaultCustom);
					break;
				default:
					DeselectFS(w,DefaultUser);
					break;
			}
			SetFSBS(w,(CurrentPart->pb_Environment[DE_SIZEBLOCK]*4)*CurrentPart->pb_Environment[DE_SECSPERBLK]);
			break;
		case FSBLOCKSIZE:
			FSBlockSizeActive = msg->Code;

			/* 39.16 --- divide blocksize by sectorsize for sectors/block */
			x = atoi(FSBlockSizeLabels[FSBlockSizeActive]);
			x = x / (CurrentPart->pb_Environment[DE_SIZEBLOCK]*4);

			if (x != CurrentPart->pb_Environment[DE_SECSPERBLK])
			{
				CurrentPart->pb_Environment[DE_SECSPERBLK] = x;
				FSUpdate = TRUE;
			}

			/* 39.18 */
			if (!(FSTypeLabels.blocksizeflags[FSTypeActive] & VALID))
			{
				UpdatePatchFlags(CurrentPart->pb_Environment[DE_DOSTYPE],TRUE);
				FileSysUpdate = TRUE;
				FSTypeLabels.blocksizeflags[FSTypeActive] |= VALID;
			}
			break;
		case FFSCHK:
			ptr = &FSIdentifierSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				if (gad->Flags & SELECTED)
				{
					x |= FFS_MASK;
					FFSCheckFlags |= SELECTED;
				}
				else
				{
					x &= ~FFS_MASK;
					FFSCheckFlags &= ~SELECTED;
				}
				if (x != CurrentPart->pb_Environment[DE_DOSTYPE])
					UpdateIdentifier(w,x);
			}
			break;
		case INTLMODE:
			ptr = &FSIdentifierSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				if (gad->Flags & SELECTED)
				{
					x |= INTER_MASK;
					IntlModeFlags |= SELECTED;
					/* 39.14 - Set international mode flag for selecting
					dir cache. */
					IntlModeUserCheck = TRUE;
				}
				else
				{
					x &= ~INTER_MASK;
					IntlModeFlags &= ~SELECTED;
					/* 39.14 - Clear international mode flag for selecting
					dir cache. */
					IntlModeUserCheck = FALSE;
				}
				if (x != CurrentPart->pb_Environment[DE_DOSTYPE])
					UpdateIdentifier(w,x);
			}
			break;
		case DIRCACHE:
			ptr = &FSIdentifierSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				if (gad->Flags & SELECTED)
				{
					DirCacheFlags |= SELECTED;
					/* Dir cache make automatically International mode */
					GT_SetGadgetAttrs(IntlMode,w,NULL,
						GTCB_Checked,TRUE,
						GA_Disabled,TRUE,TAG_DONE);
					IntlModeFlags |= (SELECTED|GADGDISABLED);
					x &= ~(INTER_MASK);
					x |= (DC_MASK);
				}
				else
				{
					DirCacheFlags &= ~SELECTED;
					/* 39.14 */
					if (!IntlModeUserCheck)
					{
						GT_SetGadgetAttrs(IntlMode,w,NULL,
							GTCB_Checked,FALSE,TAG_DONE);
						IntlModeFlags &= ~SELECTED;
        				}
					GT_SetGadgetAttrs(IntlMode,w,NULL,
						GA_Disabled,FALSE,TAG_DONE);
					IntlModeFlags &= ~GADGDISABLED;
					x &= ~(DC_MASK);
					if (IntlModeUserCheck)	/* 39.14 */
						x |= (INTER_MASK);
				}
				if (x != CurrentPart->pb_Environment[DE_DOSTYPE])
					UpdateIdentifier(w,x);
			}
			break;
		case MASK:
			ptr = &MaskSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				/* 39.13 - No need check any more */
//				if (x & 1) /* odd */
//				{
//					x &= ~1;
//					sprintf(&MaskSIBuff[0],"0x%lx",x);
//					RefreshGList(Mask,w,NULL,1L);
//				}
				if (x != CurrentPart->pb_Environment[DE_MASK])
				{
					CurrentPart->pb_Environment[DE_MASK] = x;
					FSUpdate = TRUE;
				}
			} else
			{
				DisplayBeep(w->WScreen);
				sprintf(&MaskSIBuff[0],"0x%lx",
				       CurrentPart->pb_Environment[DE_MASK]);
				RefreshGList(Mask,w,NULL,1L);
			}
			break;
		case MAXTRANSFER:
			ptr = &MaxTransferSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				if (x != CurrentPart->pb_Environment[DE_MAXTRANSFER])
				{
					CurrentPart->pb_Environment[DE_MAXTRANSFER] = x;
					FSUpdate = TRUE;
				}
			} else
			{
				DisplayBeep(w->WScreen);
				sprintf(&MaxTransferSIBuff[0],"0x%lx",
				   CurrentPart->pb_Environment[DE_MAXTRANSFER]);
				RefreshGList(MaxTransfer,w,NULL,1L);
			}
			break;
		case RESERVEDEND:
			ChangeEnvNum(DE_PREFAC,gad/*ReservedEnd*/);
			break;
		case RESERVEDBEGIN:
			ChangeEnvNum(DE_RESERVEDBLKS,gad/*ReservedBegin*/);
			break;
		case FSIDENTIFIER:
			ptr = &FSIdentifierSIBuff[0];
			if (*ptr++ == '0' && *ptr++ == 'x' &&
			    stch_l(ptr,&x) == strlen(ptr))
			{
				if (x != CurrentPart->pb_Environment[DE_DOSTYPE])
				{
					CurrentPart->pb_Environment[DE_DOSTYPE] = x;
					FSUpdate = TRUE;
				}
			}
			else
			{
				DisplayBeep(w->WScreen);
				sprintf(&FSIdentifierSIBuff[0],"0x%lx",
					CurrentPart->pb_Environment[DE_DOSTYPE]);
				RefreshGList(FSIdentifier,w,NULL,1L);
			}
			break;
		case AUTOMOUNT:
			if (gad->Flags & SELECTED)
				CurrentPart->pb_Flags &= ~PBFF_NOMOUNT;
			else
				CurrentPart->pb_Flags |= PBFF_NOMOUNT;
			FSUpdate = TRUE;
			break;
		case CUSTOMBOOT:
			if (gad->Flags & SELECTED)
			{
				CurrentPart->pb_Environment[DE_TABLESIZE]  =
								DE_BOOTBLOCKS;
				MyOnGadget(w,CustomNum);
			} else {
				CurrentPart->pb_Environment[DE_TABLESIZE]  = 16;
				CurrentPart->pb_Environment[DE_BOOTBLOCKS] = 0;
				MyOffGadget(w,CustomNum);
				GT_SetGadgetAttrs(CustomNum,w,NULL,
					GTIN_Number,0,TAG_DONE);
			}
			FSUpdate = TRUE;
			break;
		case CUSTOMNUM:
			ChangeEnvNum(DE_BOOTBLOCKS,gad/*CustomNum*/);
			break;

		case FSCANCEL:
			*CurrentPart = savecurrpart;	/* restore partition */
			FreeFSType(FALSE);
			done = HANDLE_CANCEL;
			break;
		case FSOK:
			if (FileSysUpdate)	/* 39.18 - for PatchFlags update */
				SelectedDrive->Flags |= UPDATE;

			if (FSUpdate)
				PartUpdate |= UPDATE;
			FreeFSType(TRUE);
			done = HANDLE_DONE;
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

/* 39.18 */

void
UpdatePatchFlags (DosType, Enable)
	LONG DosType;
	WORD Enable;
{
	register struct FileSysHeaderBlock *fs;

	for (fs = FirstFileSys;
	     fs;
	     fs = fs->fhb_Next)
	{
		if (IsThisStandardFS(DosType))
		{
			if (IsThisStandardFS(fs->fhb_DosType))
			{
				if (Enable)
					fs->fhb_PatchFlags &= ~(1<<DE_SECSPERBLK);
				else
					fs->fhb_PatchFlags |= (1<<DE_SECSPERBLK);
//				printf("Update PatchFlags = %x\n",fs->fhb_PatchFlags & (1<<DE_SECSPERBLK));
				break;
			}
		}
		else if (fs->fhb_DosType == DosType)
		{
			if (Enable)
				fs->fhb_PatchFlags &= ~(1<<DE_SECSPERBLK);
			else
				fs->fhb_PatchFlags |= (1<<DE_SECSPERBLK);
//			printf("Update PatchFlags = %x\n",fs->fhb_PatchFlags & (1<<DE_SECSPERBLK));
			break;
		}
	}
}

/* 39.18 */

void
SetFSBS(w,blocksize)
	struct Window *w;
	LONG blocksize;
{
	LONG x;
	STRPTR *tmp;
	x = 0;
	for (tmp = FSBlockSizeLabels;
	    *tmp;
	    tmp++,x++)
	{
		if (atoi(*tmp) == blocksize)
		{
			if (FSBlockSize)
				GT_SetGadgetAttrs(FSBlockSize,w,NULL,
					GTCY_Active,x,
					TAG_DONE);
			FSBlockSizeActive = x;
			break;
		}
	}
	if (*tmp == NULL) /* did not match */
	{
		if (FSBlockSize)
			GT_SetGadgetAttrs(FSBlockSize,w,NULL,
				GTCY_Active,0,
				TAG_DONE);
		FSBlockSizeActive = 0;
	}
}

void
SetSDFSGadsAttrs (w,ffs,intl,dircache)
	struct Window *w;
	LONG ffs;
	LONG intl;
	LONG dircache;
{
	if (ffs & SELECTED)
	{
		if (FFSCheck)
			GT_SetGadgetAttrs(FFSCheck,w,NULL,GTCB_Checked,TRUE,TAG_DONE);
		FFSCheckFlags |= SELECTED;
	}
        else
	{
		if (FFSCheck)
			GT_SetGadgetAttrs(FFSCheck,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		FFSCheckFlags &= ~SELECTED;
	}
	if (intl & SELECTED)
	{
		if (IntlMode)
			GT_SetGadgetAttrs(IntlMode,w,NULL,GTCB_Checked,TRUE,TAG_DONE);
		IntlModeFlags |= SELECTED;
	}
        else
	{
		if (IntlMode)
			GT_SetGadgetAttrs(IntlMode,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		IntlModeFlags &= ~SELECTED;
	}
	if (dircache & SELECTED)
	{
		if (DirCache)
			GT_SetGadgetAttrs(DirCache,w,NULL,GTCB_Checked,TRUE,TAG_DONE);
		DirCacheFlags |= SELECTED;
	}
        else
	{
		if (DirCache)
			GT_SetGadgetAttrs(DirCache,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		DirCacheFlags &= ~GADGDISABLED;
	}
	if (ffs & GADGDISABLED)
	{
		if (FFSCheck)
			GT_SetGadgetAttrs(FFSCheck,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		FFSCheckFlags |= GADGDISABLED;
	}
        else
	{
		if (FFSCheck)
			GT_SetGadgetAttrs(FFSCheck,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		FFSCheckFlags &= ~GADGDISABLED;
	}
	if (intl & GADGDISABLED)
	{
		if (IntlMode)
			GT_SetGadgetAttrs(IntlMode,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		IntlModeFlags |= GADGDISABLED;
	}
        else
	{
		if (IntlMode)
			GT_SetGadgetAttrs(IntlMode,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		IntlModeFlags &= ~GADGDISABLED;
	}
	if (dircache & GADGDISABLED)
	{
		if (DirCache)
			GT_SetGadgetAttrs(DirCache,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		DirCacheFlags |= GADGDISABLED;
	}
        else
	{
		if (DirCache)
			GT_SetGadgetAttrs(DirCache,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		DirCacheFlags &= ~GADGDISABLED;
	}
}

void
SetMiscGadsAttrs (w,ident,rsvend,mask,maxtransfer)
	struct Window *w;
	LONG ident;
	LONG rsvend;
	LONG mask;
	LONG maxtransfer;
{
	if (ident & GADGDISABLED)
	{
		if (FSIdentifier)
			GT_SetGadgetAttrs(FSIdentifier,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		FSIdentifierFlags |= GADGDISABLED;
	}
        else
	{
		if (FSIdentifier)
			GT_SetGadgetAttrs(FSIdentifier,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		FSIdentifierFlags &= ~GADGDISABLED;
	}
	if (rsvend & GADGDISABLED)
	{
		if (ReservedEnd)
			GT_SetGadgetAttrs(ReservedEnd,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		ReservedEndFlags |= GADGDISABLED;
	}
        else
	{
		if (ReservedEnd)
			GT_SetGadgetAttrs(ReservedEnd,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		ReservedEndFlags &= ~GADGDISABLED;
	}
	if (mask & GADGDISABLED)
	{
		if (Mask)
			GT_SetGadgetAttrs(Mask,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		MaskFlags |= GADGDISABLED;
	}
        else
	{
		if (Mask)
			GT_SetGadgetAttrs(Mask,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		MaskFlags &= ~GADGDISABLED;
	}
	if (maxtransfer  & GADGDISABLED)
	{
		if (MaxTransfer)
			GT_SetGadgetAttrs(MaxTransfer,w,NULL,GA_Disabled,TRUE,TAG_DONE);
		MaxTransferFlags |= GADGDISABLED;
	}
        else
	{
		if (MaxTransfer)
			GT_SetGadgetAttrs(MaxTransfer,w,NULL,GA_Disabled,FALSE,TAG_DONE);
		MaxTransferFlags &= ~GADGDISABLED;
	}
}

/* update identifier and refresh gadget */

void
UpdateIdentifier (w,x)
	struct Window *w;
	LONG x;
{
	CurrentPart->pb_Environment[DE_DOSTYPE] = x;
	sprintf(&FSIdentifierSIBuff[0],"0x%lx",
		CurrentPart->pb_Environment[DE_DOSTYPE]);
	GT_SetGadgetAttrs(FSIdentifier,w,NULL,
		GTST_String,FSIdentifierSIBuff,TAG_DONE);

	switch(x)
	{
		case ID_DOS_DISK:
			SetMiscGadsAttrs(w,GADGDISABLED,GADGDISABLED,
					   GADGDISABLED,GADGDISABLED);
			break;
		case ID_FFS_DISK:
			SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
		case ID_INTER_DOS_DISK:
			SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
		case ID_INTER_FFS_DISK:
			SetMiscGadsAttrs(w ,GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
		case ID_DC_DOS_DISK:
			SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
		case ID_DC_FFS_DISK:
			SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
		default:
			SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
					   ~GADGDISABLED,~GADGDISABLED);
			break;
	}

	FSUpdate = TRUE;
}

/* handle mutual exclusion of FS gadgets */

void
DeselectFS (w,resetrtn)
	struct Window *w;
	void (*resetrtn)(struct Window *);
{
	(*resetrtn)(w);
	RefreshGList(FSIdentifier,w,NULL,1L);
	RefreshGList(Mask,w,NULL,1L);
	RefreshGList(MaxTransfer,w,NULL,1L);
	RefreshGList(ReservedBegin,w,NULL,1L);
	RefreshGList(ReservedEnd,w,NULL,1L);
	RefreshGList(AutoMount,w,NULL,1L);
	FSUpdate = TRUE;
}

/* Standard File System - modified from DefaultFFS() */

void
DefaultSDFS (w)
	struct Window *w;
{
	/* if original file system user set was standard file system,
	  provide that information instead of initializing. */
	if (FSTypeSave == 0)
		*CurrentPart = savecurrpart;
	else
		InitFFS(CurrentPart);

	switch(CurrentPart->pb_Environment[DE_DOSTYPE])
	{
		case ID_DOS_DISK:
			SetSDFSGadsAttrs(w,~SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w,GADGDISABLED,
					   GADGDISABLED,
					   GADGDISABLED,
					   GADGDISABLED);
			break;
		case ID_FFS_DISK:
			SetSDFSGadsAttrs(w, SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w, GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
		case ID_INTER_DOS_DISK:
			SetSDFSGadsAttrs(w,~SELECTED & ~GADGDISABLED,
					    SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w, GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
		case ID_INTER_FFS_DISK:
			SetSDFSGadsAttrs(w, SELECTED & ~GADGDISABLED,
					    SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w ,GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
		case ID_DC_DOS_DISK:
			SetSDFSGadsAttrs(w,~SELECTED & ~GADGDISABLED,
					    SELECTED |  GADGDISABLED,
					    SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w, GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
		case ID_DC_FFS_DISK:
			SetSDFSGadsAttrs(w, SELECTED & ~GADGDISABLED,
					    SELECTED |  GADGDISABLED,
					    SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w, GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
		default:
			SetSDFSGadsAttrs(w, SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED,
					   ~SELECTED & ~GADGDISABLED);
			SetMiscGadsAttrs(w, GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED,
					   ~GADGDISABLED);
			break;
	}

	RebuildFSGads(w,CurrentPart);

//	BuffersFlags &= ~GADGDISABLED;
}

/* Custom File System */

void
DefaultCustom (w)
	struct Window *w;
{
	/* if original file system user set was user custom file system,
	  provide that information instead of initializing */
	if (FSTypeSave == 1)
		*CurrentPart = savecurrpart;
	else
		InitCustom(CurrentPart);

	RebuildFSGads(w,CurrentPart);
	SetMiscGadsAttrs(w,~GADGDISABLED,~GADGDISABLED,
			   ~GADGDISABLED,~GADGDISABLED);

//	BuffersFlags &= ~GADGDISABLED;
//	MyOnGadget(w,ReservedBegin);

	/* Set default value for standard file system gadgets */
	SetSDFSGadsAttrs(w,~SELECTED|GADGDISABLED,
			   ~SELECTED|GADGDISABLED,
			   ~SELECTED|GADGDISABLED);
}

/* File System which user add */

void
DefaultUser (w)
	struct Window *w;
{
	/* if original file system user set was user special file system,
	  provide that information instead of initializing */
	if (savecurrpart.pb_Environment[DE_DOSTYPE] ==
				FSTypeLabels.identifier[FSTypeActive])
		*CurrentPart = savecurrpart;
	else
		InitUser(CurrentPart);

	RebuildFSGads(w,CurrentPart);

	SetMiscGadsAttrs(w, GADGDISABLED,~GADGDISABLED,
			   ~GADGDISABLED,~GADGDISABLED);

//	BuffersFlags &= ~GADGDISABLED;
//	MyOnGadget(w,ReservedBegin);

	/* Set default value for standard file system gadgets */
	SetSDFSGadsAttrs(w,~SELECTED|GADGDISABLED,
			   ~SELECTED|GADGDISABLED,
			   ~SELECTED|GADGDISABLED);
}

/* reset all the internal gadget buffers for the FS gadgets from p */

void
RebuildFSGads (w,p)
	register struct Window *w;
	register struct PartitionBlock *p;
{
	sprintf(FSIdentifierSIBuff,"0x%lx",p->pb_Environment[DE_DOSTYPE]);
	sprintf(MaskSIBuff,        "0x%lx",p->pb_Environment[DE_MASK]);
	sprintf(MaxTransferSIBuff, "0x%lx",p->pb_Environment[DE_MAXTRANSFER]);

//	printf("ident = %s\n",FSIdentifierSIBuff);

	if (ReservedBegin)
		GT_SetGadgetAttrs(ReservedBegin,w,NULL,
			GTIN_Number,p->pb_Environment[DE_RESERVEDBLKS],TAG_DONE);
	ReservedBeginSInfoLongInt = p->pb_Environment[DE_RESERVEDBLKS];

	if (ReservedEnd)
		GT_SetGadgetAttrs(ReservedEnd,w,NULL,
			GTIN_Number,p->pb_Environment[DE_PREFAC],TAG_DONE);
	ReservedEndSInfoLongInt = p->pb_Environment[DE_PREFAC];

	if (p->pb_Flags & PBFF_NOMOUNT)
	{
		if (AutoMount)
			GT_SetGadgetAttrs(AutoMount,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		AutoMountFlags &= ~SELECTED;
	}
	else
	{
		if (AutoMount)
			GT_SetGadgetAttrs(AutoMount,w,NULL,GTCB_Checked,TRUE,TAG_DONE);
		AutoMountFlags |= SELECTED;

	}

	/* if using custom boot code */
	if (p->pb_Environment[DE_TABLESIZE] >= DE_BOOTBLOCKS)
	{
		if (CustomNum)
			GT_SetGadgetAttrs(CustomNum,w,NULL,
				GTIN_Number,p->pb_Environment[DE_BOOTBLOCKS],
				GA_Disabled,FALSE,
				TAG_DONE);
		CustomNumSInfoLongInt = p->pb_Environment[DE_BOOTBLOCKS];
		CustomNumFlags &= ~GADGDISABLED;

		if (CustomBoot)
			GT_SetGadgetAttrs(CustomBoot,w,NULL,GTCB_Checked,TRUE,TAG_DONE);
		CustomBootFlags |= SELECTED;
	}
	else
	{
		if (CustomNum)
			GT_SetGadgetAttrs(CustomNum,w,NULL,
				GTIN_Number,0,
				GA_Disabled,TRUE,
				TAG_DONE);
		CustomNumSInfoLongInt = 0;
		CustomNumFlags |= GADGDISABLED;

		if (CustomBoot)
			GT_SetGadgetAttrs(CustomBoot,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		CustomBootFlags &= ~SELECTED;
	}
}

/* init a partition to be FFS */

void
InitFFS (p)
	register struct PartitionBlock *p;
{
	p->pb_Environment[DE_TABLESIZE]    = 16;
	p->pb_Environment[DE_DOSTYPE]      = ID_INTER_FFS_DISK;	// 39.13
	p->pb_Environment[DE_MEMBUFTYPE]   = 0;
	p->pb_Environment[DE_NUMBUFFERS]   = 30;
	p->pb_Environment[DE_RESERVEDBLKS] = 2;
	p->pb_Environment[DE_PREFAC] 	   = 0;

	/* we want to have the driver deal with dma-ableness */
	/* for example, the A3000 can DMA anywhere. */
	/* A590/A2091 had versions of <= 6.xx.  A3000 has Version 36. */
	/* FIX! */
	if (scsiversion >= 36)
	{
		p->pb_Environment[DE_MASK] = 0x7ffffffe;
	} else {
		p->pb_Environment[DE_MASK] = 0x00fffffe;
	}
	/* FIX! should change this too */
	p->pb_Environment[DE_MAXTRANSFER]  = 0x00ffffff;
	p->pb_Environment[DE_SIZEBLOCK]    = SelectedDrive->rdb->
					     rdb_BlockBytes >> 2;
					     	/* 39.16 --- in longs */
	p->pb_Environment[DE_SECSPERBLK]   = 1;
	p->pb_Environment[DE_INTERLEAVE]   = 0;
	p->pb_Environment[DE_BOOTBLOCKS]   = 0;
	p->pb_Flags                       &= ~PBFF_NOMOUNT;
}

void
InitCustom (p)
	register struct PartitionBlock *p;
{
	InitFFS(p);
	p->pb_Environment[DE_DOSTYPE] = 0x0;
	/* others? */
}

void
InitUser (p)
	register struct PartitionBlock *p;
{
	InitFFS(p);
	p->pb_Environment[DE_DOSTYPE] = FSTypeLabels.identifier[FSTypeActive];
	/* others? */
}

/* general routine for handling Environment Int gads */

void
ChangeEnvNum (i,gad)
	int i;
	struct Gadget *gad;
{
	LONG lint;

	lint = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
	if (lint != CurrentPart->pb_Environment[i])
	{
		CurrentPart->pb_Environment[i] = lint;
		FSUpdate = TRUE;
	}
}

/* 39.18 */

int
IsThisStandardFS (DosType)
	LONG DosType;
{
	if ((DosType == ID_DOS_DISK)
	 || (DosType == ID_FFS_DISK)
	 || (DosType == ID_INTER_DOS_DISK)
	 || (DosType == ID_INTER_FFS_DISK)
	 || (DosType == ID_DC_DOS_DISK)
	 || (DosType == ID_DC_FFS_DISK))
		return(TRUE);
	else
		return(FALSE);
}

/* 39.18 */

int
MakeFSLabel (i,DosType)
	int i;
	LONG DosType;
{
	union Id
	{
		UBYTE ids[5];
		long idl;
	} id;
	union Tp
	{
		UWORD tnum;
		UBYTE tstr[2];
	} tp;

	if (!(FSTypeLabels.name[i] = AllocMem(24,MEMF_CLEAR)))
		return FALSE;

	FSTypeLabels.identifier[i] = id.idl = DosType;

	/* Now check again current file system is special file system
	 user add or fs resource. See line 1124. */
	if (CurrentPart->pb_Environment[DE_DOSTYPE] == DosType)
	{
		if (FSTypeLabels.identifier[CUSTOM_CYCLE])
			FSTypeLabels.identifier[CUSTOM_CYCLE] = 0x0;
		FSTypeActive = FSTypeSave = i;
		FSIdentifierFlags |= GADGDISABLED;
	}

	/* Check first 3 byte is printable */
	if (isalpha(id.ids[0]) && isalpha(id.ids[1]) && isalpha(id.ids[2]))
	{
               	strncpy(FSTypeLabels.name[i], id.ids, 3);
		FSTypeLabels.name[i][3] = 0x5c;	/* \ */
		tp.tstr[0] = 0x00;
		tp.tstr[1] = id.ids[3];
		sprintf(&FSTypeLabels.name[i][4],"%02x",tp.tnum);
		FSTypeLabels.name[i][6] = NULL;
	}
	else
	{
		sprintf(FSTypeLabels.name[i],"0x%x",DosType);
	}

	return(TRUE);
}

/* 39.18 */

void
SetBlockSizeFlags (PatchFlags,idx)
	LONG PatchFlags;
	int idx;
{
	if (!(PatchFlags & (1L << DE_SECSPERBLK)))
	{
//		printf("PatchFlags on\n");
		FSTypeLabels.blocksizeflags[idx] |= VALID;
	}
	else
	{
//		printf("PatchFlags off\n");
		FSTypeLabels.blocksizeflags[idx] &= ~VALID;
	}
}

/* 39.18 */

int
AddFSResource (fsr,i)
	struct FileSysResource *fsr;
	int *i;	/* file system total number in rdb.
		 anytime at least 2, even if it is in not rdb. */
{
	struct FileSysEntry *fse;
	int k;

	for (fse = (struct FileSysEntry *)
		   fsr->fsr_FileSysEntries.lh_Head;
	     fse->fse_Node.ln_Succ;
	     fse = (struct FileSysEntry *) fse->fse_Node.ln_Succ)
	{
//		printf("fsresource->Identifier,PatchFlags,Version = 0x%x,0x%x,0x%x\n",
//			fse->fse_DosType,(fse->fse_PatchFlags & (1L<<DE_SECSPERBLK)),fse->fse_Version);
		for (k = 0; k < *i; k++)
		{
//			printf("FSTypeLabels.identifier = 0x%x\n", FSTypeLabels.identifier[k]);
			if (fse->fse_DosType == FSTypeLabels.identifier[k])
			{
				/*
				 Custom File System..., actually this is more like a dummy.
				 So I decide to put fs resource label.
				 ex) 1. set "UNI\01" and select "OK". ("UNI\01" is in fs resource.)
				     2. go "File Systen Chracteristics".
				     3. If I don't put this process, File System label shows
					"Custom File System" instead of "UNI\01".
				 it does not heart, but not so great.
				*/
				if (k == CUSTOM_CYCLE)
				{
					if (!(MakeFSLabel(*i, fse->fse_DosType)))
						return(FALSE);
					SetBlockSizeFlags(fse->fse_PatchFlags,*i);
					(*i)++;
				}
//				printf("Found match: %d,%x\n",k,fse->fse_DosType);
				/* Well, if fs is not in rdb, have to check PatchFlags. */
				if (!FSTypeLabels.rdbflags[k])
				{
					SetBlockSizeFlags(fse->fse_PatchFlags,k);
				}
				break;	/* found match */
			}
		}
		if (k >= *i)	/* no match - make new label for them */
		{
			if (!IsThisStandardFS(fse->fse_DosType))
				/* not standard rom filesystem... need make label. */
			{
//				printf("No match: %x\n",fse->fse_DosType);
				/* nonstandard FS.  Add it. */
				if (!(MakeFSLabel(*i, fse->fse_DosType)))
					return(FALSE);
				SetBlockSizeFlags(fse->fse_PatchFlags,*i);
				(*i)++;
			}
		}
	}
	return(TRUE);
}

/* 39.18 */

int
CompareFSRPatchFlags (fs,fsr,idx)
	struct FileSysHeaderBlock *fs;
	struct FileSysResource *fsr;
	int idx;
{
	struct FileSysEntry *fse;

	for (fse = (struct FileSysEntry *)
		   fsr->fsr_FileSysEntries.lh_Head;
	     fse->fse_Node.ln_Succ;
	     fse = (struct FileSysEntry *) fse->fse_Node.ln_Succ)
	{
//		printf("fsresource->Identifier,PatchFlags,Version = 0x%x,0x%x,0x%x\n",
//			fse->fse_DosType,(fse->fse_PatchFlags & (1L<<DE_SECSPERBLK)),fse->fse_Version);
		if (fse->fse_DosType == fs->fhb_DosType)
		{
//			printf("Found match: %x\n",fse->fse_DosType);
			if (fs->fhb_Version >= fse->fse_Version)
			{	/* take fs in rdb. rdb one is newer version. */
				FSTypeLabels.rdbflags[idx] = TRUE;
				SetBlockSizeFlags(fs->fhb_PatchFlags,idx);
			}
			else	/* take fs resource */
			{
				SetBlockSizeFlags(fse->fse_PatchFlags,idx);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

/* Allocate File system list for cycle gadgets. */

int
AllocFSType (void)		// allocate rtn for CYCLE_KIND Gadgets
{
	int i;
	register struct FileSysHeaderBlock *fs;
	struct FileSysResource *fsr;

	/* Initialize FSTypeLabels structure */
	for (i = 0; i < 20; i++)
	{
		FSTypeLabels.identifier[i] = 0;
		FSTypeLabels.blocksizeflags[i] = 0;
		FSTypeLabels.rdbflags[i] = 0;
	}

	for (i = 0; i < 2; i++)
	{
		if (!(FSTypeLabels.name[i] = AllocMem(24,MEMF_CLEAR)))
			return FALSE;
	}

	/* Put Labe name and Identifier for default file system stuff */
	strcpy(FSTypeLabels.name[STANDARD_CYCLE],"Standard File System");
	strcpy(FSTypeLabels.name[CUSTOM_CYCLE],"Custom File System");
	if (FSTypeSave == STANDARD_CYCLE)
		FSTypeLabels.identifier[STANDARD_CYCLE] = CurrentPart->pb_Environment[DE_DOSTYPE];
	else
		FSTypeLabels.identifier[STANDARD_CYCLE] = ID_FFS_DISK;
	/* Set CUSTOM_CYCLE for tenporary. If we can find out current fs type in
	rdb or fs resource, this information will change in MakeFSLabel().
	See line 946. */
	if (FSTypeSave == CUSTOM_CYCLE)
		FSTypeLabels.identifier[CUSTOM_CYCLE] = CurrentPart->pb_Environment[DE_DOSTYPE];
	else
		FSTypeLabels.identifier[CUSTOM_CYCLE] = 0x0;

	/* Make "Custom File System" enable FSBlockSize. */
	FSTypeLabels.blocksizeflags[CUSTOM_CYCLE] |= VALID;

	FirstFileSys = CopyFileSysList(SelectedDrive->rdb->rdb_FileSysHeaderList);

	/* 39.16 - Scan the filesystem resource.  For each entry,
	 if it exists already update the FSTypeLabels entry (mainly VALID flag).
	 If it doesn't exist, add it. */
	fsr = OpenResource(FSRNAME);

	/* Make label all file system in rdb */
	for (fs = FirstFileSys;
	     fs && i < 20;
	     fs = fs->fhb_Next,i++)
	{
//		printf("rdbfs->Identifier,PatchFlags,Version = 0x%x,0x%x,0x%x\n",
//			fs->fhb_DosType,(fs->fhb_PatchFlags & (1L<<DE_SECSPERBLK)),fs->fhb_Version);

		/* Avoid making label for standard type file system */
		if (IsThisStandardFS(fs->fhb_DosType))
		{
			i--;
			if (fsr)
				if (!(CompareFSRPatchFlags(fs,fsr,STANDARD_CYCLE)))
				{
					/* Can't find in resource. */
					SetBlockSizeFlags(fs->fhb_PatchFlags,STANDARD_CYCLE);
					FSTypeLabels.rdbflags[STANDARD_CYCLE] = TRUE;
				}
		}
		else	/* not standard file system */
		{
			if (!(MakeFSLabel(i,fs->fhb_DosType)))
				return(FALSE);
			if (fsr)
				if (!(CompareFSRPatchFlags(fs,fsr,i)))
					/* Can't find in resource. */
				{
					SetBlockSizeFlags(fs->fhb_PatchFlags,i);
					FSTypeLabels.rdbflags[i] = TRUE;
				}
		}
//		printf("FSTypeLabels.name[i] = %s\n",FSTypeLabels.name[i]);
	}

	if (fsr)
		AddFSResource(fsr,&i);

	/* Put total number file system labels */
	NumFSType = i;

	/* Put NULL for cycle gadget for file system type label. */
	FSTypeLabels.name[i] = NULL;

	return TRUE;
}

/* initialize stuff for handlefs */

int
FSInitialize (w)
	struct Window *w;
{
	/* I have to clean up these value, because the FreeGadget() does not
	clean gadget pointer.
	The other gaget pointer is not nessesary, because I don't reffer.
	See RebuildFSGads() and SetSDFSGadsAttrs().
	These functions may be called before CreateAllGadgets(). */

	ReservedEnd   = 0;
	ReservedBegin = 0;
	AutoMount     = 0;
	CustomNum     = 0;
	CustomBoot    = 0;
	FFSCheck      = 0;
	IntlMode      = 0;
	DirCache      = 0;
	FSIdentifier  = 0;
	Mask          = 0;
	MaxTransfer   = 0;
	FSBlockSize   = 0;	/* 39.18 */
// 39.9
//	BuffersFlags       = 0;
	FSBlockSizeFlags   = 0;
	FSIdentifierFlags  = 0;
	MaskFlags          = 0;
	MaxTransferFlags   = 0;
	CustomNumFlags     = 0;
	ReservedEndFlags   = 0;
	ReservedBeginFlags = 0;
	FFSCheckFlags      = 0;
	IntlModeFlags      = 0;
	DirCacheFlags      = 0;
	AutoMountFlags     = 0;
	CustomBootFlags    = 0;

	ReservedBeginSInfoLongInt = 0;
	ReservedEndSInfoLongInt   = 0;
	CustomNumSInfoLongInt     = 0;
//
	FSUpdate = 0;

	savecurrpart = *CurrentPart;

	RebuildFSGads(w,CurrentPart);

	switch (CurrentPart->pb_Environment[DE_DOSTYPE]) {
	case ID_DOS_DISK:				// DOS\0
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		SetMiscGadsAttrs(w,GADGDISABLED,
				   GADGDISABLED,
				   GADGDISABLED,
				   GADGDISABLED);
		break;
	case ID_FFS_DISK:				// DOS\1
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		FSIdentifierFlags |= GADGDISABLED;
		SetSDFSGadsAttrs(w,SELECTED,
				   0,
				   0);
		break;
	case ID_INTER_DOS_DISK:				// DOS\2
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		FSIdentifierFlags |= GADGDISABLED;
		IntlModeUserCheck = TRUE;	/* 39.14 */
		SetSDFSGadsAttrs(w,0,
				   SELECTED,
				   0);
		break;
	case ID_INTER_FFS_DISK:				// DOS\3
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		FSIdentifierFlags |= GADGDISABLED;
		IntlModeUserCheck = TRUE;	/* 39.14 */
		SetSDFSGadsAttrs(w,SELECTED,
				   SELECTED,
				   0);
		break;
	case ID_DC_DOS_DISK:				// DOS\4
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		FSIdentifierFlags |= GADGDISABLED;
		SetSDFSGadsAttrs(w,0,
				   SELECTED|GADGDISABLED,
				   SELECTED);
		break;
	case ID_DC_FFS_DISK:				// DOS\5
		FSTypeActive = FSTypeSave = STANDARD_CYCLE;
		FSIdentifierFlags |= GADGDISABLED;
		SetSDFSGadsAttrs(w,SELECTED,
				   SELECTED|GADGDISABLED,
				   SELECTED);
		break;
	default:					// other
		FSTypeActive = FSTypeSave = CUSTOM_CYCLE;
		SetSDFSGadsAttrs(w,GADGDISABLED,
				   GADGDISABLED,
				   GADGDISABLED);
		break;
	}

	/* add partition name: string is long enough for a 32-char name */
	strcpy(&PartNameText[0],CurrentPart->pb_DriveName);

	/* Allocate File system list for cycle gadgets. */
	if (!AllocFSType()) return FALSE;

	/* 39.13 --- if fh_PatchFlags bit (1L<<DE_SECSPERBLK) is on,
	 get fils system block size */
	if (FSTypeLabels.blocksizeflags[FSTypeActive] & VALID)
	{
		SetFSBS(w,(CurrentPart->pb_Environment[DE_SIZEBLOCK]*4)*CurrentPart->pb_Environment[DE_SECSPERBLK]);
	}
	else	/* default is rdb_BlockBytes */
	{
		CurrentPart->pb_Environment[DE_SIZEBLOCK]  =
				SelectedDrive->rdb->rdb_BlockBytes >> 2;
								/* 39.16 */
		CurrentPart->pb_Environment[DE_SECSPERBLK] = 1; /* 39.16 */

		/* user can't change until they change flag in add/update file system" */
		SetFSBS(w,SelectedDrive->rdb->rdb_BlockBytes);
	}

	return TRUE;
}

void FreeFSType(WORD Update)
{
	int i;

	for (i = 0; i < NumFSType; i++)
	{
		if (FSTypeLabels.name[i])
			FreeMem(FSTypeLabels.name[i],24);
	}

	if (Update)
	{
		FreeFileSysList(SelectedDrive->rdb->rdb_FileSysHeaderList);
		SelectedDrive->rdb->rdb_FileSysHeaderList = FirstFileSys;
        }
        else
        {
		FreeFileSysList(FirstFileSys);
		FirstFileSys = NULL;
	}
}
