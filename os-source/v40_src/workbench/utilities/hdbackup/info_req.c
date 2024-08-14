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

#include "standard.h"
#include "brushell.h"
#include "info_req.h"
#include "images.h"
#include "mainwin.h"
#include "dbug.h"



#define REQ_LEFT	 40
#define REQ_TOP		 50
#define REQ_WIDTH	560
#define REQ_HEIGHT	 80

#define QREQ_LEFT	  2
#define QREQ_TOP	 30
#define QREQ_WIDTH	630
#define QREQ_HEIGHT	 70

#define BACKFILL_PEN		0		/* grey */




/* The "Info" Requester */

static struct IntuiText infotext[] = {
    { 1, 2, JAM2, 30, 5, NULL,
		(UBYTE *) "General Information", &infotext[1] },

    { 1, 0, JAM2, 30, 25, NULL, (UBYTE *)
		"  This is the color used for SELECTED files and directories.  ",
		&infotext[2] },

    { 1, 0, JAM2, 30, 35, NULL, (UBYTE *)
		"  This is the color of SHADOWED files and directories.        ",
		&infotext[3] },

    { 1, 0, JAM2, 30, 45, NULL, (UBYTE *)
		"  Files that were NOT backed up will be this color.          ",
		NULL }
};



/* Okay Gadget */

static struct IntuiText okay_text = {
    1, 0, JAM1, 36, 3, NULL, (UBYTE *) "OK", NULL
};

static struct Gadget okay_gadget = {
    NULL,			/* NextGadget */
    240, 60,			/* LeftEdge, TopEdge */
    BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
    GADGIMAGE | GADGHIMAGE,	/* Flags */
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_3_IMAGE,		/* GadgetRender */
    (APTR) BOX_3_HIMAGE,	/* SelectRender */
    &okay_text,			/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    1,				/* GadgetID */
    NULL			/* UserData */
};



static SHORT req_border_vectors[2][10] = {
    {
	0, 0, REQ_WIDTH - 1, 0, REQ_WIDTH - 1, REQ_HEIGHT - 1,
	0, REQ_HEIGHT - 1, 0, 0
    },
    {
	0, 0, REQ_WIDTH - 5, 0, REQ_WIDTH - 5, REQ_HEIGHT - 3,
	0, REQ_HEIGHT - 3, 0, 0
    }
};

static struct Border req_border[] = {
    { 0, 0, 2, 0, JAM1, 5, (SHORT *) &req_border_vectors[0], &req_border[1] },
    { 2, 1, 3, 0, JAM1, 5, (SHORT *) &req_border_vectors[1], NULL }
};

static struct Requester req = {
    NULL,				/* OlderRequester */
    REQ_LEFT, REQ_TOP,		/* LeftEdge, TopEdge */
    REQ_WIDTH, REQ_HEIGHT,	/* Width, Height */
    0, 0,				/* RelLeft, RelTop */
    &okay_gadget,		/* ReqGadget */
    &req_border[0],		/* ReqBorder */
    infotext,			/* ReqText */
    NULL,				/* Flags */
    BACKFILL_PEN,		/* BackFill */
    NULL,				/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,				/* RWindow */
    { NULL }			/* Pad */
};



/* This assumes coming up in the main window */

void tell_info( void )
{
    struct IntuiMessage *message;
    ULONG class;
    struct Gadget *gadget;
    ULONG old_idcmp;
    BOOL finished = FALSE;
    struct Window *window;
	struct IntuiText *it;
	
    
    DBUG_ENTER ("tell_info");

    
	okay_text.ITextFont = textattr;

	/* Set the text in the INFO requester to use our font */
	it = &infotext[0];
	while( it != NULL )
	{
		it->ITextFont = textattr;
		it = it->NextText;
	}

	infotext[1].FrontPen = sel_file_color;
	infotext[2].FrontPen = desel_file_color;
	infotext[3].FrontPen = unbacked_file_color;

    window = mainwin;
    req.RWindow = window;

    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP | GADGETDOWN | REQSET );
    /* Post the requester */
    if (Request (&req, window) == FALSE) {
	/* Could not post the requester */
	ModifyIDCMP (window, old_idcmp);
	DBUG_VOID_RETURN;
    }
    while (finished == FALSE) {
	WaitPort (window -> UserPort);
	while (message = (struct IntuiMessage *) GetMsg (window -> UserPort)) {
	    class = message -> Class;
	    gadget = (struct Gadget *) message -> IAddress;
	    ReplyMsg ((struct Message *) message);
	    switch (class) {
		case GADGETUP: 
		case GADGETDOWN: 
			finished = TRUE;
		    break;
	    }
	}
    }
    ModifyIDCMP (window, old_idcmp);
    DBUG_VOID_RETURN;
}



