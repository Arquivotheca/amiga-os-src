/* kstuff.c -- fancier stuff like kprintf()
 * $Id: kstuff.c,v 38.0 91/06/12 14:25:49 peter Exp $
 */

#include "intuall.h"

kdebug_hook()
{
    struct IntuitionBase	*IBase = fetchIBase();

    kprintf("\nIBase Debug flag at %lx\n", &IBase->DebugFlag );
    kprintf("Flag values: \n");
    kprintf("\tsprite debugging on: %lx\n", IDF_SPRITE );
    kprintf("\tdamage pattern and delay: %lx\n", IDF_BPATTERN );
    kprintf("\tdamage debug pause: %lx\n", IDF_BPAUSE );
    kprintf("\tgeneral debug pause: %lx\n", IDF_PAUSE );
    kprintf(
"New DebugFlag value (in decimal, current %ld, default 0, ignore the echo):",
	IBase->DebugFlag );

    IBase->DebugFlag = kgetnum();
    kprintf("\nnew value (decimal %ld hex %lx)\n",
	IBase->DebugFlag, IBase->DebugFlag );
}

kpause( str )
char	*str;
{
    kprintf("\n%s  (press any key) ", str );
    kgetchar();
    kprintf( "\n" );
}


/*
 * kprintf's string, getchars back a response
 */
BOOL
kquery( str )
char	*str;
{
    int		response;
    BOOL	retval;

    kprintf("%s [y]: ", str );

    response = kgetchar();
    kprintf("\n");
    retval =  ( response == 'y' || response == 'Y' || response == '\r' );
    /* kprintf("response is: %lx, retval = %ld \n", response, retval ); */
    
    return ( retval );
}

/* if appropriate DebugFlag set, will pause and ask if you want to Debug()*/
kdebug()
{
    if ( TESTFLAG( fetchIBase()->DebugFlag, IDF_PAUSE ) )
    {
	if ( kquery( "debug? (n)" ) ) Debug();
    }
}
