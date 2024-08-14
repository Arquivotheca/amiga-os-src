/*
 * Amiga Grand Wack
 *
 * menus.c -- Code to handle dynamic allocation of Wack menus.
 *
 * $Id: menus.c,v 39.4 93/05/21 17:33:40 peter Exp $
 *
 * This module contains code to handle binding of menu items to
 * Wack functions, including dynamically adjusting the menu strip,
 * and evaluation of menu operations.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "parse.h"

#include "sys_proto.h"
#include "parse_proto.h"
#include "define_proto.h"
#include "io_proto.h"
#include "menus_proto.h"

extern struct Library *SysBase;
extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;
extern struct Window *iowin;
extern struct VisualInfo *vi;

struct Menu *menu = NULL;
struct NewMenu *wackNewMenu = NULL;


long
init_Menus( void )
{
    return( (long)( wackNewMenu = AllocVec( sizeof( struct NewMenu ), MEMF_CLEAR ) ) );
}

void
exit_Menus( void )
{
    struct NewMenu *nm;
    if ( iowin )
    {
	ClearMenuStrip( iowin );
    }
    if ( menu )
    {
	FreeMenus( menu );
    }
    if ( nm = wackNewMenu )
    {
	while ( nm->nm_Type != NM_END )
	{
	    if ( nm->nm_Label != NM_BARLABEL )
	    {
		FreeVec( nm->nm_Label );
	    }
	    FreeVec( nm->nm_CommKey );
	    FreeVec( nm->nm_UserData );

	    nm++;
	}
	FreeVec( wackNewMenu );
	wackNewMenu = NULL;
    }
}

#define MENUERROR_BADSYNTAX	1
#define MENUERROR_NOMEMORY	2

void
showMenuError( long error )
{
    switch( error )
    {
	case MENUERROR_BADSYNTAX:
	    PPrintf("Bad syntax\n");
	    break;

	case MENUERROR_NOMEMORY:
	    PPrintf("Out of memory\n");
	    break;
    }
}

STRPTR
bindMenuHeader( STRPTR arg )
{
    char header_name[ MAXTOKENLEN ];
    long error;

    if ( arg = parseToken( arg, header_name ) )
    {
	error = createMenuHeader( header_name, arg );
    }
    else
    {
	error = MENUERROR_BADSYNTAX;
    }
    showMenuError( error );

    return( NULL );
}

long
createMenuHeader( STRPTR header_name, STRPTR arg )
{
    return( addMenuEntry( findLastMenu(), NM_TITLE, header_name, arg ) );
}

STRPTR
bindMenuItem( STRPTR arg )
{
    char header_name[ MAXTOKENLEN ];
    char item_name[ MAXTOKENLEN ];
    long error;

    if ( arg = parseToken( parseToken( arg, header_name ), item_name ) )
    {
	error = createMenuItem( header_name, item_name, arg );
    }
    else
    {
	error = MENUERROR_BADSYNTAX;
    }
    showMenuError( error );

    return( NULL );
}

long
createMenuItem( STRPTR header_name, STRPTR item_name, STRPTR arg )
{
    long error = 0;

    /* Create the menu header if not found */
    if ( !findHeader( header_name ) )
    {
	error = createMenuHeader( header_name, arg );
    }
    if ( !error )
    {
	error = addMenuEntry( findLastItem( header_name ),
	    NM_ITEM, item_name, arg );
    }
    return( error );
}

STRPTR
bindMenuSub( STRPTR arg )
{
    char header_name[ MAXTOKENLEN ];
    char item_name[ MAXTOKENLEN ];
    char sub_name[ MAXTOKENLEN ];

    long error = 0;

    if ( arg = parseToken( parseToken( parseToken( arg,
	header_name ), item_name ), sub_name ) )
    {
	/* Create the menu item if not found (and hence header if not found) */
	if ( !findItem( header_name, item_name ) )
	{
	    error = createMenuItem( header_name, item_name, "\"\"" );
	}
	if ( !error )
	{
	    error = addMenuEntry( findLastSub( header_name, item_name ),
		NM_SUB, sub_name, arg );
	}
    }
    else
    {
	error = MENUERROR_BADSYNTAX;
    }
    showMenuError( error );

    return( NULL );
}

/* Returns the index of the last item of the menu strip, PLUS ONE.
 */
long
findLastMenu( void )
{
    long index = 0;

    while ( wackNewMenu[index].nm_Type != NM_END )
    {
	index++;
    }
    return( index );
}

/* Returns the index of the last item of the named menu header, PLUS ONE,
 * or zero for failure.
 */
long
findLastItem( STRPTR header_name )
{
    long index;

    /* findHeader() returns the index of the header entry PLUS ONE,
     * or zero for failure.
     */
    if ( index = findHeader( header_name ) )
    {
	/* Look ahead until a non-(sub)item is found */
	while ( ( wackNewMenu[index].nm_Type == NM_ITEM ) ||
	    ( wackNewMenu[index].nm_Type == NM_SUB ) )
	{
	    index++;
	}
    }
    return( index );
}

/* Returns the index of the last subitem of the named menu header and
 * sub-item, PLUS ONE, or zero for failure.
 */
long
findLastSub( STRPTR header_name, STRPTR item_name )
{
    long index;

    /* findItem() returns the index of the item entry PLUS ONE,
     * or zero for failure.
     */
    if ( index = findItem( header_name, item_name ) )
    {
	/* Look ahead until a non-subitem is found. */
	while ( wackNewMenu[index].nm_Type == NM_SUB )
	{
	    index++;
	}
    }
    return( index );
}


long
findHeader( STRPTR header_name )
{
    long index = 0;
    long found = 0;

    while ( ( !found ) && ( wackNewMenu[index].nm_Type != NM_END ) )
    {
	if ( ( wackNewMenu[index].nm_Type == NM_TITLE ) &&
	    ( !stricmp( wackNewMenu[index].nm_Label, header_name ) ) )
	{
	    found = 1;
	}
	index++;
    }
    if ( !found )
    {
	index = 0;
    }
    return( index );
}


long
findItem( STRPTR header_name, STRPTR item_name )
{
    long found = 0;
    long index;

    if ( index = findHeader( header_name ) )
    {
	while ( ( !found ) && ( wackNewMenu[index].nm_Type != NM_END ) )
	{
	    if ( ( wackNewMenu[index].nm_Type == NM_ITEM ) &&
		( !stricmp( wackNewMenu[index].nm_Label, item_name ) ) )
	    {
		found = 1;
	    }
	    index++;
	}
    }
    if ( !found )
    {
	index = 0;
    }
    return( index );
}




long
addMenuEntry( unsigned long index, UBYTE type, STRPTR label, char *arg )
{
    char *label_store;
    char *function_store = NULL;
    char *commkey_store = NULL;

    char function_name[ MAXTOKENLEN ];
    char commkey[ MAXTOKENLEN ];

    long error = 0;
    long rmerror;

    undoMenus();

    /* If it's a separator bar, then we're all cool (no allocations to
     * do at this stage (anything beginning with a dash is a BARLABEL).
     */
    if ( ( type != NM_TITLE ) && ( label[0] == '-' ) )
    {
	label_store = NM_BARLABEL;
    }
    else
    {
	/* Assume any error would be an out-of-memory error */
	error = MENUERROR_NOMEMORY;

	if ( label_store = AllocVec( strlen(label)+1, MEMF_CLEAR ) )
	{
	    strcpy( label_store, label );

	    if ( type == NM_TITLE )
	    {
		error = 0;
	    }
	    else
	    {
		/* Function name is a required argument if not an NM_TITLE */
		if ( arg = parseToken( arg, function_name ) )
		{
		    if ( function_store = AllocVec( strlen(function_name)+1, MEMF_CLEAR ) )
		    {
			strcpy( function_store, function_name );

			/* commkey is optional */
			if ( parseToken( arg, commkey ) )
			{
			    if ( commkey_store = AllocVec( strlen(commkey)+1,
				MEMF_CLEAR ) )
			    {
				strcpy( commkey_store, commkey );
				error = 0;
			    }
			}
			else
			{
			    error = 0;
			}
		    }
		}
		else
		{
		    error = MENUERROR_BADSYNTAX;
		}
	    }
	}
    }
    if ( !error )
    {
	long count;
	struct NewMenu *new_nm;

	/* How many entries are there in the NewMenu array? */
	count = findLastMenu() + 1;

	/* Reallocate the array, one bigger */
	if ( new_nm = AllocVec( (count+1) * sizeof(struct NewMenu), MEMF_CLEAR ) )
	{
	    /* Our caller passes in 'index', which is the NewMenu array index
	     * of the item after the desired insertion point.  Let's
	     * recopy the NewMenu array, leaving a blank entry before [index].
	     */
	    CopyMem( &wackNewMenu[0], &new_nm[0],
		index*sizeof(struct NewMenu) );
	    CopyMem( &wackNewMenu[index], &new_nm[index+1],
		(count-index)*sizeof(struct NewMenu) );
	    FreeVec( wackNewMenu );
	    wackNewMenu = new_nm;

	    wackNewMenu[index].nm_Type = type;
	    wackNewMenu[index].nm_Label = label_store;
	    wackNewMenu[index].nm_CommKey = commkey_store;
	    wackNewMenu[index].nm_UserData = (APTR) function_store;
	}
	else
	{
	    error = MENUERROR_NOMEMORY;
	}
    }

    if ( rmerror = redoMenus() )
    {
	error = rmerror;
    }

    return( error );
}

void
undoMenus( void )
{
    if ( menu )
    {
	ClearMenuStrip( iowin );
	FreeMenus( menu );
	menu = NULL;
    }
}

long
redoMenus( void )
{
    long error = 0;

    if ( menu = CreateMenus( wackNewMenu,
	GTMN_FullMenu, TRUE,
	TAG_DONE ) )
    {
	LayoutMenus( menu, vi,
	    GTMN_NewLookMenus, TRUE,
	    TAG_DONE );

	SetMenuStrip( iowin, menu );
    }	
    else
    {
	error = MENUERROR_NOMEMORY;
    }
    return( error );
}

struct Menu *
CreateMenus( struct NewMenu *nm, ULONG firsttag, ... )
{
    return( CreateMenusA( nm, (struct TagItem *) &firsttag ) );
}

BOOL
LayoutMenus( struct Menu *menu, struct VisualInfo *vi, ULONG firsttag, ... )
{
    return( LayoutMenusA( menu, vi, (struct TagItem *) &firsttag ) );
}

/* Return value is non-zero to indicate some menu activity occurred.
 * A return value of zero would correspond to a MENUPICK of MENUNULL.
 */
ULONG
EvalMenus( UWORD menucode )
{
    ULONG result = 0;
    struct MenuItem *item;
    STRPTR command;

    while ( menucode != MENUNULL )
    {
	result = 1;
	item = ItemAddress( iowin->MenuStrip, menucode );
	command = (STRPTR) GTMENUITEM_USERDATA( item );

	if ( command )
	{
	    PPrintf("[%s]\n", command );
	    if ( !processLine( command ) )
	    {
		PPrintf("Unknown command\n");
	    }
	}
	else
	{
	    PPrintf("[]\n");
	}
	menucode = item->NextSelect;
    }
    return( result );
}
