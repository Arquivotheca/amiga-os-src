/*	make.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "make.h"
#include "depend.h"

static int make_implicit( char *goalname, int *remake_flag );

/*
 *		Get the modification time of a file.  If the file
 *		doesn't exist, it's modtime is set to 0.
 */

static time_t
modtime( char *filename )
{
	struct FileInfoBlock *fib;
	BPTR myLock;
	long ioErr;
	time_t mtime = 0L;

	fib = (struct FileInfoBlock *)malloc( sizeof(struct FileInfoBlock));
	if( fib ) {
		if( !(myLock = Lock( filename, ACCESS_READ ))) {
			if( (ioErr = IoErr()) != ERROR_OBJECT_NOT_FOUND)
				logprintf( "Can't Lock '%s'; error %ld\n", filename, ioErr );
		}
		else if( !Examine( myLock, fib )) {
			UnLock(myLock);
			logprintf( "Can't Examine '%s'; error %ld", filename, IoErr() );
		}
		else {
			mtime = fib->fib_Date.ds_Tick / TICKS_PER_SECOND +
				 60*fib->fib_Date.ds_Minute + 86400 * fib->fib_Date.ds_Days;
			UnLock( myLock );
		}
		free( fib );
	}
	return( mtime );
}

/* return true if targ1 is newer than targ2 OR if either is missing */
int
isnewer( char *targ1, char *targ2 )
{
	time_t t1, t2;
	long diff;
	int retval;

	t1 = modtime( targ1 );
	t2 = modtime( targ2 );
	if( !t1 || !t2 )
		retval = 1;
	else {
		diff = (long)(t1 - t2);
		retval = (diff > 0L) ? 1 : 0;
	}

	debugprintf( 5, ("isnewer %s=%ld,%s=%ld diff=%ld: %s\n", targ1, t1,
		targ2, t2, diff, (retval) ? "yes" : "no" ));

	return( retval );
}

/* recursively make a target filename */
int
make_filename( const char *goalname, int *made )
{
	int remake_flag = (Param.all_mode) ? 1 : 0;	
	struct depend *dep;
	struct List *cmdlist = NULL;
	struct target *goal = find_target( goalname );
	char *depname = NULL;

	Global.recursion_level++;
	*made = (Param.all_mode) ? 1 : 0;

	if( Global.recursion_level == 1 ) {
		logprintf( "\tmake( %s )\n", goalname );
	}
	else  {
		debugprintf( 1, ( "\n\tmake %d ( %s )\n", Global.recursion_level,
		goalname ));
	}
	if( goal ) {
		if( (goal->flags & TF_NEVER) ||
			((goal->flags & (TF_ONCE | TF_MADE)) == (TF_ONCE | TF_MADE)) ) {
			logprintf( "\tskipping %s\n", goalname );
			return( 0 );
		}
		if( goal->dependlist.lh_Head->ln_Succ ) {
			for( dep = (struct depend *)goal->dependlist.lh_Head; 
				dep->node.ln_Succ; dep = dep->node.ln_Succ ) {
				int made_flag = 0;

				debugprintf( 1, ("%s depends on %s\n", goalname, dep->name ));
				if( make_filename( dep->name, &made_flag ))
					return( 1 );
				if( made_flag || (!(goal->flags & TF_PHONY) &&
					isnewer( dep->name, goalname ))) {
					depname = dep->name;
					remake_flag = 1;
					debugprintf( 4, ("%s was made\n", dep->name ));
				}
			} /* for */
		} 
		else
			remake_flag = 1; /* has no dependencies */
		cmdlist = (goal->flags & TF_OWNER ) ? &goal->commandlist :
			goal->alternate;
	} /* if */

	/* if no explicit rule for goal OR goal has no commands */
	if( !goal || (!cmdlist->lh_Head->ln_Succ && !(goal->flags & TF_PHONY))) {
		int retval = make_implicit( goalname, &remake_flag );
		--Global.recursion_level;
		if( retval == -1 ) {
			if( !goal )
				logprintf( "don't know how to make %s\n", goalname );
			else
				retval = 0;
		}
		*made = remake_flag;
		return( retval );
	}
	else {
		--Global.recursion_level;
		if( (remake_flag || (goal->flags & TF_ALWAYS))
			&& cmdlist->lh_Head->ln_Succ ) {
			int retval;

			set_target_macros( goalname, depname );
			retval = recipe( goalname, cmdlist );
			set_target_macros( NULL, NULL );
			if( retval )
				return( 1 );
			goal->flags |= TF_MADE;
		}
		else if( !remake_flag ) {
			debugprintf( 2,("%s is up to date\n", goal->name ));
		}
	}
	if( !(goal->flags & TF_INVIS) )
		*made = remake_flag;
	debugprintf( 4, ("%s remake_flag = %d\n", goalname, remake_flag ));
	return( 0 );
}

/*	use this inference engine as a last resort
 *	use pattern rules to determine dependencies and commands for goalname
 */
static int
make_implicit( char *goalname, int *remake_flag )
{
	char *depfile = (char *)malloc( MAXPATHNAME );
	struct patternrule *pr;
	struct List *cmdlist;
	struct target *goal;
	int makeit_flag = (Param.all_mode) ? 1 : *remake_flag;

	debugprintf( 1, ("\tmake_implicit( %s,%d)\n", goalname, *remake_flag ));
				
	if( !depfile )
		goto death; /* no mem */

	*depfile = (char)0;
	for( pr = &Global.patternlist.lh_Head; pr->node.ln_Succ;
		pr = pr->node.ln_Succ ) {
		if( pattern_match( pr->tar_pat, goalname )) {
			debugprintf( 2, ("Matched Pattern rule %s: %s to %s\n",
				pr->tar_pat, pr->dep_pat, goalname ));
			if( *pr->dep_pat ) {
				map_to_pattern( depfile, pr->tar_pat, pr->dep_pat, goalname );
				if( !access( depfile, 0 )) { /* found it */
					int made_flag;

					debugprintf( 2, ("double pattern rule matches %s\n",
						depfile ));
					if( make_filename( depfile, &made_flag ))
						goto death;
					if( made_flag || isnewer( depfile, goalname ))
						makeit_flag = 1;
					break; /* found it */
				} /* else not the right rule, continue to find another */
			}
			else {
				debugprintf( 2, ("single suffix rule applies\n" ));
				makeit_flag = 1;
				break; /* found it */
			}
		} /* if */
	} /* for each suffix rule */

	goal = ( pr->node.ln_Succ ) ? pr->targ : default_target;
	if( !goal ) {
		free( depfile );
		debugprintf( 4, ("returning -1, remake_flag = %d\n", *remake_flag ));
		return( -1 );
	}
	if( makeit_flag ) {
		cmdlist = (goal->flags & TF_OWNER ) ?
			&goal->commandlist : goal->alternate;
		if( cmdlist && cmdlist->lh_Head->ln_Succ ) {
			set_target_macros( goalname, depfile );
			if( recipe( goalname, cmdlist ))
				goto death;
			set_target_macros( NULL, NULL );
			if( !(goal->flags & TF_INVIS) )
				*remake_flag = 1;
		}
	}
	free( depfile );
	debugprintf( 4, ("implicit %s remake_flag = %d\n", goalname, *remake_flag ));
	return( 0 );
death:
	if( depfile )
		free( depfile );
	return( 1 );
}
