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

#include <workbench/startup.h>

#include "bailout.h"
#include "brushell.h"
#include "dirtree.h"
#include "logfile.h"
#include "mainwin.h"
#include "slist.h"
#include "menus.h"
#include "dbug.h"
#include "rexxcom.h"
#include "eventloop.h"
#include "libfuncs.h"
#include "hdbackup_rev.h"



#define EIGHTCOLORS		0		/* Currently limited to 4 colors */


extern struct WBStartup *WBenchMsg;

/*
 *	Prototypes for local functions, not exported.
 */

static void do_font_stuff( void );
static void do_config( int, char ** );
static void do_do_config( char ** );
static char *nextvalue( char ** );
static void openlibrarys( void );
static void closelibrarys( void );
static void init_reqs( void );


struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct IconBase *IconBase = NULL;
struct Library *DiskfontBase = NULL;
struct Library *AslBase = NULL;
struct Library *UtilityBase = NULL;

static struct TextAttr special_ta = {
    (STRPTR) "topaz.font",
    8,
    0,
    0
};

static struct TextAttr default_ta = {
    (STRPTR) "topaz.font",
    8,
    0,
    0
};

struct TextFont *textfont = NULL;
struct TextAttr *textattr = &default_ta;
struct TextAttr *slist_textattr = &default_ta;

int sel_file_color = 2;				/* White */
int desel_file_color = 1;			/* Black */
int unbacked_file_color = 3;		/* Blue */


static USHORT colortable[8] = {
    0xAAA,
    0x000,
    0xFFF,
    0x68B,

	/* These are unused at this time.. */
    0xAAA,	/* Light Gray */
    0xDA8,	/* Light Tan */
    0xB75,	/* Tan */
    0xCCC	/* Very light Gray */
};

static USHORT lace_colortable[8] = {
    0x555,
    0x000,
    0x999,
    0x57A,

	/* These are unused at this time.. */
    0xAAA,	/* Light Gray */
    0xDA8,	/* Light Tan */
    0xB75,	/* Tan */
    0xCCC	/* Very light Gray */
};


static UWORD pens[] =
{
	0,	/* DETAILPEN */
	1,	/* BLOCKPEN */
	1,	/* TEXTPEN */
	2,	/* SHINEPEN */
	1,	/* SHADOWPEN */
	3,	/* FILLPEN */
	1,	/* FILLTEXTPEN */
	0,	/* BACKGROUNDPEN */
	2,	/* HIGHLIGHTTEXTPEN */
	~0,	/* terminator */
};

static struct TagItem taglist[] = {
	SA_Pens, &pens[0],
	TAG_DONE, 0,
};

struct Screen *screen = NULL;

static struct ExtNewScreen newscreen = {
    0, 0,			/* LeftEdge, TopEdge */
    640, STDSCREENHEIGHT,	/* Width, Height */
    2,				/* Depth */
    2, 1,			/* DetailPen, BlockPen */
    HIRES,			/* ViewModes */
    NS_EXTENDED |
    CUSTOMSCREEN,	/* Type */
    RTB,			/* Font */
    RTB,			/* DefaultTitle */
    NULL,			/* Gadgets */
    NULL,			/* CustomBitMap */
    taglist			/* Extension */
};

static BOOL screen_flag = TRUE;	/* default to custom screen */
BOOL auto_backup = FALSE;
BOOL auto_restore = FALSE;
BOOL auto_start = FALSE;

BOOL makeicon_flag = TRUE;
BOOL show_test_menu = FALSE;
BOOL embed_logfile = TRUE;

BOOL backup_the_dirs = TRUE;

UBYTE *Revision = VERSTAG;


/* Allow user to specify the path to find BRU here */

char bru_path[64];				/* Directory in which to look for bru */
char bru_name[32] = BRU_NAME;	/* Name of the bru executable */
char bru_args[128];				/* Special args to pass to bru */
long bru_stack;					/* Stack for bru */

char ipcport_name[64] = IPCPORT_NAME;

static BPTR oldlock;


/* New Lattice way of ensuring a minimum stack size. */
long stack = MINIMUM_STACK;



void main( int argc, char *argv[] )
{
    ERRORCODE rc;
	BOOL rv;
	struct Screen screenbuf;


    DBUG_ENTER ("main");
    /* DBUG_PUSH ("d;t;F;L;D,5;o,con:0/0/640/100/BRUshellDBUG"); */
    /* DBUG_PUSH ("d;t;F;L;o,vd0:bru.log"); */
    openlibrarys ();
    if (WBenchMsg) {
	DBUG_PRINT ("config", ("launched from WorkBench"));
	DBUG_PRINT ("config", ("change current directory"));
	oldlock = CurrentDir (WBenchMsg -> sm_ArgList[0].wa_Lock);
    }

    do_config (argc, argv);

    do_font_stuff ();

	if(  FindPort( ipcport_name ) != NULL  )
	{
		bailout( "HDBackup is already running. Cannot run twice!",
			ERC_NONE | ERR50 );
	}

	if( stacksize() < MINIMUM_STACK )
	{
		bailout( "Stack too small, must be at least " \
MINIMUM_STACK_SC " bytes.",
			ERC_NONE | ERR51 );
	}

    if (rc = init_ipc ())
	{
		bailout ("Cannot do the IPC port initialization!", rc);
    }


	/* See if the font used on the workbench screen is compatible
	 * with our gadget sizes etc.
	 */
	if( screen_flag == FALSE )
	{
		/* User wants to use the WorkBench screen.  Can we? */

		rv = GetScreenData( (UBYTE *)(&screenbuf), sizeof(struct Screen),
			WBENCHSCREEN, NULL );
		/* printf( "Barheight is %d\n", (int)screenbuf.BarHeight ); */

		/* Allow a barheight of 11.  This let's us open on the WorkBench
		 * with Topaz-8 OR Topaz-9.
		 */
		if(  ( rv == FALSE ) || ( screenbuf.BarHeight > 11 )  )
		{
			/* No, so we force custom screen */
			tell_user( "Using private screen, due to WorkBench font size." );
			screen_flag = TRUE;
		}
	}

    if (screen_flag == TRUE)
	{
		DBUG_PRINT ("screen", ("try to open a custom screen"));
		newscreen.Font = textattr;

		while ((screen = OpenScreen (&newscreen)) == NULL)
		{
	    	querybailout ("Unable to open screen.  Free up some Memory?",
		    	ERC_NO_SCREEN | ERR14);
		}

		DBUG_PRINT ("screen", ("custom screen open, now set it's colors"));

		if (FlagIsSet (newscreen.ViewModes, INTERLACE))
		{
	    	DBUG_PRINT ("config", ("interlace colors being used"));

		    LoadRGB4 (&(screen -> ViewPort), lace_colortable, 8L);
		}
		else
		{
		    DBUG_PRINT ("config", ("normal colors being used"));
		    LoadRGB4 (&(screen -> ViewPort), colortable, 8L);
		}

#if EIGHTCOLORS
		if (newscreen.Depth == 3)
		{
		    desel_file_color = 4;
		}
#endif
	}

	init_reqs();

	init_menus ();

    open_mainwin ();

    if (rc = init_slist ())
	{
		bailout ("Unable to create list support.", rc);
    }

    eventloop ();
    cleanup ();
    exit (0);

    DBUG_VOID_RETURN;
}

static void openlibrarys ()
{
    DBUG_ENTER ("openlibrarys");

    /*   Intuition   */
    IntuitionBase = (struct IntuitionBase *)
			OpenLibrary ("intuition.library", LIB_VER);
    if (!IntuitionBase) {
	bailout ("Unable to open the Intuition Library",
		 ERC_NO_LIBRARY | ERR25);
    }

    /*   Graphics   */
    GfxBase = (struct GfxBase *)
		OpenLibrary ("graphics.library", LIB_VER);
    if (!GfxBase) {
	bailout ("Unable to open the Graphics Library",
		ERC_NO_LIBRARY | ERR26);
    }

    /*   Icon   */
    IconBase = (struct IconBase *)
		OpenLibrary (ICONNAME, LIB_VER);
    if (!IconBase) {
	bailout ("Unable to open the Icon Library", ERC_NO_LIBRARY | ERR27);
    }

    /*   Diskfont   */
    DiskfontBase = (struct Library *)
		OpenLibrary ("diskfont.library", LIB_VER);
    if (!DiskfontBase) {
	bailout ("Unable to open the Disk Font Library",
		ERC_NO_LIBRARY | ERR28);
    }

    /*   Asl   */
    AslBase = (struct Library *)
		OpenLibrary ("asl.library", LIB_VER );
    if (!AslBase) {
	bailout ("Unable to open the Asl Library",
		ERC_NO_LIBRARY | ERR67);
    }

    /*   Utility   */
    UtilityBase = (struct Library *)
		OpenLibrary ("utility.library", LIB_VER );
    if (!UtilityBase) {
	bailout ("Unable to open the Utility Library",
		ERC_NO_LIBRARY | ERR69);
    }

    DBUG_VOID_RETURN;
}

static void closelibrarys ()
{
    DBUG_ENTER ("closelibrarys");

    if (IntuitionBase) {
	CloseLibrary ((struct Library *)IntuitionBase);
    }

    if (GfxBase) {
	CloseLibrary ((struct Library *)GfxBase);
    }

    if (IconBase) {
	CloseLibrary ((struct Library *)IconBase);
    }

    if (DiskfontBase) {
	CloseLibrary (DiskfontBase);
    }

    if (AslBase) {
	CloseLibrary (AslBase);
    }

    if (UtilityBase) {
	CloseLibrary (UtilityBase);
    }

    DBUG_VOID_RETURN;
}

void cleanup ()
{
    DBUG_ENTER ("cleanup");
    cleanup_ipc ();		/* a safe call */
    cleanup_slist ();		/* a safe call */
    if (root_node) {
	free_tree (root_node);
    }
    if (textfont) {
	CloseFont (textfont);
    }
    close_mainwin ();		/* a safe call */
    if (screen) {
	CloseScreen (screen);
    }
    /* Restore original directory (if launched from WorkBench) */
    if (WBenchMsg) {
	(void) CurrentDir (oldlock);
    }
    closelibrarys ();		/* a safe call */
    DBUG_VOID_RETURN;
}

static void do_font_stuff ()
{
    DBUG_ENTER ("do_font_stuff");


    /* See if the desired font can be had */
    textfont = GetFont( &special_ta );

    if( textfont != NULL )
	{
		/* Yep, it is now in RAM */

/*		printf( "Got special font\n" ); */

		/* Is the width acceptable? */
		if (textfont -> tf_XSize == 8)
		{
			/* Is the height acceptable? */
			if(textfont -> tf_YSize == 8)
			{
				/* Use it for all text, since it is 8x8 */
				textattr = &special_ta;
/*				printf( "special font %s\n", special_ta.ta_Name ); */
				DBUG_PRINT ("font", ("special font %s", special_ta.ta_Name));
	    	}
			else
			{
				/* Stick to default font for general text */
			    CloseFont (textfont);
			    textfont = GetFont (&default_ta);
/*		    	printf( "font %s height bad\n", special_ta.ta_Name ); */
			}

		    /* Use it for list window, even if the height is odd */
		    slist_textattr = &special_ta;
		}
		else
		{
	    	/* Invalid width, do not use anyplace */
		    CloseFont (textfont);

		    /* textfont = NULL; */
		    textfont = GetFont (&default_ta);

/*	    	printf( "font %s width bad\n", special_ta.ta_Name ); */
	    	DBUG_PRINT ("font", ("font %s width bad", special_ta.ta_Name));
		}
    }
	else
	{
		/* Couldn't open special font, use the default. */
	    textfont = GetFont (&default_ta);
	}

    DBUG_VOID_RETURN;
}



/*
 *	Look for reconfiguration parameters.  The lowest priority
 *	is given to parameters from the general configuration file,
 *	generally in S:BRUshell.config, so we get them first.  The
 *	next higher priority is given to parameters given on the
 *	command line for CLI startup, or in the .info file for
 *	WorkBench startup.  These two are naturally mutually exclusive
 *	since programs started from WorkBench have no command line
 *	parameters and programs started from the CLI do not generally
 *	go looking for a corresponding .info file, so neither do we.
 *
 *	Note that we look in the lowest priority location (the config
 *	file) first, since any parameters found later from a higher
 *	priority source will override the earlier parameters.  Actually,
 *	the very lowest priority parameters are the compiled in values,
 *	which get overridden by the config file parameters, so this
 *	is logical.
 *
 */

static void do_config (argc, argv)
int argc;
char *argv[];
{
    struct DiskObject *disko;
    auto char *opts[CONFIG_OPTS];
    int i = 0;
    char buf[SBUF_SIZE];
    FILE *fh;


    DBUG_ENTER ("do_config");

	/*-----------------------*/
	/* Look for a file in S: */
	/*-----------------------*/

    DBUG_PRINT ("config", ("looking for config file '%s'", BRU_CONFIG));

    if ((fh = fopen (BRU_CONFIG, "r")) != NULL)
	{
		DBUG_PRINT ("config", ("found config file"));
		while ((fgets (buf, SBUF_SIZE, fh) != NULL) && (i < CONFIG_OPTS - 1))
		{
	    	/* ignore comment lines or lines that can't be specs */
		    if (buf[0] != '#' && strchr (buf, '='))
			{
				/* obliterate the newline char */
				buf[strlen (buf) - 1] = '\0';
				/* allocate a place for the string */
				opts[i] = (char *) (RemAlloc ((ULONG) strlen (buf) + 1,
					MEMF_PUBLIC));
				if (opts[i] != NULL)
				{
				    /* copy the string there */
				    strcpy (opts[i++], buf);
				}
	    	}
		}

		fclose (fh);

		if (i > 0)
		{
		    opts[i] = "";
	    	do_do_config (opts);
		    DBUG_PRINT ("config", ("free up %d temp strings", i));
		    while (--i >= 0)
			{
				RemFree (opts[i]);
	    	}
		}
    }


	/*-------------------------------*/
	/* Look for command line options */
	/*				OR				 */
	/* read the .info file			 */
	/*-------------------------------*/

    if (argc != 0)
	{
		DBUG_PRINT ("config", ("looking for command line opts"));

		for (i = 0; (i < argc) && (i < CONFIG_OPTS - 1); i++)
		{
		    opts[i] = argv[i];
		}

		opts[i] = "";		/* the terminating null */
		do_do_config (&opts[0]);
    }
	else
	{
		DBUG_PRINT ("config", ("looking for .info file"));

		if ((disko = ReadInfoFile (argv[0])) != NULL)
		{
		    DBUG_PRINT ("config", ("got the .info file"));

		    if (disko -> do_ToolTypes != NULL)
			{
				DBUG_PRINT ("config", ("has some tooltypes"));
				do_do_config (disko -> do_ToolTypes);
		    }

		    FreeDiskObject (disko);
		}
    }

    DBUG_PRINT ("config", ("font is %s %d", special_ta.ta_Name,
		special_ta.ta_YSize));

    DBUG_VOID_RETURN;
}



/*
 *	Note that when we read the color table in, we decode it into
 *	a temporary array.  This accomplishes two things.  First it
 *	works around a bug in Lattice C 5.02 where sscanf would not
 *	decode into a short, even with the "%hx" spec.  Second, it
 *	allows us to ignore all the values if the decode does not
 *	succeed.
 */

static void do_do_config (toolarray)
char **toolarray;
{
    char *value;
    char *p;
    char *pp;
    static char font_name_buf[40];
    int n;
    int i;
    auto int colors[8];

    DBUG_ENTER ("do_do_config");
    DBUG_PRINT ("config", ("look for DBUG spec"));
    if (value = MyFindToolType (toolarray, "DBUG")) {
	DBUG_PUSH (value);
    }
    /* Screen parameters */
    DBUG_PRINT ("config", ("look for SCREEN spec"));
    if (value = MyFindToolType (toolarray, "SCREEN")) {
	if (MyMatchToolValue (value, "custom") == TRUE) {
	    screen_flag = TRUE;
	} else if (MyMatchToolValue (value, "workbench") == TRUE) {
	    screen_flag = FALSE;
	}
    }

#if EIGHTCOLORS
    DBUG_PRINT ("config", ("look for NUMCOLORS spec"));
    if (value = MyFindToolType (toolarray, "NUMCOLORS")) {
	if (MyMatchToolValue (value, "4") == TRUE) {
	    newscreen.Depth = 2;
	} else {
	if (MyMatchToolValue (value, "8") == TRUE) {
	    newscreen.Depth = 3;
	}
    }
	}
#endif

    DBUG_PRINT ("config", ("look for LACE spec"));
    if (value = MyFindToolType (toolarray, "LACE")) {
	if (MyMatchToolValue (value, "on") == TRUE) {
	    SetFlag (newscreen.ViewModes, INTERLACE);
	} else 	if (MyMatchToolValue (value, "off") == TRUE) {
	    ClearFlag (newscreen.ViewModes, INTERLACE);
	}
    }

    DBUG_PRINT ("config", ("look for COLORS spec"));
    if (value = MyFindToolType (toolarray, "COLORS")) {
	/* User has own ideas about color scheme */
#if EIGHTCOLORS
	n = sscanf (value, "%x,%x,%x,%x,%x,%x,%x,%x",
		&colors[0], &colors[1], &colors[2], &colors[3],
		&colors[4], &colors[5], &colors[6], &colors[7]);
	if (n == 8) {
	    for (n = 0; n < 8; n++) {
		DBUG_PRINT ("colors", ("color %d = %x",	n, colors[n]));
		colortable[n] = colors[n];
	    }
	}
#else
	n = sscanf (value, "%x,%x,%x,%x",
		&colors[0], &colors[1], &colors[2], &colors[3]);
	if (n == 4) {
	    for (n = 0; n < 4; n++) {
		DBUG_PRINT ("colors", ("color %d = %x",	n, colors[n]));
		colortable[n] = colors[n];
	    }
	}
#endif
    }

    DBUG_PRINT ("config", ("look for LACECOLORS spec"));
    if (value = MyFindToolType (toolarray, "LACECOLORS")) {
	/* User has own ideas about interlace color scheme */
#if EIGHTCOLORS
	n = sscanf (value, "%x,%x,%x,%x,%x,%x,%x,%x",
		&colors[0], &colors[1], &colors[2], &colors[3],
		&colors[4], &colors[5], &colors[6], &colors[7]);
	if (n == 8) {
	    for (n = 0; n < 8; n++) {
		DBUG_PRINT ("colors", ("color %d = %x",	n, colors[n]));
		lace_colortable[n] = colors[n];
	    }
	}
#else
 	n = sscanf (value, "%x,%x,%x,%x",
		&colors[0], &colors[1], &colors[2], &colors[3]);
	if (n == 4) {
	    for (n = 0; n < 4; n++) {
		DBUG_PRINT ("colors", ("color %d = %x",	n, colors[n]));
		lace_colortable[n] = colors[n];
	    }
	}
#endif
    }

    DBUG_PRINT ("config", ("look for FONTNAME spec"));
    if (value = MyFindToolType (toolarray, "FONTNAME")) {
	/* User has a font he/she wants to use */
	if (strlen (value) <= 32) {
	    /* Looks okay length-wise */
	    strcpy (font_name_buf, value);
	    strcat (font_name_buf, ".font");
	    special_ta.ta_Name = (UBYTE *) font_name_buf;
	}
    }
    DBUG_PRINT ("config", ("look for FONTSIZE spec"));
    if (value = MyFindToolType (toolarray, "FONTSIZE")) {
	n = atoi (value);
	if ((n > 5) && (n < 12)) {
	    /* seems to be in a reasonable range */
	    special_ta.ta_YSize = n;
	}
    }
    DBUG_PRINT ("config", ("look for BACKUP spec"));
    if (value = MyFindToolType (toolarray, "BACKUP")) {
	auto_backup = TRUE;
	if (strlen (value) > 0) {
	    strlcpy (root_name, value, MAX_CUR_PATH);
	}
    }
    DBUG_PRINT ("config", ("look for RESTORE spec"));
    if (value = MyFindToolType (toolarray, "RESTORE")) {
	auto_restore = TRUE;
	if (strlen (value) > 0) {
	    strlcpy (root_name, value, MAX_CUR_PATH);
	}
    }
    DBUG_PRINT ("config", ("look for START spec"));
    if (value = MyFindToolType (toolarray, "START")) {
	/* If "START=on" then set the flag true */
	if (MyMatchToolValue (value, "on") == TRUE) {
	    auto_start = TRUE;
	} else 	if (MyMatchToolValue (value, "off") == TRUE) {
	    auto_start = FALSE;
	}
    }
    DBUG_PRINT ("config", ("look for DEVS spec"));
    if (value = MyFindToolType (toolarray, "DEVS")) {
	/* parse out the device names */
	i = 0;
	while ((p = nextvalue (&value)) != NULL) {
	    if (strlen (p) < USERDEV_MAXWIDTH) {
		strcpy (&userdev[i++][0], p);
	    }
	    if (i > 3) {
		/* That was the last device name we
		 * can fit into the array.
		 */
		break;
	    }
	}
    }
    DBUG_PRINT ("config", ("look for USE spec"));
    if (value = MyFindToolType (toolarray, "USE")) {
	/* Clear the default DF0: checkmark */
	ClearFlag (devs_item[0].Flags, CHECKED);
	/* parse out the device names */
	while ((p = nextvalue (&value)) != NULL) {
	    /* got a device name from the option */
	    for (i = 0; i < NUM_DEVS_ITEMS; i++) {
		/* look at devices to see if it matches one */
		pp = (char *) ((((struct IntuiText *)
				(devs_item[i].ItemFill)) -> IText));
		if (strequal (p, pp)) {
		    /* A match, set it's checkmark */
		    SetFlag (devs_item[i].Flags, CHECKED);
		}
	    }
	}
    }
    DBUG_PRINT ("config", ("look for BRUPATH spec"));
    if (value = MyFindToolType (toolarray, "BRUPATH")) {
	if (strlen (value) > 0) {
	    strlcpy (bru_path, value, sizeof (bru_path));
	}
    }
    DBUG_PRINT ("config", ("look for BRU spec"));
    if (value = MyFindToolType (toolarray, "BRU")) {
	if (strlen (value) > 0) {
	    strlcpy (bru_name, value, sizeof (bru_name));
	}
    }
    DBUG_PRINT ("config", ("look for BRUARGS spec"));
    if (value = MyFindToolType (toolarray, "BRUARGS")) {
	if (strlen (value) > 0) {
	    strlcpy (bru_args, value, sizeof (bru_args));
	}
    }
    DBUG_PRINT ("config", ("look for BRUSTACK spec"));
    if (value = MyFindToolType (toolarray, "BRUSTACK")) {
	bru_stack = atol (value);
	DBUG_PRINT ("stack", ("got stack value %ld", bru_stack));
	if (bru_stack < 0) {
	    bru_stack = 0;
	}
    }
    DBUG_PRINT ("config", ("look for ICONS spec"));
    if (value = MyFindToolType (toolarray, "ICONS")) {
	/* If "ICONS=on" then set the flag true */
	if (MyMatchToolValue (value, "on") == TRUE) {
	    makeicon_flag = TRUE;
	} else if (MyMatchToolValue (value, "off") == TRUE) {
	    makeicon_flag = FALSE;
	}
    }

    DBUG_PRINT ("config", ("look for TESTMENU spec"));
    if (value = MyFindToolType (toolarray, "TESTMENU")) {
	if (MyMatchToolValue (value, "on") == TRUE) {
	    show_test_menu = TRUE;
	} else if (MyMatchToolValue (value, "off") == TRUE) {
	    show_test_menu = FALSE;
	}
    }

	if( show_test_menu == TRUE )
	{
		/* Enable recognizing these options */

	    DBUG_PRINT ("config", ("look for IPCPORT spec"));
    	if (value = MyFindToolType (toolarray, "IPCPORT")) {
		if (strlen (value) > 0) {
		    strlcpy (ipcport_name, value, sizeof (ipcport_name));
		}
    	}

	    DBUG_PRINT ("config", ("look for TEST01 flag"));
    	if (value = MyFindToolType (toolarray, "TEST01"))
		{
			if (MyMatchToolValue (value, "on") == TRUE)
			{
				SetFlag( project_item[INSPECT_ITEM].Flags, ITEMENABLED );
				SetFlag( project_item[DIFFERENCE_ITEM].Flags, ITEMENABLED );
			}
    	}
	}


    DBUG_PRINT ("config", ("look for EMBEDLOG spec"));
    if (value = MyFindToolType (toolarray, "EMBEDLOG")) {
	if (MyMatchToolValue (value, "on") == TRUE) {
	    embed_logfile = TRUE;
	} else if (MyMatchToolValue (value, "off") == TRUE) {
	    embed_logfile = FALSE;
	}
    }

    DBUG_PRINT ("config", ("look for LOGDIR spec"));
    if (value = MyFindToolType (toolarray, "LOGDIR")) {
	if (strlen (value) > 0) {
	    strlcpy (bru_logdir, value, sizeof (bru_logdir));
	} else {
		bru_logdir[0] = '\0';
	}
    }

    DBUG_PRINT ("config", ("look for LOGFILE spec"));
    if (value = MyFindToolType (toolarray, "LOGFILE")) {
	if (strlen (value) > 0) {
	    strlcpy (logfile_pattern, value, sizeof (logfile_pattern));
	}
    }

    DBUG_PRINT ("config", ("look for DIRICON spec"));
    if (value = MyFindToolType (toolarray, "DIRICON")) {
	if (strlen (value) > 0) {
	    strlcpy (logdir_icon_template, value,
		sizeof (logdir_icon_template));
	}
    }

    DBUG_PRINT ("config", ("look for FILEICON spec"));
    if (value = MyFindToolType (toolarray, "FILEICON")) {
	if (strlen (value) > 0) {
	    strlcpy (logfile_icon_template, value,
		sizeof (logfile_icon_template));
	}
    }

    DBUG_VOID_RETURN;
}



static char *nextvalue (val)
char **val;
{
    static char buf[40];
    int i = 0;
    char c;

    if ((**val == '\0') || (**val == ' ')) {
	return (NULL);
    }
    for (;;) {
	c = **val;
	(*val)++;
	switch (c) {
	    case '\0': 
	    case ' ': 
		(*val)--;	/* "push back" the terminator */
		/* fall through */
	    case '|': 
		buf[i] = '\0';
		return (&buf[0]);
	    default: 
		if (i < 39) {
		    /* add character to buffer if there is room */
		    buf[i++] = c;
		}
	}
    }
}



static struct IntuiText okay_text = {
	2, 1, JAM1, 5, 3, NULL, (UBYTE *) "OK", NULL
};



/* The "About" Requester */

static struct IntuiText abouttext[] = {
    { 2, 1, JAM2, 30, 5, NULL, (UBYTE *)VERS, &abouttext[1] },
    { 2, 1, JAM2, 30, 17, NULL,
	(UBYTE *) "Copyright \251 1989,1990 Enhanced Software Technologies, Inc.",
	&abouttext[2] },
    { 2, 1, JAM2, 30, 29, NULL,
	(UBYTE *) "All Rights Reserved", &abouttext[3] },
    { 2, 1, JAM2, 30, 45, NULL,
	(UBYTE *) "",
	&abouttext[4] },
    { 2, 1, JAM2, 30, 60, NULL,
	(UBYTE *) "HDBackup was written by Don Meyer, Stormgate Software",
		&abouttext[5] },
    { 2, 1, JAM2, 30, 70, NULL,
	(UBYTE *) "", &abouttext[6] },
    { 2, 1, JAM2, 30, 80, NULL,
	(UBYTE *) "BRU was written by Fred Fish, Enhanced Software \
Technologies, Inc.",
		&abouttext[7] },
    { 2, 1, JAM2, 30, 90, NULL,
	(UBYTE *) "", &abouttext[8] },
    { 2, 1, JAM2, 30, 100, NULL,
	(UBYTE *) "", &abouttext[9] },
    { 2, 1, JAM2, 30, 115, NULL,
	(UBYTE *) "", &abouttext[10] },
    { 2, 1, JAM2, 30, 125, NULL,
	(UBYTE *) "", NULL }
};

void tell_about ()
{
    DBUG_ENTER ("tell_about");

    AutoRequest (mainwin, &abouttext[0], NULL, &okay_text,
	    NULL, NULL, 500L, 170L);
    DBUG_VOID_RETURN;
}

static void init_reqs( void )
{
	struct IntuiText *it;


	okay_text.ITextFont = textattr;

	/* Set the text in the ABOUT requester to use our font */
	it = &abouttext[0];
	while( it != NULL )
	{
		it->ITextFont = textattr;
		it = it->NextText;
	}
}
