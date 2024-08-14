/*
 * Amiga Grand Wack
 *
 * sat.c -- Breakpoint and trace routines.
 *
 * $Id: sat.c,v 39.1 93/05/21 17:34:26 peter Exp $
 *
 * As can readily be discerned, this version of Wack didn't come with
 * much breakpoint and trace support.  Perhaps that code can be found
 * from a different breed of Wack, or else re-invented.
 *
 */

#include "std.h"
#include "sat_proto.h"

void
clearBPT( void )
{
}

unsigned long
getBrkCnt( unsigned long addr )
{
 return 0;
}

void
freeCPU( void )
{
}

void
setBrkPt( unsigned long addr )
{
}

void
evalBrkPts( void )
{
}

STRPTR
ToglTrace( void )
{
    return( NULL );
}
