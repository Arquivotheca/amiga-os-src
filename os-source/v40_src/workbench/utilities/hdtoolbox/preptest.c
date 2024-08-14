
/* header for powerwindows file - pw.h */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

#include "refresh.h"
#include "gadgetids.h"
#include "global.h"

UBYTE UNDOBUFFER[128];

struct TextAttr TOPAZ80 = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};
struct IntuiText IText1 = {
	1,0,JAM1,
	30,2,
	&TOPAZ80,
	"Change Drive Type",
	NULL
};

struct Gadget SetType = {
	NULL,
	39,85,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText1,
	NULL,
	NULL,
	SETTYPE|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText2 = {
	1,0,JAM1,
	10,4,
	&TOPAZ80,
	"Help",
	NULL
};

struct Gadget HelpPrep = {
	&SetType,
	288,106,
	53,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText2,
	NULL,
	NULL,
	HELPPREP|BORDER,
	NULL
};

struct IntuiText IText3 = {
	1,0,JAM1,
	12,2,
	&TOPAZ80,
	"Low-level Format Drive",
	NULL
};

struct Gadget LowFormat = {
	&HelpPrep,
	40,129,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText3,
	NULL,
	NULL,
	LOWFORMAT|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText4 = {
	1,0,JAM1,
	15,2,
	&TOPAZ80,
	"Modify Bad Block List",
	NULL
};

struct Gadget BadBlock = {
	&LowFormat,
	39,107,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText4,
	NULL,
	NULL,
	BADBLOCK|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText5 = {
	1,0,JAM1,
	74,4,
	&TOPAZ80,
	"Exit",
	NULL
};

struct Gadget ExitPrep = {
	&BadBlock,
	224,168,
	175,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText5,
	NULL,
	NULL,
	EXITPREP|BORDER|FANCY,
	NULL
};

struct IntuiText IText6 = {
	1,0,JAM1,
	37,2,
	&TOPAZ80,
	"Partition Drive",
	NULL
};

struct Gadget PartitionDrive = {
	&ExitPrep,
	388,85,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText6,
	NULL,
	NULL,
	PARTITIONDRIVE|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText7 = {
	1,0,JAM1,
	16,2,
	&TOPAZ80,
	"Save Changes to Drive",
	NULL
};

struct Gadget FormatDrive = {
	&PartitionDrive,
	388,129,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText7,
	NULL,
	NULL,
	FORMATDRIVE|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText8 = {
	1,0,JAM1,
	18,2,
	&TOPAZ80,
	"Verify Data on Drive",
	NULL
};

struct Gadget SurfCheck = {
	&FormatDrive,
	388,107,
	195,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText8,
	NULL,
	NULL,
	SURFCHECK|BORDER|DROPSHADOW,
	NULL
};

USHORT ImageData1[] = {
	0x0000,0x0000,0x0060,0x0000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x3861,0xC000,0x1E67,0x8000,0x07FE,0x0000,
	0x01F8,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xC79E,0x3000,0xE198,0x7000,0xF801,0xF000,0xFE07,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image1 = {
	0,0,
	20,11,
	2,
	ImageData1,
	0x0003,0x0000,
	NULL
};

struct Gadget ListDown = {
	&SurfCheck,
	612,63,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image1,
	NULL,
	NULL,
	NULL,
	NULL,
	LISTDOWN|DONTDRAW,
	NULL
};

USHORT ImageData2[] = {
	0x0000,0x0000,0x0060,0x0000,0x01F8,0x0000,0x07FE,0x0000,
	0x1E67,0x8000,0x3861,0xC000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFE07,0xF000,0xF801,0xF000,0xE198,0x7000,
	0xC79E,0x3000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image2 = {
	0,0,
	20,11,
	2,
	ImageData2,
	0x0003,0x0000,
	NULL
};

struct Gadget ListUp = {
	&ListDown,
	612,35,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image2,
	NULL,
	NULL,
	NULL,
	NULL,
	LISTUP|DONTDRAW,
	NULL
};

struct PropInfo DriveDragSInfo = {
	AUTOKNOB+FREEVERT,
	-1,-1,
	-1,-1,
};

struct Image Image3 = {
	0,0,
	12,11,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

struct Gadget DriveDrag = {
	&ListUp,
	612,47,
	20,15,
	NULL,
	RELVERIFY,
	PROPGADGET,
	(APTR)&Image3,
	NULL,
	NULL,
	NULL,
	(APTR)&DriveDragSInfo,
	DRIVEDRAG|DONTDRAW,
	NULL
};

SHORT BorderVectors1[] = {
	0,0,
	605,0,
	605,37,
	0,37,
	0,0
};
struct Border Border1 = {
	-2,-1,
	1,0,JAM1,
	5,
	BorderVectors1,
	NULL
};

struct Gadget DriveList = {
	&DriveDrag,
	6,36,
	602,36,
	GADGHBOX+GADGHIMAGE,
	GADGIMMEDIATE,
	BOOLGADGET,
	(APTR)&Border1,
	NULL,
	NULL,
	NULL,
	NULL,
	DRIVELIST|BORDER,
	NULL
};

#define GadgetList1 DriveList

struct IntuiText IText9 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Make the HD window full-size",
	NULL
};

struct MenuItem MenuItem2 = {
	NULL,
	0,9,
	224,8,
	ITEMTEXT+HIGHCOMP,
	0,
	(APTR)&IText9,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

struct IntuiText IText10 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Make the HD window small",
	NULL
};

struct MenuItem MenuItem1 = {
	&MenuItem2,
	0,0,
	224,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText10,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

struct Menu Menu1 = {
	NULL,
	0,0,
	129,0,
	MENUENABLED,
	"Resize Window",
	&MenuItem1
};

#define MenuList1 Menu1

struct IntuiText IText16 = {
	1,0,JAM1,
	221,14,
	&TOPAZ80,
	"Hard Drives in System",
	NULL
};

struct IntuiText IText15 = {
	1,0,JAM1,
	325,26,
	&TOPAZ80,
	"Drive Type",
	&IText16
};

struct IntuiText IText14 = {
	1,0,JAM1,
	230,26,
	&TOPAZ80,
	"Status",
	&IText15
};

struct IntuiText IText13 = {
	1,0,JAM1,
	7,26,
	&TOPAZ80,
	"Interface",
	&IText14
};

struct IntuiText IText12 = {
	1,0,JAM1,
	156,26,
	&TOPAZ80,
	"LUN",
	&IText13
};

struct IntuiText IText11 = {
	1,0,JAM1,
	86,26,
	&TOPAZ80,
	"Address",
	&IText12
};

#define IntuiTextList1 IText11

struct NewWindow NewWindowStructure1 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+MENUPICK+RAWKEY+REQCLEAR,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&DriveList,
	NULL,
	"Hard Drive Preparation, Partitioning and Formatting - Version " HDTVERSION,
	NULL,
	NULL,
	40,30,
	-1,-1,
	WBENCHSCREEN
};

struct IntuiText IText18 = {
	1,0,JAM1,
	30,11,
	&TOPAZ80,
	"File Systems",
	NULL
};

struct IntuiText IText17 = {
	1,0,JAM1,
	38,1,
	&TOPAZ80,
	"Add/Update",
	&IText18
};

struct Gadget AddFileSys = {
	NULL,
	373,172,
	151,20,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText17,
	NULL,
	NULL,
	ADDFILESYS|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText20 = {
	1,0,JAM1,
	24,11,
	&TOPAZ80,
	"for Partition",
	NULL
};

struct IntuiText IText19 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"Change File System",
	&IText20
};

struct Gadget ChangeFileSys = {
	&AddFileSys,
	373,145,
	151,20,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText19,
	NULL,
	NULL,
	CHANGEFILESYS|BORDER|DROPSHADOW,
	NULL
};

UBYTE BootPriSIBuff[13] =
	"0";
struct StringInfo BootPriSInfo = {
	BootPriSIBuff,
	UNDOBUFFER,
	0,
	5,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget BootPri = {
	&ChangeFileSys,
	308,174,
	40,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BootPriSInfo,
	BOOTPRI|BORDER,
	NULL
};

UBYTE BuffersSIBuff[6] =
	"30";
struct StringInfo BuffersSInfo = {
	BuffersSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget Buffers = {
	&BootPri,
	103,184,
	49,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BuffersSInfo,
	BUFFERS|BORDER,
	NULL
};

UBYTE TotalCylSIBuff[6] =
	"324";
struct StringInfo TotalCylSInfo = {
	TotalCylSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget TotalCyl = {
	&Buffers,
	103,169,
	49,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&TotalCylSInfo,
	TOTALCYL|BORDER,
	NULL
};

UBYTE EndCylSIBuff[6] =
	"1023";
struct StringInfo EndCylSInfo = {
	EndCylSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget EndCyl = {
	&TotalCyl,
	103,158,
	49,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&EndCylSInfo,
	ENDCYL|BORDER,
	NULL
};

UBYTE StartCylSIBuff[6] =
	"700";
struct StringInfo StartCylSInfo = {
	StartCylSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget StartCyl = {
	&EndCyl,
	103,147,
	49,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&StartCylSInfo,
	STARTCYL|BORDER,
	NULL
};

UBYTE NamePartSIBuff[32] =
	"F:";
struct StringInfo NamePartSInfo = {
	NamePartSIBuff,
	UNDOBUFFER,
	0,
	32,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NamePart = {
	&StartCyl,
	175,141,
	180,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NamePartSInfo,
	NAMEPART|BORDER,
	NULL
};

struct IntuiText IText21 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"Advanced Options",
	NULL
};

struct Gadget Advanced = {
	&NamePart,
	18,129,
	134,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText21,
	NULL,
	NULL,
	ADVANCED|BORDER,
	NULL
};

struct IntuiText IText22 = {
	1,0,JAM1,
	16,4,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget CancelPart = {
	&Advanced,
	547,178,
	76,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText22,
	NULL,
	NULL,
	CANCELPART|FANCY|BORDER,
	NULL
};

struct IntuiText IText23 = {
	1,0,JAM1,
	30,4,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget DonePart = {
	&CancelPart,
	547,154,
	76,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText23,
	NULL,
	NULL,
	DONEPART|FANCY|BORDER,
	NULL
};

struct IntuiText IText24 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"Help",
	NULL
};

struct Gadget Help = {
	&DonePart,
	480,109,
	40,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText24,
	NULL,
	NULL,
	HELP|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText25 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"Default Setup",
	NULL
};

struct Gadget QuickSetup = {
	&Help,
	359,109,
	110,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText25,
	NULL,
	NULL,
	QUICKSETUP|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText26 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"New Partition",
	NULL
};

struct Gadget NewPart = {
	&QuickSetup,
	237,109,
	110,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText26,
	NULL,
	NULL,
	NEWPART|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText27 = {
	1,0,JAM1,
	4,1,
	&TOPAZ80,
	"Delete Partition",
	NULL
};

struct Gadget DeletePart = {
	&NewPart,
	91,109,
	135,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText27,
	NULL,
	NULL,
	DELETEPART|BORDER|DROPSHADOW,
	NULL
};

struct PropInfo SizePartSInfo = {
	FREEHORIZ+PROPBORDERLESS,
	4167,-1,
	3276,1,
};

USHORT ImageData4[] = {
	0x0300,0x0780,0x0FC0,0x1FE0,0x3FF0,0x7FF8,0xFFFC,0x0300,
	0x0780,0x0FC0,0x1FE0,0x3FF0,0x7FF8,0xFFFC
};

struct Image Image4 = {
	11,0,
	14,7,
	2,
	ImageData4,
	0x0003,0x0000,
	NULL
};

USHORT ImageData5[] = {
	0x0300,0x0780,0x0FC0,0x1FE0,0x3FF0,0x7FF8,0xFFFC
};

struct Image Image5 = {
	11,0,
	14,7,
	2,
	ImageData5,
	0x0002,0x0000,
	NULL
};

struct Gadget SizePart = {
	&DeletePart,
	436,74,
	187,7,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY+GADGIMMEDIATE+FOLLOWMOUSE,
	PROPGADGET,
	(APTR)&Image4,
	(APTR)&Image5,
	NULL,
	NULL,
	(APTR)&SizePartSInfo,
	SIZEPART|DONTDRAW,
	NULL
};

struct PropInfo DragPartSInfo = {
	AUTOKNOB+FREEHORIZ+PROPBORDERLESS,
	12738,-1,
	3276,-1,
};

struct Image Image6 = {
	48,0,
	14,20,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

struct Gadget DragPart = {
	&SizePart,
	356,52,
	261,20,
	GADGHBOX+GADGHIMAGE,
	RELVERIFY+GADGIMMEDIATE+FOLLOWMOUSE,
	PROPGADGET,
	(APTR)&Image6,
	NULL,
	NULL,
	NULL,
	(APTR)&DragPartSInfo,
	DRAGPART|DONTDRAW,
	NULL
};

USHORT ImageData7[] = {
	0x0000,0x0000,0x0063,0x0000,0x0073,0x0000,0x007B,0x1E00,
	0x006F,0x3300,0x0067,0x3300,0x0063,0x3300,0x0063,0x1E00,
	0x0000,0x0000
};

struct Image Image7 = {
	0,0,
	31,9,
	2,
	ImageData7,
	0x0001,0x0000,
	NULL
};

USHORT ImageData8[] = {
	0x0000,0x0000,0x0C30,0x0000,0x0C30,0x0000,0x0663,0xC3E0,
	0x03C6,0x6600,0x0187,0xE3C0,0x0186,0x0060,0x03C3,0xC7C0,
	0x0000,0x0000
};

struct Image Image8 = {
	0,0,
	31,9,
	2,
	ImageData8,
	0x0001,0x0000,
	NULL
};

struct Gadget Bootable = {
	&DragPart,
	318,161,
	31,9,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Image7,
	(APTR)&Image8,
	NULL,
	NULL,
	NULL,
	BOOTABLE|DONTDRAW|BORDER,
	NULL
};

#define GadgetList2 Bootable

struct IntuiText IText43 = {
	1,0,JAM1,
	190,174,
	&TOPAZ80,
	"Boot Priority:",
	NULL
};

struct IntuiText IText42 = {
	1,0,JAM1,
	33,184,
	&TOPAZ80,
	"Buffers:",
	&IText43
};

struct IntuiText IText41 = {
	1,0,JAM1,
	17,169,
	&TOPAZ80,
	"Total Cyl:",
	&IText42
};

struct IntuiText IText40 = {
	1,0,JAM1,
	33,158,
	&TOPAZ80,
	"End Cyl:",
	&IText41
};

struct IntuiText IText39 = {
	1,0,JAM1,
	17,147,
	&TOPAZ80,
	"Start Cyl:",
	&IText40
};

struct IntuiText IText38 = {
	1,0,JAM1,
	230,162,
	&TOPAZ80,
	"Bootable?",
	&IText39
};

struct IntuiText IText37 = {
	1,2,JAM2,
	373,130,
	&TOPAZ80,
	"File System:                 ",
	&IText38
};

struct IntuiText IText36 = {
	1,0,JAM1,
	428,41,
	&TOPAZ80,
	"= Current partition",
	&IText37
};

struct IntuiText IText35 = {
	1,0,JAM1,
	238,41,
	&TOPAZ80,
	"= A partition",
	&IText36
};

struct IntuiText IText34 = {
	1,0,JAM1,
	89,41,
	&TOPAZ80,
	"= Unused",
	&IText35
};

struct IntuiText IText33 = {
	1,0,JAM1,
	181,130,
	&TOPAZ80,
	"Partition Device Name",
	&IText34
};

struct IntuiText IText32 = {
	1,0,JAM1,
	231,24,
	&TOPAZ80,
	"      Address  , LUN  ",
	&IText33
};

struct IntuiText IText31 = {
	1,0,JAM1,
	246,12,
	&TOPAZ80,
	"Partitioning Drive",
	&IText32
};

struct IntuiText IText30 = {
	1,0,JAM1,
	607,26,
	&TOPAZ80,
	"Cyl",
	&IText31
};

struct IntuiText IText29 = {
	1,0,JAM1,
	13,35,
	&TOPAZ80,
	"0",
	&IText30
};

struct IntuiText IText28 = {
	1,0,JAM1,
	6,26,
	&TOPAZ80,
	"Cyl",
	&IText29
};

#define IntuiTextList2 IText28

struct NewWindow NewWindowStructure2 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&Bootable,
	NULL,
	"Partitioning",
	NULL,
	NULL,
	20,20,
	-1,-1,
	WBENCHSCREEN
};

UBYTE CustomNumSIBuff[12] =
	"0";
struct StringInfo CustomNumSInfo = {
	CustomNumSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget CustomNum = {
	NULL,
	387,175,
	40,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&CustomNumSInfo,
	CUSTOMNUM|BORDER,
	NULL
};

USHORT ImageData9[] = {
	0x0000,0x0000,0x0063,0x0000,0x0073,0x0000,0x007B,0x1E00,
	0x006F,0x3300,0x0067,0x3300,0x0063,0x3300,0x0063,0x1E00,
	0x0000,0x0000
};

struct Image Image9 = {
	0,0,
	31,9,
	2,
	ImageData9,
	0x0001,0x0000,
	NULL
};

USHORT ImageData10[] = {
	0x0000,0x0000,0x0C30,0x0000,0x0C30,0x0000,0x0663,0xC3E0,
	0x03C6,0x6600,0x0187,0xE3C0,0x0186,0x0060,0x03C3,0xC7C0,
	0x0000,0x0000
};

struct Image Image10 = {
	0,0,
	31,9,
	2,
	ImageData10,
	0x0001,0x0000,
	NULL
};

struct Gadget CustomBoot = {
	&CustomNum,
	391,163,
	31,9,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Image9,
	(APTR)&Image10,
	NULL,
	NULL,
	NULL,
	CUSTOMBOOT|DONTDRAW|BORDER,
	NULL
};

USHORT ImageData11[] = {
	0x0000,0x0000,0x0063,0x0000,0x0073,0x0000,0x007B,0x1E00,
	0x006F,0x3300,0x0067,0x3300,0x0063,0x3300,0x0063,0x1E00,
	0x0000,0x0000
};

struct Image Image11 = {
	0,0,
	31,9,
	2,
	ImageData11,
	0x0001,0x0000,
	NULL
};

USHORT ImageData12[] = {
	0x0000,0x0000,0x0C30,0x0000,0x0C30,0x0000,0x0663,0xC3E0,
	0x03C6,0x6600,0x0187,0xE3C0,0x0186,0x0060,0x03C3,0xC7C0,
	0x0000,0x0000
};

struct Image Image12 = {
	0,0,
	31,9,
	2,
	ImageData12,
	0x0001,0x0000,
	NULL
};

struct Gadget AutoMount = {
	&CustomBoot,
	400,86,
	31,9,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Image11,
	(APTR)&Image12,
	NULL,
	NULL,
	NULL,
	AUTOMOUNT|DONTDRAW|BORDER,
	NULL
};

UBYTE MaskSIBuff[12] =
	"0x00fffffe";
struct StringInfo MaskSInfo = {
	MaskSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget Mask = {
	&AutoMount,
	145,128,
	88,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&MaskSInfo,
	MASK|BORDER,
	NULL
};

UBYTE ReservedEndSIBuff[12] =
	"0";
struct StringInfo ReservedEndSInfo = {
	ReservedEndSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget ReservedEnd = {
	&Mask,
	405,142,
	88,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&ReservedEndSInfo,
	RESERVEDEND|BORDER,
	NULL
};

UBYTE ReservedBeginSIBuff[12] =
	"2";
struct StringInfo ReservedBeginSInfo = {
	ReservedBeginSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget ReservedBegin = {
	&ReservedEnd,
	405,128,
	88,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&ReservedBeginSInfo,
	RESERVEDBEGIN|BORDER,
	NULL
};

UBYTE MaxTransferSIBuff[12] =
	"0x00ffffff";
struct StringInfo MaxTransferSInfo = {
	MaxTransferSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget MaxTransfer = {
	&ReservedBegin,
	145,142,
	88,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&MaxTransferSInfo,
	MAXTRANSFER|BORDER,
	NULL
};

UBYTE FSIdentifierSIBuff[12] =
	"0x444f5301";
struct StringInfo FSIdentifierSInfo = {
	FSIdentifierSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget FSIdentifier = {
	&MaxTransfer,
	145,114,
	88,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&FSIdentifierSInfo,
	FSIDENTIFIER|BORDER,
	NULL
};

struct IntuiText IText44 = {
	1,0,JAM1,
	37,2,
	&TOPAZ80,
	"Reserved Partition",
	NULL
};

struct Gadget ReservedFS = {
	&FSIdentifier,
	330,55,
	214,11,
	NULL,
	GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&IText44,
	NULL,
	NULL,
	RESERVEDFS|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText45 = {
	1,0,JAM1,
	47,2,
	&TOPAZ80,
	"Old File System",
	NULL
};

struct Gadget OldFS = {
	&ReservedFS,
	330,36,
	214,11,
	NULL,
	GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&IText45,
	NULL,
	NULL,
	OLDFS|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText46 = {
	1,0,JAM1,
	35,2,
	&TOPAZ80,
	"Custom File System",
	NULL
};

struct Gadget CustomFS = {
	&OldFS,
	96,55,
	214,11,
	NULL,
	GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&IText46,
	NULL,
	NULL,
	CUSTOMFS|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText47 = {
	1,0,JAM1,
	43,2,
	&TOPAZ80,
	"Fast File System",
	NULL
};

struct Gadget FastFS = {
	&CustomFS,
	96,36,
	214,11,
	NULL,
	GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&IText47,
	NULL,
	NULL,
	FASTFS|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText48 = {
	1,0,JAM1,
	16,6,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget FSCancel = {
	&FastFS,
	533,167,
	79,18,
	NULL,
	RELVERIFY+ENDGADGET,
	BOOLGADGET,
	NULL,
	NULL,
	&IText48,
	NULL,
	NULL,
	FSCANCEL|FANCY|BORDER,
	NULL
};

struct IntuiText IText49 = {
	1,0,JAM1,
	29,6,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget FSOk = {
	&FSCancel,
	533,144,
	79,18,
	NULL,
	RELVERIFY+ENDGADGET,
	BOOLGADGET,
	NULL,
	NULL,
	&IText49,
	NULL,
	NULL,
	FSOK|FANCY|BORDER,
	NULL
};

#define GadgetList3 FSOk

struct IntuiText IText59 = {
	1,0,JAM1,
	149,175,
	&TOPAZ80,
	"Number of custom boot blocks?",
	NULL
};

struct IntuiText IText58 = {
	1,0,JAM1,
	213,164,
	&TOPAZ80,
	"Use custom boot code?",
	&IText59
};

struct IntuiText IText57 = {
	1,0,JAM1,
	367,142,
	&TOPAZ80,
	"end:",
	&IText58
};

struct IntuiText IText56 = {
	1,0,JAM1,
	195,87,
	&TOPAZ80,
	"Automount this partition?",
	&IText57
};

struct IntuiText IText55 = {
	1,0,JAM1,
	238,17,
	&TOPAZ80,
	"Partition                                  ",
	&IText56
};

struct IntuiText IText54 = {
	1,0,JAM1,
	253,114,
	&TOPAZ80,
	"Reserved blocks at",
	&IText55
};

struct IntuiText IText53 = {
	1,0,JAM1,
	33,142,
	&TOPAZ80,
	"MaxTransfer =",
	&IText54
};

struct IntuiText IText52 = {
	1,0,JAM1,
	89,128,
	&TOPAZ80,
	"Mask =",
	&IText53
};

struct IntuiText IText51 = {
	1,0,JAM1,
	319,128,
	&TOPAZ80,
	"beginning:",
	&IText52
};

struct IntuiText IText50 = {
	1,0,JAM1,
	41,114,
	&TOPAZ80,
	"Identifier =",
	&IText51
};

#define IntuiTextList3 IText50

struct NewWindow NewWindowStructure3 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&FSOk,
	NULL,
	"File System Characteristics",
	NULL,
	NULL,
	20,20,
	-1,-1,
	WBENCHSCREEN
};

struct IntuiText IText60 = {
	1,0,JAM1,
	6,3,
	&TOPAZ80,
	"Update File System",
	NULL
};

struct Gadget FSMUpdate = {
	NULL,
	349,110,
	160,14,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText60,
	NULL,
	NULL,
	BORDER|DROPSHADOW|FSMUPDATE,
	NULL
};

struct IntuiText IText61 = {
	1,0,JAM1,
	6,3,
	&TOPAZ80,
	"Delete File System",
	NULL
};

struct Gadget FSMDelete = {
	&FSMUpdate,
	180,110,
	160,14,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText61,
	NULL,
	NULL,
	BORDER|DROPSHADOW|FSMDELETE,
	NULL
};

struct IntuiText IText62 = {
	1,0,JAM1,
	4,3,
	&TOPAZ80,
	"Add New File System",
	NULL
};

struct Gadget FSMAdd = {
	&FSMDelete,
	11,110,
	160,14,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText62,
	NULL,
	NULL,
	BORDER|DROPSHADOW|FSMADD,
	NULL
};

struct IntuiText IText63 = {
	1,0,JAM1,
	10,4,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget FSMCancel = {
	&FSMAdd,
	544,170,
	68,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText63,
	NULL,
	NULL,
	FANCY|BORDER|FSMCANCEL,
	NULL
};

struct IntuiText IText64 = {
	1,0,JAM1,
	26,4,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget FsmOK = {
	&FSMCancel,
	544,148,
	68,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText64,
	NULL,
	NULL,
	FANCY|BORDER|FSMOK,
	NULL
};

USHORT ImageData13[] = {
	0x0000,0x0000,0x0060,0x0000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x3861,0xC000,0x1E67,0x8000,0x07FE,0x0000,
	0x01F8,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xC79E,0x3000,0xE198,0x7000,0xF801,0xF000,0xFE07,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image13 = {
	0,0,
	20,11,
	2,
	ImageData13,
	0x0003,0x0000,
	NULL
};

struct Gadget FSMDown = {
	&FsmOK,
	512,92,
	20,11,
	GADGIMAGE,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	(APTR)&Image13,
	NULL,
	NULL,
	NULL,
	NULL,
	FSMDOWN|DONTDRAW,
	NULL
};

USHORT ImageData14[] = {
	0x0000,0x0000,0x0060,0x0000,0x01F8,0x0000,0x07FE,0x0000,
	0x1E67,0x8000,0x3861,0xC000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFE07,0xF000,0xF801,0xF000,0xE198,0x7000,
	0xC79E,0x3000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image14 = {
	0,0,
	20,11,
	2,
	ImageData14,
	0x0003,0x0000,
	NULL
};

struct Gadget FSMUp = {
	&FSMDown,
	512,52,
	20,11,
	GADGIMAGE,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	(APTR)&Image14,
	NULL,
	NULL,
	NULL,
	NULL,
	FSMUP|DONTDRAW,
	NULL
};

struct PropInfo FSMDragSInfo = {
	AUTOKNOB+FREEVERT,
	-1,-1,
	-1,-1,
};

struct Image Image15 = {
	0,0,
	12,25,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

struct Gadget FSMDrag = {
	&FSMUp,
	512,63,
	20,29,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	PROPGADGET,
	(APTR)&Image15,
	NULL,
	NULL,
	NULL,
	(APTR)&FSMDragSInfo,
	FSMDRAG|DONTDRAW,
	NULL
};

SHORT BorderVectors2[] = {
	0,0,
	505,0,
	505,51,
	0,51,
	0,0
};
struct Border Border2 = {
	-2,-1,
	1,0,JAM1,
	5,
	BorderVectors2,
	NULL
};

struct Gadget FSMList = {
	&FSMDrag,
	7,52,
	502,50,
	GADGHBOX+GADGHIMAGE,
	GADGIMMEDIATE,
	BOOLGADGET,
	(APTR)&Border2,
	NULL,
	NULL,
	NULL,
	NULL,
	BORDER|FSMLIST,
	NULL
};

#define GadgetList4 FSMList

struct IntuiText IText69 = {
	1,0,JAM1,
	123,41,
	&TOPAZ80,
	"Version",
	NULL
};

struct IntuiText IText68 = {
	1,0,JAM1,
	13,41,
	&TOPAZ80,
	"Identifier",
	&IText69
};

struct IntuiText IText67 = {
	1,0,JAM1,
	148,21,
	&TOPAZ80,
	"File Systems on        Address  , LUN   ",
	&IText68
};

struct IntuiText IText66 = {
	1,0,JAM1,
	202,41,
	&TOPAZ80,
	"Size",
	&IText67
};

struct IntuiText IText65 = {
	1,0,JAM1,
	261,41,
	&TOPAZ80,
	"File System Name",
	&IText66
};

#define IntuiTextList4 IText65

struct NewWindow NewWindowStructure4 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&FSMList,
	NULL,
	"File System Maintainance",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

UBYTE NDCylBlocksSIBuff[10] =
	"102";
struct StringInfo NDCylBlocksSInfo = {
	NDCylBlocksSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDCylBlocks = {
	NULL,
	185,114,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDCylBlocksSInfo,
	BORDER|NDCYLBLOCKS,
	NULL
};

USHORT ImageData16[] = {
	0x0000,0x0000,0x0063,0x0000,0x0073,0x0000,0x007B,0x1E00,
	0x006F,0x3300,0x0067,0x3300,0x0063,0x3300,0x0063,0x1E00,
	0x0000,0x0000
};

struct Image Image16 = {
	0,0,
	31,9,
	2,
	ImageData16,
	0x0001,0x0000,
	NULL
};

USHORT ImageData17[] = {
	0x0000,0x0000,0x0C30,0x0000,0x0C30,0x0000,0x0663,0xC3E0,
	0x03C6,0x6600,0x0187,0xE3C0,0x0186,0x0060,0x03C3,0xC7C0,
	0x0000,0x0000
};

struct Image Image17 = {
	0,0,
	31,9,
	2,
	ImageData17,
	0x0001,0x0000,
	NULL
};

struct Gadget NDReselect = {
	&NDCylBlocks,
	529,105,
	31,9,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Image16,
	(APTR)&Image17,
	NULL,
	NULL,
	NULL,
	DONTDRAW|NDRESELECT|BORDER,
	NULL
};

UBYTE NDReducedSIBuff[10] =
	"820";
struct StringInfo NDReducedSInfo = {
	NDReducedSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDReduced = {
	&NDReselect,
	185,145,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDReducedSInfo,
	BORDER|NDREDUCED,
	NULL
};

UBYTE NDPreCompSIBuff[10] =
	"820";
struct StringInfo NDPreCompSInfo = {
	NDPreCompSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDPreComp = {
	&NDReduced,
	185,166,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDPreCompSInfo,
	BORDER|NDPRECOMP,
	NULL
};

UBYTE NDRevisionNameSIBuff[5] =
	"4.8";
struct StringInfo NDRevisionNameSInfo = {
	NDRevisionNameSIBuff,
	UNDOBUFFER,
	0,
	5,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDRevisionName = {
	&NDPreComp,
	284,65,
	158,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDRevisionNameSInfo,
	BORDER|NDREVISIONNAME,
	NULL
};

UBYTE NDDriveNameSIBuff[17] =
	"ST-251N";
struct StringInfo NDDriveNameSInfo = {
	NDDriveNameSIBuff,
	UNDOBUFFER,
	0,
	17,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDDriveName = {
	&NDRevisionName,
	284,54,
	158,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDDriveNameSInfo,
	BORDER|NDDRIVENAME,
	NULL
};

UBYTE NDManuNameSIBuff[9] =
	"Seagate";
struct StringInfo NDManuNameSInfo = {
	NDManuNameSIBuff,
	UNDOBUFFER,
	0,
	9,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDManuName = {
	&NDDriveName,
	284,43,
	158,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDManuNameSInfo,
	BORDER|NDMANUNAME,
	NULL
};

UBYTE NDBlocksSIBuff[10] =
	"17";
struct StringInfo NDBlocksSInfo = {
	NDBlocksSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDBlocks = {
	&NDManuName,
	185,103,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDBlocksSInfo,
	BORDER|NDBLOCKS,
	NULL
};

UBYTE NDHeadsSIBuff[10] =
	"6";
struct StringInfo NDHeadsSInfo = {
	NDHeadsSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDHeads = {
	&NDBlocks,
	185,92,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDHeadsSInfo,
	BORDER|NDHEADS,
	NULL
};

UBYTE NDCylindersSIBuff[10] =
	"820";
struct StringInfo NDCylindersSInfo = {
	NDCylindersSIBuff,
	UNDOBUFFER,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDCylinders = {
	&NDHeads,
	185,81,
	64,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDCylindersSInfo,
	BORDER|NDCYLINDERS,
	NULL
};

struct IntuiText IText71 = {
	1,0,JAM1,
	35,10,
	&TOPAZ80,
	"From Drive",
	NULL
};

struct IntuiText IText70 = {
	1,0,JAM1,
	2,1,
	&TOPAZ80,
	"Read Configuration",
	&IText71
};

struct Gadget NDReadParms = {
	&NDCylinders,
	466,43,
	148,19,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText70,
	NULL,
	NULL,
	BORDER|DROPSHADOW|NDREADPARMS,
	NULL
};

UBYTE NDParkWhereSIBuff[6] =
	"820";
struct StringInfo NDParkWhereSInfo = {
	NDParkWhereSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDParkWhere = {
	&NDReadParms,
	499,130,
	56,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDParkWhereSInfo,
	BORDER|NDPARKWHERE,
	NULL
};

UBYTE NDNameSIBuff[128] =
	"drive definitions";
struct StringInfo NDNameSInfo = {
	NDNameSIBuff,
	UNDOBUFFER,
	0,
	128,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget NDName = {
	&NDParkWhere,
	284,32,
	158,8,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&NDNameSInfo,
	BORDER|NDNAME,
	NULL
};

struct IntuiText IText72 = {
	1,0,JAM1,
	16,5,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget NDCancel = {
	&NDName,
	543,175,
	79,18,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText72,
	NULL,
	NULL,
	FANCY|BORDER|NDCANCEL,
	NULL
};

struct IntuiText IText73 = {
	1,0,JAM1,
	29,6,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget NDOk = {
	&NDCancel,
	543,151,
	79,18,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText73,
	NULL,
	NULL,
	FANCY|BORDER|NDOK,
	NULL
};

#define GadgetList5 NDOk

struct IntuiText IText89 = {
	1,0,JAM1,
	22,114,
	&TOPAZ80,
	"Blocks per Cylinder:",
	NULL
};

struct IntuiText IText88 = {
	1,0,JAM1,
	277,106,
	&TOPAZ80,
	"Supports reselection?",
	&IText89
};

struct IntuiText IText87 = {
	1,0,JAM1,
	110,166,
	&TOPAZ80,
	"Cylinder:",
	&IText88
};

struct IntuiText IText86 = {
	1,0,JAM1,
	70,156,
	&TOPAZ80,
	"Write Precomp",
	&IText87
};

struct IntuiText IText85 = {
	1,0,JAM1,
	110,145,
	&TOPAZ80,
	"Cylinder:",
	&IText86
};

struct IntuiText IText84 = {
	1,0,JAM1,
	5,135,
	&TOPAZ80,
	"Reduced Write Current",
	&IText85
};

struct IntuiText IText83 = {
	1,0,JAM1,
	158,65,
	&TOPAZ80,
	"Drive Revision:",
	&IText84
};

struct IntuiText IText82 = {
	1,0,JAM1,
	190,54,
	&TOPAZ80,
	"Drive Name:",
	&IText83
};

struct IntuiText IText81 = {
	1,0,JAM1,
	206,32,
	&TOPAZ80,
	"Filename:",
	&IText82
};

struct IntuiText IText80 = {
	1,0,JAM1,
	278,130,
	&TOPAZ80,
	"Park head where (cylinder):",
	&IText81
};

struct IntuiText IText79 = {
	1,0,JAM1,
	255,92,
	&TOPAZ80,
	"Size:",
	&IText80
};

struct IntuiText IText78 = {
	1,0,JAM1,
	46,103,
	&TOPAZ80,
	"Blocks per Track:",
	&IText79
};

struct IntuiText IText77 = {
	1,0,JAM1,
	134,92,
	&TOPAZ80,
	"Heads:",
	&IText78
};

struct IntuiText IText76 = {
	1,0,JAM1,
	102,81,
	&TOPAZ80,
	"Cylinders:",
	&IText77
};

struct IntuiText IText75 = {
	1,0,JAM1,
	221,19,
	&TOPAZ80,
	"Define a New Drive Type",
	&IText76
};

struct IntuiText IText74 = {
	1,0,JAM1,
	126,43,
	&TOPAZ80,
	"Manufacturers name:",
	&IText75
};

#define IntuiTextList5 IText74

struct NewWindow NewWindowStructure5 = {
	0,200,
	640,200,
	0,1,
	MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&NDOk,
	NULL,
	"Define/Edit Drive Type",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

SHORT BorderVectors3[] = {
	0,0,
	184,0,
	184,15,
	0,15,
	0,0
};
struct Border Border3 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors3,
	NULL
};

struct IntuiText IText90 = {
	2,0,JAM1,
	2,3,
	&TOPAZ80,
	"Don't Low Level Format",
	NULL
};

struct Gadget NoLLFormat = {
	NULL,
	236,64,
	181,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border3,
	NULL,
	&IText90,
	NULL,
	NULL,
	BORDER|NOLLFORMAT,
	NULL
};

SHORT BorderVectors4[] = {
	0,0,
	175,0,
	175,15,
	0,15,
	0,0
};
struct Border Border4 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors4,
	NULL
};

struct IntuiText IText91 = {
	2,0,JAM1,
	2,3,
	&TOPAZ80,
	"Low Level Format Disk",
	NULL
};

struct Gadget LLFormat = {
	&NoLLFormat,
	24,64,
	172,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border4,
	NULL,
	&IText91,
	NULL,
	NULL,
	BORDER|LLFORMAT,
	NULL
};

#define GadgetList6 LLFormat

struct IntuiText IText94 = {
	2,0,JAM1,
	132,42,
	&TOPAZ80,
	"stored on the drive!",
	NULL
};

struct IntuiText IText93 = {
	2,0,JAM1,
	28,32,
	&TOPAZ80,
	"Warning: Low level format will destroy everything",
	&IText94
};

struct IntuiText IText92 = {
	2,0,JAM1,
	11,13,
	&TOPAZ80,
	"Are you sure you want to low-level format this drive?",
	&IText93
};

#define IntuiTextList6 IText92

struct Requester RequesterStructure6 = {
	NULL,
	92,47,
	448,89,
	0,0,
	&GadgetList6,
	NULL,
	&IntuiTextList6,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

UBYTE GetVersionSIBuff[11] =
	"0";
struct StringInfo GetVersionSInfo = {
	GetVersionSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget GetVersion = {
	NULL,
	99,71,
	100,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&GetVersionSInfo,
	GETVERSION,
	NULL
};

UBYTE GetDosTypeSIBuff[11] =
	"0x12345678";
struct StringInfo GetDosTypeSInfo = {
	GetDosTypeSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget GetDosType = {
	&GetVersion,
	99,46,
	100,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&GetDosTypeSInfo,
	GETDOSTYPE,
	NULL
};

UBYTE GetFileNameSIBuff[100] =
	"l:FastFileSystem";
struct StringInfo GetFileNameSInfo = {
	GetFileNameSIBuff,
	UNDOBUFFER,
	0,
	100,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget GetFileName = {
	&GetDosType,
	32,22,
	239,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&GetFileNameSInfo,
	GETFILENAME,
	NULL
};

SHORT BorderVectors5[] = {
	0,0,
	63,0,
	63,14,
	0,14,
	0,0
};
struct Border Border5 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors5,
	NULL
};

struct IntuiText IText95 = {
	2,0,JAM1,
	7,3,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget FsGetNameCancel = {
	&GetFileName,
	219,98,
	60,13,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border5,
	NULL,
	&IText95,
	NULL,
	NULL,
	FSGETNAMECANCEL,
	NULL
};

SHORT BorderVectors6[] = {
	0,0,
	63,0,
	63,14,
	0,14,
	0,0
};
struct Border Border6 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors6,
	NULL
};

struct IntuiText IText96 = {
	2,0,JAM1,
	23,3,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget FsGetNameOk = {
	&FsGetNameCancel,
	22,98,
	60,13,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border6,
	NULL,
	&IText96,
	NULL,
	NULL,
	FSGETNAMEOK,
	NULL
};

#define GadgetList7 FsGetNameOk

struct IntuiText IText99 = {
	2,0,JAM1,
	36,60,
	&TOPAZ80,
	"Enter Version for File System",
	NULL
};

struct IntuiText IText98 = {
	2,0,JAM1,
	37,36,
	&TOPAZ80,
	"Enter DosType for File System",
	&IText99
};

struct IntuiText IText97 = {
	2,0,JAM1,
	37,12,
	&TOPAZ80,
	"Enter Filename of File System",
	&IText98
};

#define IntuiTextList7 IText97

struct Requester RequesterStructure7 = {
	NULL,
	154,28,
	299,119,
	0,0,
	&GadgetList7,
	NULL,
	&IntuiTextList7,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

SHORT BorderVectors7[] = {
	0,0,
	82,0,
	82,11,
	0,11,
	0,0
};
struct Border Border7 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors7,
	NULL
};

struct IntuiText IText100 = {
	2,0,JAM1,
	16,1,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget SureCancelGad = {
	NULL,
	-85,-13,
	79,10,
	GRELBOTTOM+GRELRIGHT,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border7,
	NULL,
	&IText100,
	NULL,
	NULL,
	NULL,
	NULL
};

SHORT BorderVectors8[] = {
	0,0,
	82,0,
	82,11,
	0,11,
	0,0
};
struct Border Border8 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors8,
	NULL
};

struct IntuiText IText101 = {
	2,0,JAM1,
	8,1,
	&TOPAZ80,
	"Continue",
	NULL
};

struct Gadget SureContinueGad = {
	&SureCancelGad,
	6,-13,
	79,10,
	GRELBOTTOM,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border8,
	NULL,
	&IText101,
	NULL,
	NULL,
	NULL,
	NULL
};

#define GadgetList8 SureContinueGad

struct Requester RequesterStructure8 = {
	NULL,
	125,53,
	342,57,
	0,0,
	&GadgetList8,
	NULL,
	NULL,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

struct IntuiText IText102 = {
	2,0,JAM1,
	39,20,
	&TOPAZ80,
	"Formatting drive: please wait",
	NULL
};

#define IntuiTextList9 IText102

struct Requester RequesterStructure9 = {
	NULL,
	141,59,
	314,48,
	0,0,
	NULL,
	NULL,
	&IntuiTextList9,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

struct IntuiText IText103 = {
	1,0,JAM1,
	28,7,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget BadOk = {
	NULL,
	555,144,
	69,21,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText103,
	NULL,
	NULL,
	BADOK|BORDER|FANCY,
	NULL
};

struct IntuiText IText104 = {
	1,0,JAM1,
	12,7,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget BadCancel = {
	&BadOk,
	555,171,
	69,21,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText104,
	NULL,
	NULL,
	BADCANCEL|BORDER|FANCY,
	NULL
};

struct IntuiText IText105 = {
	1,0,JAM1,
	5,2,
	&TOPAZ80,
	"Delete Bad Block",
	NULL
};

struct Gadget BadDelete = {
	&BadCancel,
	487,79,
	137,11,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText105,
	NULL,
	NULL,
	BADDELETE|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText106 = {
	1,0,JAM1,
	17,2,
	&TOPAZ80,
	"Add Bad Block",
	NULL
};

struct Gadget BadAdd = {
	&BadDelete,
	487,62,
	137,11,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText106,
	NULL,
	NULL,
	BADADD|BORDER|DROPSHADOW,
	NULL
};

struct PropInfo BadDragSInfo = {
	AUTOKNOB+FREEVERT,
	-1,-1,
	-1,-1,
};

struct Image Image18 = {
	0,0,
	12,102,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

struct Gadget BadDrag = {
	&BadAdd,
	400,58,
	20,106,
	NULL,
	RELVERIFY,
	PROPGADGET,
	(APTR)&Image18,
	NULL,
	NULL,
	NULL,
	(APTR)&BadDragSInfo,
	BADDRAG|DONTDRAW,
	NULL
};

SHORT BorderVectors9[] = {
	0,0,
	393,0,
	393,128,
	0,128,
	0,0
};
struct Border Border9 = {
	-2,-1,
	1,0,JAM1,
	5,
	BorderVectors9,
	NULL
};

struct Gadget BadList = {
	&BadDrag,
	6,47,
	390,127,
	GADGHBOX+GADGHIMAGE,
	GADGIMMEDIATE,
	BOOLGADGET,
	(APTR)&Border9,
	NULL,
	NULL,
	NULL,
	NULL,
	BADLIST|BORDER,
	NULL
};

USHORT ImageData19[] = {
	0x0000,0x0000,0x0060,0x0000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x3861,0xC000,0x1E67,0x8000,0x07FE,0x0000,
	0x01F8,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xC79E,0x3000,0xE198,0x7000,0xF801,0xF000,0xFE07,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image19 = {
	0,0,
	20,11,
	2,
	ImageData19,
	0x0003,0x0000,
	NULL
};

struct Gadget BadDown = {
	&BadList,
	400,165,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image19,
	NULL,
	NULL,
	NULL,
	NULL,
	BADDOWN|DONTDRAW,
	NULL
};

USHORT ImageData20[] = {
	0x0000,0x0000,0x0060,0x0000,0x01F8,0x0000,0x07FE,0x0000,
	0x1E67,0x8000,0x3861,0xC000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFE07,0xF000,0xF801,0xF000,0xE198,0x7000,
	0xC79E,0x3000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image20 = {
	0,0,
	20,11,
	2,
	ImageData20,
	0x0003,0x0000,
	NULL
};

struct Gadget BadUp = {
	&BadDown,
	400,46,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image20,
	NULL,
	NULL,
	NULL,
	NULL,
	BADUP|DONTDRAW,
	NULL
};

#define GadgetList10 BadUp

struct IntuiText IText110 = {
	1,0,JAM1,
	35,183,
	&TOPAZ80,
	"Bad Blocks mapped out by drive: xxxxxxxxxx",
	NULL
};

struct IntuiText IText109 = {
	1,0,JAM1,
	166,27,
	&TOPAZ80,
	"(approximate)",
	&IText110
};

struct IntuiText IText108 = {
	1,0,JAM1,
	8,36,
	&TOPAZ80,
	"Cylinder   Head   Bytes from Index   Sector",
	&IText109
};

struct IntuiText IText107 = {
	1,0,JAM1,
	249,15,
	&TOPAZ80,
	"Bad Block Entry",
	&IText108
};

#define IntuiTextList10 IText107

struct NewWindow NewWindowStructure10 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&BadUp,
	NULL,
	"Bad Blocks",
	NULL,
	NULL,
	40,30,
	-1,-1,
	WBENCHSCREEN
};

UBYTE BadSectorSIBuff[11] =
	"0";
struct StringInfo BadSectorSInfo = {
	BadSectorSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget BadSector = {
	NULL,
	22,80,
	96,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BadSectorSInfo,
	BADSEC,
	NULL
};

SHORT BorderVectors10[] = {
	0,0,
	63,0,
	63,15,
	0,15,
	0,0
};
struct Border Border10 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors10,
	NULL
};

struct IntuiText IText111 = {
	2,0,JAM1,
	7,3,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget BadGetCancel = {
	&BadSector,
	216,105,
	60,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border10,
	NULL,
	&IText111,
	NULL,
	NULL,
	BADGETCANCEL,
	NULL
};

SHORT BorderVectors11[] = {
	0,0,
	63,0,
	63,14,
	0,14,
	0,0
};
struct Border Border11 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors11,
	NULL
};

struct IntuiText IText112 = {
	2,0,JAM1,
	23,3,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget BadGetOK = {
	&BadGetCancel,
	22,105,
	60,13,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border11,
	NULL,
	&IText112,
	NULL,
	NULL,
	BADGETOK,
	NULL
};

UBYTE BadBfiSIBuff[11] =
	"0";
struct StringInfo BadBfiSInfo = {
	BadBfiSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget BadBfi = {
	&BadGetOK,
	178,80,
	96,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BadBfiSInfo,
	BADBFI,
	NULL
};

UBYTE BadHeadSIBuff[11];
struct StringInfo BadHeadSInfo = {
	BadHeadSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget BadHead = {
	&BadBfi,
	101,54,
	96,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BadHeadSInfo,
	BADHEAD,
	NULL
};

UBYTE BadCylinderSIBuff[11];
struct StringInfo BadCylinderSInfo = {
	BadCylinderSIBuff,
	UNDOBUFFER,
	0,
	11,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

struct Gadget BadCylinder = {
	&BadHead,
	101,30,
	96,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET+REQGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&BadCylinderSInfo,
	BADCYLINDER,
	NULL
};

#define GadgetList11 BadCylinder

struct IntuiText IText118 = {
	2,0,JAM1,
	140,81,
	&TOPAZ80,
	"or",
	NULL
};

struct IntuiText IText117 = {
	2,0,JAM1,
	47,70,
	&TOPAZ80,
	"Sector",
	&IText118
};

struct IntuiText IText116 = {
	2,0,JAM1,
	88,5,
	&TOPAZ80,
	"Enter Bad Block",
	&IText117
};

struct IntuiText IText115 = {
	2,0,JAM1,
	160,70,
	&TOPAZ80,
	"Bytes from Index",
	&IText116
};

struct IntuiText IText114 = {
	2,0,JAM1,
	132,45,
	&TOPAZ80,
	"Head",
	&IText115
};

struct IntuiText IText113 = {
	2,0,JAM1,
	117,20,
	&TOPAZ80,
	"Cylinder",
	&IText114
};

#define IntuiTextList11 IText113

struct Requester RequesterStructure11 = {
	NULL,
	172,24,
	299,127,
	0,0,
	&GadgetList11,
	NULL,
	&IntuiTextList11,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

struct IntuiText IText119 = {
	1,0,JAM1,
	29,3,
	&TOPAZ80,
	"Ok",
	NULL
};

struct Gadget SetTypeOK = {
	NULL,
	545,157,
	70,14,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText119,
	NULL,
	NULL,
	SETTYPEOK|BORDER|FANCY,
	NULL
};

struct IntuiText IText120 = {
	1,0,JAM1,
	13,3,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget SetTypeCancel = {
	&SetTypeOK,
	545,178,
	70,14,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText120,
	NULL,
	NULL,
	SETTYPECANCEL|BORDER|FANCY,
	NULL
};

struct IntuiText IText122 = {
	1,0,JAM1,
	7,9,
	&TOPAZ80,
	"drive type",
	NULL
};

struct IntuiText IText121 = {
	1,0,JAM1,
	7,1,
	&TOPAZ80,
	"Delete old",
	&IText122
};

struct Gadget DeleteType = {
	&SetTypeCancel,
	376,148,
	95,18,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText121,
	NULL,
	NULL,
	DELETETYPE|BORDER|DROPSHADOW,
	NULL
};

struct IntuiText IText124 = {
	1,0,JAM1,
	7,9,
	&TOPAZ80,
	"drive type",
	NULL
};

struct IntuiText IText123 = {
	1,0,JAM1,
	15,1,
	&TOPAZ80,
	"Edit old",
	&IText124
};

struct Gadget EditType = {
	&DeleteType,
	268,148,
	95,18,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText123,
	NULL,
	NULL,
	EDITTYPE|BORDER|DROPSHADOW,
	NULL
};

struct PropInfo TypeDragSInfo = {
	AUTOKNOB+FREEVERT,
	-1,-1,
	-1,-1,
};

struct Image Image21 = {
	0,0,
	12,55,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

struct Gadget TypeDrag = {
	&EditType,
	437,63,
	20,59,
	NULL,
	RELVERIFY,
	PROPGADGET,
	(APTR)&Image21,
	NULL,
	NULL,
	NULL,
	(APTR)&TypeDragSInfo,
	TYPEDRAG|DONTDRAW,
	NULL
};

SHORT BorderVectors12[] = {
	0,0,
	250,0,
	250,82,
	0,82,
	0,0
};
struct Border Border12 = {
	-2,-1,
	1,0,JAM1,
	5,
	BorderVectors12,
	NULL
};

struct Gadget TypeList = {
	&TypeDrag,
	186,52,
	247,81,
	GADGHBOX+GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Border12,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPELIST|BORDER,
	NULL
};

struct IntuiText IText126 = {
	1,0,JAM1,
	7,9,
	&TOPAZ80,
	"drive type",
	NULL
};

struct IntuiText IText125 = {
	1,0,JAM1,
	7,1,
	&TOPAZ80,
	"Define new",
	&IText126
};

struct Gadget DefineDrive = {
	&TypeList,
	160,148,
	95,18,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText125,
	NULL,
	NULL,
	DEFINEDRIVE|BORDER|DROPSHADOW,
	NULL
};

USHORT ImageData22[] = {
	0x0000,0x0000,0x0060,0x0000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x3861,0xC000,0x1E67,0x8000,0x07FE,0x0000,
	0x01F8,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xC79E,0x3000,0xE198,0x7000,0xF801,0xF000,0xFE07,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image22 = {
	0,0,
	20,11,
	2,
	ImageData22,
	0x0003,0x0000,
	NULL
};

struct Gadget TypeDown = {
	&DefineDrive,
	437,124,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image22,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPEDOWN|DONTDRAW,
	NULL
};

USHORT ImageData23[] = {
	0x0000,0x0000,0x0060,0x0000,0x01F8,0x0000,0x07FE,0x0000,
	0x1E67,0x8000,0x3861,0xC000,0x0060,0x0000,0x0060,0x0000,
	0x0060,0x0000,0x0060,0x0000,0x0000,0x0000,0xFFFF,0xF000,
	0xFF9F,0xF000,0xFE07,0xF000,0xF801,0xF000,0xE198,0x7000,
	0xC79E,0x3000,0xFF9F,0xF000,0xFF9F,0xF000,0xFF9F,0xF000,
	0xFF9F,0xF000,0xFFFF,0xF000
};

struct Image Image23 = {
	0,0,
	20,11,
	2,
	ImageData23,
	0x0003,0x0000,
	NULL
};

struct Gadget TypeUp = {
	&TypeDown,
	437,51,
	20,11,
	GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Image23,
	NULL,
	NULL,
	NULL,
	NULL,
	TYPEUP|DONTDRAW,
	NULL
};

struct IntuiText IText127 = {
	1,0,JAM1,
	2,1,
	&TOPAZ80,
	"ST-506",
	NULL
};

struct Gadget St506Type = {
	&TypeUp,
	360,34,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText127,
	NULL,
	NULL,
	ST506TYPE|BORDER,
	NULL
};

struct IntuiText IText128 = {
	1,0,JAM1,
	7,1,
	&TOPAZ80,
	"SCSI",
	NULL
};

struct Gadget SCSIType = {
	&St506Type,
	305,34,
	46,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText128,
	NULL,
	NULL,
	SCSITYPE|BORDER,
	NULL
};

#define GadgetList12 SCSIType

struct IntuiText IText129 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Make the HD window full-size",
	NULL
};

struct MenuItem MenuItem4 = {
	NULL,
	0,9,
	224,8,
	ITEMTEXT+HIGHCOMP,
	0,
	(APTR)&IText129,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

struct IntuiText IText130 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Make the HD window small",
	NULL
};

struct MenuItem MenuItem3 = {
	&MenuItem4,
	0,0,
	224,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText130,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

struct Menu Menu2 = {
	NULL,
	0,0,
	129,0,
	MENUENABLED,
	"Resize Window",
	&MenuItem3
};

#define MenuList12 Menu2

struct IntuiText IText132 = {
	1,0,JAM1,
	241,15,
	&TOPAZ80,
	"Select a drive type",
	NULL
};

struct IntuiText IText131 = {
	1,0,JAM1,
	204,35,
	&TOPAZ80,
	"Drive Types:",
	&IText132
};

#define IntuiTextList12 IText131

struct NewWindow NewWindowStructure12 = {
	0,0,
	640,200,
	0,1,
	NULL,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH,
	&SCSIType,
	NULL,
	"Set Drive Type",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

SHORT BorderVectors13[] = {
	0,0,
	184,0,
	184,15,
	0,15,
	0,0
};
struct Border Border13 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors13,
	NULL
};

struct IntuiText IText133 = {
	2,0,JAM1,
	36,3,
	&TOPAZ80,
	"Stop Checking",
	NULL
};

struct Gadget MapStop = {
	NULL,
	130,127,
	181,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border13,
	NULL,
	&IText133,
	NULL,
	NULL,
	BORDER|MAPSTOP,
	NULL
};

SHORT BorderVectors14[] = {
	0,0,
	184,0,
	184,15,
	0,15,
	0,0
};
struct Border Border14 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors14,
	NULL
};

struct IntuiText IText134 = {
	2,0,JAM1,
	26,3,
	&TOPAZ80,
	"Ignore Bad Block",
	NULL
};

struct Gadget MapIgnore = {
	&MapStop,
	237,101,
	181,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border14,
	NULL,
	&IText134,
	NULL,
	NULL,
	NULL,
	NULL
};

SHORT BorderVectors15[] = {
	0,0,
	175,0,
	175,15,
	0,15,
	0,0
};
struct Border Border15 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors15,
	NULL
};

struct IntuiText IText135 = {
	2,0,JAM1,
	16,3,
	&TOPAZ80,
	"Mark Block as Bad",
	NULL
};

struct Gadget MapOut = {
	&MapIgnore,
	26,101,
	172,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border15,
	NULL,
	&IText135,
	NULL,
	NULL,
	NULL,
	NULL
};

#define GadgetList13 MapOut

struct IntuiText IText144 = {
	2,0,JAM1,
	343,43,
	&TOPAZ80,
	"12345678901",
	NULL
};

struct IntuiText IText143 = {
	2,0,JAM1,
	149,54,
	&TOPAZ80,
	"123456789",
	&IText144
};

struct IntuiText IText142 = {
	2,0,JAM1,
	149,43,
	&TOPAZ80,
	"123456789",
	&IText143
};

struct IntuiText IText141 = {
	2,0,JAM1,
	149,32,
	&TOPAZ80,
	"123456789",
	&IText142
};

struct IntuiText IText140 = {
	2,0,JAM1,
	234,43,
	&TOPAZ80,
	"Block Number:",
	&IText141
};

struct IntuiText IText139 = {
	2,0,JAM1,
	87,54,
	&TOPAZ80,
	"Sector:",
	&IText140
};

struct IntuiText IText138 = {
	2,0,JAM1,
	103,43,
	&TOPAZ80,
	"Head:",
	&IText139
};

struct IntuiText IText137 = {
	2,0,JAM1,
	71,32,
	&TOPAZ80,
	"Cylinder:",
	&IText138
};

struct IntuiText IText136 = {
	2,0,JAM1,
	142,14,
	&TOPAZ80,
	"New bad block found!",
	&IText137
};

#define IntuiTextList13 IText136

struct Requester RequesterStructure13 = {
	NULL,
	79,19,
	444,153,
	0,0,
	&GadgetList13,
	NULL,
	&IntuiTextList13,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};
