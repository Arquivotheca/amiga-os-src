/* help.c */


#include "standard.h"
#include "bailout.h"
#include "brushell.h"
#include "dbug.h"
#include "images.h"
#include "help.h"
#include "libfuncs.h"



#define MAX_STRINGS			64
#define STRING_SIZE			81
#define PAGE_STACK_SIZE		16
#define PAGENAME_LEN		32

#define PAGE_TAG_CHAR		'@'

#define AUTO_YSPACE			10

#define NUM_GADGETS		2


static int open_pagewin( void );
static void close_pagewin( void );
static char *read_line( void );
static void reset_line( void );
static int open_helpfile( char *filename );
static void close_helpfile( void );
static void eventloop( void );
static void show_root( void );
static int show_page( char *page );
static void load_page( void );
static int seek_page( char *page );
static void push( char *string );
static char *pop( void );
static void trim_trailing( char *p, char c );
static void draw_page( void );
static void parse_entry( int num, char *p );
static void text_size( int *width, int *height, char *string );
static int check_hit( int x, int y );
static void clear( void );



static WORD left[MAX_STRINGS];
static WORD top[MAX_STRINGS];
static WORD right[MAX_STRINGS];
static WORD bottom[MAX_STRINGS];

static char text[MAX_STRINGS][STRING_SIZE];
static char link[MAX_STRINGS][PAGENAME_LEN];

static char page_stack[PAGE_STACK_SIZE][PAGENAME_LEN];
static int page_stack_ptr = 0;

static int text_count;

static struct Window *pagewin = NULL;

ULONG helpwin_sig_bit = NULL;

static BOOL finished = FALSE;

FILE *fh;

static char current_page[PAGENAME_LEN];


static int header_pen = 1;		/* Black */
static int normal_pen = 2;		/* White */


/* Previous Page Gadget */

/* #define GWI		140 */
/* #define GHI		11 */
#define GLEFT	60
#define GTOP	14

static struct IntuiText prev_text = {
    1, 0, JAM1, 20, 2, NULL, (UBYTE *) "Previous Page", NULL
};

static struct Gadget prev_gadget = {
    NULL,							/* NextGadget */
    -(BOX_6_WIDTH+GLEFT), GTOP,	/* LeftEdge, TopEdge */
    BOX_6_WIDTH, BOX_6_HEIGHT,		/* Width, Height */
    GRELRIGHT |
	GADGIMAGE |
	GADGHIMAGE,				/* Flags */
    RELVERIFY,				/* Activation */
    BOOLGADGET,				/* GadgetType */
    (APTR) BOX_6_IMAGE,		/* GadgetRender */
    (APTR) BOX_6_HIMAGE,	/* SelectRender */
    &prev_text,				/* GadgetText */
    NULL,					/* MutualExclude */
    NULL,					/* SpecialInfo */
    1,						/* GadgetID */
    NULL					/* UserData */
};


static struct IntuiText root_text = {
    1, 0, JAM1, 40, 2, NULL, (UBYTE *) "Main Page", NULL
};

static struct Gadget root_gadget = {
    &prev_gadget,						/* NextGadget */
    -(BOX_6_WIDTH*2+20+GLEFT), GTOP,	/* LeftEdge, TopEdge */
    BOX_6_WIDTH, BOX_6_HEIGHT,			/* Width, Height */
    GRELRIGHT |
	GADGIMAGE |
	GADGHIMAGE,				/* Flags */
    RELVERIFY,				/* Activation */
    BOOLGADGET,				/* GadgetType */
    (APTR) BOX_6_IMAGE,		/* GadgetRender */
    (APTR) BOX_6_HIMAGE,	/* SelectRender */
    &root_text,				/* GadgetText */
    NULL,					/* MutualExclude */
    NULL,					/* SpecialInfo */
    2,						/* GadgetID */
    NULL					/* UserData */
};



static struct NewWindow new_pagewin = {
    0, 0,			/* LeftEdge, TopEdge */
    640, 200,			/* Width, Height */
    0, 1,			/* DetailPen, BlockPen */

	GADGETUP |
	CLOSEWINDOW |
	VANILLAKEY |
	MOUSEBUTTONS,		/* IDCMP Flags */

	WINDOWCLOSE |
	WINDOWDRAG |
    ACTIVATE |
	SMART_REFRESH |
    NOCAREREFRESH,		/* Flags */

    &root_gadget,		/* FirstGadget */
    NULL,			/* CheckMark */
    TITLESTRING "  Help Window",			/* Title */
    NULL,			/* Screen */
    NULL,			/* BitMap */
    0, 0,			/* MinWidth, MinHeight */
    0, 0,			/* MaxWidth, MaxHeight */
    WBENCHSCREEN	/* Type */
};


/* These offsets are added to the x,y position of the strings when
 * loaded to compensate for the window borders, etc.
 */
static int x_offset = 5;
static int y_offset = BOX_6_HEIGHT + GTOP + 4;




/* Bring up the help system.  This will use the given helpfile, searching
 * first for the filename as given.  This should probably not contain
 * a path or device spec.  If it cannot be found as given, "S:" will
 * be prepended and we will try to open it that way.
 */

void help( char *filename )
{
	char buf[128];


	if( open_helpfile( filename ) != 0 )
	{
		/* Not found.  Try in "S:" mabye? */
		strcpy( buf, "S:" );
		strcat( buf, filename );
		if( open_helpfile( buf ) != 0 )
		{
			sprintf( buf, "Cannot find helpfile: \"%s\".", filename );
			mention_error( buf, ERC_NO_FILE | ERR53 );
			return;
		}
	}

	if( open_pagewin() != 0 )
	{
		mention_error( "Cannot open the help window.",
			ERC_NO_WINDOW | ERR54 );
		close_helpfile();
		return;
	}

	show_root();

	finished = FALSE;

	while( finished == FALSE )
	{
		eventloop();
	}

	cleanup_help();
}



void cleanup_help( void )
{
	close_helpfile();

	close_pagewin();
}



void check_helpwin( void )
{
	eventloop();

	if( finished == TRUE )
	{
		cleanup_help();
	}
}


/* Returns 0 for success */

static int open_pagewin( void )
{
    if( screen )
	{
		new_pagewin.Type = CUSTOMSCREEN;
		new_pagewin.Screen = screen;
	}
	else
	{
		/* Opening on the workbench screen  */
		SetFlag( new_pagewin.Flags, WINDOWDEPTH );
	}

	pagewin = OpenWindow( &new_pagewin );
	if( pagewin == NULL )
	{
		return( -1 );
	}

    /* Set the window up to use our own font (if it was available) */
	if( textfont )
	{
		SetFont( pagewin->RPort, textfont );
    }

	prev_text.ITextFont = textattr;
	root_text.ITextFont = textattr;

	helpwin_sig_bit = 1L << pagewin->UserPort->mp_SigBit;

	return( 0 );
}



/* This is a "safe" call which can be made at any time. */

static void close_pagewin( void )
{
	if( pagewin )
	{
		CloseWindow( pagewin );
		pagewin = NULL;
		helpwin_sig_bit = NULL;
	}
}



/* Gets the next line of text from the file.
 * Returns a NULL if there are no more lines.
 */

static char *read_line( void )
{
	static char buf[256];
	char *p;
	BOOL done = FALSE;



	while( done == FALSE )
	{
		p = buf;

		if(  fgets( p, sizeof(buf)-1, fh )  )
		{
			/* Truncate off the newline character if present */
			p = strchr( p, '\n' );
			if( p )
			{
				*p = '\0';
			}
			else
			{
				/* Should have been a newline!  Line may have been too
				 * long.
				 */
			}
		}
		else
		{
			if(  feof( fh )  )
			{
				/* The call to fgets() failed due to us being at the end of the
				 * file.
				 */
				return( NULL );
			}
			else	
			{
				/* Some other error. */
				return( NULL );
			}
		}

		if(  ( buf[0] != ';' ) && ( strlen(buf) > 2 )  )
		{
			done = TRUE;
		}
	}

	return( buf );
}



/* Sets the next line to be read to the start of the helpfile. */

static void reset_line( void )
{
	rewind( fh );
}



static int open_helpfile( char *filename )
{
	fh = fopen( filename, "r" );
	if( fh == NULL )
	{
		return( -1 );
	}

	return( 0 );
}



/* This is a "safe" call which can be made at any time. */

static void close_helpfile( void )
{
	if( fh )
	{
		fclose( fh );
		fh = NULL;
	}
}



static void eventloop( void )
{
	struct IntuiMessage *message, imsg;
	int num;


	(void) Wait( 1L << pagewin -> UserPort -> mp_SigBit );

	while(  message = (struct IntuiMessage *)GetMsg(pagewin->UserPort)  )
	{
		memcpy( (char *)&imsg, (char *)message,
			sizeof(struct IntuiMessage) );

		ReplyMsg( (struct Message *)message );

		/* Do something with message */
		switch( imsg.Class )
		{
			case VANILLAKEY:
				switch( imsg.Code )
				{
					case 27:		/* Escape Key */
						finished = TRUE;
						break;

					case 10:
					case 13:
						show_root();
						break;

					default:
						show_page( NULL );
						break;
				}
				break;


			case CLOSEWINDOW:
				finished = TRUE;
				break;

			case GADGETUP:
				switch(  ((struct Gadget *)(imsg.IAddress))->GadgetID )
				{
					case 1:
						/* Previous page gadget */
						show_page( NULL );
						break;

					case 2:
						/* Root page gadget */
						show_root();
						break;
				}
				break;

		    case MOUSEBUTTONS: 
				if( imsg.Code == SELECTDOWN )
				{
					num = check_hit( imsg.MouseX, imsg.MouseY );
					if( num != -1 )
					{
						if( link[num][0] != '\0' )
						{
							show_page( &link[num][0] );
						}
					}
				}
				break;
	    }
	}
}



static void show_root( void )
{
	page_stack_ptr = 0;
	strcpy( current_page, "" );

	show_page( "root" );
}



/* 
 * If the page name pointer is NULL, show the previous page.
 */

static int show_page( char *page )
{
	BOOL pushflag = TRUE;


	if( page == NULL )
	{
		/* Previous page requested */
		page = pop();
		if( page == NULL )
		{
			/* No previous page, ignore the request */
			return( 0 );
		}

		pushflag = FALSE;		/* Don't push where we are since we
								 * are going UP the page order.
								 */
	}


	/* find new page */
	if(  seek_page( page ) != 0  )
	{
		/* Invalid page */
		mention_error( "Help file corrupt -- page missing.",
			ERC_NONE | ERR55 );
		return( -1 );
	}

	if(  ( pushflag == TRUE ) && ( current_page[0] != '\0' )  )
	{
		push( current_page );
	}

	strcpy( current_page, page );

	/* read it in */
	load_page();


	/* display it */
	draw_page();

	return( 0 );
}



static void draw_page( void )
{
	int i, hi, wide;
	char *string;


	clear();		/* Clear the window */

	RefreshGList( &root_gadget, pagewin, NULL, NUM_GADGETS );

	for( i=0; i<text_count; i++ )
	{
		string = &text[i][0];

		text_size( &wide, &hi, string );

		if( link[i][0] == '\0' )
		{
			SetAPen( pagewin->RPort, header_pen );
		}
		else
		{
			SetAPen( pagewin->RPort, normal_pen );
		}

	    Move( pagewin->RPort, left[i], top[i]+hi );

	    Text( pagewin->RPort, string, strlen(string) );
	}
}



static void load_page( void )
{
	int i, hi, wide;
	char *p;


	text_count = 0;

	for( i=0; i<MAX_STRINGS; i++ )
	{
		p = read_line();
		if(  ( p == NULL ) || ( *p == PAGE_TAG_CHAR )  )
		{
			/* End of file or not an entry string */
			return;
		}


		/* Parse the entry line */
		parse_entry( i, p );

		text_size( &wide, &hi, &text[i][0] );

		bottom[i] = top[i] + hi;
		right[i] = left[i] + wide;		

		text_count++;
	}
}



static void parse_entry( int num, char *p )
{
	char buf[128];
	int bump;


	text[num][0] = '\0';
	link[num][0] = '\0';

	if( *p == '[' )
	{
		while(  ! isdigit( *p )  )		/* find a digit */
			p++;

		left[num] = atoi( p ) + x_offset;

		while(  isdigit( *p )  )		/* move past digits */
			p++;

		while(  ! isdigit( *p )  )		/* find a digit */
			p++;

		top[num] = atoi( p ) + y_offset;

		while(  isdigit( *p )  )		/* move past digits */
			p++;

		bump = strspn( p, "] \t" );		/* Find start of body text */
		p += bump;
	}
	else
	{
		left[num] = (num>0) ? left[num-1] : x_offset;
		top[num] = (num>0) ? (top[num-1]+AUTO_YSPACE) : y_offset;
	}

	/* Grab the body text */
	p = stptok( p, buf, sizeof(buf), "[" );
	strcpy( &text[num][0], buf );


	/* See if we have a link field */
	p = strchr( p, '[' );
	if( p )
	{
		/* There is a link field */
		p++;
		p = stptok( p, buf, sizeof(buf), "]" );
		strcpy( &link[num][0], buf );
	}
}



/*
 * Return is zero for success.
 */

static int seek_page( char *page )
{
	char *p;


	reset_line();

	while(  ( p = read_line() ) != NULL  )
	{
		/* remove leading and trailing spaces */
		trim_leading( p, ' ' );
		trim_trailing( p, ' ' );

		if( *p == PAGE_TAG_CHAR )
		{
			p++;

			if(  stricmp( p, page ) == 0  )
			{
				/* Found it, we just return */
				return( 0 );
			}
		}
	}

	/* If we fall out, the page was not found. */
	return( -1 );
}


static int check_hit( int x, int y )
{
	int i;


	for( i=0; i<text_count; i++ )
	{
		if(  ( y >= top[i] ) && ( y <= bottom[i] )  )
		{
			if(  ( x >= left[i] ) && ( x <= right[i] )  )
			{
				/* A hit */
				return( i );
			}
		}
	}

	/* A miss */
	return( -1 );
}


static void trim_trailing( char *p, char c )
{
	while( *p++ != '\0' )
		;

	p--;

	while( *--p == c )
		;

	*(++p) = '\0';
}



static void text_size( int *width, int *height, char *string )
{
	*width = TextLength( pagewin->RPort, string, strlen(string) );
	*height = pagewin->RPort->TxHeight;
}



static void clear( void )
{
	struct Image image;


	/* Initialize background Image structure */

	image.LeftEdge = pagewin->BorderLeft;
/*	image.TopEdge = pagewin->BorderTop; */
	image.TopEdge = pagewin->BorderTop;
	image.Width = pagewin->Width -
		(pagewin->BorderRight + pagewin->BorderLeft);
	image.Height = pagewin->Height -
		(pagewin->BorderTop + pagewin->BorderBottom);
	image.Depth = 0;
	image.ImageData = NULL;
	image.PlanePick = 0;
	image.PlaneOnOff = 0;
	image.NextImage = NULL;

	DrawImage( pagewin->RPort, &image, 0L, 0L );
}



/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::   Stack functions   :::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


static void push( char *string )
{
	if( page_stack_ptr >= PAGE_STACK_SIZE )
	{
		/* Stack overflow */
		return;
	}

	strcpy( &page_stack[ page_stack_ptr++ ][0], string );
}



static char *pop( void )
{
	if( page_stack_ptr == 0 )
	{
		/* Underflow */
		return( NULL );
	}

	return(  &page_stack[ --page_stack_ptr ][0]  );
}



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


#if 0	/*-----------------------------------------------------------*/
/* Gets the next line of text from the file.
 * Returns a NULL if there are no more lines.
 */

static char *read_line( void )
{
#error Function not complete
	return( NULL );
}



/* Sets the next line to be read to the start of the helpfile. */

static void reset_line( void )
{
#error Function not complete
	Seek( 0, fromstart );
}



static int open_helpfile( char *filename )
{
	fh = Open( filename, MODE_READONLY );
	if( fh == NULL )
	{
		return( -1 );
	}

	return( 0 );
}



static void close_helpfile( void )
{
	if( fh )
	{
		Close( fh );
		fh = NULL;
	}
}
#endif	/*--------------------------------------------------------------*/




/*			Page Syntax

The root page is named "root" (pretty clever, eh?).

Lines that do not begin with a special tag are considered to be general
text and will be autospaced down from the last position.
Blank lines are ignored.



	Tags:

";"		A comment tag.  This line is ignored.
[		A text string with position.
@		Page tag.



	Example file:

@root
[0,0] This is the header for the root page
[0,20] Topic #1 [page2]

@page2
[0,0] Header for the second page
Text on second page
more text
and even more

*/

