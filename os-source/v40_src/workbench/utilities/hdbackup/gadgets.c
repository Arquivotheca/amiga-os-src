/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

#include <exec/types.h>
#include <intuition/sghooks.h>

#include "standard.h"
#include "brushell.h"
#include "images.h"
#include "mainwin.h"
#include "dbug.h"
#include "libfuncs.h"


#if 0		/* If true, turn off extended string gadgets */
#ifdef STRINGEXTEND
#undef STRINGEXTEND
#define STRINGEXTEND	0L
#endif
#endif


#define GLEFT		10
	/* This is the primary left offset for the row of selection gadgets
	 * on the left side of the screen.
	 */



/************************************************/
/*				Misc. Gadgets					*/
/************************************************/

#define GTOP	13

struct IntuiText mainwin_gadget_text[NUM_GADGETS] = {
    { 1, 0, JAM1, 19, 2, RTB, (UBYTE *) "Root", NULL },
    { 1, 0, JAM1, 8, 2, RTB, (UBYTE *) "Parent", NULL },
    { 1, 0, JAM1, 4, 2, RTB, (UBYTE *) "Include", NULL },
    { 1, 0, JAM1, 4, 2, RTB, (UBYTE *) "Exclude", NULL },
    { 1, 0, JAM1, 10, 2, RTB, (UBYTE *) "Start", NULL }
};


struct Gadget mainwin_gadget[NUM_GADGETS] = {
    {							/* Root Gadget */
	&mainwin_gadget[1],			/* NextGadget */
	260, GTOP,					/* LeftEdge, TopEdge */
	BOX_4_WIDTH, BOX_4_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,					/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_4_IMAGE,			/* GadgetRender */
	(APTR) BOX_4_HIMAGE,		/* SelectRender */
	&mainwin_gadget_text[0],	/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	ROOT_GADGET_ID,				/* GadgetID */
	NULL						/* UserData */
    },
    {							/* Parent Gadget */
	&mainwin_gadget[2],			/* NextGadget */
	260 + BOX_4_WIDTH + 25, GTOP,	/* LeftEdge, TopEdge */
	BOX_4_WIDTH, BOX_4_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,					/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_4_IMAGE,			/* GadgetRender */
	(APTR) BOX_4_HIMAGE,		/* SelectRender */
	&mainwin_gadget_text[1],	/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	PARENT_GADGET_ID,			/* GadgetID */
	NULL						/* UserData */
    },
    {							/* Include Gadget */
	&mainwin_gadget[3],			/* NextGadget */
	20, GTOP,					/* LeftEdge, TopEdge */
	BOX_4_WIDTH, BOX_4_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,					/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_4_IMAGE,			/* GadgetRender */
	(APTR) BOX_4_HIMAGE,		/* SelectRender */
	&mainwin_gadget_text[2],	/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	INCLUDE_ID,					/* GadgetID */
	NULL						/* UserData */
    },
    {							/* Exclude Gadget */
	&mainwin_gadget[4],			/* NextGadget */
	BOX_4_WIDTH + 30, GTOP,		/* LeftEdge, TopEdge */
	BOX_4_WIDTH, BOX_4_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,					/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_4_IMAGE,			/* GadgetRender */
	(APTR) BOX_4_HIMAGE,		/* SelectRender */
	&mainwin_gadget_text[3],	/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	EXCLUDE_ID,					/* GadgetID */
	NULL						/* UserData */
    },
    {							/* Start Gadget */
	&date_gadget[0],			/* NextGadget */
	540, GTOP,					/* LeftEdge, TopEdge */
	BOX_4_WIDTH, BOX_4_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,					/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_4_IMAGE,			/* GadgetRender */
	(APTR) BOX_4_HIMAGE,		/* SelectRender */
	&mainwin_gadget_text[4],	/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	START_ID,					/* GadgetID */
	NULL						/* UserData */
    }
};



/************************************************/
/*				Date Gadget Cluster				*/
/************************************************/

#undef GTOP
#define GTOP	70

static char date_string[25];
static char date_udb[25];	/* Undo Buffer for requester string gadgets */

static struct StringExtend date_string_ex = {
	RTB,			/* Font */
	1, 0,			/* Pens */
	1, 0,			/* ActivePens */
	0,				/* InitialModes */
	NULL,			/* EditHook */
	NULL,			/* WorkBuffer */
	0L, 0L, 0L, 0L	/* Reserved */
};
	

static struct StringInfo date_string_info = {
    (UBYTE *) date_string,			/* Buffer */
    (UBYTE *) date_udb,				/* UndoBuffer */
    0,								/* BufferPos */
    sizeof(date_string)-1,			/* MaxChars */
    0,								/* DispPos */
    0, 0, 0, 0, 0,
	&date_string_ex,				/* StringExtend */
	0,
	NULL
};

struct IntuiText date_text[] = {
    { 1, 0, JAM1, 20, 2, NULL, (UBYTE *) "Files Dated:", NULL },
    { 1, 0, JAM1, 5, 2, NULL, (UBYTE *) "Before", NULL },
    { 1, 0, JAM1, 9, 2, NULL, (UBYTE *) "After", NULL },
    { 1, 0, JAM1, 26, 2, NULL, (UBYTE *) "On", NULL }
};

struct Gadget date_gadget[] = {
    {
	&date_gadget[1],			/* NextGadget */
	GLEFT, GTOP,				/* LeftEdge, TopEdge */
	BOX_6_WIDTH, BOX_6_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	TOGGLESELECT | RELVERIFY,	/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_6_IMAGE,			/* GadgetRender */
	(APTR) BOX_6_HIMAGE,		/* SelectRender */
	&date_text[0],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    },
    {
	&date_gadget[2],			/* NextGadget */
	GLEFT, GTOP+13,				/* LeftEdge, TopEdge */
	BOX_5_WIDTH, BOX_5_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	GADGIMMEDIATE,				/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_5_IMAGE,			/* GadgetRender */
	(APTR) BOX_5_HIMAGE,		/* SelectRender */
	&date_text[1],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	DATE_GADGET_ID,				/* GadgetID */
	(APTR) 1					/* UserData */
    },
    {
	&size_gadget[0],			/* NextGadget */
	GLEFT+68, GTOP+15,			/* LeftEdge, TopEdge */
	BOX_7_WIDTH - 16,			/* Width */
	8 /* BOX_7_HEIGHT */,				/* Height */
	GADGIMAGE | GADGHCOMP,		/* Flags */
	STRINGEXTEND |
	RELVERIFY,					/* Activation */
	STRGADGET,					/* GadgetType */
	(APTR) BOX_7_IMAGE,			/* GadgetRender */
	NULL,						/* SelectRender */
	NULL,						/* GadgetText */
	NULL,						/* MutualExclude */
	(APTR) &date_string_info,	/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    }
};



/************************************************/
/*				Size Gadget Cluster				*/
/************************************************/

#undef GTOP
#define GTOP	98


static char size_string[16] = "1000";
static char size_udb[16];	/* Undo Buffer for requester string gadgets */

static struct StringExtend size_string_ex = {
	RTB,			/* Font */
	1, 0,			/* Pens */
	1, 0,			/* ActivePens */
	0,				/* InitialModes */
	NULL,			/* EditHook */
	NULL,			/* WorkBuffer */
	0L, 0L, 0L, 0L	/* Reserved */
};
	

static struct StringInfo size_string_info = {
    (UBYTE *) size_string,			/* Buffer */
    (UBYTE *) size_udb,				/* UndoBuffer */
    0,								/* BufferPos */
    sizeof(size_string)-1,			/* MaxChars */
    0,								/* DispPos */
    0, 0, 0, 0, 0,
	&size_string_ex,
	1000, NULL
};

struct IntuiText size_text[] = {
    { 1, 0, JAM1, 20, 2, NULL, (UBYTE *) "Files Size:", NULL },
    { 1, 0, JAM1, 4, 2, NULL, (UBYTE *) "Smaller", NULL },
    { 1, 0, JAM1, 9, 2, NULL, (UBYTE *) "Larger", NULL },
    { 1, 0, JAM1, 9, 2, NULL, (UBYTE *) "Equal", NULL }
};

struct Gadget size_gadget[] = {
    {
	&size_gadget[1],			/* NextGadget */
	GLEFT, GTOP,				/* LeftEdge, TopEdge */
	BOX_6_WIDTH, BOX_6_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	TOGGLESELECT | RELVERIFY,	/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_6_IMAGE,			/* GadgetRender */
	(APTR) BOX_6_HIMAGE,		/* SelectRender */
	&size_text[0],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    },
    {
	&size_gadget[2],			/* NextGadget */
	GLEFT, GTOP+13,				/* LeftEdge, TopEdge */
	BOX_5_WIDTH, BOX_5_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	GADGIMMEDIATE,				/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_5_IMAGE,			/* GadgetRender */
	(APTR) BOX_5_HIMAGE,		/* SelectRender */
	&size_text[1],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	SIZE_GADGET_ID,				/* GadgetID */
	(APTR) 1					/* UserData */
    },
    {
	&pattern_gadget[0],			/* NextGadget */
	GLEFT+68, GTOP+15,			/* LeftEdge, TopEdge */
	BOX_7_WIDTH - 16,			/* Width */
	8 /* BOX_7_HEIGHT */,				/* Height */
	GADGIMAGE | GADGHCOMP,		/* Flags */
	STRINGEXTEND |
	LONGINT | RELVERIFY,		/* Activation */
	STRGADGET,					/* GadgetType */
	(APTR) BOX_7_IMAGE,			/* GadgetRender */
	NULL,						/* SelectRender */
	NULL,						/* GadgetText */
	NULL,						/* MutualExclude */
	(APTR) &size_string_info,	/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    }
};


/************************************************/
/*				Pattern Gadget Cluster			*/
/************************************************/

#undef GTOP
#define GTOP	42


static char pattern_string[32] = "#?.info";
static char pattern_udb[32];	/* Undo Buffer for requester string gadgets */

static struct StringExtend pattern_string_ex = {
	RTB,			/* Font */
	1, 0,			/* Pens */
	1, 0,			/* ActivePens */
	0,				/* InitialModes */
	NULL,			/* EditHook */
	NULL,			/* WorkBuffer */
	0L, 0L, 0L, 0L	/* Reserved */
};
	

static struct StringInfo pattern_string_info = {
    (UBYTE *) pattern_string,		/* Buffer */
    (UBYTE *) pattern_udb,			/* UndoBuffer */
    0,								/* BufferPos */
    sizeof(pattern_string)-1,		/* MaxChars */
    0,								/* DispPos */
    0, 0, 0, 0, 0,
	&pattern_string_ex,
	0, NULL
};

struct IntuiText pattern_text[] = {
    { 1, 0, JAM1, 12, 2, NULL, (UBYTE *) "Files Pattern:", NULL },
    { 1, 0, JAM1, 10, 2, NULL, (UBYTE *) "Match", NULL },
    { 1, 0, JAM1, 22, 2, NULL, (UBYTE *) "==", &pattern_text[3] },
    { 1, 0, JAM1, 26, 2, NULL, (UBYTE *) "/", NULL }
/*    { 1, 0, JAM1, 16, 2, NULL, (UBYTE *) "Not", NULL } */
};

struct Gadget pattern_gadget[] = {
    {
	&pattern_gadget[1],			/* NextGadget */
	GLEFT, GTOP,				/* LeftEdge, TopEdge */
	BOX_6_WIDTH, BOX_6_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	TOGGLESELECT | RELVERIFY,	/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_6_IMAGE,			/* GadgetRender */
	(APTR) BOX_6_HIMAGE,		/* SelectRender */
	&pattern_text[0],			/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    },
    {
	&pattern_gadget[2],			/* NextGadget */
	GLEFT, GTOP+13,				/* LeftEdge, TopEdge */
	BOX_5_WIDTH, BOX_5_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	GADGIMMEDIATE,				/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_5_IMAGE,			/* GadgetRender */
	(APTR) BOX_5_HIMAGE,		/* SelectRender */
	&pattern_text[1],			/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	PATTERN_GADGET_ID,			/* GadgetID */
	(APTR) 1					/* UserData */
    },
    {
	&arc_gadget[0],				/* NextGadget */
	GLEFT+68, GTOP+15,			/* LeftEdge, TopEdge */
	BOX_7_WIDTH - 8,			/* Width */
	8 /* BOX_7_HEIGHT */,				/* Height */
	GADGIMAGE | GADGHCOMP,		/* Flags */
	STRINGEXTEND |
	RELVERIFY,					/* Activation */
	STRGADGET,					/* GadgetType */
	(APTR) BOX_7_IMAGE,			/* GadgetRender */
	NULL,						/* SelectRender */
	NULL,						/* GadgetText */
	NULL,						/* MutualExclude */
	(APTR)&pattern_string_info,	/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    }
};



/************************************************/
/*				Archive Bit Gadget Cluster		*/
/************************************************/

#undef GTOP
#define GTOP	29

struct IntuiText arc_text[] = {
    { 1, 0, JAM1, 1, 0, NULL, (UBYTE *) "Archive Bit", NULL },
    { 1, 0, JAM1, 20, 2, NULL, (UBYTE *) "Set", NULL },
    { 1, 0, JAM1, 12, 2, NULL, (UBYTE *) "Clear", NULL }
};

struct Gadget arc_gadget[] = {
    {
	&arc_gadget[1],				/* NextGadget */
	GLEFT+4, GTOP,				/* LeftEdge, TopEdge */
	BOX_7_WIDTH, BOX_7_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	TOGGLESELECT | RELVERIFY,	/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_7_IMAGE,			/* GadgetRender */
	(APTR) BOX_7_HIMAGE,		/* SelectRender */
	&arc_text[0],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	NULL,						/* GadgetID */
	NULL						/* UserData */
    },
    {
	NULL,						/* NextGadget */
	BOX_7_WIDTH + 10, GTOP-2,	/* LeftEdge, TopEdge */
	BOX_5_WIDTH, BOX_5_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHCOMP,		/* Flags */
	GADGIMMEDIATE  /* RELVERIFY */,		/* Activation */
	BOOLGADGET,					/* GadgetType */
	(APTR) BOX_5_IMAGE,			/* GadgetRender */
	NULL,						/* SelectRender */
	&arc_text[1],				/* GadgetText */
	NULL,						/* MutualExclude */
	NULL,						/* SpecialInfo */
	ARC_GADGET_ID,				/* GadgetID */
	(APTR) 1					/* UserData */
    }
};



/* DTM_NEW */

void init_mainwin_gadgets( void )
{
	date_string_ex.Font = textfont;	
	size_string_ex.Font = textfont;	
	pattern_string_ex.Font = textfont;	

	strcpy(  date_string, when_pattern( "%d-%b-%y" )  );
}

