/*	log.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include "make.h"

#define LOGFILE "make.log"

static int
open_logfile( void )
{
	if( !Global.logfile ) {
		Global.logfile = fopen( LOGFILE, "a" );
		if( !Global.logfile )
			return( 1 );
	}
	return( 0 );
}

void
close_logfile( void )
{
	if( Global.logfile )
		fclose( Global.logfile );
}

void
logfile( char *string )
{
	FILE *out;

	if( Param.log && !Global.logfile )
		open_logfile();

	out = ( Param.log ) ? Global.logfile : stdout;
	if( Param.log || Param.verbosity ) {
		fputs( string, out );
		fputc( '\n', out );
		fflush( out );
	}
}

void
logprintf( const char *fmt, ... )
{
	va_list argptr;
	FILE *out;

	va_start( argptr, fmt );
	if( Param.log && !Global.logfile )
		open_logfile();

	out = ( Param.log ) ? Global.logfile : stdout;
	if( Param.log || Param.verbosity ) {
		vfprintf( out, fmt, argptr );
	}
	va_end( argptr );
}
