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
#include "filereq.h"
#include "logfile.h"
#include "mainwin.h"
#include "scansort.h"
#include "dbug.h"
#include "reqs.h"
#include "libfuncs.h"
#include "hdbackup_rev.h"


/* static const char logfile_id[] = "HDBackup Logfile"; */
#define LOGFILE_ID		"HDBackup Logfile"
	/* This is placed at the start of the logfile for identification
	 * purposes.
	 */


char bru_logdir[80] = BRU_LOGDIR;

char logfile_pattern[80] = "%d%b%y";

char logfile_icon_template[80] = "";
char logdir_icon_template[80] = "";


static void build_logname( BOOL forsave );
static char *when_pattern( char *pattern );
static void make_dir_icon( char *fname );
static void make_file_icon( char *fname );
static BOOL icon_exists( char *fname );
static BOOL save_node ( struct TreeNode *tn );
static struct TreeNode *load_node ( struct TreeNode *parent );
static BOOL do_save ( struct TreeNode *tn );
static struct TreeNode *do_load ( void );
static int create_logfile( char *name );
static LONG get_file_size( BPTR fh );


static char log_buf[FMSIZE];

static BPTR fh;

static ERRORCODE log_err = 0;



/*..........  Icon data for the LogFile directories ..................*/

static USHORT  logdir_data[] = {
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0600, 0x0000, 0x0000, 0x0000, 0x0020,
 0x0400, 0x0000, 0x0000, 0x0000, 0x0120,
 0x0400, 0x0008, 0x0000, 0x8000, 0x0120,
 0x0400, 0x0009, 0xFFFC, 0x8000, 0x0120,
 0x0400, 0x0009, 0xD554, 0x8000, 0x0120,
 0x0400, 0x0008, 0x0000, 0x8000, 0x0120,
 0x0400, 0x0007, 0xFFFF, 0x0000, 0x0120,
 0x0400, 0x0000, 0x0000, 0x0000, 0x0120,
 0x04FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0020,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFE0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/**/
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x19FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1BFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFEC0,
 0x1BFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF6, 0x0003, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF6, 0x2AAB, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF8, 0x0000, 0xFFFF, 0xFEC0,
 0x1BFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFEC0,
 0x1B00, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

static struct Image logdir_image = {
 0,  /* LeftEdge */
 0,  /* TopEdge */
 80,  /* Width */
 16,  /* Height */
 2,  /* Depth */
 (USHORT *)&logdir_data,  /* ImageData */
 0x3,  /* PlanePick */
 0x0,  /* PlaneOnOff */
 0,  /* [NextImage] */
 };


static USHORT  logdir_data2[] = {
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0C00, 0x0000, 0x0000, 0x0000, 0x00A0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x0060,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFE0,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0030,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0034,
 0x2000, 0x0008, 0x0000, 0x8000, 0x0034,
 0x2000, 0x0009, 0xFFFC, 0x8000, 0x0035,
 0x2000, 0x0009, 0xD554, 0x8000, 0x0035,
 0x2000, 0x0008, 0x0000, 0x8000, 0x0035,
 0x2000, 0x0007, 0xFFFF, 0x0000, 0x0035,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0035,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFF5,
 0x0055, 0x5555, 0x5555, 0x5555, 0x5555,
 0x0015, 0x5555, 0x5555, 0x5555, 0x5555,
/**/
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x13FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF40,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF80,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF6, 0x0003, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF6, 0x2AAB, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF8, 0x0000, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 };

struct Image logdir_image2 = {
 0,  /* LeftEdge */
 0,  /* TopEdge */
 80,  /* Width */
 16,  /* Height */
 2,  /* Depth */
 (USHORT *)&logdir_data2,  /* ImageData */
 0x3,  /* PlanePick */
 0x0,  /* PlaneOnOff */
 0,  /* [NextImage] */
 };



static struct DrawerData logdir_dd = {
	/* A NewWindow structure */
	{
    0, 12,			/* LeftEdge, TopEdge */
    240, 60,			/* Width, Height */
    -1, -1,			/* DetailPen, BlockPen */

	NULL,			/* IDCMP Flags */

	WINDOWDEPTH |
	WINDOWSIZING |
	WINDOWDRAG |
	WINDOWCLOSE |
	SIZEBRIGHT |
	SIZEBBOTTOM |
	SIMPLE_REFRESH,		/* Flags */

    NULL,			/* FirstGadget */
    NULL,			/* CheckMark */
    NULL,			/* Title */
    NULL,			/* Screen */
    NULL,			/* BitMap */
    90, 40,			/* MinWidth, MinHeight */
    640, 200,		/* MaxWidth, MaxHeight */
    WBENCHSCREEN	/* Type */
	},
	20, 50		/* X, Y */
};

static struct DiskObject logdir_disko = {
 WB_DISKMAGIC,   /* do_Magic */
 WB_DISKVERSION, /* do_Version */

/* Embedded Gadget Structure */
 0,  /* [NextGadget] */
 124,  /* LeftEdge */
 76,  /* TopEdge */
 80,  /* Width */
 16,  /* Height */
 0x6,  /* Flags */
 0x3,  /* Activation */
 0x1,  /* GadgetType */
 (APTR)&logdir_image, /* GadgetRender */
 (APTR)&logdir_image2, /* SelectRender */
 NULL,     /* GadgetText */
 0x0,  /* MutualExclude */
 0x0,  /* [SpecialInfo] */
 0,  /* GadgetID */
 0x0,  /* [UserData] */

/* Rest of DiskObject structure */
 0x2,  /* do_Type */
 NULL,   /* do_DefaultTool */
 0x0,  /* [do_ToolTypes] */
 NO_ICON_POSITION,  /* do_CurrentX */
 NO_ICON_POSITION,  /* do_CurrentY */
 &logdir_dd,  /* [do_DrawerData] */
 0x0,  /* [do_ToolWindow] */
 0x0,  /* do_StackSize */
 };



/*....................  Icon data for the LogFile  ..................*/

static USHORT logfile_data[] = {
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800,
 0x3320, 0xE07F, 0xFFE3, 0xFFFF, 0xF800,
 0x3332, 0x733F, 0xFFF3, 0xFFFF, 0xF800,
 0x3333, 0x3338, 0x7873, 0x3324, 0x7800,
 0x3033, 0x307F, 0x3332, 0x7333, 0x3800,
 0x3333, 0x333C, 0x33F0, 0xF333, 0x3800,
 0x3332, 0x7333, 0x3332, 0x7330, 0x7800,
 0x3320, 0xE078, 0x9863, 0x3893, 0xF800,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFE1, 0xF800,
 0x3FFF, 0xFE1F, 0xFFFF, 0xFFFF, 0xF800,
 0x3FFF, 0xFF3F, 0xFFFF, 0xFFFF, 0xF800,
 0x3FFF, 0xFF3F, 0x8789, 0xFFFF, 0xF800,
 0x3FFF, 0xFF3F, 0x3333, 0xFFFF, 0xF800,
 0x3FFF, 0xFF3B, 0x3333, 0xFFFF, 0xF800,
 0x3FFF, 0xFF33, 0x3387, 0xFFFF, 0xF800,
 0x3FFF, 0xFE03, 0x8673, 0xFFFF, 0xF800,
 0x3FFF, 0xFFFF, 0xFF07, 0xFFFF, 0xF800,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/**/
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFE00,
 0xC000, 0x0000, 0x0000, 0x0000, 0x0600,
 0xCCDF, 0x1F80, 0x001C, 0x0000, 0x0600,
 0xCCCD, 0x8CC0, 0x000C, 0x0000, 0x0600,
 0xCCCC, 0xCCC7, 0x878C, 0xCCDB, 0x8600,
 0xCFCC, 0xCF80, 0xCCCD, 0x8CCC, 0xC600,
 0xCCCC, 0xCCC3, 0xCC0F, 0x0CCC, 0xC600,
 0xCCCD, 0x8CCC, 0xCCCD, 0x8CCF, 0x8600,
 0xCCDF, 0x1F87, 0x679C, 0xC76C, 0x0600,
 0xC000, 0x0000, 0x0000, 0x001E, 0x0600,
 0xC000, 0x01E0, 0x0000, 0x0000, 0x0600,
 0xC000, 0x00C0, 0x0000, 0x0000, 0x0600,
 0xC000, 0x00C0, 0x7876, 0x0000, 0x0600,
 0xC000, 0x00C0, 0xCCCC, 0x0000, 0x0600,
 0xC000, 0x00C4, 0xCCCC, 0x0000, 0x0600,
 0xC000, 0x00CC, 0xCC78, 0x0000, 0x0600,
 0xC000, 0x01FC, 0x798C, 0x0000, 0x0600,
 0xC000, 0x0000, 0x00F8, 0x0000, 0x0600,
 0xC000, 0x0000, 0x0000, 0x0000, 0x0600,
 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFE00,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/**/
 };

static struct Image logfile_image = {
 0,  /* LeftEdge */
 0,  /* TopEdge */
 72,  /* Width */
 22,  /* Height */
 2,  /* Depth */
 (USHORT *)&logfile_data,  /* ImageData */
 0x3,  /* PlanePick */
 0x0,  /* PlaneOnOff */
 0,  /* [NextImage] */
 };

static struct DiskObject logfile_disko = {
 WB_DISKMAGIC,   /* do_Magic */
 WB_DISKVERSION, /* do_Version */

/* Embedded Gadget Structure */
 0,  /* [NextGadget] */
 0,  /* LeftEdge */
 0,  /* TopEdge */
 72,  /* Width */
 22,  /* Height */
 0x5,  /* Flags */
 0x3,  /* Activation */
 0x1,  /* GadgetType */
 (APTR)&logfile_image, /* GadgetRender */
 NULL,     /* SelectRender */
 NULL,     /* GadgetText */
 0x0,  /* MutualExclude */
 0x0,  /* [SpecialInfo] */
 0,  /* GadgetID */
 0x0,  /* [UserData] */

/* Rest of DiskObject structure */
 0x4,  /* do_Type */
 "SYS:HDBackup",   /* do_Default Tool */
 0x0,  /* [do_ToolTypes] */
 NO_ICON_POSITION,  /* do_CurrentX */
 NO_ICON_POSITION,  /* do_CurrentY */
 0x0,  /* [do_DrawerData] */
 0x0,  /* [do_ToolWindow] */
 0x1000,  /* do_StackSize */
 };



/* Returns the size of the file which will be created when we
 * save the logfile.
 * If the scanflag argument is TRUE, the node status will be refreshed.
 * Otherwise we use whatever was last calculated.
 */

ULONG logfile_size( BOOL scanflag )
{
	ULONG size;

	if( scanflag == TRUE )
	{
		node_status( root_node );
	}

	size = ( node_count * sizeof(LONG) ) + node_memsize;

	size += strlen(LOGFILE_ID);

	size += 4;		/* For length field */
	size += 4;		/* For version field */

	return( size );
}



/*..............................................................*/
/*								*/
/*		Save the tree to a logfile			*/
/*..............................................................*/

void save_tree ( struct TreeNode *tn )
{
    DBUG_ENTER ("save_tree");

	build_logname( TRUE );

    if (do_file_req ("Save BRUshell logfile", log_buf) == FALSE) {
	DBUG_VOID_RETURN;
    }

	if(  create_logfile( log_buf ) != 0  ) {
	mention_error ("Cannot create logfile", ERC_IO_FAILED | ERR17);
	DBUG_VOID_RETURN;
    }
    post_cancel_req (mainwin, "Saving Logfile");

    if (do_save (tn) == FALSE) {
	/* Error while writing */
	Close (fh);
	DeleteFile (log_buf);
	mention_error ("Error while writing logfile", ERC_IO_FAILED | ERR18);
    } else {
	/* Write successful */
	Close (fh);
    }
    clear_cancel_req ();
    DBUG_VOID_RETURN;
}



/* Return is zero for success */

int save_embedded_logfile( struct TreeNode *tn )
{
	BOOL save_iconflag;


	save_iconflag = makeicon_flag;
	makeicon_flag = FALSE;				/* Make sure we don't waste
										 * time etc. creating an icon.
										 */

	if(  create_logfile( EMBED_LOGNAME ) != 0  )
	{
		makeicon_flag = save_iconflag;
		return( -1 );
    }

    if (do_save (tn) == FALSE)
	{
		/* Error while writing */
		Close (fh);
		DeleteFile (EMBED_LOGNAME);
		return( -1 );
    }
	else
	{
		/* Write successful */
		Close (fh);
    }

	makeicon_flag = save_iconflag;

	return( 0 );
}



/* DTM_NEW */

static void build_logname( BOOL forsave )
{
	char buf[256], *p, c;
	BPTR lock;
	BOOL exists;


	strcpy( log_buf, bru_logdir );

	if( *log_buf != '\0' )
	{
		if( log_buf[strlen(log_buf)-1] != ':' )
		{
			strcat( log_buf, "/" );
		}
	}

	/* The log buffer now contains a path ending in either "/" or ":".
	 * This is a preliminary path, the subdirectory has yet to be
	 * appended.
	 */

	/* This is new to X1.4.  X1.3 and below plunked the user into
	 * the last directory (as well as filename) that they had saved
	 * from.
	 */
	if( forsave == FALSE )
	{
		/* We don't want to assume a subdirectory.  Open the file
		 * requestor in the root log dir.
		 */
		return;
	}


	strcpy( buf, root_name );
	p = buf;
	while( *p != '\0' )
	{
		if( *p == ':' )
		{
			*p = '/';
		}

		p++;
	}

	strcat( log_buf, buf );

	/* Now the log buffer contains a full path (no file name yet)
	 * which ends in either a "/" or else no seperator char.
	 */


	/* Add the "/" if neccessary */
	if( log_buf[strlen(log_buf)-1] != '/' )
	{
		strcat( log_buf, "/" );
	}

#if 0
	if( forsave == TRUE )
	{
		/* See if the directory exists before we post the
		 * file requester.  If it doesn't, then create it.
		 * This will avoid the "Wrong Disk" message.
		 */
	}
#endif

	/* Add the filename (if there is a pattern defined */
	p = when_pattern( logfile_pattern );
	if( p )
	{
		strcat( log_buf, p );
	}

#if 0
	if( forsave == FALSE )
	{
		/* To load, we don't care about being unique */
		return;
	}
#endif

	/* This is a logfile save. */

	do
	{
		lock = Lock( log_buf, ACCESS_READ );
		if( lock )
		{
			/* The file exists, mutate the name */
			UnLock( lock );
			exists = TRUE;

			p = &log_buf[strlen(log_buf)-2];
			if( *p == '.' )
			{
				p++;
				c = *p;
				if(  ( c >= '1' ) && ( c <= '8' )  )
				{
					/* Bump the number */
					c++;
					*p = c;
				}
				else
				{
					/* Not bumpable, warn user */
					mention_error(
						"Warning: Unable to make unique logfile name.",
						ERC_NONE | ERR62 );
					exists = FALSE;				/* Give up */
				}
			}
			else
			{
				/* No number on end, add one */
				strcat( log_buf, ".1" );
			}
		}
		else
		{
			/* Unique name */
			exists = FALSE;
		}
	}
	while( exists == TRUE );
}


#if 0
static char *build_from_pat( char *pattern )
{
	time_t tod;
	struct tm *ts;
	static char buf[32];


	time( &tod );

	ts = localtime( tod );

	if(  strftime( buf, sizeof(buf), pattern, ts ) == 0  )
	{
		return( NULL );
	}

	return( buf );
}
#endif


/* DTM_NEW */

/* Open the logfile for writing.  This will create any required directory
 * structure and icons.
 * Return is zero upon success.
 */

static int create_logfile( char *name )
{
	char dev[MAX_NAME], path[FMSIZE], node[MAX_NAME], ext[MAX_NAME];
	int dirs[16];
	int i=0;
	BPTR lock, root_lock;
	char fname[FMSIZE];


	if( strlen(name) >= FMSIZE )
	{
		mention_error( "Logfile directory path/name too long.",
				ERC_IO_FAILED | ERR48 );
		return( -1 );
	}

	/* Fragment the filespec */
	strsfn( name, dev, path, node, ext );

	/* Re-create the filename */
	if( *ext != '\0' )
	{
		sprintf( fname, "%s.%s", node, ext );
	}
	else
	{
		strcpy( fname, node );
	}

	/* Fragment the path components */
	if(  stspfp( path, &dirs[0] ) != 0  )
	{
		mention_error( "Logfile directory path too deep.",
				ERC_IO_FAILED | ERR45 );
		return( -1 );
	}


	/* Create the directory structure */

	if( *dev != '\0' )
	{
		lock = Lock( dev, ACCESS_READ );
		if( lock == NULL )
		{
			/* The device doesn't exist, give up */

			mention_error( "Unable to find the Logfile volume.",
				ERC_IO_FAILED | ERR44 );
			return( -1 );
		}

		root_lock = CurrentDir( lock );
	}
	else
	{
		root_lock = CurrentDir( 0 );
		lock = DupLock( root_lock );
		(void)CurrentDir( lock );
	}

	/* Now, "root_lock" contains a directory we should change back into
	 * when done.  We are also in the root of the device we wish
	 * to place the logfile tree.
	 */ 

	i = 0;

	while(  ( dirs[i] != -1 ) && ( path[dirs[i]] != '\0' )  )
	{
		/* See if it exists */
		lock = Lock( &path[dirs[i]], ACCESS_READ );
		if( lock == NULL ) 
		{
			/* nope, create it */
			lock = CreateDir( &path[dirs[i]] );
			if( lock == NULL )
			{
				/* Could not create it */

				UnLock(  CurrentDir( root_lock )  );
					/* Restore dir where we started. */

				mention_error( "Unable to create the Logfile directory.",
					ERC_IO_FAILED | ERR46 );
				return( -1 );
			}
		}

		/* create it's icon */
		make_dir_icon( &path[dirs[i]] );

		/* Descend into it. */
		UnLock(  CurrentDir( lock )  );

		i++;
	}


	/* And open the logfile */

	fh = Open( fname, MODE_NEWFILE );
	if( fh == NULL )
	{
		UnLock(  CurrentDir( root_lock )  );	/* Restore dir where we
												 * started.
												 */
		return( -1 );
	}

	/* Create it's icon */
	make_file_icon( fname );

	UnLock(  CurrentDir( root_lock )  );	/* Restore dir where we
											 * started.
											 */
	return( 0 );
}


/* DTM_NEW */

/* If needed and wanted, this will create a directory icon for the
 * given directory.
 */

static void make_dir_icon( char *fname )
{
	struct DiskObject *disko=NULL;


	if( makeicon_flag != TRUE )
	{
		return;
	}

	/* Does an icon exist already? */
	if(  icon_exists( fname ) == TRUE  )
	{
		return;
	}


	if( logdir_icon_template[0] != '\0' )
	{
		/* Attempt to use the template which the user specified */
		disko = GetDiskObject( logdir_icon_template );
		if( disko )
		{
			disko->do_CurrentX = NO_ICON_POSITION;
			disko->do_CurrentY = NO_ICON_POSITION;
		}
	}

	if(  PutDiskObject( fname,
		((disko!=NULL)?disko:&logdir_disko) ) == 0  )
	{
		/* Failure, but not fatal.  Just tell the user. */
		mention_error( "Warning: Unable to create a logfile directory icon.",
			ERC_NONE | ERR47 );
	}

	if( disko )
	{
		FreeDiskObject( disko );
	}
}



/* DTM_NEW */

/* If needed and wanted, this will create a logfile icon for the
 * given file.
 */

static void make_file_icon( char *fname )
{
	struct DiskObject *disko=NULL;


	if( makeicon_flag != TRUE )
	{
		return;
	}

	/* Does an icon exist already? */
	if(  icon_exists( fname ) == TRUE  )
	{
		return;
	}


	if( logfile_icon_template[0] != '\0' )
	{
		/* Attempt to use the template which the user specified */
		disko = GetDiskObject( logfile_icon_template );
		if( disko )
		{
			disko->do_CurrentX = NO_ICON_POSITION;
			disko->do_CurrentY = NO_ICON_POSITION;
		}
	}

	if(  PutDiskObject( fname,
		((disko!=NULL)?disko:&logfile_disko) ) == 0  )
	{
		/* Failure, but not fatal.  Just tell the user. */
		mention_error( "Warning: Unable to create the logfile's icon.",
			ERC_NONE | ERR43 );
	}

	if( disko )
	{
		FreeDiskObject( disko );
	}
}


/* DTM_NEW */

/* Return TRUE if an icon for the given file or directory exists.
 */

static BOOL icon_exists( char *fname )
{
	BPTR lock;
	char buf[MAX_NAME];


	sprintf( buf, "%s.info", fname );

	lock = Lock( buf, ACCESS_READ );
	if( lock )
	{
		UnLock( lock );
		return( TRUE );
	}

	return( FALSE );	
}


#if 0
static int create_logfile( name )
char *name;
{
    if ((fh = Open (name, MODE_NEWFILE)) == NULL)
	{
		/* Failure */
		return( -1 );
	}

	if(  PutDiskObject( name, &logfile_disko ) == 0  )
	{
		/* Failure, but not fatal.  Just tell the user. */
		mention_error( "Warning: Unable to create the logfile's icon.",
			ERC_NONE | ERR43 );
	}

	return( 0 );	/* Success */
}
#endif



static BOOL do_save ( struct TreeNode *tn )
{
	BOOL rc;
	LONG l, size, ver;


	l = strlen(LOGFILE_ID);
    if (Write (fh, LOGFILE_ID, l) != l)
	{
		DBUG_RETURN (FALSE);
    }


	/* Embed the version of the program */
	ver = (VERSION*100)+REVISION;
    if (Write (fh, (char *) & ver, 4L) != 4L)
	{
		DBUG_RETURN (FALSE);
    }

	/* Write the file size into the file */
	size = logfile_size( TRUE );
    if (Write (fh, (char *) & size, 4L) != 4L)
	{
		DBUG_RETURN (FALSE);
    }

	rc = save_node( tn );

	return( rc );
}



/* This recursing function writes a node to the filehandle */

static BOOL save_node ( struct TreeNode *tn )
{
    ULONG nodelen;
    int i;
    struct TreeEntry *te;

    DBUG_ENTER ("save_node");

    /* Check for user saying stop */
    if (check_cancel () == TRUE)
	{
		DBUG_RETURN (FALSE);
    }

    /* Is this an empty dir? */
    if (tn == NULL)
	{
		DBUG_RETURN (TRUE);
    }

    /* First, save this node */
    /* Use the following formula instead of tn->Size, because the tree
     * trimming could have reduced the number of entrys (which does not
     * reduce the nodesize in memory) and we do not want to save the
     * deleted entrys
     */
    nodelen = sizeof (struct TreeNode) +
                 (MAX ((tn -> Entries - 1), 0) * sizeof (struct TreeEntry));
    /* nodelen = tn -> Size; *//* obsolete method */

    /* Write the nodesize to the file */
    if (Write (fh, (char *) & nodelen, 4L) != 4L)
	{
		DBUG_RETURN (FALSE);
    }

    if (Write (fh, (char *) tn, nodelen) != nodelen)
	{
		DBUG_RETURN (FALSE);
    }

    /* Now move through it, saving it's children */
    for (i = 0; i < tn -> Entries; i++)
	{
		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR))
		{
		    /* Recurse */
	    	if (save_node ((struct TreeNode *) (te -> Size)) == FALSE)
			{
				DBUG_RETURN (FALSE);
	    	}
		}
    }

    DBUG_RETURN (TRUE);
}


/*..............................................................*/
/*								*/
/*		Load the tree from a logfile			*/
/*..............................................................*/

void load_tree ( void )
{
    DBUG_ENTER ("load_tree");
    log_err = 0;

	build_logname( FALSE );

    if( do_file_req ("Load BRUshell logfile", log_buf) == FALSE )
	{
		DBUG_VOID_RETURN;
    }

    if ((fh = Open (log_buf, MODE_READONLY)) == NULL) {
	mention_error ("Cannot find logfile", ERC_IO_FAILED | ERR19);
	DBUG_VOID_RETURN;
    }
    post_cancel_req (mainwin, "Loading Logfile");

    root_node = do_load ();

    Close (fh);

    if (log_err) {
	/* Error while reading */
	mention_error ("Error while reading logfile", log_err);
	free_tree (root_node);
	root_node = NULL;
    } else {
    /* Allocate format buffers */
    node_status (root_node);
    if ((entry_string = RemAlloc ((ULONG) (70 * most_entrys * sizeof (char)),
		    MEMF_CLEAR)) == NULL) {
	free_tree (root_node);
	root_node = NULL;
	mention_error ("Not enough memory", ERC_NO_MEMORY | ERR21);
    }
    if ((entry_color = RemAlloc ((ULONG) (most_entrys * sizeof (BOOL)),
		    MEMF_CLEAR)) == NULL) {
	free_tree (root_node);
	root_node = NULL;
	mention_error ("Not enough memory", ERC_NO_MEMORY | ERR22);
    }
	}

    clear_cancel_req ();
    DBUG_VOID_RETURN;
}



static struct TreeNode *do_load ( void )
{
	struct TreeNode *tn;
	char buf[64];
	LONG l, size, ver;


	l = strlen(LOGFILE_ID);
    if (Read (fh, buf, l) != l)
	{
		log_err = ERC_IO_FAILED | ERR58;
		return( NULL );
    }

	if(  strncmp( buf, LOGFILE_ID, l ) != 0  )
	{
		/* This is NOT a logfile! */
		log_err = ERC_NONE | ERR59;
		return( NULL );
	}


	/* Read the version in.
	 * This can be used later to accomodate different logfile formats
	 * if neccessary.  For now we ignore it.
	 */
    if (Read (fh, (char *)(&ver), 4) != 4)
	{
		log_err = ERC_IO_FAILED | ERR63;
		return( NULL );
    }

	/* Read the file size */
    if (Read (fh, (char *)(&size), 4) != 4)
	{
		log_err = ERC_IO_FAILED | ERR60;
		return( NULL );
    }

	/* And check it */
	if(  size != get_file_size( fh )  )
	{
		log_err = ERC_IO_FAILED | ERR61;
		return( NULL );
	}

	tn = load_node( NULL );

	return( tn );
}



static struct TreeNode *load_node ( struct TreeNode *parent )
{
    ULONG nodelen;
    struct TreeNode *tn;
    struct TreeEntry *te;
    int i;

    DBUG_ENTER ("load_node");

    /* Check for user saying stop */
    if (check_cancel () == TRUE)
	{
		log_err = ERC_USER | ERR20;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

    /* Get size of this node */
    if (Read (fh, (char *) &nodelen, 4L) != 4L)
	{
		log_err = ERC_IO_FAILED | ERR21;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

    if ((tn = AllocMem (nodelen, MEMF_PUBLIC)) == NULL)
	{
		log_err = ERC_NO_MEMORY | ERR22;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

    if (Read (fh, (char *) tn, nodelen) != nodelen)
	{
		FreeMem (tn, nodelen);
		log_err = ERC_IO_FAILED | ERR23;
		DBUG_RETURN ((struct TreeNode *) NULL);
    }

    /* We have now created a filled node in memory */
    /* Set the size based on what it was saved as (may differ from what
     * was originaly in the tn -> Size field).  This is done to support
     * the tree trimming function.
     */
    tn -> Size = nodelen;

	tn -> Parent = parent;

    /* Now move through it, loading it's children */
    for (i = 0; i < tn -> Entries; i++)
	{
		te = find_entry (tn, i);

		if (FlagIsSet (te -> Flags, ENTRY_IS_DIR) &&
			(te -> Size != NULL))
		{
		    /* Recurse */
		    te -> Size = (ULONG) load_node ( tn );
		}
    }

    DBUG_RETURN (tn);
}



/* DTM_NEW */

static LONG get_file_size( BPTR fh )
{
	LONG opos, size=-1;


	opos = Seek( fh, 0, OFFSET_END );
	if( opos != -1 )
	{
		size = Seek( fh, opos, OFFSET_BEGINNING );
	}

	return( size );
}

