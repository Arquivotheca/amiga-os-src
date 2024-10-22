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
#include "bailout.h"
#include "brushell.h"
#include "dirtree.h"
#include "mainwin.h"
#include "slist.h"
#include "dbug.h"
#include "libfuncs.h"

static int create_item_gadgets PROTO((void));
static void calculate_slist PROTO((void));
static void do_scrolling PROTO((void));

#define SCROLL_WIDTH	16

/* Refresh rate in INTUITICKS for the real-time scrolling mode */
#define SCROLL_RATE	2

#define ID_OFFSET	1000

static BOOL slist_flag = FALSE;

static int height;
static int width;
static int left;
static int top;
static int gadget_count;

static struct Gadget *slist_gadget;
static struct IntuiText *slist_itext;

int entry_count = 0;

static LONG first_item = 0;

static int font_height;
static int font_width;

/* 53 spaces */
static char blankstring[] = "                                           \
          ";

static struct Image knob;

static struct PropInfo prop_info = {
    AUTOKNOB |
	FREEVERT |
	PROPNEWLOOK,	/* Flags */
    0, 0,			/* HorizPot, VertPot */
    0, 0xFFFF,		/* HorizBody, VertBody */
    0, 0,			/* CWidth, CHeight */
    0, 0,			/* HPotRes, VPotRes */
    0, 0			/* LeftBorder, TopBorder */
};

static struct Gadget scroll_gadget = {
    NULL,			/* NextGadget */
    RTB, RTB,			/* LeftEdge, TopEdge */
    SCROLL_WIDTH, RTB,		/* Width, Height */
    GADGHNONE,			/* Flags */
    GADGIMMEDIATE | RELVERIFY,	/* Activation */
    PROPGADGET,			/* GadgetType */
    (APTR) &knob,		/* GadgetRender */
    NULL,			/* SelectRender */
    NULL,			/* GadgetText */
    NULL,			/* MutualExclude */
    (APTR) &prop_info,		/* SpecialInfo */
    ID_OFFSET,			/* GadgetID */
    NULL			/* UserData */
};

/* This is used as a template for the dynamic gadget allocation */

static struct Gadget item_gadget = {
    RTB,			/* NextGadget */
    4, RTB,			/* LeftEdge, TopEdge */
    RTB, RTB,			/* Width, Height */
    GADGHCOMP,			/* Flags */
    GADGIMMEDIATE,		/* Activation */
    BOOLGADGET,			/* GadgetType */
    NULL,			/* GadgetRender */
    NULL,			/* SelectRender */
    RTB,			/* GadgetText */
    NULL,			/* MutualExclude */
    NULL,			/* SpecialInfo */
    RTB,			/* GadgetID */
    NULL			/* UserData */
};

static struct IntuiText intuitext_template = {
    RTB, 0,			/* FrontPen, BackPen */
    JAM2,			/* DrawMode */
    0, 0,			/* LeftEdge, TopEdge */
    RTB,			/* ITextFont */
    RTB,			/* IText */
    NULL			/* NextText */
};

static SHORT box_vectors[10] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static struct Border box = {
    0, 0,			/* LeftEdge, TopEdge */
    1, 0,			/* FrontPen, BackPen */
    JAM1,			/* DrawMode */
    5,				/* Count */
    &box_vectors[0],		/* XY */
    NULL			/* NextBorder */
};

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*								*/
/*		init_slist ()					*/
/*								*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

ERRORCODE init_slist ()
{
    DBUG_ENTER ("init_slist");

    /* Set height and width of the font in the window */
    font_height = slist_textattr -> ta_YSize;
    /* mainwin -> RPort -> TxHeight; */
    font_width = 8;
    /* mainwin -> RPort -> TxWidth; */
    /* Set Height */
    top = mainwin -> BorderTop + font_height + 3 + 14;
    left = 178;
    width = 452;
    height = mainwin -> Height - (top + mainwin -> BorderBottom);

    /* Initialize the Scroll Gadget */
    scroll_gadget.LeftEdge = ((width - SCROLL_WIDTH) - 3) + left;
    scroll_gadget.TopEdge = top + 3;
    scroll_gadget.Height = height - 6;

    /* Initialize the border box structure and vectors */
    box_vectors[2] = width - 1;
    box_vectors[4] = width - 1;
    box_vectors[5] = height - 1;
    box_vectors[7] = height - 1;

    /* Allocate the gadgets and their IntuiText */
    if (create_item_gadgets ()) {
	DBUG_RETURN (99);
    }

#if 0		/* This cannot be done just yet...   DTM 11-May-90 */
    /* Link the gadget text, init the PropInfo paramaters, etc. */
    calculate_slist ();
#endif

    /* make a bordering box for the roster */
    DrawBorder (mainwin -> RPort, &box, (LONG) left, (LONG) top);

    /* Put our gadgets in the window */
    AddGList (mainwin, slist_gadget, 0L, (ULONG) gadget_count + 1, NULL);

    /* And draw them! */
    RefreshGList (slist_gadget, mainwin, 0L, (ULONG) gadget_count + 1);

    slist_flag = TRUE;

    DBUG_RETURN (0);
}

static int create_item_gadgets ()
{
    int i;
    int gwidth;
    ULONG size;

    DBUG_ENTER ("create_item_gadgets");
    /* Decide how many gadgets will fit and their width */
    gadget_count = (height - 5) / font_height;
    gwidth = (width - SCROLL_WIDTH) - 14;
    /* Allocate memory for the gadget structures */
    size = sizeof (struct Gadget) * gadget_count;
    slist_gadget = (struct Gadget *) RemAlloc (size, MEMF_CLEAR | MEMF_PUBLIC);
    if (slist_gadget == NULL) {
	DBUG_RETURN (99);
    }
    /* Allocate memory for the IntuiText structures */
    size = sizeof (struct IntuiText) * gadget_count;
    slist_itext = (struct IntuiText *) RemAlloc (size, MEMF_CLEAR | MEMF_PUBLIC);
    if (slist_itext == NULL) {
	RemFree (slist_gadget);
	DBUG_RETURN (99);
    }
    /* Initialize the list item Gadget and IntuiText structures */
    for (i = 0; i < gadget_count; i++) {
	/* Gadget */
	CopyMem ((char *) &item_gadget, (char *) &slist_gadget[i],
		(long) sizeof (item_gadget));
	slist_gadget[i].NextGadget = &slist_gadget[i + 1];
	slist_gadget[i].LeftEdge = left + 4;
	slist_gadget[i].TopEdge = top + (i * font_height) + 2;
	slist_gadget[i].Width = gwidth;
	slist_gadget[i].Height = font_height;
	slist_gadget[i].GadgetText = &slist_itext[i];
	slist_gadget[i].GadgetID = i + 1 + ID_OFFSET;
	/* Intuitext */
	CopyMem ((char *) &intuitext_template, (char *) &slist_itext[i],
		(long) sizeof (intuitext_template));
	slist_itext[i].ITextFont = slist_textattr;
    }
    /* Link the scroll gadget with the item gadgets. */
    slist_gadget[gadget_count - 1].NextGadget = &scroll_gadget;
    DBUG_RETURN (0);
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*								*/
/*		handle_slist ()					*/
/*								*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

/* If the gadget ID passed belongs to a gadget in the list, then
 * perform the appropriate action.  If a list item gadget, then return
 * the string number (starting at 1).  If the scroll gadget,
 * then make the display changes neccessary and return a 0.  Note that
 * at this time, the scroll gadget acts only upon gadget DOWN messages.
 */

int handle_slist (gadg, class)
ULONG gadg;
ULONG class;
{
    int status = 0;

    DBUG_ENTER ("handle_slist");

    if ((gadg >= ID_OFFSET) && (gadg <= ID_OFFSET + gadget_count))
	{
		/* It was a file list gadget that was hit */
		if (gadg == ID_OFFSET)
		{
		    /* It was the scroll gadget */
		    do_scrolling ();
		}
		else
		{
		    /* The user selected a list item gadget */
		    status = (gadg - ID_OFFSET) + first_item;

	    	if (status > entry_count)
			{
				/* User selected a gadget without text in it. */
				status = 0;
		    }
		}
    }

    DBUG_RETURN (status);
}



static void do_scrolling ()
{
    ULONG old_idcmp_flags;
    BOOL finished = FALSE;
    int tickcount = 0;
    struct IntuiMessage *message;
    ULONG class;
	USHORT last_potval;
	

    DBUG_ENTER ("do_scrolling");


    old_idcmp_flags = mainwin -> IDCMPFlags;
    ModifyIDCMP( mainwin, GADGETUP | INTUITICKS );

	last_potval = prop_info.VertPot + 1;

    while( finished == FALSE )
	{
		/* do this until a gadgetup message */
		WaitPort( mainwin->UserPort );

		/* Okay, got a message */
		while(  ( finished == FALSE ) &&
			( message = (struct IntuiMessage *)GetMsg( mainwin -> UserPort)))
		{
		    class = message->Class;
		    ReplyMsg( (struct Message *)message );

		    /* Do something with message */
		    switch( class )
			{
				case GADGETUP:
				    finished = TRUE;
				    break;

				case INTUITICKS: 
				    if( tickcount++ > SCROLL_RATE )
					{
						tickcount = 0;
						if( prop_info.VertPot != last_potval )
						{
							update_slist( 0 );
							last_potval = prop_info.VertPot;
						}
				    }
				    break;
		    }
		}
    }

    ModifyIDCMP (mainwin, old_idcmp_flags);

    update_slist (0);		/* just to be sure... */

    DBUG_VOID_RETURN;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*								*/
/*		update_slist ()					*/
/*								*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

/* This will cause the list items to be re-displayed.  Any text
 * string content change will be seen in the gadgets text.  Also
 * if the number of strings has changed, that will be reflected
 * in a re-calculation of the knob size, etc.
 */

void update_slist (mode)
BOOL mode;
{
    LONG pos;

    DBUG_ENTER ("update_slist");

    /* Remove our gadgets so we can modify them safely */
    pos = RemoveGList (mainwin, slist_gadget, (ULONG) gadget_count /* + 1 */);

    if (mode == 1)
	{
		/* A brand new list, reset some PropInfo params. */
	    pos = RemoveGList (mainwin, &scroll_gadget, 1L );

		/* this may not be needed? */
		ClearFlag (prop_info.Flags, KNOBHIT);
		prop_info.VertPot = 0;

	    AddGList (mainwin, &scroll_gadget, pos, 1L, NULL);
    }

    calculate_slist ();

    /* Return our gadgets to the window */
    AddGList (mainwin, slist_gadget, pos, (ULONG) gadget_count /* + 1 */, NULL);

    /* And refresh them */
    RefreshGList (slist_gadget, mainwin, 0L, (ULONG) gadget_count + 1);

    DBUG_VOID_RETURN;
}

/*
 * This routine will link (or re-link) the text strings.
 * This can be caused by either a change made in the
 * text strings by the calling program, or by the user playing with
 * the scroll gadget.
 */

static void calculate_slist ()
{
    int i;
    LONG total_items;
    LONG visible_items;
	ULONG vbody;


    DBUG_ENTER ("calculate_slist");
    total_items = entry_count;
    visible_items = gadget_count;

    if (total_items >= visible_items)
	{
		/* List will actually need to scroll, formulas are valid */
		/* Set the scroll gadet body size */
		vbody = (ULONG) (visible_items * 0xFFFFL) /
		    total_items;
		first_item = ((ULONG) (total_items - visible_items) *
			prop_info.VertPot + (1L << 15)) >> 16;
    }
	else
	{
		/* Not enough items to fill window, use presets for Body and
		 * first item.
		 */
		vbody = MAXBODY;
		first_item = 0;
    }


	Forbid();
	NewModifyProp( &scroll_gadget, mainwin, NULL,
		prop_info.Flags, 0L, prop_info.VertPot, 0L, vbody, 1L );
	Permit();

	if( entry_color == NULL )
	{
		/* Currently, the entry color and entry string arrays are
		 * not alllocated.  Don't do the linkages!
		 */
	    DBUG_VOID_RETURN;
	}
	
    /* Link text strings to IntuiText structures */
    for (i = 0; i < gadget_count; i++)
	{
		if ((i + first_item) < entry_count)
		{
		    slist_itext[i].IText = (UBYTE *) (&entry_string[i + first_item]);
		}
		else
		{
	    	slist_itext[i].IText = (UBYTE *) blankstring;
		}

		/* Is the entry selected etc.?  Set appropriate text color */
		slist_itext[i].FrontPen = entry_color[i + first_item];
    }

    DBUG_VOID_RETURN;
}

void cleanup_slist ()
{
    DBUG_ENTER ("cleanup_slist");
    if (slist_flag == FALSE) {
	DBUG_VOID_RETURN;
    }
    RemoveGList (mainwin, slist_gadget, (ULONG) gadget_count + 1);
    RemFree (slist_gadget);
    RemFree (slist_itext);
    DBUG_VOID_RETURN;
}
