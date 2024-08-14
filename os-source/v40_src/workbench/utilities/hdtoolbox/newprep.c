/* header for powerwindows file - pw.h */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>

#include "refresh.h"
#include "gadgetids.h"
#include "global.h"

UBYTE UNDOBUFFER[128];

struct StringExtend StringExt = {
	NULL,	/* put TOPAZ80 in testmain.c */
	1,0,
	1,0,
	0,
	NULL,
	NULL,
	0
};

extern struct TextAttr TOPAZ80;

/*******************************************************************************/

struct NewWindow NewWindowStructure1 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+INTUITICKS\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,	/* Flags */
	NULL,
	NULL,
//	"Hard Drive Preparation, Partitioning and Formatting - Version " HDTVERSION,
	"Hard Drive Preparation, Partitioning and Formatting",
	NULL,
	NULL,
	40,30,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/

UBYTE NamePartSIBuff[32] =
	"F:";
struct StringInfo NamePartSInfo = {
	NamePartSIBuff,
	UNDOBUFFER,
	0,
	32,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
	NULL
};


struct PropInfo SizePartSInfo = {
	FREEHORIZ+PROPBORDERLESS+PROPNEWLOOK,
	4167,-1,
	3276,1,
};

USHORT ImageData4[] = {
	0x0300,
	0x0780,
	0x0FC0,
	0x1FE0,
	0x3FF0,
	0x7FF8,
	0xFFFC,

	0x0300,
	0x0780,
	0x0FC0,
	0x1FE0,
	0x3FF0,
	0x7FF8,
	0xFFFC
};

/*
	00000011000000 00,	// Original
	00000111100000 00,
	00001111110000 00,
	00011111111000 00,
	00111111111100 00,
	01111111111110 00,
	11111111111111 00,

	1010 1011 0101 0100,
	0101 0111 1010 1000,
	1010 1111 1101 0100,
	0101 1111 1110 1000,
	1011 1111 1111 0100,
	0111 1111 1111 1000,
	1111 1111 1111 1100,
*/

/*
USHORT ImageData4[] = {
	0xaaa8,
	0x5754,
	0xafa8,
	0x5fd4,
	0xbfe8,
	0x7ff4,
	0xfffa,

	0x0200,
	0x0700,
	0x0f80,
	0x1fc0,
	0x3fe0,
	0x7ff0,
	0xfff8,

	0x0300,	// Original
	0x0780,
	0x0FC0,
	0x1FE0,
	0x3FF0,
	0x7FF8,
	0xFFFC,
};
*/

struct Image Image4[] = {
	11,0,
	14,7,
	2,
	ImageData4,
	0x0003,0x0000,
	NULL
};

USHORT ImageData5[] = {
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,

	0x0300,
	0x0780,
	0x0FC0,
	0x1FE0,
	0x3FF0,
	0x7FF8,
	0xFFFC,
};

/*
USHORT ImageData5[] = {
	0xfcff,
	0xf87f,
	0xf03f,
	0xe01f,
	0xc00f,
	0x8007,
	0x0003,

	0x0300,
	0x0780,
	0x0FC0,
	0x1FE0,
	0x3FF0,
	0x7FF8,
	0xFFFC
};
*/
struct Image Image5 = {
	11,0,
	14,7,
	2,
	ImageData5,
	0x0003,0x0000,
	NULL
};

struct Gadget SizePart = {
	NULL,
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
	AUTOKNOB+FREEHORIZ+PROPBORDERLESS/*+PROPNEWLOOK*/,
	12738,-1,
	3276,-1,
};

USHORT ImageData6[] = {
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,

	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
};

struct Image Image6 = {
	48,0,
	14,20,
//	0,
	2,
//	NULL,
	ImageData4,
//	0x0000,0x0000,
	0x0003,0x0000,
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

#define GadgetList2 DragPart

struct IntuiText IText32 = {
	1,0,JAM1,
	231,24,
	&TOPAZ80,
	"      Address  , LUN  ",
	NULL
};

#define IntuiTextList2 IText32

struct NewWindow NewWindowStructure2 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP/*+INTUITICKS*/\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	NULL,
	NULL,
	"Partitioning",
	NULL,
	NULL,
	20,20,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/

UBYTE MaskSIBuff[12] =
	"0x00fffffe";
struct StringInfo MaskSInfo = {
	MaskSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
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
	(APTR)&StringExt,
	0,
	NULL
};

UBYTE FSIdentifierSIBuff[12] =
	"0x444f5303";	/* 39.13 - now default is international ffs */
struct StringInfo FSIdentifierSInfo = {
	FSIdentifierSIBuff,
	UNDOBUFFER,
	0,
	12,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
	NULL
};


struct NewWindow NewWindowStructure3 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP/*+INTUITICKS*/\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	NULL,
	NULL,
	"File System Characteristics",
	NULL,
	NULL,
	20,20,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/
/*
struct IntuiText IText67 = {
	1,0,JAM1,
	160,18,
	&TOPAZ80,
	"File Systems on        Address  , LUN   ",
	NULL
};

#define IntuiTextList4 IText67
*/
struct NewWindow NewWindowStructure4 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+INTUITICKS\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	NULL,
	NULL,
	"File System Maintenance",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/

UBYTE NDRevisionNameSIBuff[5] =
	"4.8";
struct StringInfo NDRevisionNameSInfo = {
	NDRevisionNameSIBuff,
	UNDOBUFFER,
	0,
	5,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
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
	(APTR)&StringExt,
	0,
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
	(APTR)&StringExt,
	0,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct NewWindow NewWindowStructure5 = {
	0,200,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP/*+INTUITICKS*/\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	NULL,
	NULL,
	"Define/Edit Drive Type",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/

struct NewWindow NewWindowStructure10 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+INTUITICKS\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+REQSET+REQCLEAR+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	NULL,
	NULL,
	"Bad Blocks",
	NULL,
	NULL,
	40,30,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/

struct NewWindow NewWindowStructure12 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+MOUSEBUTTONS+GADGETDOWN+GADGETUP+INTUITICKS\
	+REQSET/*+MENUPICK*/+RAWKEY+REQCLEAR, /* IDCMPFlags */
//	NULL,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH,
	NULL,
	NULL,
	"Set Drive Type",
	NULL,
	NULL,
	5,5,
	-1,-1,
	WBENCHSCREEN
};

/*******************************************************************************/
/*
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
*/
/*******************************************************************************/

/***** Border stuff for Button Gadget *****/

SHORT BBorderVectorsB[] = {
	1,13,
	77,13,
	77,1,
	78,0,
	78,13,
};

struct Border BBorderBlack = {
	0,0,
	1,0,JAM1,
	5,
	BBorderVectorsB,
	NULL
};

SHORT BBorderVectorsW[] = {
	77,0,
	1,0,
	1,12,
	0,13,
	0,0,
};

struct Border BBorderWhite = {
	0,0,
	2,0,JAM1,
	5,
	BBorderVectorsW,
	(APTR)&BBorderBlack
};


/***** Border stuff for Integer Gadget *****/

SHORT IBorderVectorsB[] = {
	99,0,
	1,0,
	1,8,
	0,9,
	0,0,
	0,9,
	-1,10,
	101,10,
	101,0,
	102,-1,
	102,10,
};

struct Border IBorderBlack = {
	-2,-1,
	1,0,JAM1,
	11,
	IBorderVectorsB,
	NULL
};

SHORT IBorderVectorsW[] = {
	1,9,
	99,9,
	99,1,
	100,0,
	100,9,
	100,0,
	101,-1,
	-1,-1,
	-1,9,
	-2,10,
	-2,-1,
};

struct Border IBorderWhite = {
	-2,-1,
	2,0,JAM1,
	11,
	IBorderVectorsW,
	(APTR)&IBorderBlack
};


/***** Border stuff for Version Integer Gadget *****/

SHORT IVBorderVectorsB[] = {
	50,0,
	1,0,
	1,8,
	0,9,
	0,0,
	0,9,
	-1,10,
	52,10,
	52,0,
	53,-1,
	53,10,
};

struct Border IVBorderBlack = {
	-2,-1,
	1,0,JAM1,
	11,
	IVBorderVectorsB,
	NULL
};

SHORT IVBorderVectorsW[] = {
	1,9,
	50,9,
	50,1,
	51,0,
	51,9,
	51,0,
	52,-1,
	-1,-1,
	-1,9,
	-2,10,
	-2,-1,
};

struct Border IVBorderWhite = {
	-2,-1,
	2,0,JAM1,
	11,
	IVBorderVectorsW,
	(APTR)&IVBorderBlack
};


/***** Border stuff for String Gadget *****/

SHORT SBorderVectorsB[] = {
	235,0,
	1,0,
	1,8,
	0,9,
	0,0,
	0,9,
	-1,10,
	237,10,
	237,0,
	238,-1,
	238,10,
};

struct Border SBorderBlack = {
	-2,-1,
	1,0,JAM1,
	11,
	SBorderVectorsB,
	NULL
};

SHORT SBorderVectorsW[] = {
	1,9,
	235,9,
	235,1,
	236,0,
	236,9,
	236,0,
	237,-1,
	-1,-1,
	-1,9,
	-2,10,
	-2,-1,
};

struct Border SBorderWhite = {
	-2,-1,
	2,0,JAM1,
	11,
	SBorderVectorsW,
	(APTR)&SBorderBlack
};


/***** Border stuff for Requester *****/

SHORT BorderVectorsB[] = {
	0,10,
	236,10,
	236,0,
};

struct Border BorderBlack = {
	0,0,
	1,0,JAM1,
	3,
	BorderVectorsB,
	NULL
};

SHORT BorderVectorsW[] = {
	0,10,
	0,0,
	236,0,
};

struct Border BorderWhite = {
	0,0,
	2,0,JAM1,
	3,
	BorderVectorsW,
	(APTR)&BorderBlack
};

/***** Standart Text stuff for Requester *****/

struct IntuiText ContinueText = {
	1,0,JAM1,
	8,3,
	&TOPAZ80,
	"Continue",
	NULL
};

struct IntuiText CancelText = {
	1,0,JAM1,
	17,3,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct IntuiText OkText = {
	1,0,JAM1,
	30,3,
	&TOPAZ80,
	"Ok",
	NULL
};

/*******************************************************************************/

struct IntuiText NotifyIText = {
	1,0,JAM1,
	10,15,
	&TOPAZ80,
	"",
	NULL
};

struct Gadget NotifyGad = {
	NULL,
	141,-20,
	79,14,
	GRELBOTTOM,
	RELVERIFY | ENDGADGET,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&ContinueText,
	NULL,
	NULL,
	NULL,
	NULL
};

struct Requester NotifyRequester = {
	NULL,
	125,53,
	342,57,
	0,0,
	&NotifyGad,
	(APTR)&BorderWhite,
	&NotifyIText,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/

SHORT LBorderVectorsB[] = {
	1,13,
	179,13,
	179,1,
	180,0,
	180,13,
};

struct Border LBorderBlack = {
	0,0,
	1,0,JAM1,
	5,
	LBorderVectorsB,
	NULL
};

SHORT LBorderVectorsW[] = {
	179,0,
	1,0,
	1,12,
	0,13,
	0,0,
};

struct Border LBorderWhite = {
	0,0,
	2,0,JAM1,
	5,
	LBorderVectorsW,
	(APTR)&LBorderBlack
};


struct IntuiText IText90 = {
	1,0,JAM1,
	3,3,
	&TOPAZ80,
	"Don't Low Level Format",
	NULL
};

struct Gadget NoLLFormat = {
	NULL,
	240,64,
	181,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&LBorderWhite,
	NULL,
	&IText90,
	NULL,
	NULL,
	BORDER|NOLLFORMAT,
	NULL
};

struct IntuiText IText91 = {
	1,0,JAM1,
	5,3,
	&TOPAZ80,
	"Low Level Format Disk",
	NULL
};

struct Gadget LLFormat = {
	&NoLLFormat,
	24,64,
	181,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&LBorderWhite,
	NULL,
	&IText91,
	NULL,
	NULL,
	BORDER|LLFORMAT,
	NULL
};

#define GadgetList6 LLFormat

struct IntuiText IText94 = {
	1,0,JAM1,
	132,42,
	&TOPAZ80,
	"stored on the drive!",
	NULL
};

struct IntuiText IText93 = {
	1,0,JAM1,
	28,32,
	&TOPAZ80,
	"Warning: Low level format will destroy everything",
	&IText94
};

struct IntuiText IText92 = {
	1,0,JAM1,
	11,13,
	&TOPAZ80,
	"Are you sure you want to low-level format this drive?",
	&IText93
};

#define IntuiTextList6 IText92

struct Requester RequesterStructure6 = {
	NULL,
	92,64,
	448,89,
	0,0,
	&GadgetList6,
	(APTR)&BorderWhite,
	&IntuiTextList6,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/

struct Gadget FsGetTypeCancel = {
	NULL,
	204,76,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&CancelText,
	NULL,
	NULL,
	FSGETTYPECANCEL,
	NULL
};

struct Gadget FsGetTypeOk = {
	&FsGetTypeCancel,
	17,76,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&OkText,
	NULL,
	NULL,
	FSGETTYPEOK,
	NULL
};

UBYTE GetVersionSIBuff[6] =
	"0";
struct StringInfo GetVersionSInfo = {
	GetVersionSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget GetVersion = {
	&FsGetTypeOk,
	154,49,
	49,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IVBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&GetVersionSInfo,
	GETVERSION,
	NULL
};

UBYTE GetRevisionSIBuff[6] =
	"0";
struct StringInfo GetRevisionSInfo = {
	GetRevisionSIBuff,
	UNDOBUFFER,
	0,
	6,
	0,
	0,0,0,0,0,
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget GetRevision = {
	&GetVersion,
	154,61,
	49,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IVBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&GetRevisionSInfo,
	GETREVSION,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget GetDosType = {
	&GetRevision,
	99,21,
	98,8,
	NULL,
	RELVERIFY+STRINGCENTER+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&GetDosTypeSInfo,
	GETDOSTYPE,
	NULL
};

#define GadgetList7 GetDosType

struct IntuiText IText101 = {
	1,0,JAM1,
	72,61,
	&TOPAZ80,
	"Revision: ",
	NULL
};

struct IntuiText IText100 = {
	1,0,JAM1,
	72,49,
	&TOPAZ80,
	"Version: ",
	&IText101
};

struct IntuiText IText99 = {
	1,0,JAM1,
	34,38,
	&TOPAZ80,
	"Enter Version for File System",
	&IText100
};

struct IntuiText IText98 = {
	1,0,JAM1,
	34,10,
	&TOPAZ80,
	"Enter DosType for File System",
	&IText99
};

#define IntuiTextList7 IText98

struct Requester RequesterStructure7 = {
	NULL,
	168,38,
	299,98,
	0,0,
	&GadgetList7,
	(APTR)&BorderWhite,
	&IntuiTextList7,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/
// 39.9

struct Gadget FsGetNameCancel = {
	NULL,
	204,58,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&CancelText,
	NULL,
	NULL,
	FSGETNAMECANCEL,
	NULL
};

struct Gadget FsGetNameOk = {
	&FsGetNameCancel,
	17,58,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&OkText,
	NULL,
	NULL,
	FSGETNAMEOK,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget GetFileName = {
	&FsGetNameOk,
	32,29,
	234,8,
	NULL,
	RELVERIFY+STRINGCENTER+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&SBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&GetFileNameSInfo,
	GETFILENAME,
	NULL
};

#define GadgetList77 GetFileName

struct IntuiText IText97 = {
	1,0,JAM1,
	35,18,
	&TOPAZ80,
	"Enter Filename of File System",
	NULL
};

#define IntuiTextList77 IText97

struct Requester RequesterStructure77 = {
	NULL,
	168,40,
	299,88,
	0,0,
	&GadgetList77,
	(APTR)&BorderWhite,
	&IntuiTextList77,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/

struct Gadget SureCancelGad = {
	NULL,
	-89,-20,
	79,14,
	GRELBOTTOM+GRELRIGHT,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&CancelText,
	NULL,
	NULL,
	NULL,
	NULL
};

struct Gadget SureContinueGad = {
	&SureCancelGad,
	12,-20,
	79,14,
	GRELBOTTOM,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&ContinueText,
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
	(APTR)&BorderWhite,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/

struct IntuiText IText102 = {
	1,0,JAM1,
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
	(APTR)&BorderWhite,
	&IntuiTextList9,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};


/*******************************************************************************/

struct Gadget BadGetCancel = {
	NULL,
	200,105,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&CancelText,
	NULL,
	NULL,
	BADGETCANCEL,
	NULL
};

struct Gadget BadGetOK = {
	&BadGetCancel,
	22,105,
	79,14,
	NULL,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&BBorderWhite,
	NULL,
	&OkText,
	NULL,
	NULL,
	BADGETOK,
	NULL
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget BadSector = {
	&BadGetOK,
	22,81,
	98,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&BadSectorSInfo,
	BADSEC,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget BadBfi = {
	&BadSector,
	182,81,
	98,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IBorderWhite,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget BadHead = {
	&BadBfi,
	101,56,
	98,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IBorderWhite,
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
	(APTR)&StringExt,
	0,
	NULL
};

struct Gadget BadCylinder = {
	&BadHead,
	101,31,
	98,8,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT+GACT_STRINGEXTEND,
	STRGADGET+REQGADGET,
	(APTR)&IBorderWhite,
	NULL,
	NULL,
	NULL,
	(APTR)&BadCylinderSInfo,
	BADCYLINDER,
	NULL
};

#define GadgetList11 BadCylinder

struct IntuiText IText118 = {
	1,0,JAM1,
	142,81,
	&TOPAZ80,
	"or",
	NULL
};

struct IntuiText IText117 = {
	1,0,JAM1,
	47,70,
	&TOPAZ80,
	"Sector",
	&IText118
};

struct IntuiText IText116 = {
	1,0,JAM1,
	88,5,
	&TOPAZ80,
	"Enter Bad Block",
	&IText117
};

struct IntuiText IText115 = {
	1,0,JAM1,
	164,70,
	&TOPAZ80,
	"Bytes from Index",
	&IText116
};

struct IntuiText IText114 = {
	1,0,JAM1,
	132,45,
	&TOPAZ80,
	"Head",
	&IText115
};

struct IntuiText IText113 = {
	1,0,JAM1,
	117,20,
	&TOPAZ80,
	"Cylinder",
	&IText114
};

#define IntuiTextList11 IText113

struct Requester RequesterStructure11 = {
	NULL,
	172,30,
	299,127,
	0,0,
	&GadgetList11,
	(APTR)&BorderWhite,
	&IntuiTextList11,
	NULL,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/

SHORT BorderVectors13[] = {
	0,0,
	184,0,
	184,15,
	0,15,
	0,0
};
struct Border Border13 = {
	-2,-1,
	1,0,JAM1,
	5,
	BorderVectors13,
	NULL
};

struct IntuiText IText133 = {
	1,0,JAM1,
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
	1,0,JAM1,
	5,
	BorderVectors14,
	NULL
};

struct IntuiText IText134 = {
	1,0,JAM1,
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
	1,0,JAM1,
	5,
	BorderVectors15,
	NULL
};

struct IntuiText IText135 = {
	1,0,JAM1,
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
	1,0,JAM1,
	343,43,
	&TOPAZ80,
	"12345678901",
	NULL
};

struct IntuiText IText143 = {
	1,0,JAM1,
	149,54,
	&TOPAZ80,
	"123456789",
	&IText144
};

struct IntuiText IText142 = {
	1,0,JAM1,
	149,43,
	&TOPAZ80,
	"123456789",
	&IText143
};

struct IntuiText IText141 = {
	1,0,JAM1,
	149,32,
	&TOPAZ80,
	"123456789",
	&IText142
};

struct IntuiText IText140 = {
	1,0,JAM1,
	234,43,
	&TOPAZ80,
	"Block Number:",
	&IText141
};

struct IntuiText IText139 = {
	1,0,JAM1,
	87,54,
	&TOPAZ80,
	"Sector:",
	&IText140
};

struct IntuiText IText138 = {
	1,0,JAM1,
	103,43,
	&TOPAZ80,
	"Head:",
	&IText139
};

struct IntuiText IText137 = {
	1,0,JAM1,
	71,32,
	&TOPAZ80,
	"Cylinder:",
	&IText138
};

struct IntuiText IText136 = {
	1,0,JAM1,
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
//	NULL,
	(APTR)&BorderWhite,
	&IntuiTextList13,
	NULL,
//	1,
	0,
	NULL,
	NULL,
	NULL
};

/*******************************************************************************/
