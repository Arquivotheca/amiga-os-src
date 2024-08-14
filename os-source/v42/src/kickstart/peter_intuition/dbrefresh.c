/*** dbrefresh.c *************************************************************
 *
 *  debug code for BorderPatrol refresh window stuff
 *	part of intuition debug.lib
 *
 *  $Id: dbrefresh.c,v 38.1 92/02/11 13:35:15 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

#define REFRESHDELAY	( 300000L )
#define QREFRESHDELAY	( 500000L/4 )


debugRefresh( rp, width, height )
struct RastPort	*rp;
{
    extern USHORT	BPattern[];
    struct IntuitionBase	*IBase = fetchIBase();

    /* just turn the sucker on, OK?	*/
    IBase->DebugFlag =  IDF_BPATTERN | IDF_BPAUSE;

    if ( TESTFLAG( IBase->DebugFlag, IDF_BPATTERN|IDF_BPAUSE ) )
    {

	BlastPattern( rp, 0, 0, width - 1, height - 1,
		BPattern, 1, 3, 0, JAM1);

	if ( TESTFLAG(IBase->DebugFlag, IDF_BPAUSE ) )
	{
	    if (!kquery("keep asking?"))
		    CLEARFLAG(IBase->DebugFlag, IDF_BPAUSE);
	}
	else if ( TESTFLAG(IBase->DebugFlag, IDF_BPATTERN ) )
	{
	    spinDelay( QREFRESHDELAY );
	    BlastPattern( rp, 0, 0, width - 1, height - 1,
		    BPattern, 1, 1, 0, JAM1);
	    spinDelay( QREFRESHDELAY );
	    BlastPattern( rp, 0, 0, width - 1, height - 1,
		    BPattern, 1, 2, 0, JAM1);
	    spinDelay( QREFRESHDELAY );
	    BlastPattern( rp, 0, 0, width - 1, height - 1,
		    BPattern, 1, 3, 0, JAM1);
	    spinDelay( QREFRESHDELAY );
	}

	SetRast( rp, 0 );
    }
}

spinDelay( delay )
LONG	delay;
{
    LONG	delaycnt;
    delaycnt = delay;
    while ( delaycnt-- );
}

waitFrames( numframes )
{
    while ( numframes-- ) WaitTOF();
}
