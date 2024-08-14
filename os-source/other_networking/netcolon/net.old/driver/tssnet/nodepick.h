/***************************************************

Copyright © 1989, 1990 Syndesis Corporation
All Rights Reserved.

ncpdef.h - Intuition definitions for node definitions screen

***************************************************/


#include "strdef.h"
#include "defines.h"


USHORT ImageData1[] = {
	0x5FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xF800,0xAFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xF000,0x57FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xE000,0xAFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
	0x57FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,0xAFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xE000,0x57FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xE000,0xAFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
	0x57FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,0xAFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xE000,0x57FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xE000,0xAFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
	0x57FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,0xAFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xE000,0x57FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xE000,0xAFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xF000,
	0x5FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xF800,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0400,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0C00,0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,
	0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,
	0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,
	0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFC00,0x07FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,
	0x0FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,0x1FFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFC00,0xA000,0x0000,0x0000,0x0000,
	0x0000,0x0400,0x5000,0x0000,0x0000,0x0000,0x0000,0x0C00,
	0xA800,0x0000,0x0000,0x0000,0x0000,0x1C00,0x5000,0x0000,
	0x0000,0x0000,0x0000,0x1C00,0xA800,0x0000,0x0000,0x0000,
	0x0000,0x1C00,0x5000,0x0000,0x0000,0x0000,0x0000,0x1C00,
	0xA800,0x0000,0x0000,0x0000,0x0000,0x1C00,0x5000,0x0000,
	0x0000,0x0000,0x0000,0x1C00,0xA800,0x0000,0x0000,0x0000,
	0x0000,0x1C00,0x5000,0x0000,0x0000,0x0000,0x0000,0x1C00,
	0xA800,0x0000,0x0000,0x0000,0x0000,0x1C00,0x5000,0x0000,
	0x0000,0x0000,0x0000,0x1C00,0xA800,0x0000,0x0000,0x0000,
	0x0000,0x1C00,0x5000,0x0000,0x0000,0x0000,0x0000,0x1C00,
	0xAFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFC00,0x5FFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFC00,0xBFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFC00
};
struct TextAttr TOPAZ80 = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};

struct TextAttr TOPAZ80BOLD = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,FSF_BOLD,0
};

static UBYTE UNDOBUFFER[17];

struct Image Image1 = {
	0,0,	/* XY origin relative to container TopLeft */
	86,17,	/* Image width and height in pixels */
	3,	/* number of bitplanes in Image */
	ImageData1,	/* pointer to ImageData */
	0x0007,0x0000,	/* PlanePick and PlaneOnOff */
	NULL	/* next Image structure */
};

struct IntuiText IText21a = {
	1,3,JAM1,	/* front and back text pens, drawmode and fill byte */
	22,4,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)Cancel,	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct IntuiText IText21Ret = {
	0,3,JAM2,	/* front and back text pens, drawmode and fill byte */
	23,5,	/* XY origin relative to container TopLeft */
	&TOPAZ80BOLD,	/* font pointer or NULL for default */
	(UBYTE *)Cancel,	/* pointer to text */
	&IText21a /* next IntuiText structure */ 
};
struct IntuiText IText22a = {
	1,3,JAM1,	/* front and back text pens, drawmode and fill byte */
	20,4,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)Use,	/* pointer to text */
	NULL	/* next IntuiText structure */
};
struct IntuiText IText22Upd = {
	0,3,JAM2,	/* front and back text pens, drawmode and fill byte */
	21,5,	/* XY origin relative to container TopLeft */
	&TOPAZ80BOLD,	/* font pointer or NULL for default */
	(UBYTE *)Use,	/* pointer to text */
	&IText22a	/* next IntuiText structure */
};

SHORT BorderVectors4[] = {
	0,0,
	54,0,
	54,13,
	0,13,
	0,0
};
struct Border Border4 = {
	-2,-3,	/* XY origin relative to container TopLeft */
	3,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	BorderVectors4,	/* pointer to XY vectors */
	NULL	/* next border in list */
};


static SHORT BorderVectors1[] = {
	0,0,
	133,0,
	133,50,
	0,50,
	0,0
};
static struct Border Border1 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	3,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	BorderVectors1,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

static struct Gadget def_list = {
	NULL,	/* next gadget */
	405,35,	/* origin XY of hit box relative to window TopLeft */
	129,48,	/* hit box width and height */
	GADGHNONE,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&Border1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	NULL,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	DEF_LIST,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

UBYTE ListBuffers[LIST_COUNT][LIST_LENGTH+1];
struct IntuiText ListText[LIST_COUNT] = {
        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 0),
        &TOPAZ80,
        &ListBuffers[0][0],
        NULL,
        },

        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 1),
        &TOPAZ80,
        &ListBuffers[1][0],
        &ListText[0],
        },

        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 2),
        &TOPAZ80,
        &ListBuffers[2][0],
        &ListText[1],
        },

        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 3),
        &TOPAZ80,
        &ListBuffers[3][0],
        &ListText[2],
        },

        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 4),
        &TOPAZ80,
        &ListBuffers[4][0],
        &ListText[3],
        },

        {
        1, 0, 
        JAM2,
        406, 36 + (LIST_HEIGHT * 5),
        &TOPAZ80,
        &ListBuffers[5][0],
        &ListText[4],
        },

    };

static struct PropInfo def_propSInfo = {
	AUTOKNOB+FREEVERT,	/* PROPINFO flags */
	0x0000,0x0000,	/* horizontal and vertical pot values */
	0xFFFF,0xFFFF,	/* horizontal and vertical body values */
};

static struct Image Image4 = {
	0,0,	/* XY origin relative to container TopLeft */
	11,132,	/* Image width and height in pixels */
	0,	/* number of bitplanes in Image */
	NULL,	/* pointer to ImageData */
	0x0000,0x0000,	/* PlanePick and PlaneOnOff */
	NULL	/* next Image structure */
};

static struct Gadget def_prop = {
	&def_list,	/* next gadget */
	542,34,	/* origin XY of hit box relative to window TopLeft */
	19,51,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+GADGIMMEDIATE+FOLLOWMOUSE,	/* activation flags */
	PROPGADGET,	/* gadget type flags */
	(APTR)&Image4,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	NULL,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	(APTR)&def_propSInfo,	/* SpecialInfo structure */
	DEF_PROP,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static struct Gadget def_can = {
	&def_prop,	/* next gadget */
	355,168,	/* origin XY of hit box relative to window TopLeft */
	86,17,	/* hit box width and height */
	GADGIMAGE | GADGHBOX,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&Image1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&IText21Ret,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	DEF_CAN,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static struct Gadget def_save = {
	&def_can,	/* next gadget */
	156,168,	/* origin XY of hit box relative to window TopLeft */
	86,17,  	/* hit box width and height */
	GADGIMAGE | GADGHBOX,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&Image1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&IText22Upd,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	DEF_SAVE,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static UBYTE def_nameSIBuff[17];
static struct StringInfo def_nameSInfo = {
	def_nameSIBuff,	/* buffer where text will be edited */
	UNDOBUFFER,	/* optional undo buffer */
	0,	/* character position in buffer */
	17,	/* maximum number of characters to allow */
	0,	/* first displayed character buffer position */
	0,0,0,0,0,	/* Intuition initialized and maintained variables */
	0,	/* Rastport of gadget */
	0,	/* initial value for integer gadgets */
	NULL	/* alternate keymap (fill in if you set the flag) */
};

/* fudge */
static SHORT BorderVectors6[] = {
	0,0,
	154,0,
	154,13,
	0,13,
	0,0
};
static struct Border Border6 = {
	-2,-3,	/* XY origin relative to container TopLeft */
	3,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	BorderVectors6,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

static struct Gadget def_name = {
	&def_save,	/* next gadget */
	187,72,	/* origin XY of hit box relative to window TopLeft */
	152,12,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+STRINGCENTER,	/* activation flags */
	STRGADGET,	/* gadget type flags */
	(APTR)&Border6,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	NULL,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	(APTR)&def_nameSInfo,	/* SpecialInfo structure */
	DEF_NAME,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static UBYTE def_nodeSIBuff[5];
static struct StringInfo def_nodeSInfo = {
	def_nodeSIBuff,	/* buffer where text will be edited */
	UNDOBUFFER,	/* optional undo buffer */
	0,	/* character position in buffer */
	5,	/* maximum number of characters to allow */
	0,	/* first displayed character buffer position */
	0,0,0,0,0,	/* Intuition initialized and maintained variables */
	0,	/* Rastport of gadget */
	0,	/* initial value for integer gadgets */
	NULL	/* alternate keymap (fill in if you set the flag) */
};

static struct Gadget def_node = {
	&def_name,	/* next gadget */
	187,55,	/* origin XY of hit box relative to window TopLeft */
	53,12,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+STRINGRIGHT+LONGINT,	/* activation flags */
	STRGADGET,	/* gadget type flags */
	(APTR)&Border4,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	NULL,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	(APTR)&def_nodeSInfo,	/* SpecialInfo structure */
	DEF_NODE,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static UBYTE def_areaSIBuff[3];
static struct StringInfo def_areaSInfo = {
	def_areaSIBuff,	/* buffer where text will be edited */
	UNDOBUFFER,	/* optional undo buffer */
	0,	/* character position in buffer */
	3,	/* maximum number of characters to allow */
	0,	/* first displayed character buffer position */
	0,0,0,0,0,	/* Intuition initialized and maintained variables */
	0,	/* Rastport of gadget */
	0,	/* initial value for integer gadgets */
	NULL	/* alternate keymap (fill in if you set the flag) */
};

static struct Gadget def_area = {
	&def_node,	/* next gadget */
	187,38,	/* origin XY of hit box relative to window TopLeft */
	53,12,	/* hit box width and height */
	SELECTED,	/* gadget flags */
	RELVERIFY+STRINGRIGHT+LONGINT,	/* activation flags */
	STRGADGET,	/* gadget type flags */
	(APTR)&Border4,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	NULL,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	(APTR)&def_areaSInfo,	/* SpecialInfo structure */
	DEF_AREA,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

static struct IntuiText ITextDef20 = {
	2,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	418,24,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)NetworkNodes,	/* pointer to text */
	NULL	/* next IntuiText structure */
};

static struct IntuiText ITextDef21 = {
	2,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	101,74,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)NodeName,	/* pointer to text */
	&ITextDef20	/* next IntuiText structure */
};

static struct IntuiText ITextDef22 = {
	2,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	86,57,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)NodeNumberText,	/* pointer to text */
	&ITextDef21	/* next IntuiText structure */
};

static struct IntuiText ITextDef23 = {
	2,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	86,40,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	(UBYTE *)AreaNumber,	/* pointer to text */
	&ITextDef22	/* next IntuiText structure */
};

struct NewWindow NewWindowStructure1 = {
	0,0,	/* window XY origin relative to TopLeft of screen */
	640, 200,	/* window width and height */
	0,1,	/* detail and block pens */
	GADGETUP | MENUPICK,	/* IDCMP flags */
	BORDERLESS | BACKDROP | ACTIVATE,	/* other window flags */
	&def_area,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	NULL,   /* window title */
	NULL,	/* custom screen pointer, fill in later after screen open */
	NULL,	/* custom bitmap */
	640,200,	/* minimum width and height */
	640,200,	/* maximum width and height */
	CUSTOMSCREEN	/* destination screen type */
};


struct NewScreen NewScreenStructure1 = {
    0, 0, 0, 0, 3, /* 3 planes, 8 colors */
    (UBYTE) 0, (UBYTE) 1,
    HIRES,
    CUSTOMSCREEN,
    &TOPAZ80,
    (UBYTE *)"TSSnet (TM) Set Host", /* screen title */
    NULL,
    NULL
};

