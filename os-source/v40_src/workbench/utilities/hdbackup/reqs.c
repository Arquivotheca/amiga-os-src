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
#include "bailout.h"
#include "images.h"
#include "mainwin.h"
#include "reqs.h"
#include "dbug.h"
#include "libfuncs.h"



#define REQ_LEFT	200
#define REQ_TOP		 60		/* 112 */
#define REQ_WIDTH	340
#define REQ_HEIGHT	 70

#define QREQ_LEFT	 20
#define QREQ_TOP	 30
#define QREQ_WIDTH	580
#define QREQ_HEIGHT	 70

#define BACKFILL_PEN		0		/* grey */


static void hexdump_format( char *, UBYTE *, int, LONG );
static ERRORCODE format_fdata( char * );



/* Cancel Gadget */

static struct IntuiText cancel_text = {
    1, 0, JAM1, 18, 3, NULL, (UBYTE *) "Cancel", NULL
};

static struct Gadget cancel_gadget = {
    NULL,			/* NextGadget */
    -113, 50,			/* LeftEdge, TopEdge */
    BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
    GRELRIGHT | GADGIMAGE |
    GADGHIMAGE,			/* Flags */
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_3_IMAGE,		/* GadgetRender */
    (APTR) BOX_3_HIMAGE,	/* SelectRender */
    &cancel_text,		/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    2,				/* GadgetID */
    NULL			/* UserData */
};

/* Okay Gadget */

static struct IntuiText okay_text = {
    1, 0, JAM1, 30, 3, NULL, (UBYTE *) "OK", NULL
};

static struct Gadget okay_gadget = {
    &cancel_gadget,		/* NextGadget */
    20, 50,			/* LeftEdge, TopEdge */
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



/* String gadget */

static char stringbuf[SBUF_SIZE];
static char udb[SBUF_SIZE];	/* Undo Buffer for requester string gadget */

static struct StringExtend string_ex = {
	RTB,			/* Font */
	1, 0,			/* Pens */
	1, 0,			/* ActivePens */
	0,				/* InitialModes */
	NULL,			/* EditHook */
	NULL,			/* WorkBuffer */
	0L, 0L, 0L, 0L	/* Reserved */
};
	

static struct StringInfo string_info = {
    (UBYTE *) stringbuf,	/* Buffer */
    (UBYTE *) udb,		/* UndoBuffer */
    0,				/* BufferPos */
    SBUF_SIZE - 1,		/* MaxChars */
    0,				/* DispPos */
    0, 0, 0, 0, 0,
	&string_ex,
	0, NULL
};


#define GHI	 11
#define GWI	240

static SHORT string_vectors[] = {
    0, 0, GWI + 6, 0, GWI + 6, GHI, 0, GHI, 0, 0
};

static struct Border    string_box = {
    -4, -2, 3, 0, JAM1, 5, string_vectors, NULL
};

static struct Gadget string_gadget = {
    &okay_gadget,		/* NextGadget */
    36, 34,			/* LeftEdge, TopEdge */
    GWI, 8 /* GHI - 1 */,		/* Width, Height */
    GADGHCOMP,			/* Flags */
	STRINGEXTEND |
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | STRGADGET,	/* GadgetType */
    (APTR) &string_box,		/* GadgetRender */
    NULL,			/* SelectRender */
    NULL,			/* GadgetText */
    NULL,			/* MutualExclude */
    (APTR) &string_info,	/* SpecialInfo */
    10,				/* GadgetID */
    NULL			/* UserData */
};


/*-------------------------------------------------------------------*/

static struct IntuiText title_itext = {
    1, 0, JAM1, 10, 4, NULL, RTB, NULL
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
    NULL,			/* OlderRequester */
    REQ_LEFT, REQ_TOP,		/* LeftEdge, TopEdge */
    REQ_WIDTH, REQ_HEIGHT,	/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &string_gadget,		/* ReqGadget */
    &req_border[0],		/* ReqBorder */
    &title_itext,		/* ReqText */
    NULL,			/* Flags */
    BACKFILL_PEN,	/* BackFill */
    NULL,			/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,			/* RWindow */
    { NULL }			/* Pad */
};



/**********************  BRU Query Requester  *******************/

static struct IntuiText text2_itext = {
    1, 0, JAM1, 10, 20, NULL, RTB, NULL
};

static struct IntuiText text1_itext = {
    1, 0, JAM1, 10, 8, NULL, RTB, &text2_itext
};

static SHORT qreq_border_vectors[2][10] = {
    {
	0, 0, QREQ_WIDTH - 1, 0, QREQ_WIDTH - 1, QREQ_HEIGHT - 1,
	0, QREQ_HEIGHT - 1, 0, 0
    },
    {
	0, 0, QREQ_WIDTH - 5, 0, QREQ_WIDTH - 5, QREQ_HEIGHT - 3,
	0, QREQ_HEIGHT - 3, 0, 0
    }
};

static struct Border qreq_border[] = {
    { 0, 0, 2, 0, JAM1, 5, (SHORT *) &qreq_border_vectors[0], &qreq_border[1] },
    { 2, 1, 3, 0, JAM1, 5, (SHORT *) &qreq_border_vectors[1], NULL }
};

static struct Requester query_req = {
    NULL,			/* OlderRequester */
    QREQ_LEFT, QREQ_TOP,	/* LeftEdge, TopEdge */
    QREQ_WIDTH, QREQ_HEIGHT,	/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &string_gadget,		/* ReqGadget */
    &qreq_border[0],		/* ReqBorder */
    &text1_itext,		/* ReqText */
    NULL,			/* Flags */
    BACKFILL_PEN,	/* BackFill */
    NULL,			/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,			/* RWindow */
    { NULL }			/* Pad */
};



/* This assumes coming up in the main window */

BOOL do_string_req (title, string)
char *title;
char *string;
{
    struct IntuiMessage *message;
    ULONG class;
    struct Gadget *gadget;
    ULONG old_idcmp;
    BOOL finished = FALSE;
    struct Window *window;
    BOOL rc;

    DBUG_ENTER ("do_string_req");

	okay_text.ITextFont = textattr;
	cancel_text.ITextFont = textattr;
	title_itext.ITextFont = textattr;

	okay_gadget.NextGadget = &cancel_gadget;
	
    window = mainwin;
    req.RWindow = window;
	string_ex.Font = textfont;
    strlcpy (stringbuf, string, SBUF_SIZE);
    title_itext.IText = (UBYTE *) title;

    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP | GADGETDOWN | REQSET );
    /* Post the requester */
    if (Request (&req, window) == FALSE) {
	/* Could not post the requester */
	ModifyIDCMP (window, old_idcmp);
	DBUG_RETURN (FALSE);
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
		    if (gadget == &cancel_gadget) {
			/* User canceled */
			rc = FALSE;
		    } else {
			/* User said Okay or hit return in string gadg */
			strcpy (string, stringbuf);
			rc = TRUE;
		    }
			finished = TRUE;
		    break;
		case REQSET:
			if(  FlagIsSet( req.Flags, REQACTIVE ) )
			{
			    ActivateGadget (&string_gadget, window, &req);
			}
		    break;
	    }
	}
    }
    ModifyIDCMP (window, old_idcmp);
    DBUG_RETURN (rc);
}



void do_bru_query_req( char *text1, char *text2, char *string)
{
    struct IntuiMessage *message;
    ULONG class;
    struct Gadget *gadget;
    ULONG old_idcmp;
    BOOL finished = FALSE;
    struct Window *window;


    DBUG_ENTER ("do_bru_query_req");

 	okay_text.ITextFont = textattr;
	text1_itext.ITextFont = textattr;
	text2_itext.ITextFont = textattr;

	okay_gadget.NextGadget = NULL;

    window = mainwin;
    req.RWindow = window;
	string_ex.Font = textfont;
    strlcpy (stringbuf, string, SBUF_SIZE);
    text1_itext.IText = (UBYTE *) text1;
    text2_itext.IText = (UBYTE *) text2;

    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP | GADGETDOWN | REQSET );

    ActivateWindow (window);	/* So we can activate the string gadget */

    /* Post the requester */
    if (Request (&query_req, window) == FALSE)
	{
		/* Could not post the requester */
		ModifyIDCMP (window, old_idcmp);
		DBUG_VOID_RETURN;
    }

    while (finished == FALSE)
	{
		WaitPort (window -> UserPort);

		while (message = (struct IntuiMessage *) GetMsg (window -> UserPort))
		{
		    class = message -> Class;
	    	gadget = (struct Gadget *) message -> IAddress;
		    ReplyMsg ((struct Message *) message);
		    switch (class)
			{
				case GADGETUP: 
				case GADGETDOWN: 
					/* User said Okay or hit return in string gadg */
					strcpy (string, stringbuf);
					finished = TRUE;
				    break;

				case REQSET: 
					if(  FlagIsSet( query_req.Flags, REQACTIVE ) )
					{	
					    ActivateGadget (&string_gadget, window, &query_req);
					}
				    break;
		    }
		}
    }

    ModifyIDCMP (window, old_idcmp);

    DBUG_VOID_RETURN;
}



/* Cancel Requester */

#define CANCEL_LEFT		280
#define CANCEL_TOP		120
#define CANCEL_WIDTH		300
#define CANCEL_HEIGHT		60

static struct IntuiText cancel_itext = {
    1, 0, JAM1, 12, 3, NULL, (UBYTE *) "Click to Cancel", NULL
};

static struct IntuiText cancel_titletext = {
    1, 0, JAM1, 32, 10, NULL, RTB, NULL
};

static struct Gadget cancel_req_gadget = {
    NULL,			/* NextGadget */
    70, 30,			/* LeftEdge, TopEdge */
    BOX_2_WIDTH, BOX_2_HEIGHT,	/* Width, Height */
    GADGIMAGE | GADGHIMAGE,	/* Flags */
    RELVERIFY,			/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_2_IMAGE,		/* GadgetRender */
    (APTR) BOX_2_HIMAGE,	/* SelectRender */
    &cancel_itext,		/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    1,				/* GadgetID */
    NULL			/* UserData */
};

static SHORT cancel_vectors[2][10] = {
    {
	0, 0, CANCEL_WIDTH - 1, 0, CANCEL_WIDTH - 1, CANCEL_HEIGHT - 1,
	0, CANCEL_HEIGHT - 1, 0, 0
    },
    {
	0, 0, CANCEL_WIDTH - 3, 0, CANCEL_WIDTH - 3, CANCEL_HEIGHT - 3,
	0, CANCEL_HEIGHT - 3, 0, 0
    }
};

static struct Border cancel_border[] = {
    { 0, 0, 2, 0, JAM1, 5, (SHORT *) &cancel_vectors[0], &cancel_border[1] },
    { 1, 1, 3, 0, JAM1, 5, (SHORT *) &cancel_vectors[1], NULL }
};

static struct Requester cancel_req = {
    NULL,			/* OlderRequester */
    CANCEL_LEFT, CANCEL_TOP,	/* LeftEdge, TopEdge */
    CANCEL_WIDTH, CANCEL_HEIGHT,/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &cancel_req_gadget,		/* ReqGadget */
    &cancel_border[0],		/* ReqBorder */
    &cancel_titletext,		/* ReqText */
    NULL,			/* Flags */
    BACKFILL_PEN,	/* BackFill */
    NULL,			/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,			/* RWindow */
    { NULL }			/* Pad */
};

/*
 * Points to the window containing the Cancel requester.
 * Also used as the indicator of wether or not the Cancel
 * requester is open.
 */

static struct Window *cancel_win = NULL;
static ULONG old_idcmp;

/* Returns TRUE if the user said CANCEL */

BOOL check_cancel ()
{
    struct IntuiMessage *message;
    ULONG class;
    /* struct Gadget *gadget; */

    DBUG_ENTER ("check_cancel");
    if (FlagIsClear (cancel_req.Flags, REQACTIVE)) {
	/* Requester is not posted... */
	DBUG_RETURN (FALSE);
    }
    /* See if user clicked the CANCEL gadget */
    if (message = (struct IntuiMessage *) GetMsg (cancel_win -> UserPort)) {
	class = message -> Class;
	/* gadget = (struct Gadget *) message -> IAddress; */
	ReplyMsg ((struct Message *) message);
	if (class == GADGETUP) {
	    /* They said cancel, make sure though */
	    if (ask_question ("  Really Cancel?  ") == TRUE) {
		clear_cancel_req ();
		DBUG_RETURN (TRUE);
	    } else {
		DBUG_RETURN (FALSE);
	    }
	}
	/* Should not ever get here!  But just in case... */
	DBUG_RETURN (FALSE);
    }
    DBUG_RETURN (FALSE);
}

/*
 * Post the CANCEL button requester in the desired window.
 * This will take over the IDCMP, no input should be expected
 * from this window while the CANCEL button is posted.
 * If the cancel requester cannot be posted (which would be
 * a rare happening), no action is taken, and calls to
 * user_cancel () will return that the user did not click the
 * cancel gadget.
 * WARNING: This is not re-entrant!
 *
 *	BUGS:
 * If this requester is cleared from the window and then immediatly
 * re-posted, the re-posting may fail due to the requester not actually
 * being cleared yet.  By the same token, the requester may not actually
 * be posted yet when this call returns.  The real clean way to handle
 * posting and removing is to not return until Intuition sends us a
 * message of type REQSET or REQCLEAR.
 */

void post_cancel_req (window, string)
struct Window *window;
char *string;
{
    DBUG_ENTER ("post_cancel_req");
    /* just in case we are already posted */
    if (FlagIsSet (cancel_req.Flags, REQACTIVE)) {
	DBUG_VOID_RETURN;
    }

	cancel_itext.ITextFont = textattr;
	cancel_titletext.ITextFont = textattr;

    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP);
    cancel_win = window;
    cancel_titletext.IText = (UBYTE *) string;
    if (Request (&cancel_req, window) == FALSE) {
	ModifyIDCMP (window, old_idcmp);
    }
    DBUG_VOID_RETURN;
}

/*
 * Removes the CANCEL requester from the window.
 * This is a "safe" call which can always be made.
 */

void clear_cancel_req ()
{
    DBUG_ENTER ("clear_cancel_req");
    if (FlagIsClear (cancel_req.Flags, REQACTIVE)) {
	DBUG_VOID_RETURN;
    }
    EndRequest (&cancel_req, cancel_win);
    ModifyIDCMP (cancel_win, old_idcmp);
    DBUG_VOID_RETURN;
}

/* File Data Requester */

#undef REQ_LEFT
#undef REQ_TOP
#undef REQ_WIDTH
#undef REQ_HEIGHT

#define REQ_LEFT	4
#define REQ_TOP		30
#define REQ_WIDTH	632
#define REQ_HEIGHT	140

/* Done Gadget */

static struct IntuiText done_text = {
    1, 0, JAM1, 26, 3, NULL, (UBYTE *) "OK", NULL
};

static struct Gadget done_gadget = {
    NULL,			/* NextGadget */
    520, 122,			/* LeftEdge, TopEdge */
    BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
    GADGIMAGE | GADGHIMAGE,	/* Flags */
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_3_IMAGE,		/* GadgetRender */
    (APTR) BOX_3_HIMAGE,	/* SelectRender */
    &done_text,			/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    1,				/* GadgetID */
    NULL			/* UserData */
};

char *atrib_set_string[8] = {
    "",				/* d */
    "",				/* e */
    "",				/* w */
    "",				/* r */
    "Archived  ",
    "Pure  ",
    "Script  ",
    ""
};

char *atrib_clear_string[8] = {
    "Deleteable  ",
    "Executable  ",
    "Writeable  ",
    "Readable  ",
    "",				/* a */
    "",				/* p */
    "",				/* s */
    ""
};

static char buf2[108];
static char buf3[40];
static char buf4[80];
static char buf5[30];
static char buf6[80];

static char dumpline[4][81];

static struct IntuiText fdata_itext[] = {
    { 3, 0, JAM1, 241, 7, NULL, (UBYTE *) "File Data", &fdata_itext[1] },
    { 1, 0, JAM1, 240, 6, NULL, (UBYTE *) "File Data", &fdata_itext[2] },
    { 1, 0, JAM1, 80, 20, NULL, (UBYTE *) buf2, &fdata_itext[3] },
    { 1, 0, JAM1, 80, 30, NULL, (UBYTE *) buf3, &fdata_itext[4] },
    { 1, 0, JAM1, 10, 61, NULL, (UBYTE *) buf4, &fdata_itext[5] },
    { 1, 0, JAM1, 394, 30, NULL, (UBYTE *) buf5, &fdata_itext[6] },
    { 1, 0, JAM1, 80, 40, NULL, (UBYTE *) buf6, &fdata_itext[7] },
    { 1, 0, JAM1, 10, 80, NULL, (UBYTE *) &dumpline[0], &fdata_itext[8] },
    { 1, 0, JAM1, 10, 90, NULL, (UBYTE *) &dumpline[1], &fdata_itext[9] },
    { 1, 0, JAM1, 10, 100, NULL, (UBYTE *) &dumpline[2], &fdata_itext[10] },
    { 1, 0, JAM1, 10, 110, NULL, (UBYTE *) &dumpline[3], &fdata_itext[11] },
    { 3, 0, JAM1, 10, 51, NULL, (UBYTE *) "File Note:", &fdata_itext[12] },
    { 3, 0, JAM1, 10, 30, NULL, (UBYTE *) "Size:", &fdata_itext[13] },
    { 3, 0, JAM1, 340, 30, NULL, (UBYTE *) "Date:", &fdata_itext[14] },
    { 3, 0, JAM1, 10, 40, NULL, (UBYTE *) "Status:", &fdata_itext[15] },
    { 3, 0, JAM1, 10, 20, NULL, (UBYTE *) "Name:", &fdata_itext[16] },
    { 3, 0, JAM1, 10, 71, NULL, (UBYTE *)
	"------------------------------------------------------\
---------------------",
	NULL }
};

static SHORT freq_border_vectors[2][10] = {
    {
	0, 0, REQ_WIDTH - 1, 0, REQ_WIDTH - 1, REQ_HEIGHT - 1,
	0, REQ_HEIGHT - 1, 0, 0
    },
    {
	0, 0, REQ_WIDTH - 3, 0, REQ_WIDTH - 3, REQ_HEIGHT - 3,
	0, REQ_HEIGHT - 3, 0, 0
    }
};

static struct Border freq_border[] = {
    { 2, 0, 2, 0, JAM1, 5, (SHORT *) &freq_border_vectors[0], &freq_border[1] },
    { 3, 1, 3, 0, JAM1, 5, (SHORT *) &freq_border_vectors[1], NULL }
};

static struct Requester fdata_req = {
    NULL,			/* OlderRequester */
    REQ_LEFT, REQ_TOP,		/* LeftEdge, TopEdge */
    REQ_WIDTH, REQ_HEIGHT,	/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &done_gadget,		/* ReqGadget */
    &freq_border[0],		/* ReqBorder */
    &fdata_itext[0],		/* ReqText */
    NOISYREQ,			/* Flags */
    BACKFILL_PEN,		/* BackFill */
    NULL,			/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,			/* RWindow */
    { NULL }			/* Pad */
};

/* This assumes coming up in the main window */

BOOL do_fdata_req (string)
char *string;
{
    struct IntuiMessage *message;
    ULONG class;
    /* struct Gadget *gadget; */
    ULONG old_idcmp;
    BOOL finished = FALSE;
    struct Window *window;
    ERRORCODE rc;
	struct IntuiText *it;


    DBUG_ENTER ("do_fdata_req");

	/* Set the text in the requester to use our font */
	it = &fdata_itext[0];
	while( it != NULL )
	{
		it->ITextFont = textattr;
		it = it->NextText;
	}

	done_text.ITextFont = textattr;


    window = mainwin;
    fdata_req.RWindow = window;
    if (rc = format_fdata (string)) {
	mention_error ("Unable to display additional file data", rc);
	DBUG_RETURN (FALSE);
    }
    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP | GADGETDOWN | REQSET | VANILLAKEY);
    /* Post the requester */
    if (Request (&fdata_req, window) == FALSE) {
	/* Could not post the requester */
	ModifyIDCMP (window, old_idcmp);
	DBUG_RETURN (FALSE);
    }
    while (finished == FALSE) {
	WaitPort (window -> UserPort);
	while (message = (struct IntuiMessage *) GetMsg (window -> UserPort)) {
	    class = message -> Class;
	    /* gadget = (struct Gadget *) message -> IAddress; */
	    ReplyMsg ((struct Message *) message);
	    switch (class) {
		case GADGETUP: 
		case GADGETDOWN: 
			/* Only gadget is the "done" gadget */
			finished = TRUE;
		    break;

		case VANILLAKEY: 
		    EndRequest (&fdata_req, window);
			finished = TRUE;
		    break;
	    }
	}
    }
    ModifyIDCMP (window, old_idcmp);
    DBUG_RETURN (TRUE);
}

static ERRORCODE format_fdata (string)
char *string;
{
    struct FileInfoBlock *fib;
    BPTR lock;
    int i;
    LONG len;
    BPTR fh;
    char buf[64];
	char *s1, *s2;


    DBUG_ENTER ("format_fdata");
    DBUG_PRINT ("string", ("string = '%s'", string));
    lock = Lock (string, ACCESS_READ);
    if (lock == NULL) {
	DBUG_RETURN (ERC_NO_FILE | ERR24);
    }
    /* Get a file info block we can use to read directory entries into. */
    fib = (struct FileInfoBlock *)
	RemAlloc ((ULONG) sizeof (struct FileInfoBlock), MEMF_PUBLIC);
    if (fib == NULL) {
	UnLock (lock);
	DBUG_RETURN (ERC_NO_MEMORY | ERR25);
    }

    if (Examine (lock, fib) == 0) {
	UnLock (lock);
	RemFree (fib);
	DBUG_RETURN (ERC_IO_FAILED | ERR26);
    }

    strcpy (buf2, fib -> fib_FileName);

	s1 = ( fib->fib_Size != 1 ) ? "bytes" : "byte";
	s2 = ( fib->fib_NumBlocks != 1 ) ? "blocks" : "block";
    sprintf (buf3, "%ld %s or %ld %s", fib -> fib_Size, s1,
		fib -> fib_NumBlocks, s2 );

	if ( *(fib -> fib_Comment) == '\0' ) {
	strcpy (buf4, "<no file comment>");
	} else {
    strcpy (buf4, fib -> fib_Comment);
	}

    days_to_string (fib -> fib_Date.ds_Days, buf5);
    sprintf (buf, "  %02ld:%02ld:%02ld",
	    fib -> fib_Date.ds_Minute / 60,
	    fib -> fib_Date.ds_Minute % 60,
	    fib -> fib_Date.ds_Tick / TICKS_PER_SECOND);
    strcat (buf5, buf);

    buf6[0] = '\0';
    for (i = 7; i >= 0; i--) {
	if (FlagIsSet (fib -> fib_Protection, 1L << (long) i)) {
	    strcat (buf6, atrib_set_string[i]);
	} else {
	    strcat (buf6, atrib_clear_string[i]);
	}
    }
    /* Do a partial file dump */
    dumpline[2][0] = '\0';
    dumpline[3][0] = '\0';
    fh = Open (string, MODE_READONLY);
    if (fh == NULL) {
	strcpy (dumpline[0], "   Unable to open");
	strcpy (dumpline[1], "     the file!");
    } else {
	len = Read (fh, buf, 64L);
	if (len == 0) {
	    strcpy (dumpline[0], "   Unable to read");
	    strcpy (dumpline[1], "     the file!");
	} else {
	    for (i = 0; i < 4; i++) {
		hexdump_format (dumpline[i], &buf[i * 16],
			(int) (MAX ((len - (i * 16)), 0)), (LONG) i * 16);
	    }
	}
	Close (fh);
    }

    UnLock (lock);
    RemFree (fib);
    DBUG_RETURN (0);
}

static void hexdump_format (linebuf, databuf, count, num)
char *linebuf;
UBYTE *databuf;
int count;
LONG num;
{
    int i;
    char buf[16];
    char buf2[17];

    DBUG_ENTER ("hexdump_format");
    sprintf (linebuf, "%08lx  ", num);
    buf2[16] = '\0';
    for (i = 0; i < 16; i++) {
	if (i < count) {
	    sprintf (buf, "%02x ", (int) (databuf[i]));
	    strcat (linebuf, buf);
	    buf2[i] = (databuf[i] >= 0x20) && (databuf[i] <= 0x7F)
		? databuf[i] : '.';
	} else {
	    strcat (linebuf, "   ");
	    buf2[i] = ' ';
	}
    }
    strcat (linebuf, " ");
    strcat (linebuf, buf2);
    DBUG_VOID_RETURN;
}



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


static struct Window *canceling_win = NULL;



static struct IntuiText canceling_titletext = {
    1, 0, JAM1, 32, 10, NULL, (UBYTE *)"Canceling...", NULL
};

static SHORT canceling_vectors[2][10] = {
    {
	0, 0, CANCEL_WIDTH - 1, 0, CANCEL_WIDTH - 1, CANCEL_HEIGHT - 1,
	0, CANCEL_HEIGHT - 1, 0, 0
    },
    {
	0, 0, CANCEL_WIDTH - 3, 0, CANCEL_WIDTH - 3, CANCEL_HEIGHT - 3,
	0, CANCEL_HEIGHT - 3, 0, 0
    }
};

static struct Border canceling_border[] = {
    { 0, 0, 2, 0, JAM1, 5, (SHORT *) &canceling_vectors[0],
		&canceling_border[1] },
    { 1, 1, 3, 0, JAM1, 5, (SHORT *) &canceling_vectors[1], NULL }
};

static struct Requester canceling_req = {
    NULL,				/* OlderRequester */
    CANCEL_LEFT, CANCEL_TOP,	/* LeftEdge, TopEdge */
    CANCEL_WIDTH, CANCEL_HEIGHT,/* Width, Height */
    0, 0,				/* RelLeft, RelTop */
    NULL,				/* ReqGadget */
    &canceling_border[0],		/* ReqBorder */
    &canceling_titletext,		/* ReqText */
    NULL,				/* Flags */
    BACKFILL_PEN,		/* BackFill */
    NULL,				/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,				/* RWindow */
    { NULL }			/* Pad */
};


void post_canceling( win )
struct Window *win;
{
	canceling_titletext.ITextFont = textattr;

	if(  Request( &canceling_req, win ) == TRUE  )
	{
		canceling_win = win;
	}
}

void clear_canceling()
{
	if( canceling_win != NULL )
	{
	    EndRequest (&canceling_req, canceling_win);
		canceling_win = NULL;
	}
}

