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
#include "menus.h"
#include "dbug.h"
#include "getdisks.h"
#include "brushell.h"

void NewList( struct List *list );
#define MIHI 10			/* Height of normal textual menu items */


static void set_menu_font( void );



/*-----------------------   Debug and Testing Menu   ---------------*/

/*-------------  Test Menu ----------------------*/

struct IntuiText test_item_text[NUM_TEST_ITEMS] = {
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "Write Logfile", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "Set Logfile Pattern", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "Internal About", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "", NULL },
    { 0, 1, JAM1, 1, 1, NULL, (UBYTE *) "", NULL }
};

#undef MIWIDTH
#define MIWIDTH 170

struct MenuItem test_item[NUM_TEST_ITEMS] = {
    {
	&test_item[1], 0, 0, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[0], NULL, 0, NULL, 0
    },
    {
	&test_item[2], 0, 10, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[1], NULL, 0, NULL, 0
    },
    {
	&test_item[3], 0, 20, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[2], NULL, 0, NULL, 0
    },
    {
	&test_item[4], 0, 30, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[3], NULL, 0, NULL, 0
    },
    {
	&test_item[5], 0, 40, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[4], NULL, 0, NULL, 0
    },
    {
	&test_item[6], 0, 55, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &test_item_text[5], NULL, 0, NULL, 0
    },
    {
	 NULL, 0, 65, MIWIDTH, MIHI,
	 ITEMENABLED | HIGHCOMP | ITEMTEXT,
	 0, (APTR) &test_item_text[6], NULL, 0, NULL, 0
    }
};

struct Menu test_menu = {
    NULL,			/* NextMenu */
    460, 0,			/* LeftEdge, TopEdge */
    100, 0,			/* Width, Height */
    MENUENABLED,		/* Flags */
    (BYTE *) "Test",		/* MenuName */
    &test_item[0]		/* FirstItem */
};

#if 0
/*-------------------------   Colors Sub Menu  -------------------*/

struct IntuiText showcolors_item_text[NUM_SHOWCOLORS_ITEMS] = {
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL,
		(UBYTE *) "Selected Files", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL,
		(UBYTE *) "Shadowed Files/Dirs", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL,
		(UBYTE *) "Backup Failed", NULL }
};

#undef MIWIDTH
#define MIWIDTH 200

struct MenuItem showcolors_item[NUM_COMPRESSION_ITEMS] = {
    {
	&compression_item[1], 60, 5, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT | CHECKED,
	6L, (APTR) &compression_item_text[0], NULL, 0, 0, 0
    },
    {
	&compression_item[2], 60, 15, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	5L, (APTR) &compression_item_text[1], NULL, 0, 0, 0
    },
    {
	NULL, 60, 25, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	3L, (APTR) &compression_item_text[2], NULL, 0, 0, 0
    }
};

#endif


/*-----------------------   Devices Menu   -----------------------*/

char userdev[4][USERDEV_MAXWIDTH] = {
    "", "", "", ""
};

struct IntuiText devs_item_text[NUM_DEVS_ITEMS] = {
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) "DF0:", NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) "DF1:", NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) "DF2:", NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) "DF3:", NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) &userdev[0][0], NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) &userdev[1][0], NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) &userdev[2][0], NULL },
    { 0, 1, JAM1, CHECKWIDTH + 1, 1, NULL, (UBYTE *) &userdev[3][0], NULL }
};

#undef MIWIDTH
#define MIWIDTH 200

struct MenuItem devs_item[NUM_DEVS_ITEMS] = {
    {
	&devs_item[1], 0, 0, MIWIDTH, MIHI,
	HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT | CHECKED,
	0, (APTR) &devs_item_text[0], NULL, 0, 0, 0
    },
    {
	&devs_item[2], 0, 10, MIWIDTH, MIHI,
	HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[1], NULL, 0, 0, 0
    },
    {
	&devs_item[3], 0, 20, MIWIDTH, MIHI,
	HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[2], NULL, 0, 0, 0
    },
    {
	&devs_item[4], 0, 30, MIWIDTH, MIHI,
	HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[3], NULL, 0, 0, 0
    },
    {
	&devs_item[5], 0, 40, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[4], NULL, 0, 0, 0
    },
    {
	&devs_item[6], 0, 50, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[5], NULL, 0, 0, 0
    },
    {
	&devs_item[7], 0, 60, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[6], NULL, 0, 0, 0
    },
    {
	NULL, 0, 70, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &devs_item_text[7], NULL, 0, 0, 0
    }
};

struct Menu devs_menu = {
    NULL,			/* NextMenu */
    370, 0,			/* LeftEdge, TopEdge */
    70, 0,			/* Width, Height */
    MENUENABLED,		/* Flags */
    (BYTE *) "Devices",		/* MenuName */
    &devs_item[0]		/* FirstItem */
};

/*-----------------------   Options Menu   -----------------------*/

struct IntuiText options_item_text[NUM_OPTIONS_ITEMS] = {
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "Set Archive Bits", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "Smaller Log Files", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "Compression", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "Backup Dir Structure",
		NULL }
};

struct IntuiText compression_item_text[NUM_COMPRESSION_ITEMS] = {
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "None", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "All", NULL },
    { 0, 1, JAM1, CHECKWIDTH+1, 1, NULL, (UBYTE *) "Larger Than [100k]",
		NULL },
};

#undef MIWIDTH
#define MIWIDTH 200

struct MenuItem compression_item[NUM_COMPRESSION_ITEMS] = {
    {
	&compression_item[1], 60, 5, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT | CHECKED,
	6L, (APTR) &compression_item_text[0], NULL, 0, 0, 0
    },
    {
	&compression_item[2], 60, 15, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	5L, (APTR) &compression_item_text[1], NULL, 0, 0, 0
    },
    {
	NULL, 60, 25, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	3L, (APTR) &compression_item_text[2], NULL, 0, 0, 0
    }
};

#undef MIWIDTH
#define MIWIDTH 200

struct MenuItem options_item[NUM_OPTIONS_ITEMS] = {
    {
	&options_item[1], 0, 0, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &options_item_text[0], NULL, 0, 0, 0
    },
    {
	&options_item[2], 0, 10, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT,
	0, (APTR) &options_item_text[1], NULL, 0, 0, 0
    },
    {
	&options_item[3], 0, 20, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &options_item_text[2], NULL, 0, &compression_item[0], 0
    },
    {
	NULL, 0, 30, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT | CHECKED,
	0, (APTR) &options_item_text[3], NULL, 0, 0, 0
    }
};

struct Menu options_menu = {
    &devs_menu,			/* NextMenu */
    270, 0,			/* LeftEdge, TopEdge */
    100, 0,			/* Width, Height */
    MENUENABLED,		/* Flags */
    (BYTE *) "Options",		/* MenuName */
    &options_item[0]		/* FirstItem */
};


/*-----------------------   Sort Menu   -----------------------*/

struct IntuiText sort_item_text[NUM_SORT_ITEMS + 1] = {
    {
	0, 1, JAM1, CHECKWIDTH + 1, 1, NULL,
	(UBYTE *) "List Directories First", &sort_item_text[1]
    },
    {
	0, 1, JAM1, CHECKWIDTH + 1, 10, NULL,
	(UBYTE *) "----------------------", NULL
    },
    {
	0, 1, JAM1, CHECKWIDTH + 1, 1, NULL,
	(UBYTE *) "Sort by Name", NULL
    },
    {
	0, 1, JAM1, CHECKWIDTH + 1, 1, NULL,
	(UBYTE *) "Sort by Date", NULL
    },
    {
	0, 1, JAM1, CHECKWIDTH + 1, 1, NULL,
	(UBYTE *) "Sort by Size", NULL
    },
    {
	0, 1, JAM1, CHECKWIDTH + 1, 1, NULL,
	(UBYTE *) "Sort by Archive Bit", NULL
    }
};

#undef MIWIDTH
#define MIWIDTH 204

struct MenuItem sort_item[NUM_SORT_ITEMS] = {
    {
	&sort_item[1], 0, 0, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | MENUTOGGLE | CHECKIT | CHECKED,
	0, (APTR) &sort_item_text[0], NULL, 0, 0, 0
    },
    {
	&sort_item[2], 0, 20, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT | CHECKED,
	0xFFFC, (APTR) &sort_item_text[2], NULL, 0, 0, 0
    },
    {
	&sort_item[3], 0, 30, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	0xFFFA, (APTR) &sort_item_text[3], NULL, 0, 0, 0
    },
    {
	&sort_item[4], 0, 40, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	0xFFF6, (APTR) &sort_item_text[4], NULL, 0, 0, 0
    },
    {
	NULL, 0, 50, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | CHECKIT,
	0xFFEE, (APTR) &sort_item_text[5], NULL, 0, 0, 0
    },
};


struct Menu sort_menu = {
    &options_menu,		/* NextMenu */
    100, 0,			/* LeftEdge, TopEdge */
    170, 0,			/* Width, Height */
    MENUENABLED,		/* Flags */
    (BYTE *) "Display Sorting",	/* MenuName */
    &sort_item[0]		/* FirstItem */
};

/*-------------  Project Menu ----------------------*/

struct IntuiText project_item_text[] = {
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Backup", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Inspect", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Differences", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Restore", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "No-log Restore",
		&project_item_text[5] },
    { 0, 1, JAM1, 1, 10, NULL, (UBYTE *) "-----------", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Help", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "About", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Info", NULL },
    { 0, 1, JAM1, 1,  1, NULL, (UBYTE *) "Quit", NULL }
};


#undef MIWIDTH
#define MIWIDTH 130

struct MenuItem project_item[NUM_PROJECT_ITEMS] = {
    {
	&project_item[1], 0, 0, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | COMMSEQ,
	0, (APTR) &project_item_text[0], NULL, (BYTE) 'B', NULL, 0
    },
    {
	&project_item[2], 0, 10, MIWIDTH, MIHI,
	/* ITEMENABLED | */ HIGHCOMP | ITEMTEXT | COMMSEQ,
	0, (APTR) &project_item_text[1], NULL, (BYTE) 'I', NULL, 0
    },
    {
	&project_item[3], 0, 20, MIWIDTH, MIHI,
	/* ITEMENABLED | */ HIGHCOMP | ITEMTEXT | COMMSEQ,
	0, (APTR) &project_item_text[2], NULL, (BYTE) 'D', NULL, 0
    },
    {
	&project_item[4], 0, 30, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | COMMSEQ,
	0, (APTR) &project_item_text[3], NULL, (BYTE) 'R', NULL, 0
    },
    {
	&project_item[5], 0, 40, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT ,
	0, (APTR) &project_item_text[4], NULL, 0, NULL, 0
    },
    {
	&project_item[6], 0, 60, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT | COMMSEQ,
	0, (APTR) &project_item_text[6], NULL, (BYTE) 'H', NULL, 0
    },
    {
	&project_item[7], 0, 70, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &project_item_text[7], NULL, 0, NULL, 0
    },
	{
	&project_item[8], 0, 80, MIWIDTH, MIHI,
	ITEMENABLED | HIGHCOMP | ITEMTEXT,
	0, (APTR) &project_item_text[8], NULL, 0, NULL, 0
    },
    {
	 NULL, 0, 95, MIWIDTH, MIHI,
	 ITEMENABLED | HIGHCOMP | ITEMTEXT | COMMSEQ,
	 0, (APTR) &project_item_text[9], NULL, (BYTE) 'Q', NULL, 0
    }
};

struct Menu project_menu = {
    &sort_menu,			/* NextMenu */
    0, 0,			/* LeftEdge, TopEdge */
    100, 0,			/* Width, Height */
    MENUENABLED,		/* Flags */
    (BYTE *) "Project",		/* MenuName */
    &project_item[0]		/* FirstItem */
};


static struct List disks;



/*--------------------------------------------------------------*/
/*			Functions				*/
/*--------------------------------------------------------------*/

/* DTM_NEW */

void init_menus( void )
{
	struct Node *disk;
	int i, width, max_width=0;


	/* Do we want to enable the test menu? */
	if( show_test_menu == TRUE )
	{
		devs_menu.NextMenu = &test_menu;
	}


	set_menu_font();


	/* Ghost the menu items for backup devices that are not
	 * mounted in the system.
	 */

    NewList (&disks);		/* Initialize list header */

    getdisks (&disks);		/* Fill list */

    if (disks.lh_TailPred != (struct Node  *) & disks )
	{
		for (disk = disks.lh_Head; disk -> ln_Succ;
			disk = disk -> ln_Succ)
		{
			if(  strcmp( disk -> ln_Name, "DF0" ) == 0  )
			{
				SetFlag( devs_item[0].Flags, ITEMENABLED );
			}

			if(  strcmp( disk -> ln_Name, "DF1" ) == 0  )
			{
				SetFlag( devs_item[1].Flags, ITEMENABLED );
			}
			if(  strcmp( disk -> ln_Name, "DF2" ) == 0  )
			{
				SetFlag( devs_item[2].Flags, ITEMENABLED );
			}
			if(  strcmp( disk -> ln_Name, "DF3" ) == 0  )
			{
				SetFlag( devs_item[3].Flags, ITEMENABLED );
			}
	    }
    }

    freedisks (&disks);


	/* Adjust the devs: menu item width */

	for( i=0; i<NUM_DEVS_ITEMS; i++ )
	{
		width = IntuiTextLength( &devs_item_text[i] );
		max_width = max( max_width, width );
	}

	for( i=0; i<NUM_DEVS_ITEMS; i++ )
	{
		devs_item[i].Width = max_width + 6 + CHECKWIDTH;
	}


	/* Shorten the menu to not show blank space for undefined user
	 * devices.
	 */

	for( i=0; i<4; i++ )
	{
		if( userdev[i][0] == '\0' )
		{
			devs_item[i+3].NextItem = NULL;		/* break the chain */
			break;
		}
	}
}



static void set_menu_font( void )
{
	struct Menu *menu;
	struct MenuItem *mi, *si;
	struct IntuiText *it;


	menu = &project_menu;

	while( menu != NULL )
	{
		mi = menu->FirstItem;
		while( mi != NULL )
		{
			if(  FlagIsSet( mi->Flags, ITEMTEXT ) &&
				( mi->ItemFill != NULL )  )
			{
				it = (struct IntuiText *)(mi->ItemFill);

				while( it != NULL )
				{
					it->ITextFont = textattr;
					it = it->NextText;
				}
			}

			si = mi->SubItem;
			while( si != NULL )
			{
				if(  FlagIsSet( si->Flags, ITEMTEXT ) &&
					( si->ItemFill != NULL )  )
				{
					it = (struct IntuiText *)(si->ItemFill);

					while( it != NULL )
					{
						it->ITextFont = textattr;
						it = it->NextText;
					}
				}

				si = si->NextItem;
			}

			mi = mi->NextItem;
		}

		menu = menu->NextMenu;
	}
}



#if 0

/*
 * This routine is specific to a window.  Passing it the address of
 * a menu item and a boolean (int) flag will cause the checkmark to
 * either be set or cleared as desired.
 * Note that this should only be called if the menustrip is posted
 * in the window!  If the menu has not been added yet, just manipulate
 * the flag directly.
 */

void menucheckmark (item, state)
struct MenuItem *item;
int state;
{
    DBUG_ENTER ("menucheckmark");
    ClearMenuStrip (mainwin);
    if (state) {
	SetFlag (item -> Flags, CHECKED);
    } else {
	ClearFlag (item -> Flags, CHECKED);
    }
    SetMenuStrip (mainwin, &project_menu);
    DBUG_VOID_RETURN;
}

void read_menus ()
{
    DBUG_ENTER ("read_menus");
    if (sort_item[ASK_CLOSE_ITEM].Flags & CHECKED) {
	/* do something (like set a flag) */
    }
    DBUG_VOID_RETURN;
}

/*
 * This will turn a menu on or off depending on the boolean value
 * of the state parameter.
 */

static  void setmenu (state, number)
int state;
long number;
{
    DBUG_ENTER ("setmenu");
    if (state) {
	OnMenu (mainwin, number);
    } else {
	OffMenu (mainwin, number);
    }
    DBUG_VOID_RETURN;
}

void menu_status ()
{
    DBUG_ENTER ("menu_status");
    DBUG_VOID_RETURN;
}

#endif
