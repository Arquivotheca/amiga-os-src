/*	main.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>

#include <exec/exec.h>
#include <exec/execbase.h>

#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;

#include "make.h"
#include "depend.h"

/* Globals */
struct globals Global = {
	NULL,	/* me */
	NULL,	/* logfile */
	NULL,	/* screen */
	NULL,	/* window */
	NULL,	/* drinfo */

	{
	(struct Node *)&Global.targetlist.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,						/* lh_Tail */
	(struct Node *)&Global.targetlist.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
	},	/* targetlist */
	{
	(struct Node *)&Global.speciallist.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,						/* lh_Tail */
	(struct Node *)&Global.speciallist.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
	},	/* speciallist */
	{
	(struct Node *)&Global.patternlist.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,						/* lh_Tail */
	(struct Node *)&Global.patternlist.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
	},	/* patternlist */
	{
	(struct Node *)&Global.macrolist.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,						/* lh_Tail */
	(struct Node *)&Global.macrolist.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
	},	/* macrolist */
	0,	/* builtin flag */
	0,	/* recursion level */
	0L	/* oldcwd */
};

static int
open_libraries( void )
{
#ifndef _DCC
	if( !( IntuitionBase = (struct IntuitionBase *)
		OpenLibrary( "intuition.library", 33 ))) {
		return( 1 );
	}
	if( !( GfxBase = (struct GfxBase *)
		OpenLibrary( "graphics.library", 33 ))) {
		return( 1 );
	}
#endif
	return( 0 );
}

static void
close_libraries( void )
{
#ifndef _DCC
	if( IntuitionBase )
		CloseLibrary( IntuitionBase );

	if( GfxBase )
		CloseLibrary( GfxBase );
#endif
}

static int
new_globals( struct globals *globptr )
{
	globptr->me = (struct Process *) FindTask( NULL ); /* find this task */
	return( 0 );
death:
	printf( "problem initializing globals\n" );
	return( 1 );
}

static int
delete_globals( struct globals *globptr )
{
	if( Global.oldcwd ) {
		UnLock( CurrentDir( Global.oldcwd ));
	}

	memset( globptr, 0, sizeof(struct globals));

	/*	just allow exit() to take care of free()ing all of our
	 *	allocations, because I don't feel like doing it right now
	 */
	NewList( &globptr->targetlist );
	NewList( &globptr->speciallist );
	NewList( &globptr->patternlist );
	NewList( &globptr->macrolist );
	return( 0 );
}


static void
die( void )
{
	delete_params();

	close_logfile();

	delete_globals( &Global );
	close_libraries();
}

static int
init( void )
{
	if( open_libraries())
		goto death;
	if( new_globals( &Global ))
		goto death;

	return( 0 );
death:
	return( 1 );
}

static long
do_cl_macro( struct string_node *one )
{
	long retval;
	int made;

	if( strchr( one->data, '=' )) {
		process_macroline( one->data );
		if( !Global.builtin_flag ) {
			logprintf( "\t%s\n", one->data );
			Remove( &one->node );
			delete_snode( one );
		}
	}
	return( 0 );
}

static long
run_one( struct string_node *one )
{
	long retval;
	int made;

	debugprintf( 2, ("\n** run_one Make %s **\n", one->data ));
	retval = (long) make_filename( one->data, &made );
	if( !retval && !made ) {
		logprintf( "\"%s\" is up to date\n", one->data );
	}
	return( retval );
}

static void
run_it( void )
{
	long retval;
	int made;

	if( Param.filelist.lh_Head->ln_Succ )
		(void)for_list( &Param.filelist, do_cl_macro );
	if( Param.filelist.lh_Head->ln_Succ ) {
		(void)for_list( &Param.filelist, run_one );
	}
	else {
		struct target *first_goal;
		for( first_goal = (struct target *)Global.targetlist.lh_Head;
			first_goal->node.ln_Succ;
			first_goal = (struct target *)first_goal->node.ln_Succ ) {
			if( !(first_goal->flags & (TF_PATTERN|TF_BUILTIN )))
				break;
		}
		if( first_goal->node.ln_Succ ) {
			debugprintf( 2, ("\n** first_goal Make %s **\n",
				first_goal->name ));
			retval = make_filename( first_goal->name, &made );
			if( !retval && !made ) {
				logprintf( "\"%s\" is up to date\n", first_goal->name );
			}
		}
	}
	logprintf( "\tMake done.\n" );
}

int
main( int argc, char *argv[] )
{
/*	Allow it to work with limitations on xsystem() and scdir()
	if( SysBase->lib_Version < 36L ) {
		printf( "This program requires Amiga OS 2.0\n" );
		return( 20 );
	}
*/
	atexit( die );	/* add die() to normal exit procedure */

	if( parse_parameters( argc, argv ))
		goto bailout;

	logprintf( "%s %s\n", version_string+7, verdate_string );
	logprintf( "%s\n", copyright_string );

	if( init() )
		goto bailout;
	Global.builtin_flag = 1;
	if( init_builtins())
		goto bailout;
	if( Param.filelist.lh_Head->ln_Succ )
		(void)for_list( &Param.filelist, do_cl_macro );
	Global.builtin_flag = 0;

	if( input_makefile( Param.makefile ))
		goto bailout;
#if DEBUG
	if( Param.print_database )
		dump_all();
	else
		run_it();
#else
	run_it();
#endif

	return( 0 );

bailout:
	/* die(); is called by exit */
	return( 20 );
}

