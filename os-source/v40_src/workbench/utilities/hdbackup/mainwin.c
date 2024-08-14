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
#include "eventloop.h"
#include "mainwin.h"
#include "menus.h"
#include "scansort.h"
#include "dbug.h"
#include "libfuncs.h"



/* The gadgets.c file should probably have a header now... */
extern void init_mainwin_gadgets( void );


int processed_count;
	/* Maintain a count of how many files we have processed */
	

static void drawstuff(void);
static void printat(int, int, char *);

struct Window *mainwin;
ULONG mainwin_sig_bit = 0;

struct NewWindow new_mainwin = {
    0, 1,			/* LeftEdge, TopEdge */
    640, 199,			/* Width, Height */
    0, 1,			/* DetailPen, BlockPen */
    GADGETDOWN | GADGETUP |
    MENUPICK,			/* IDCMP Flags */
    ACTIVATE | SMART_REFRESH |
    NOCAREREFRESH,		/* Flags */
    &mainwin_gadget[0],		/* FirstGadget */
    NULL,			/* CheckMark */
    RTB,			/* Title */
    RTB,			/* Screen */
    NULL,			/* BitMap */
    0, 0,			/* MinWidth, MinHeight */
    0, 0,			/* MaxWidth, MaxHeight */
    RTB				/* Type */
};

static SHORT path_box_vectors[] = {
    0, 0, 630, 0, 630, 9, 0, 9, 0, 0
};

static struct Border path_box = {
    -5, -1, 1, 0, JAM1, 5, path_box_vectors, NULL
};

/*
 * This can be used to adjust the display of all text and drawn boxes
 * etc. in the window.
 */

static int topoff = 0;

#if 0
static char tbuf[80];
static char b1[24];
static char b2[24];
static char b3[24];
static char b4[24];
static char b5[24];
static char b6[24];
static char b7[24];


static struct IntuiText itext[] = {
    { 3, 0, JAM2, 0, 0, NULL, (UBYTE *) tbuf, NULL },
    { 1, 0, JAM2, 28, 136, NULL, (UBYTE *) b1, &itext[2] },
    { 1, 0, JAM2, 98, 136, NULL, (UBYTE *) b2, &itext[3] },
    { 1, 0, JAM2, 8, 158, NULL, (UBYTE *) b3, &itext[4] },
    { 1, 0, JAM2, 102, 158, NULL, (UBYTE *) b4, &itext[7] },
    { 1, 0, JAM2, 0, 0, NULL, (UBYTE *) b5, NULL },
    { 2, 0, JAM2, 310, 80, NULL, (UBYTE *) "Files Scanned:", NULL },
    { 1, 0, JAM2, 8, 178, NULL, (UBYTE *) b6, &itext[8] },
    { 1, 0, JAM2, 104, 178, NULL, (UBYTE *) b7, NULL }
};

#endif

static struct Process *thisproc;
static APTR old_dos_win;



void open_mainwin( void )
{
    int i;

    
    DBUG_ENTER ("open_mainwin");
    if (screen)
	{
		DBUG_PRINT ("mainwin", ("open main window on a custom screen"));
		new_mainwin.Type = CUSTOMSCREEN;
		new_mainwin.Screen = screen;
		new_mainwin.Flags |= BACKDROP;
		new_mainwin.TopEdge = screen -> BarHeight + 2;
		new_mainwin.Height = (screen -> Height - screen -> BarHeight) - 2;
		topoff = 0;
    }
	else
	{
		DBUG_PRINT ("mainwin", ("open main window on WorkBench screen"));
		new_mainwin.Type = WBENCHSCREEN;
		new_mainwin.Screen = NULL;
		new_mainwin.Flags |= WINDOWDEPTH | WINDOWDRAG;
		new_mainwin.Title = (UBYTE *)TITLESTRING;
		topoff = 9;
    }

    /* Initialize the font pointer in the intuitext structs */
    for (i = 0; i < NUM_GADGETS; i++)
	{
		mainwin_gadget_text[i].ITextFont = textattr;
		mainwin_gadget[i].TopEdge += topoff;
    }

    for (i = 0; i < 4; i++)
	{
		date_text[i].ITextFont = textattr;
		size_text[i].ITextFont = textattr;
    }

    for (i = 0; i < 3; i++)
	{
		arc_text[i].ITextFont = textattr;
		pattern_text[i].ITextFont = textattr;
    }

    for (i = 0; i < 3; i++)
	{
		size_gadget[i].TopEdge += topoff;
		date_gadget[i].TopEdge += topoff;
		pattern_gadget[i].TopEdge += topoff;
    }

    for (i = 0; i < 2; i++)
	{
		arc_gadget[i].TopEdge += topoff;
    }

	init_mainwin_gadgets();

    while ((mainwin = OpenWindow (&new_mainwin)) == NULL)
	{
		querybailout ("Cannot open main window", ERC_NO_WINDOW | ERR12);
    }

    DBUG_PRINT ("mainwin", ("main window now open"));

    mainwin_sig_bit = 1L << mainwin -> UserPort -> mp_SigBit;

    if (screen)
	{
		DBUG_PRINT ("mainwin", ("set up for DOS requestors on our screen"));
		thisproc = (struct Process *) FindTask (NULL);
		thisproc -> pr_WindowPtr = (APTR) mainwin;
    }

#if 1
    /* Set the window up to use our own font (if it was available) */
    if (textfont)
	{
		SetFont (mainwin -> RPort, textfont);
    }
#endif

    SetAPen (mainwin -> RPort, 3L);
    SetDrMd (mainwin -> RPort, JAM1);
    drawstuff ();
	stats();
    SetMenuStrip (mainwin, &project_menu);
    setmode (NO_MODE);

    DBUG_VOID_RETURN;
}

void close_mainwin ()
{
    DBUG_ENTER ("close_mainwin");
    if (screen) {
	thisproc -> pr_WindowPtr = old_dos_win;
    }
    if (mainwin) {
	ClearMenuStrip (mainwin);
	CloseWindow (mainwin);
	mainwin = 0;
    }
    DBUG_VOID_RETURN;
}


static void drawstuff ()
{
    DBUG_ENTER ("drawstuff");
    /* Form text for the file size stuff */
    printat (26, 132 + topoff, "Selected Files");
    printat (76, 142 + topoff, "of");
    printat (33, 154 + topoff, "Selected Size");
    printat (80, 164 + topoff, "of");
    printat (34, 174 + topoff, "Archive Size");
    printat (69, 184 + topoff, "K on     Vols");
    /* Box for the current path string */
    DrawBorder (mainwin -> RPort, &path_box, 10L, (LONG) (3 + topoff));
    DBUG_VOID_RETURN;
}

/*
 * This will print the pathname passed in the upper line of the display
 * window.  This will actually print any string, does not have to be a
 * path.  Clipping is performed as well as padding to erase previous
 * contents of the line.
 */

void disp_path (pathname)
char *pathname;
{
	static char buf[80];
	static struct IntuiText itext = {
    	3, 0, JAM2, 0, 0, NULL, (UBYTE *)buf, NULL };

    	
    DBUG_ENTER ("disp_path");
    if (strlen (pathname) > 70) {
	/* Too long, modify it */
	strcpy (buf, "...");
	strcat (buf, stringright (pathname, 67));
    } else {
	/* The entire path will fit, make sure it is long enough to
	 * blank out the "old" string
	 */
	strcpy( buf, pathname );
	padstring( buf, 70 );
    }
    PrintIText (mainwin -> RPort, &itext, 10L, (LONG) (3 + topoff));
    DBUG_VOID_RETURN;
}

void stats ()
{
	static char b0[24];
	static char b1[24];
	static char b2[24];
	static char b3[24];
	static char b4[24];
	static char b5[24];
	static struct IntuiText itext[] = {
    	{ 1, 0, JAM2, 28, 136, NULL, (UBYTE *) b0, &itext[1] },
	    { 1, 0, JAM2, 98, 136, NULL, (UBYTE *) b1, &itext[2] },
    	{ 1, 0, JAM2, 8, 158, NULL, (UBYTE *) b2, &itext[3] },
	    { 1, 0, JAM2, 102, 158, NULL, (UBYTE *) b3, &itext[4] },
    	{ 1, 0, JAM2, 8, 178, NULL, (UBYTE *) b4, &itext[5] },
	    { 1, 0, JAM2, 104, 178, NULL, (UBYTE *) b5, NULL }
	};
	
    DBUG_ENTER ("stats");
    /* Calculate... */
    file_status (root_node);
    /* ...and display them! */
    sprintf (b0, "%5d", file_selected_count);
    sprintf (b1, "%-5d", file_count);
    sprintf (b2, "%8ld", file_selected_size);
    sprintf (b3, "%-8ld", file_size);

    sprintf (b4, "%7ld", final_archive_size / 1024 );

	if( final_archive_vols == -1 )
	{
		/* Could not be calculated */
		sprintf( b5, " ???" );
	}
	else
	{
		if( final_archive_vols > 999 )
		{
    		sprintf (b5, " %3d", final_archive_vols / 10 );
		}
		else
		{
    		sprintf (b5, "%2d.%1d", final_archive_vols / 10,
				final_archive_vols % 10 );
		}
	}

    PrintIText (mainwin -> RPort, &itext[0], 0L, (LONG) topoff);
    DBUG_VOID_RETURN;
}



void disp_run_count (count, file)
int count;
char *file;
{
	static char buf[24];
	static struct IntuiText itext = {
    	1, 0, JAM2, 0, 0, NULL, (UBYTE *)buf, NULL };


    DBUG_ENTER ("disp_run_count");
    /* Print the file count */
    sprintf (buf, "%-5d", count);
    PrintIText (mainwin -> RPort, &itext, 424L, 80L);
    /* print the file path and name */
    add_path (current_path, file);
    disp_path (current_path);
    subtract_path (current_path);
    DBUG_VOID_RETURN;
}



void disp_processed( char *name )
{
	static char buf[70];
	static struct IntuiText it = {
		1, 0, JAM2, 0, 0, NULL, (UBYTE *)buf, NULL };

    DBUG_PRINT ("processed", ("%5d %s processed", processed_count, name ));

	disp_path( name );

	sprintf( buf, "Completed: %5d of %5d   (%2d%%)",
		processed_count, file_selected_count,
		(int)((processed_count * 100L) / file_selected_count) );

    PrintIText( mainwin->RPort, &it, 250, 80 );
}



static void printat (x, y, text)
int x;
int y;
char *text;
{
    DBUG_ENTER ("printat");
    Move (mainwin -> RPort, (LONG) x, (LONG) y);
    Text (mainwin -> RPort, text, (LONG) strlen (text));
    DBUG_VOID_RETURN;
}
