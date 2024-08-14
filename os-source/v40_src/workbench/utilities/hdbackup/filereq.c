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

#include <dos/dosasl.h>
#include <libraries/asl.h>
/* #include <libraries/aslbase.h> */
/* #include "clib/dos_protos.h" */
#include "clib/asl_protos.h"
#include "clib/utility_protos.h"
#include "pragmas/asl_pragmas.h"

/* #include <proto/asl.h> */
/* #include <proto/utility.h> */

#include "standard.h"
#include "bailout.h"
#include "brushell.h"
#include "images.h"
#include "mainwin.h"
#include "filereq.h"
#include "dbug.h"
#include "getdisks.h"
#include "libfuncs.h"


void NewList( struct List *list );

/* This file contains the volume requester used to specify which volume
 * is to be backed up.
 */


#define REQ_LEFT	160
#define REQ_TOP		 40
#define REQ_WIDTH	428
#define REQ_HEIGHT	128

#define OKAY_ID		1
#define CANCEL_ID	2
#define STRING_ID	3

static  void init_volume_gadgets ( void );

static struct List disks;

/* Cancel Gadget */

static struct IntuiText cancel_text = {
    1, 0, JAM1, 18, 3, NULL, (UBYTE *) "Cancel", NULL
};

static struct Gadget cancel_gadget = {
    NULL,			/* NextGadget */
    260, 105,			/* LeftEdge, TopEdge */
    BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
    GADGIMAGE | GADGHIMAGE,	/* Flags */
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_3_IMAGE,		/* GadgetRender */
    (APTR) BOX_3_HIMAGE,	/* SelectRender */
    &cancel_text,		/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    CANCEL_ID,			/* GadgetID */
    NULL			/* UserData */
};

/* Okay Gadget */

static struct IntuiText okay_text = {
    1, 0, JAM1, 32, 3, NULL, (UBYTE *) "OK", NULL
};

static struct Gadget okay_gadget = {
    &cancel_gadget,		/* NextGadget */
    20, 105,			/* LeftEdge, TopEdge */
    BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
    GADGIMAGE | GADGHIMAGE,	/* Flags */
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | BOOLGADGET,	/* GadgetType */
    (APTR) BOX_3_IMAGE,		/* GadgetRender */
    (APTR) BOX_3_HIMAGE,	/* SelectRender */
    &okay_text,			/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    OKAY_ID,			/* GadgetID */
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

#define GHI	11
#define GWI	240

static  SHORT string_vectors[] = {
    0, 0, GWI + 6, 0, GWI + 6, GHI, 0, GHI, 0, 0
};

static struct Border string_box = {
    -4, -2, 3, 0, JAM1, 5, string_vectors, NULL
};

static struct Gadget string_gadget = {
    &okay_gadget,		/* NextGadget */
    86, 90,			/* LeftEdge, TopEdge */
    GWI, 8 /*GHI - 1*/,		/* Width, Height */
    GADGHCOMP,			/* Flags */
	STRINGEXTEND |
    ENDGADGET | RELVERIFY,	/* Activation */
    REQGADGET | STRGADGET,	/* GadgetType */
    (APTR) &string_box,		/* GadgetRender */
    NULL,			/* SelectRender */
    NULL,			/* GadgetText */
    NULL,			/* MutualExclude */
    (APTR) &string_info,	/* SpecialInfo */
    STRING_ID,			/* GadgetID */
    NULL			/* UserData */
};

/* Volume name gadgets */

#define NUM_VOL_GADGETS 16
#define GTEXT_WIDTH	9	/* including the terminating null */

static char gadgtext[NUM_VOL_GADGETS][GTEXT_WIDTH];

static struct IntuiText vol_text[NUM_VOL_GADGETS] = {
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[0], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[1], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[2], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[3], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[4], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[5], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[6], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[7], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[8], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[9], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[10], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[11], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[12], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[13], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[14], NULL },
    { 1, 0, JAM1, 14, 2, NULL, (UBYTE *) & gadgtext[15], NULL }
};


static struct Gadget vol_gadget[NUM_VOL_GADGETS] = {
    {
	&vol_gadget[1],			/* NextGadget */
	10, 10,				/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[0],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[2],			/* NextGadget */
	114, 10,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[1],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[3],			/* NextGadget */
	218, 10,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[2],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[4],			/* NextGadget */
	322, 10,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[3],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[5],			/* NextGadget */
	10, 25,				/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[4],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[6],			/* NextGadget */
	114, 25,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[5],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[7],			/* NextGadget */
	218, 25,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[6],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[8],			/* NextGadget */
	322, 25,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[7],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[9],			/* NextGadget */
	10, 40,				/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[8],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[10],		/* NextGadget */
	114, 40,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[9],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[11],		/* NextGadget */
	218, 40,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[10],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[12],		/* NextGadget */
	322, 40,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[11],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[13],		/* NextGadget */
	10, 55,				/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[12],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[14],		/* NextGadget */
	114, 55,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[13],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&vol_gadget[15],		/* NextGadget */
	218, 55,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[14],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    },
    {
	&string_gadget,			/* NextGadget */
	322, 55,			/* LeftEdge, TopEdge */
	BOX_3_WIDTH, BOX_3_HEIGHT,	/* Width, Height */
	GADGIMAGE | GADGHIMAGE,		/* Flags */
	RELVERIFY,			/* Activation */
	REQGADGET | BOOLGADGET,		/* GadgetType */
	(APTR) BOX_3_IMAGE,		/* GadgetRender */
	(APTR) BOX_3_HIMAGE,		/* SelectRender */
	&vol_text[15],			/* GadgetText */
	NULL,				/* MutualExclude */
	NULL,				/* SpecialInfo */
	0,				/* GadgetID */
	NULL				/* UserData */
    }
};

static struct IntuiText title_itext = {
    1, 0, JAM1, 40, 77, NULL, RTB, NULL
};

#if 0
static struct IntuiText header_itext = {
    1, 0, JAM1, 60, 6, NULL, (UBYTE *) " BRUshell", &title_itext
};
#endif

static SHORT req_border_vectors[2][10] = {
    { 0, 0, REQ_WIDTH-1, 0, REQ_WIDTH-1, REQ_HEIGHT-1, 0, REQ_HEIGHT-1, 0, 0 },
    { 0, 0, REQ_WIDTH-3, 0, REQ_WIDTH-3, REQ_HEIGHT-3, 0, REQ_HEIGHT-3, 0, 0 }
};

static struct Border req_border[] = {
    { 2, 0, 1, 0, JAM1, 5, (SHORT *) & req_border_vectors[0], &req_border[1] },
    { 1, 1, 1, 0, JAM1, 5, (SHORT *) & req_border_vectors[1], NULL }
};

static struct Requester req = {
    NULL,			/* OlderRequester */
    REQ_LEFT, REQ_TOP,		/* LeftEdge, TopEdge */
    REQ_WIDTH, REQ_HEIGHT,	/* Width, Height */
    0, 0,			/* RelLeft, RelTop */
    &vol_gadget[0],		/* ReqGadget */
    &req_border[0],		/* ReqBorder */
    &title_itext,		/* ReqText */
    NULL,			/* Flags */
    0,				/* BackFill */
    NULL,			/* ReqLayer */
    { NULL },			/* Pad */
    { NULL },			/* BitMap */
    NULL,			/* RWindow */
    { NULL },			/* Pad */
};

/* This assumes coming up in the main window */

BOOL do_vol_req ( char *title, char *string )
{
    struct IntuiMessage *message;
    ULONG class;
    struct Gadget  *gadget;
    ULONG old_idcmp;
    BOOL finished = FALSE;
    struct Window *window;
    BOOL rc;
    ULONG pos;

    DBUG_ENTER ("do_file_req");

	/* Set the text font for the okay and cancel gadgets */
	cancel_text.ITextFont = textattr;
	string_ex.Font = textfont;
	okay_text.ITextFont = textattr;
	title_itext.ITextFont = textattr;

    init_volume_gadgets ();

    window = mainwin;
    req.RWindow = window;
    strlcpy (stringbuf, string, SBUF_SIZE);
    title_itext.IText = (UBYTE *) title;

    /* Save the old IDCMP flags and set it to what we need */
    old_idcmp = window -> IDCMPFlags;
    ModifyIDCMP (window, GADGETUP | GADGETDOWN | REQSET );

    /* Post the requester */
    if (Request (&req, window) == FALSE)
	{
		/* Could not post the requester */
		ModifyIDCMP (window, old_idcmp);
		DBUG_RETURN (FALSE);
    }

    while (finished == FALSE)
	{
		WaitPort (window -> UserPort);

		while (message = (struct IntuiMessage *) GetMsg (window -> UserPort)) {
		    class = message -> Class;
	    gadget = (struct Gadget *) message -> IAddress;
	    ReplyMsg ((struct Message *) message);

	    switch (class)
		{
			case GADGETUP: 
			case GADGETDOWN: 
			    switch (gadget -> GadgetID)
				{
					case CANCEL_ID: 
				    /* User canceled */
				    rc = FALSE;
					finished = TRUE;
			    	break;

				case OKAY_ID: 
				case STRING_ID: 
				    /* User said Okay or hit return in the
				     * string gadget.
			    	 */
				    strcpy (string, stringbuf);
				    rc = TRUE;
					finished = TRUE;
				    break;

				default: 
				    /* Only other gadgets are volume gadgets */
				    pos = RemoveGadget (window, &string_gadget);
				    strcpy (stringbuf, gadget -> GadgetText -> IText);
			    	string_info.BufferPos = strlen (stringbuf);
				    string_info.DispPos = 0;
				    string_info.MaxChars = SBUF_SIZE - 1;
				    /* string_info.NumChars = strlen(stringbuf); */
				    AddGList (window, &string_gadget, pos, 1L, &req);
				    RefreshGList (&string_gadget, window, &req, 1L);
				    ActivateGadget (&string_gadget, window, &req);
				    break;
		    	}
			    break;

			case REQSET: 
		    	ActivateGadget (&string_gadget, window, &req);
			    break;
#if 0
			case REQCLEAR: 
			    finished = TRUE;
		    	break;
#endif
		    }
		}
    }

    ModifyIDCMP (window, old_idcmp);

    DBUG_RETURN (rc);
}

static  void init_volume_gadgets ( void )
{
    struct Node *disk;
    int i;

    DBUG_ENTER ("init_volume_gadgets");

    /* Clear the volume gadget text strings and set the font */
    for (i = 0; i < NUM_VOL_GADGETS; i++)
	{
		gadgtext[i][0] = '\0';
		vol_text[i].ITextFont = textattr;
    }

    i = 0;
    NewList (&disks);		/* Initialize list header */
    getdisks (&disks);		/* Fill list */
    /* print any devices in list.... if any */
    if (disks.lh_TailPred != (struct Node  *) & disks) {
	for (disk = disks.lh_Head; disk -> ln_Succ; disk = disk -> ln_Succ) {
	    if (i < NUM_VOL_GADGETS) {
		if (strlen (disk -> ln_Name) <= GTEXT_WIDTH - 2) {
		    strcpy (gadgtext[i], disk -> ln_Name);
		    strcat (gadgtext[i++], ":");
		    DBUG_PRINT ("disk", ("%s", disk -> ln_Name));
		}
	    }
	}
    }
    freedisks (&disks);
    DBUG_VOID_RETURN;
}



/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

struct FileRequester *asl_filereq = NULL;

struct TagItem filetags[] = {
	ASL_Window, NULL,
	ASL_Hail, NULL,
	ASL_OKText, (ULONG)"OK",
/*	ASL_Pattern, (ULONG)"#?.s", */
	ASL_File, NULL,
	ASL_Dir, NULL,
/*	ASL_FuncFlags, (ULONG) ( FILF_SAVE + FILF_PATGAD ), */
	TAG_DONE,
};



BOOL do_file_req ( char *title, char *string )
{
	char path_buf[FMSIZE], file_buf[FNSIZE];


	stcgfp( path_buf, string );
	stcgfn( file_buf, string );

	/* Initialize the requester dynamic data fields */
	FindTagItem( ASL_Window, &filetags[0] ) -> ti_Data = (ULONG) mainwin;
	FindTagItem( ASL_Hail, &filetags[0] ) -> ti_Data = (ULONG) title;
	FindTagItem( ASL_File, &filetags[0] ) -> ti_Data = (ULONG) file_buf;
	FindTagItem( ASL_Dir, &filetags[0] ) -> ti_Data = (ULONG) path_buf;
	
	asl_filereq = (struct FileRequest *)
		AllocAslRequest( ASL_FileRequest, &filetags[0] );

	if ( asl_filereq == NULL )
	{
		mention_error( "Can't open file requester", ERC_NONE | ERR68 );
		return( FALSE );
	}

	if(  RequestFile( asl_filereq ) == 0  )
	{
		if( asl_filereq != NULL )
		{
			FreeFileRequest( asl_filereq );
			asl_filereq = NULL;
		}

		return( FALSE );
	}

	
	/* build full path + file name string */
	strmfp( string, asl_filereq->rf_Dir, asl_filereq->rf_File );

	if( asl_filereq != NULL )
	{
		FreeFileRequest( asl_filereq );
		asl_filereq = NULL;
	}

	return( TRUE );
}


