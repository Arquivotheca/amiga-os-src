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

#include "standard.h"
#include "images.h"
#include "mainwin.h"

#define REQ_WIDTH	340
#define REQ_HEIGHT	 70


/*--------------------------------------------------------------*/
/*		Variable Definitions				*/
/*--------------------------------------------------------------*/

/* String Info */

static char stringbuf[60];

static char udb[60];		/* Undo Buffer for requester string gadget
				   */

static struct StringInfo    string_info = {
    (UBYTE *) stringbuf,	/* Buffer */
    (UBYTE *) udb,		/* UndoBuffer */
    0,				/* BufferPos */
    60,				/* MaxChars */
    0,				/* DispPos */
    0, 0, 0, 0, 0, NULL, 0, NULL
};


static struct IntuiText rtext[] = {
    {
	1, 0, JAM1, 40, 12, NULL, RTB, NULL
    }
};


#define GHI 11

static  SHORT string_vectors[] = {
    0, 0, 200 + 6, 0, 200 + 6, GHI, 0, GHI, 0, 0
};

static struct Border    string_box = {
    -4, -2, 3, 0, JAM1, 5, string_vectors, NULL
};


static struct Gadget    gadget[] = {
    /* String gadget for directory name */
    {
	&gadget[1],		/* NextGadget */
	20, 24,			/* LeftEdge, TopEdge */
	200, GHI - 1,		/* Width, Height */
	GADGHCOMP,		/* Flags */
	RELVERIFY,		/* Activation */
	REQGADGET |
	STRGADGET,		/* GadgetType */
	(APTR) & string_box,	/* GadgetRender */
	NULL,			/* SelectRender */
	NULL,			/* GadgetText */
	NULL,			/* MutualExclude */
	(APTR) & libreq_string_info[0],/* SpecialInfo */
	1,			/* GadgetID */
	NULL			/* UserData */
    },
    /* Okay Gadget */
    {
	&gadget[2],		/* NextGadget */
	20, 50,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,/* Width, Height */
	GADGIMAGE |
	GADGHIMAGE,		/* Flags */
	ENDGADGET |
	RELVERIFY,		/* Activation */
	REQGADGET |
	BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,	/* GadgetRender */
	(APTR) BOX_3_HIMAGE,	/* SelectRender */
	&gtext[1],		/* GadgetText */
	NULL,			/* MutualExclude */
	NULL,			/* SpecialInfo */
	2,			/* GadgetID */
	NULL			/* UserData */
    }                      ,
    /* Cancel Gadget */
    {
	                        NULL,/* NextGadget */
	                        200, 50,/* LeftEdge, TopEdge */
	                        BOX_3_WIDTH, BOX_3_HEIGHT,
				/* Width, Height */
	                        GADGIMAGE |
	                        GADGHIMAGE,/* Flags */
	                        ENDGADGET |
	                        RELVERIFY,/* Activation */
	                        REQGADGET |
	                        BOOLGADGET,/* GadgetType */
	                        (APTR) BOX_3_IMAGE,/* GadgetRender */
	                        (APTR) BOX_3_HIMAGE,/* SelectRender */
	                       &gtext[2],/* GadgetText */
	                        NULL,/* MutualExclude */
	                        NULL,/* SpecialInfo */
	                        3,/* GadgetID */
	                        NULL/* UserData */
    }
};



static  SHORT req_border_vectors[2][10] = {
    {
	0, 0, REQ_WIDTH - 1, 0, REQ_WIDTH - 1, REQ_HEIGHT - 1,
	0, REQ_HEIGHT - 1, 0, 0
    },
    {
	0, 0, REQ_WIDTH - 3, 0, REQ_WIDTH - 3, REQ_HEIGHT - 3,
	0, REQ_HEIGHT - 3, 0, 0
    }
};

static struct Border    req_border[] = {
    {
	0, 0, 1, 0, JAM1, 5, (SHORT *) & req_border_vectors[0], &req_border[1]
    },
    {
	1, 1, 1, 0, JAM1, 5, (SHORT *) & req_border_vectors[1], NULL
    }
};

struct Requester    libreq = {
    NULL,			/* OlderRequester */
    10, 12,			/* LeftEdge, TopEdge */
    REQ_WIDTH, REQ_HEIGHT,	/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &gadget[0],			/* ReqGadget */
    &req_border[0],		/* ReqBorder */
    &rtext[0],			/* ReqText */
    NULL,			/* Flags */
    2,				/* BackFill */
    NULL,			/* ReqLayer */
    {
	NULL
    },				/* Pad */
    {
	NULL
    },				/* BitMap */
    NULL,			/* RWindow */
    {
	NULL
    },				/* Pad */
};
