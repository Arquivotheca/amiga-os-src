/**************************************************************************
**									 **
**  newgads.c								 **
**									 **
**	Purpose: Create 2.0 type gadgets.				 **
**									 **
**  Copyright (c) 1992 Commodore-Amiga, Inc.				 **
**									 **
**************************************************************************/
#include <exec/types.h>
#include <libraries/gadtools.h>
#include <intuition/gadgetclass.h>

#include <clib/gadtools_protos.h>
#include <pragmas/gadtools_pragmas.h>

#include "stdio.h"

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "protos.h"
#include "fstype.h"

extern struct Library *GadToolsBase;
extern void *vi;
extern UWORD XtraTop;

struct Gadget *glist1;		// Preparation
struct Gadget *glist2;		// Patitioning
struct Gadget *glist3;		// File System Characteristics
struct Gadget *glist4;		// File System Maintenance
struct Gadget *glist5;		// Define/Edit Drive Type
struct Gadget *glist10;		// Bad Blocks
struct Gadget *glist12;		// Set Drive Type

/**************************************************************************
***************************************************************************
***                             Preparation                             ***
***************************************************************************
**************************************************************************/

struct Gadget *SetType;
struct Gadget *BadBlock;
struct Gadget *LowFormat;
struct Gadget *PartitionDrive;
struct Gadget *SurfCheck;
struct Gadget *FormatDrive;
struct Gadget *HelpPrep;
struct Gadget *ExitPrep;
struct Gadget *DriveList;

extern USHORT SetTypeFlags;
extern USHORT BadBlockFlags;
extern USHORT LowFormatFlags;
extern USHORT PartitionDriveFlags;
extern USHORT SurfCheckFlags;
extern USHORT FormatDriveFlags;

extern struct List lh;			/* List for DriveList	*/
extern UWORD DriveListPosition;		/* heighlight position	*/

/*************************************************************************
***
*** struct Gadget *CreateAllGadgets1(void)
***
***	- Create all gadgets for NewWindowStructure1 (V36).
***
*************************************************************************/
struct Gadget *CreateAllGadgets1(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** Hard Drives in System ******/
    ng.ng_LeftEdge = 320;
    ng.ng_TopEdge = 16 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Hard Drives in System";		// This is title.
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = PLACETEXT_IN|NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Change Drive Type *****/
    ng.ng_LeftEdge = 44;
    ng.ng_TopEdge = 100 + XtraTop;
    ng.ng_Width = 195;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Change Drive Type";
    ng.ng_GadgetID = SETTYPE;
    ng.ng_Flags = 0;
    SetType = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (SetTypeFlags & GADGDISABLED), TAG_DONE);

    /***** Modify Bad Block List *****/
    ng.ng_TopEdge = 120 + XtraTop;
    ng.ng_GadgetText = "Modify Bad Block List";
    ng.ng_GadgetID = BADBLOCK;
    BadBlock = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (BadBlockFlags & GADGDISABLED), TAG_DONE);

    /***** Low-level Format Drive *****/
    ng.ng_TopEdge = 140 + XtraTop;
    ng.ng_GadgetText = "Low-level Format Drive";
    ng.ng_GadgetID = LOWFORMAT;
    LowFormat = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (LowFormatFlags & GADGDISABLED), TAG_DONE);

    /***** Partition Drive *****/
    ng.ng_LeftEdge = 392;
    ng.ng_TopEdge = 100 + XtraTop;
    ng.ng_GadgetText = "Partition Drive";
    ng.ng_GadgetID = PARTITIONDRIVE;
    PartitionDrive = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (PartitionDriveFlags & GADGDISABLED), TAG_DONE);

    /***** Verify Data on Drive *****/
    ng.ng_TopEdge = 120 + XtraTop;
    ng.ng_GadgetText = "Verify Data on Drive";
    ng.ng_GadgetID = SURFCHECK;
    SurfCheck = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (SurfCheckFlags & GADGDISABLED), TAG_DONE);

    /***** Save Change to Drive *****/
    ng.ng_TopEdge = 140 + XtraTop;
    ng.ng_GadgetText = "Save Changes to Drive";
    ng.ng_GadgetID = FORMATDRIVE;
    FormatDrive = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (FormatDriveFlags & GADGDISABLED), TAG_DONE);
    /***** Help *****/
    ng.ng_LeftEdge = 290;
    ng.ng_TopEdge = 120 + XtraTop;
    ng.ng_Width = 53;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Help";
    ng.ng_GadgetID = HELPPREP;
    HelpPrep = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Exit *****/
    ng.ng_LeftEdge = 228;
    ng.ng_TopEdge = 164 + XtraTop;
    ng.ng_Width = 178;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Exit";
    ng.ng_GadgetID = EXITPREP;
    ExitPrep = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    AllocDriveList();

    /***** DriveList *****/
    ng.ng_LeftEdge = 10;
    ng.ng_TopEdge = 42 + XtraTop;
    ng.ng_Width = 616;
    ng.ng_Height = 56;
    ng.ng_GadgetText = "Interface Address  LUN      Status      Drive Type                          ";
    ng.ng_GadgetID = DRIVELIST;
    ng.ng_Flags = NG_HIGHLABEL;
    DriveList = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &lh,
	GTLV_Top, 0,
	LAYOUTA_SPACING, 1,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, DriveListPosition,
	GTLV_ScrollWidth, 18,
	TAG_DONE);

    glist1 = glist;
    return(gad);
}


/**************************************************************************
***************************************************************************
***                            Partitioning                             ***
***************************************************************************
**************************************************************************/

struct Gadget *FileSys;
struct Gadget *AddFileSys;
struct Gadget *ChangeFileSys;
struct Gadget *HostID;		/* 39.13 */
struct Gadget *BootPri;
struct Gadget *Buffers;
struct Gadget *TotalCyl;
struct Gadget *EndCyl;
struct Gadget *StartCyl;
struct Gadget *NamePart;
struct Gadget *Advanced;
struct Gadget *CancelPart;
struct Gadget *DonePart;
struct Gadget *Help;
struct Gadget *QuickSetup;
struct Gadget *NewPart;
struct Gadget *DeletePart;
struct Gadget *Bootable;

struct IntuiText IText26 = {	// This is for only when select New Partition.
	1,0,JAM1,
	8,3,
	&TOPAZ80,
	"New Partition",
	NULL
};

extern USHORT BootableFlags;
extern USHORT BootPriFlags;
extern USHORT BuffersFlags;
extern USHORT AdvancedFlags;

extern LONG StartCylSInfoLongInt;
extern LONG EndCylSInfoLongInt;
extern LONG TotalCylSInfoLongInt;
extern LONG BuffersSInfoLongInt;
extern LONG BootPriSInfoLongInt;
extern LONG HostIDSInfoLongInt;		/* 39.13 */

extern char FileSysText[18];

/**************************************************************************
B***
*** struct Gadget *CreateAllGadgets2(void)
***
***	- Create all gadgets for NewWindowStructure2 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets2(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** Partitioning Drive ******/
    ng.ng_LeftEdge = 400;
    ng.ng_TopEdge = 14 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Partitioning Drive";
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** = Current partition ******/
    ng.ng_LeftEdge = 584;
    ng.ng_TopEdge = 37 + XtraTop;
    ng.ng_GadgetText = "= Current partition";
    ng.ng_Flags = 0;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** = A partition ******/
    ng.ng_LeftEdge = 349;
    ng.ng_TopEdge = 37 + XtraTop;
    ng.ng_GadgetText = "= A partition";
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** = Unused ******/
    ng.ng_LeftEdge = 160;
    ng.ng_TopEdge = 37 + XtraTop;
    ng.ng_GadgetText = "= Unused";
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Cyl ******/
    ng.ng_LeftEdge = 636;
    ng.ng_TopEdge = 26 + XtraTop;
    ng.ng_GadgetText = "Cyl";
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Cyl ******/
    ng.ng_LeftEdge = 42;
    ng.ng_TopEdge = 26 + XtraTop;
    ng.ng_GadgetText = "Cyl";
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** 0 ******/
    ng.ng_LeftEdge = 34;
    ng.ng_TopEdge = 35 + XtraTop;
    ng.ng_GadgetText = "0";
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** File System: ******/
    ng.ng_LeftEdge = 484;
    ng.ng_TopEdge = 124 + XtraTop;
    ng.ng_GadgetText = "File System:";
    ng.ng_Flags = NG_HIGHLABEL;
    FileSys = gad = CreateGadget(TEXT_KIND, gad, &ng,
				GTTX_Text, FileSysText, TAG_DONE);

    /***** Bootable *****/
    ng.ng_LeftEdge = 312 + 32;
    ng.ng_TopEdge = 153 + XtraTop;
    ng.ng_Width = 31;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Bootable";
    ng.ng_VisualInfo = vi;
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = BOOTABLE;
    ng.ng_Flags = 0;
    Bootable = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (BootableFlags & GADGDISABLED),
			GTCB_Checked, (BootableFlags & SELECTED),
			TAG_DONE);

    /***** Delete Partition *****/
    ng.ng_LeftEdge = 76;
    ng.ng_TopEdge = 97 + XtraTop;
    ng.ng_Width = 140;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Delete Partition";
    ng.ng_GadgetID = DELETEPART;
    ng.ng_Flags = 0;
    DeletePart = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** New Partition *****/
    ng.ng_LeftEdge = 224;
    ng.ng_TopEdge = 97 + XtraTop;
    ng.ng_Width = 120;
    ng.ng_GadgetText = "New Partition";
    ng.ng_GadgetID = NEWPART;
    NewPart = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);
    NewPart->GadgetText = &IText26;

    /***** Default Setup *****/
    ng.ng_LeftEdge = 352;
    ng.ng_TopEdge = 97 + XtraTop;
    ng.ng_Width = 120;
    ng.ng_GadgetText = "Default Setup";
    ng.ng_GadgetID = QUICKSETUP;
    QuickSetup = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Help *****/
    ng.ng_LeftEdge = 480;
    ng.ng_TopEdge = 97 + XtraTop;
    ng.ng_Width = 60;
    ng.ng_GadgetText = "Help";
    ng.ng_GadgetID = HELP;
    Help = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_GadgetText = "Ok";
    ng.ng_GadgetID = DONEPART;
    DonePart = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = CANCELPART;
    CancelPart = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Advanced Options *****/
    ng.ng_LeftEdge = 150;
    ng.ng_TopEdge = 116 + XtraTop;
    ng.ng_Width = 31;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Advanced Options";
    ng.ng_GadgetID = ADVANCED;
    Advanced = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GTCB_Checked, (AdvancedFlags & SELECTED),
			TAG_DONE);

    /***** Partition Device Name *****/
    ng.ng_LeftEdge = 186 - 4;
    ng.ng_TopEdge = 136 + XtraTop;
    ng.ng_Width = 188;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Partition Device Name";
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = NAMEPART;
    ng.ng_Flags = PLACETEXT_ABOVE;
    NamePart = gad = CreateGadget(STRING_KIND, gad, &ng,
//			STRINGA_Buffer, NamePartSIBuff,
//			STRINGA_UndoBuffer, UNDOBUFFER,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, NamePartSIBuff,
			GTST_MaxChars, 32,
			TAG_DONE);
    NamePart->SpecialInfo = (APTR)&NamePartSInfo;

    if (AdvancedFlags & SELECTED)
    {
    /***** StartCyl *****/
    ng.ng_LeftEdge = 103;
    ng.ng_TopEdge = 130 + XtraTop;
    ng.ng_Width = 60;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Start Cyl:";
    ng.ng_GadgetID = STARTCYL;
    ng.ng_Flags = 0;
    StartCyl = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, StartCylSInfoLongInt,
			GTIN_MaxChars, 6,
			TAG_DONE);

    /***** EndCyl *****/
    ng.ng_TopEdge = 143 + XtraTop;
    ng.ng_GadgetText = "End Cyl:";
    ng.ng_GadgetID = ENDCYL;
    EndCyl = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, EndCylSInfoLongInt,
			GTIN_MaxChars, 6,
			TAG_DONE);

    /***** TotalCyl *****/
    ng.ng_TopEdge = 156 + XtraTop;
    ng.ng_GadgetText = "Total Cyl:";
    ng.ng_GadgetID = TOTALCYL;
    TotalCyl = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, TotalCylSInfoLongInt,
			GTIN_MaxChars, 6,
			TAG_DONE);

    /***** Buffers *****/
    ng.ng_TopEdge = 169 + XtraTop;
    ng.ng_GadgetText = "Buffers:";
    ng.ng_GadgetID = BUFFERS;
    Buffers = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, BuffersSInfoLongInt,
			GTIN_MaxChars, 6,
			GA_Disabled, (BuffersFlags & GADGDISABLED),
			TAG_DONE);
/* 39.13 */
    /***** Host ID *****/
    ng.ng_LeftEdge = 236;
    ng.ng_TopEdge = 153 + XtraTop;
    ng.ng_Width = 28;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "HostID:";
    ng.ng_GadgetID = HOSTID;
    HostID = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, HostIDSInfoLongInt,
			GTIN_MaxChars, 1,
//			GA_Disabled, (HostIDFlags & GADGDISABLED),
			TAG_DONE);

    /***** Boot Priority *****/
    ng.ng_LeftEdge = 312 + 6;
    ng.ng_TopEdge = 169 + XtraTop;
    ng.ng_Width = 52;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Boot Priority:";
    ng.ng_GadgetID = BOOTPRI;
    BootPri = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, BootPriSInfoLongInt,
			GTIN_MaxChars, 5,
			GA_Disabled, (BootPriFlags & GADGDISABLED),
			TAG_DONE);

    /***** Change File System *****/
    ng.ng_LeftEdge = 381 + 2;
    ng.ng_TopEdge = 138 + XtraTop;
    ng.ng_Width = 140;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Change...";
    ng.ng_GadgetID = CHANGEFILESYS;
    ChangeFileSys = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);


    /***** Add File System *****/
    ng.ng_LeftEdge = 381 + 2;
    ng.ng_TopEdge = 160 + XtraTop;
    ng.ng_Width = 140;
    ng.ng_GadgetText = "Add/Update...";
    ng.ng_GadgetID = ADDFILESYS;
    AddFileSys = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);
    }


    SizePart.TopEdge =  70 + XtraTop;
    DragPart.TopEdge =  48 + XtraTop;

    gad->NextGadget = &DragPart;

    glist2 = glist;
    return(gad);
}


/**************************************************************************
***************************************************************************
***                    File System Charatacteristics                    ***
***************************************************************************
**************************************************************************/

struct Gadget *PartName;
struct Gadget *FSOk;
struct Gadget *FSCancel;
struct Gadget *FSType;		/* 39.13 */
struct Gadget *FSBlockSize;	/* 39.13 */
struct Gadget *FSIdentifier;
struct Gadget *Mask;
struct Gadget *MaxTransfer;
struct Gadget *ReservedBegin;
struct Gadget *ReservedEnd;
struct Gadget *CustomNum;
struct Gadget *FFSCheck;	/* 39.13 */
struct Gadget *IntlMode;	/* 39.13 */
struct Gadget *DirCache;	/* 39.13 */
struct Gadget *AutoMount;
struct Gadget *CustomBoot;

extern USHORT FSBlockSizeFlags;	/* 39.13 */
extern USHORT FSIdentifierFlags;
extern USHORT MaskFlags;
extern USHORT MaxTransferFlags;
extern USHORT CustomNumFlags;
extern USHORT ReservedEndFlags;
extern USHORT ReservedBeginFlags;
extern USHORT FFSCheckFlags;	/* 39.13 */
extern USHORT IntlModeFlags;	/* 39.13 */
extern USHORT DirCacheFlags;	/* 39.13 */
extern USHORT AutoMountFlags;
extern USHORT CustomBootFlags;

extern LONG ReservedBeginSInfoLongInt;
extern LONG ReservedEndSInfoLongInt;
extern LONG CustomNumSInfoLongInt;

extern char PartNameText[44];

extern UWORD FSTypeActive;			/* active cycle position */
extern UWORD FSBlockSizeActive;			/* active cycle position */
extern struct FSTypeLabels FSTypeLabels;	/* file system labels */
extern STRPTR FSBlockSizeLabels[];		/* block size labels */

/**************************************************************************
***
*** struct Gadget *CreateAllGadgets3(void)
***
***	- Create all gadgets for NewWindowStructure3 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets3(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** Partition: ******/
    ng.ng_LeftEdge = 330;
    ng.ng_TopEdge = 16 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Partition:";		// This is title.
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    PartName = gad = CreateGadget(TEXT_KIND, gad, &ng,
				GTTX_Text, PartNameText, TAG_DONE);

    /***** Reserved blocks at ******/
    ng.ng_LeftEdge = 420;
    ng.ng_TopEdge = 108 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Reserved blocks at";
    ng.ng_GadgetID = 0;
    ng.ng_Flags = 0;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Ok";
    ng.ng_Flags = 0;
    ng.ng_GadgetID = FSOK;
    FSOk = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_GadgetID = FSCANCEL;
    FSCancel = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

/* 39.13 */
    /***** File System ******/
    ng.ng_LeftEdge = 250;
    ng.ng_TopEdge = 30 + XtraTop;
    ng.ng_Width = 214;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "File System:";
    ng.ng_GadgetID = FSTYPE;
    ng.ng_Flags = NG_HIGHLABEL;
    FSType = gad = CreateGadget(CYCLE_KIND, gad, &ng,
			GTCY_Labels, FSTypeLabels.name,
			GTCY_Active, FSTypeActive,
			TAG_DONE);

    /***** File system block size *****/
    ng.ng_LeftEdge = 480;
    ng.ng_TopEdge = 90 + XtraTop;
    ng.ng_Width = 80;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "File system block size:";
    ng.ng_GadgetID = FSBLOCKSIZE;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    FSBlockSize = gad = CreateGadget(CYCLE_KIND, gad, &ng,
			GTCY_Labels, FSBlockSizeLabels,
			GTCY_Active, FSBlockSizeActive,
			GA_Disabled, (FSBlockSizeFlags & GADGDISABLED),
			TAG_DONE);
//

    /***** Identifier *****/
    ng.ng_LeftEdge = 145;
    ng.ng_TopEdge = 106 + XtraTop;
    ng.ng_Width = 100;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Identifier =";
    ng.ng_GadgetID = FSIDENTIFIER;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    FSIdentifier = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, FSIdentifierSIBuff,
			GTST_MaxChars, 12,
			GA_Disabled, (FSIdentifierFlags & GADGDISABLED),
			TAG_DONE);
    FSIdentifier->SpecialInfo = (APTR)&FSIdentifierSInfo;

    /***** Mask *****/
    ng.ng_LeftEdge = 145;
    ng.ng_TopEdge = 120 + XtraTop;
    ng.ng_GadgetText = "Mask =";
    ng.ng_GadgetID = MASK;
    Mask = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, MaskSIBuff,
			GTST_MaxChars, 12,
			GA_Disabled, (MaskFlags & GADGDISABLED),
			TAG_DONE);
    Mask->SpecialInfo = (APTR)&MaskSInfo;

    /***** MaxTransfer *****/
    ng.ng_LeftEdge = 145;
    ng.ng_TopEdge = 134 + XtraTop;
    ng.ng_GadgetText = "MaxTransfer =";
    ng.ng_GadgetID = MAXTRANSFER;
    MaxTransfer = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, MaxTransferSIBuff,
			GTST_MaxChars, 12,
			GA_Disabled, (MaxTransferFlags & GADGDISABLED),
			TAG_DONE);
    MaxTransfer->SpecialInfo = (APTR)&MaxTransferSInfo;

    /***** beginning *****/
    ng.ng_LeftEdge = 422;
    ng.ng_TopEdge = 120 + XtraTop;
    ng.ng_GadgetText = "beginning:";
    ng.ng_GadgetID = RESERVEDBEGIN;
    ReservedBegin = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, ReservedBeginSInfoLongInt,
			GTIN_MaxChars, 12,
			GA_Disabled, (ReservedBeginFlags & GADGDISABLED),
			TAG_DONE);

    /***** end *****/
    ng.ng_LeftEdge = 422;
    ng.ng_TopEdge = 134 + XtraTop;
    ng.ng_GadgetText = "end:";
    ng.ng_GadgetID = RESERVEDEND;
    ReservedEnd = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, ReservedEndSInfoLongInt,
			GTIN_MaxChars, 12,
			GA_Disabled, (ReservedEndFlags & GADGDISABLED),
			TAG_DONE);

    /***** Number of custom boot blocks *****/
    ng.ng_LeftEdge = 387;
    ng.ng_TopEdge = 168 + XtraTop;
    ng.ng_Width = 40;
    ng.ng_GadgetText = "Number of custom boot blocks";
    ng.ng_GadgetID = CUSTOMNUM;
    CustomNum = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, CustomNumSInfoLongInt,
			GTIN_MaxChars, 12,
			GA_Disabled, (CustomNumFlags & GADGDISABLED),
			TAG_DONE);
/* 39.13 */
    /***** Fast File System: *****/
    ng.ng_LeftEdge = 370;
    ng.ng_TopEdge = 48 + XtraTop;
    ng.ng_Width = 31;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Fast File System:";
    ng.ng_GadgetID = FFSCHK;
    FFSCheck = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (FFSCheckFlags & GADGDISABLED),
			GTCB_Checked, (FFSCheckFlags & SELECTED),
			TAG_DONE);

    /***** International Mode: *****/
    ng.ng_LeftEdge = 370;
    ng.ng_TopEdge = 60 + XtraTop;
    ng.ng_GadgetText = "International Mode:";
    ng.ng_GadgetID = INTLMODE;
    IntlMode = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (IntlModeFlags & GADGDISABLED),
			GTCB_Checked, (IntlModeFlags & SELECTED),
			TAG_DONE);

    /***** Directory Cache: *****/
    ng.ng_LeftEdge = 370;
    ng.ng_TopEdge = 72 + XtraTop;
    ng.ng_GadgetText = "Directory Cache:";
    ng.ng_GadgetID = DIRCACHE;
    DirCache = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (DirCacheFlags & GADGDISABLED),
			GTCB_Checked, (DirCacheFlags & SELECTED),
			TAG_DONE);

//
    /***** Automount this partition *****/
    ng.ng_LeftEdge = 230;
    ng.ng_TopEdge = 90 + XtraTop;
    ng.ng_GadgetText = "Automount this partition:";
    ng.ng_GadgetID = AUTOMOUNT;
    AutoMount = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (AutoMountFlags & GADGDISABLED),
			GTCB_Checked, (AutoMountFlags & SELECTED),
			TAG_DONE);

    /***** Use custom boot code *****/
    ng.ng_LeftEdge = 388;
    ng.ng_TopEdge = 152 + XtraTop;
    ng.ng_GadgetText = "Use custom boot code:";
    ng.ng_GadgetID = CUSTOMBOOT;
    CustomBoot = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (CustomBootFlags & GADGDISABLED),
			GTCB_Checked, (CustomBootFlags & SELECTED),
			TAG_DONE);

    glist3 = glist;
    return(gad);
}


/**************************************************************************
***************************************************************************
***                        File System Maintenance                      ***
***************************************************************************
**************************************************************************/

struct Gadget *FSMOk;
struct Gadget *FSMCancel;
struct Gadget *FSMAdd;
struct Gadget *FSMDelete;
struct Gadget *FSMUpdate;
struct Gadget *FSMList;

extern USHORT FSMDeleteFlags;
extern USHORT FSMUpdateFlags;

extern struct List lh4;			/* List for File System	*/
extern UWORD FSMListPosition;		/* heighlight position	*/

char AddrLunText[] = "File Systems on        Address  , LUN   ";

/**************************************************************************
***
*** struct Gadget *CreateAllGadgets4(void)
***
***	- Create all gadgets for NewWindowStructure4 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets4(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** File Systems on        Address  , LUN   ******/
    ng.ng_LeftEdge = 160;
    ng.ng_TopEdge = 18 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = NULL;
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = 0;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng,
				GTTX_Text, AddrLunText, TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Ok";
    ng.ng_Flags = 0;
    ng.ng_GadgetID = FSMOK;
    FSMOk = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_GadgetID = FSMCANCEL;
    FSMCancel = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Add New File System *****/
    ng.ng_LeftEdge = 28;
    ng.ng_TopEdge = 110 + XtraTop;
    ng.ng_Width = 188;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Add New File System...";
    ng.ng_GadgetID = FSMADD;
    FSMAdd = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Delete File System *****/
    ng.ng_LeftEdge = 223;
    ng.ng_GadgetText = "Delete File System";
    ng.ng_GadgetID = FSMDELETE;
    FSMDelete = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (FSMDeleteFlags & GADGDISABLED), TAG_DONE);

    /***** Update File System *****/
    ng.ng_LeftEdge = 418;
    ng.ng_GadgetText = "Update File System...";
    ng.ng_GadgetID = FSMUPDATE;
    FSMUpdate = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (FSMUpdateFlags & GADGDISABLED), TAG_DONE);

    AllocFSMList();

    /***** File System Maintenance List *****/
    ng.ng_LeftEdge = 60;
    ng.ng_TopEdge = 52 + XtraTop;
    ng.ng_Width = 522;
    ng.ng_Height = 50;
    ng.ng_GadgetText = 	"Identifier    Version   Size    File System Name               ";
    ng.ng_GadgetID = FSMLIST;
    ng.ng_Flags = NG_HIGHLABEL;
    FSMList = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &lh4,
	GTLV_Top, 0,
	LAYOUTA_SPACING, 1,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, FSMListPosition,
	GTLV_ScrollWidth, 18,
	TAG_DONE);

    glist4 = glist;

    return(gad);
}


/**************************************************************************
***************************************************************************
***                         Define/Edit Drive Type                      ***
***************************************************************************
**************************************************************************/

struct Gadget *NDOk;
struct Gadget *NDCancel;
struct Gadget *NDReadParms;
struct Gadget *NDRevisionName;
struct Gadget *NDDriveName;
struct Gadget *NDManuName;
struct Gadget *NDName;
struct Gadget *NDCylinders;
struct Gadget *NDHeads;
struct Gadget *NDBlocks;
struct Gadget *NDCylBlocks;
struct Gadget *NDReduced;
struct Gadget *NDPreComp;
struct Gadget *NDParkWhere;
struct Gadget *NDReselect;
struct Gadget *NDSize;

extern USHORT NDReadParmsFlags;
extern USHORT NDReducedFlags;
extern USHORT NDPreCompFlags;
extern USHORT NDReselectFlags;

extern LONG NDCylindersSInfoLongInt;
extern LONG NDHeadsSInfoLongInt;
extern LONG NDBlocksSInfoLongInt;
extern LONG NDCylBlocksSInfoLongInt;
extern LONG NDReducedSInfoLongInt;
extern LONG NDPreCompSInfoLongInt;
extern LONG NDParkWhereSInfoLongInt;

extern char NDSizeText[30];

/**************************************************************************
***
*** struct Gadget *CreateAllGadgets5(void)
***
***	- Create all gadgets for NewWindowStructure5 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets5(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);


    /***** Define a New Drive Type ******/
    ng.ng_LeftEdge = 420;
    ng.ng_TopEdge = 16 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Define a New Drive Type";	// This is title.
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Reduced Write Current ******/
    ng.ng_LeftEdge = 192;
    ng.ng_TopEdge = 138 + XtraTop;
    ng.ng_GadgetText = "Reduced Write Current";
    ng.ng_Flags = 0;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Write Precomp ******/
    ng.ng_LeftEdge = 192;
    ng.ng_TopEdge = 160 + XtraTop;
    ng.ng_GadgetText = "Write Precomp";
    ng.ng_Flags = 0;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Size: ******/
    ng.ng_LeftEdge = 308;
    ng.ng_TopEdge = 100 + XtraTop;
    ng.ng_GadgetText = "Size:";
    ng.ng_Flags = 0;
    NDSize = gad = CreateGadget(TEXT_KIND, gad, &ng,
				GTTX_Text, NDSizeText, TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Ok";
    ng.ng_Flags = 0;
    ng.ng_GadgetID = NDOK;
    NDOk = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_GadgetID = NDCANCEL;
    NDCancel = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Read Configuration *****/
    ng.ng_LeftEdge = 464;
    ng.ng_TopEdge = 43 + XtraTop;
    ng.ng_Width = 160;
    ng.ng_GadgetText = "Read Configuration";
    ng.ng_GadgetID = NDREADPARMS;
    NDReadParms = gad = CreateGadget(BUTTON_KIND, gad, &ng,
		GA_Disabled, (NDReadParmsFlags & GADGDISABLED), TAG_DONE);

    /***** FileName *****/
    ng.ng_LeftEdge = 284;
    ng.ng_TopEdge = 32 + XtraTop;
    ng.ng_Width = 158;
    ng.ng_Height = 12;
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetText = "FileName:";
    ng.ng_GadgetID = NDNAME;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    NDName = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, NDNameSIBuff,
			GTST_MaxChars, 128,
			TAG_DONE);
    NDName->SpecialInfo = (APTR)&NDNameSInfo;

    /***** Manufacturers Name *****/
    ng.ng_TopEdge = 44 + XtraTop;
    ng.ng_GadgetText = "Manufacturers Name:";
    ng.ng_GadgetID = NDMANUNAME;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    NDManuName = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, NDManuNameSIBuff,
			GTST_MaxChars, 9,
			TAG_DONE);
    NDManuName->SpecialInfo = (APTR)&NDManuNameSInfo;

    /***** Drive Name *****/
    ng.ng_TopEdge = 56 + XtraTop;
    ng.ng_GadgetText = "Drive Name:";
    ng.ng_GadgetID = NDDRIVENAME;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    NDDriveName = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, NDDriveNameSIBuff,
			GTST_MaxChars, 17,
			TAG_DONE);
    NDDriveName->SpecialInfo = (APTR)&NDDriveNameSInfo;

    /***** Drive Revision *****/
    ng.ng_TopEdge = 68 + XtraTop;
    ng.ng_GadgetText = "Drive Revision:";
    ng.ng_GadgetID = NDREVISIONNAME;
    ng.ng_Flags = 0;		/* NG_HIGHLABEL or PLACETEXT_IN ??? */
    NDRevisionName = gad = CreateGadget(STRING_KIND, gad, &ng,
			STRINGA_Justification, STRINGCENTER,
			GTST_String, NDRevisionNameSIBuff,
			GTST_MaxChars, 5,
			TAG_DONE);
    NDRevisionName->SpecialInfo = (APTR)&NDRevisionNameSInfo;

    /***** Cylinders *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 86 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Cylinders:";
    ng.ng_GadgetID = NDCYLINDERS;
    NDCylinders = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDCylindersSInfoLongInt,
			GTIN_MaxChars, 10,
			TAG_DONE);

    /***** Heads *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 98 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Heads:";
    ng.ng_GadgetID = NDHEADS;
    NDHeads = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDHeadsSInfoLongInt,
			GTIN_MaxChars, 10,
			TAG_DONE);

    /***** Blocks per Track *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 110 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Blocks per Track:";
    ng.ng_GadgetID = NDBLOCKS;
    NDBlocks = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDBlocksSInfoLongInt,
			GTIN_MaxChars, 10,
			TAG_DONE);

    /***** Blocks per Cylinders *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 122 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Blocks per Cylinder:";
    ng.ng_GadgetID = NDCYLBLOCKS;
    NDCylBlocks = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDCylBlocksSInfoLongInt,
			GTIN_MaxChars, 10,
			TAG_DONE);

    /***** Reduced Write Current Cylinder *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 146 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Cylinder:";
    ng.ng_GadgetID = NDREDUCED;
    NDReduced = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDReducedSInfoLongInt,
			GTIN_MaxChars, 10,
			GA_Disabled, (NDReducedFlags & GADGDISABLED),
			TAG_DONE);

    /***** Write Precomp Cylinder *****/
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 168 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Cylinder:";
    ng.ng_GadgetID = NDPRECOMP;
    NDPreComp = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDPreCompSInfoLongInt,
			GTIN_MaxChars, 10,
			GA_Disabled, (NDPreCompFlags & GADGDISABLED),
			TAG_DONE);

    /***** Park head where (cylinder) *****/
    ng.ng_LeftEdge = 500;
    ng.ng_TopEdge = 126 + XtraTop;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Park head where (cylinder):";
    ng.ng_GadgetID = NDPARKWHERE;
    NDParkWhere = gad = CreateGadget(INTEGER_KIND, gad, &ng,
			GTIN_Number, NDParkWhereSInfoLongInt,
			GTIN_MaxChars, 6,
			TAG_DONE);

    /***** Supports reselection *****/
    ng.ng_LeftEdge = 460;
    ng.ng_TopEdge = 148 + XtraTop;
    ng.ng_Width = 31;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Supports reselection";
    ng.ng_VisualInfo = vi;
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = NDRESELECT;
    ng.ng_Flags = 0;
    NDReselect = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
			GA_Disabled, (NDReselectFlags & GADGDISABLED),
			GTCB_Checked, (NDReselectFlags & SELECTED),
			TAG_DONE);

    glist5 = glist;
    return(gad);
}


/**************************************************************************
***************************************************************************
***                               Bad Blocks                            ***
***************************************************************************
**************************************************************************/

struct Gadget *BadOk;
struct Gadget *BadCancel;
struct Gadget *BadAdd;
struct Gadget *BadDelete;
struct Gadget *BadList;

extern USHORT BadDeleteFlags;

extern struct List lh10;		/* List for Bad Blocks	*/
extern UWORD BadListPosition;		/* heighlight position	*/

extern char BadBlockText[10];

/**************************************************************************
***
*** struct Gadget *CreateAllGadgets10(void)
***
***	- Create all gadgets for NewWindowStructure10 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets10(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** Bad Block Entry ******/
    ng.ng_LeftEdge = 388;
    ng.ng_TopEdge = 16 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Bad Block Entry";		// This is title.
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = /*PLACETEXT_IN|*/NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** "(approximate)" ******/
    ng.ng_LeftEdge = 310;
    ng.ng_TopEdge = 30 + XtraTop;
    ng.ng_GadgetText = "(approximate)";
    ng.ng_Flags = 0;//PLACETEXT_IN|NG_HIGHLABEL;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** "Bad Blocks mapped out by drive: " ******/
    ng.ng_LeftEdge = 350;
    ng.ng_TopEdge = 168 + XtraTop;
    ng.ng_GadgetText = "Bad Blocks mapped out by drive:";
    ng.ng_Flags = 0;//PLACETEXT_IN|NG_HIGHLABEL;
    gad = CreateGadget(TEXT_KIND, gad, &ng,
			GTTX_Text,BadBlockText, TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Ok";
    ng.ng_Flags = 0;
    ng.ng_GadgetID = BADOK;
    BadOk = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_GadgetID = BADCANCEL;
    BadCancel = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			TAG_DONE);

    /***** Add Bad Block *****/
    ng.ng_LeftEdge = 462;
    ng.ng_TopEdge = 60 + XtraTop;
    ng.ng_Width = 150;
    ng.ng_GadgetText = "Add Bad Block...";
    ng.ng_GadgetID = BADADD;
    BadAdd = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			TAG_DONE);

    /***** Delete Bad Block *****/
    ng.ng_TopEdge = 82 + XtraTop;
    ng.ng_GadgetText = "Delete Bad Block";
    ng.ng_GadgetID = BADDELETE;
    BadDelete = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (BadDeleteFlags & GADGDISABLED), TAG_DONE);

    AllocBadList();

    /***** Bad Blocks List *****/
    ng.ng_LeftEdge = 32;
    ng.ng_TopEdge = 52 + XtraTop;
    ng.ng_Width = 410;
    ng.ng_Height = 104;
    ng.ng_GadgetText = "Cylinder  Head   Bytes from Index   Sector     ";
    ng.ng_GadgetID = BADLIST;
    ng.ng_Flags = NG_HIGHLABEL;
    BadList = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &lh10,
	GTLV_Top, 0,
	LAYOUTA_SPACING, 1,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, BadListPosition,
	GTLV_ScrollWidth, 18,
	TAG_DONE);

    glist10 = glist;
    return(gad);
}


/**************************************************************************
***************************************************************************
***                            Set Drive Type                           ***
***************************************************************************
**************************************************************************/

struct Gadget *SetTypeOK;
struct Gadget *SetTypeCancel;
struct Gadget *DefineDrive;
struct Gadget *EditType;
struct Gadget *DeleteType;
struct Gadget *TypeList;
struct Gadget *SCSIType;

extern USHORT SetTypeOKFlags;
extern USHORT EditTypeFlags;
extern USHORT DeleteTypeFlags;

extern struct List lh12;		/* List for Drive Type	*/
extern UWORD TypeListPosition;		/* heighlight position	*/
extern UWORD SCSITypeActive;		/* active cycle position */

extern char St506Text[6];

STRPTR SCSITypeLabels[] =
{
    "SCSI",
    St506Text,
    NULL
};

/**************************************************************************
***
*** struct Gadget *CreateAllGadgets12(void)
***
***	- Create all gadgets for NewWindowStructure12 (V36).
***
**************************************************************************/
struct Gadget *CreateAllGadgets12(void)
{
    struct NewGadget ng;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /***** Select a Drive Type ******/
    ng.ng_LeftEdge = 310;
    ng.ng_TopEdge = 16 + XtraTop;
    ng.ng_Width = 0;
    ng.ng_Height = 8;
    ng.ng_GadgetText = "Select a Drive Type";		// This is title.
    ng.ng_TextAttr = &TOPAZ80;
    ng.ng_GadgetID = 0;
    ng.ng_Flags = PLACETEXT_IN|NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(TEXT_KIND, gad, &ng, TAG_DONE);

    /***** Drive Types: ******/
    ng.ng_LeftEdge = 320;
    ng.ng_TopEdge = 34 + XtraTop;
    ng.ng_Width = 80;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Drive Types:";
    ng.ng_GadgetID = SCSITYPE;
    ng.ng_Flags = NG_HIGHLABEL;
    SCSIType = gad = CreateGadget(CYCLE_KIND, gad, &ng,
	GTCY_Labels, SCSITypeLabels,
	GTCY_Active, SCSITypeActive,
	TAG_DONE);

    /***** Ok *****/
    ng.ng_LeftEdge = 547;
    ng.ng_TopEdge = 144 + XtraTop;
    ng.ng_Width = 76;
    ng.ng_Height = 14;
    ng.ng_GadgetText = "Ok";
    ng.ng_Flags = 0;
    ng.ng_GadgetID = SETTYPEOK;
    SetTypeOK = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (SetTypeOKFlags & GADGDISABLED), TAG_DONE);

    /***** Cancel *****/
    ng.ng_TopEdge = 166 + XtraTop;
    ng.ng_GadgetText = "Cancel";
    ng.ng_GadgetID = SETTYPECANCEL;
    SetTypeCancel = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** Define New *****/
    ng.ng_LeftEdge = 140;
    ng.ng_TopEdge = 148 + XtraTop;
    ng.ng_Width = 114;
    ng.ng_GadgetText = "Define New...";
    ng.ng_GadgetID = DEFINEDRIVE;
    DefineDrive = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

    /***** "Edit Old" *****/
    ng.ng_LeftEdge = 263;
    ng.ng_TopEdge = 148 + XtraTop;
    ng.ng_GadgetText = "Edit Old...";
    ng.ng_GadgetID = EDITTYPE;
    EditType = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (EditTypeFlags & GADGDISABLED), TAG_DONE);

    /***** "Delete Old" *****/
    ng.ng_LeftEdge = 386;
    ng.ng_GadgetText = "Delete Old";
    ng.ng_GadgetID = DELETETYPE;
    DeleteType = gad = CreateGadget(BUTTON_KIND, gad, &ng,
			GA_Disabled, (DeleteTypeFlags & GADGDISABLED), TAG_DONE);

    AllocTypeList();

    /***** Drive Type List *****/
    ng.ng_LeftEdge = 186;
    ng.ng_TopEdge = 56 + XtraTop;
    ng.ng_Width = 268;
    ng.ng_Height = 80;
    ng.ng_GadgetText = NULL;
    ng.ng_GadgetID = TYPELIST;
    ng.ng_Flags = 0;
    TypeList = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &lh12,
	GTLV_Top, 0,
	LAYOUTA_SPACING, 1,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, TypeListPosition,
	GTLV_ScrollWidth, 18,
	TAG_DONE);

    glist12 = glist;
    return(gad);
}


/**************************************************************************
***
*** void do_reposition(struct IntuiText *)
***
***	- Addjust text positions which depend on the system font.
***
**************************************************************************/
void do_reposition(struct IntuiText *itext)
{
    struct IntuiText *it;

    it = itext;
    while(it)
    {
	it->TopEdge += XtraTop;
	it = it->NextText;
    }
}

void do_repositionRequester(struct Requester *req)
{
    req->TopEdge += XtraTop;
}


void MakePrettyRequestBorder(UWORD width, UWORD height)
{
	extern SHORT BorderVectorsW[];
	extern SHORT BorderVectorsB[];

	BorderVectorsW[4] = width - 2;
	BorderVectorsB[2] = BorderVectorsB[4] = width - 1;
	BorderVectorsW[1] = height - 2;
	BorderVectorsB[1] = BorderVectorsB[3] = height - 1;
/*
	BorderVectorsW[0] = width + 1;
	BorderVectorsB[2] = BorderVectorsB[4] = width + 1;
	BorderVectorsB[6] = BorderVectorsB[8] = width + 2;

	BorderVectorsW[5] = height + 1;
	BorderVectorsW[7] = height + 2;
	BorderVectorsB[1] = BorderVectorsB[3] = BorderVectorsB[9] = height + 1;
*/
}


