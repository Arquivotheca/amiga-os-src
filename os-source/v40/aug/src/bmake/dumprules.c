/*	dumprules.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include "make.h"
#include "depend.h"

#if DEBUG

static void
dump_macros( struct List *mlist )
{
	struct macro *mac;

	debugprintf( 2,( "** Macros **\n" ));

	for( mac = mlist->lh_Head; mac->node.ln_Succ; mac = mac->node.ln_Succ ) {
		logprintf( "%s %s %s\n", mac->name,
			(mac->flags & MF_SIMPLE) ? ":=" : "=", mac->expansion );
	}
	logprintf( "\n" );
}

static void
dump_dependencies( struct List *dlist )
{
	struct depend *dep;

	for( dep = dlist->lh_Head; dep->node.ln_Succ; dep = dep->node.ln_Succ ) {
		logprintf( " %s", dep->name );
	}
	logprintf( "\n" );
}

static void
dump_commands( struct List *clist )
{
	struct command *cmd;

	for( cmd = clist->lh_Head; cmd->node.ln_Succ; cmd = cmd->node.ln_Succ ) {
		logprintf( "\t%s\n", cmd->cmd );
	}
}

static void
dump_targets( struct List *tlist )
{
	struct target *targ;

	debugprintf( 2, ( "** Target Rules **\n" ));

	for( targ = tlist->lh_Head; targ->node.ln_Succ; targ =
		targ->node.ln_Succ ) {
		logprintf( "\n%s", targ->name );
		if( targ->flags & TF_BUILTIN )
			logprintf( " [BUILTIN]" );
		if( targ->flags & TF_PHONY )
			logprintf( " [PHONY]" );
		if( targ->flags & TF_ALWAYS )
			logprintf( " [ALWAYS]" );
		if( targ->flags & TF_NEVER )
			logprintf( " [NEVER]" );
		if( targ->flags & TF_ONCE )
			logprintf( " [ONCE]" );
		logprintf( ":" );
		dump_dependencies( &targ->dependlist );
		dump_commands( ( targ->flags & TF_OWNER ) ?
			&targ->commandlist : targ->alternate );
	}
}

static void
dump_patternrules( struct List *plist )
{
	struct patternrule *pr;

	debugprintf( 2, ( "** Pattern Rules **\n" ));

	for( pr = plist->lh_Head; pr->node.ln_Succ; pr = pr->node.ln_Succ ) {
		logprintf( "\n%s : %s\n", pr->tar_pat, pr->dep_pat );
		dump_commands( ( pr->targ->flags & TF_OWNER ) ?
			&pr->targ->commandlist : pr->targ->alternate );
	}
}


void
dump_all( void )
{
	debugprintf( 2, ("\n** Debug output for rules ** \n" ));
	dump_macros( &Global.macrolist );
	dump_patternrules( &Global.patternlist );
	dump_targets( &Global.targetlist );	
}

#endif
