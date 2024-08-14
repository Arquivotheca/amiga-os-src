date ; /*

failat 1
SC LINK MODIFIED NOSTKCHK NOICONS xlmsg TO xlmsg
quit

*/

/*************

  xlmsgtest.c

  W.D.L 930708

**************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

// Tab size is 8!

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xlm.h"

/*
 * print the status bits as a string.
 */
VOID
status2str( ULONG status )
{
    int		  i;
    UBYTE	  str[250];

    static struct status_def {
	ULONG 	  status;
	UBYTE	* str;
    } stat[] = {
	XLM_STATUS_PLAYING,	"XLM_STATUS_PLAYING",
	XLM_STATUS_ABORTING,	"XLM_STATUS_ABORTING",
	0,
    };

    *str = 0;

    for ( i=0; stat[i].status; i++ ) {
	if ( status & stat[i].status ) {
	    if (*str)
		strcat(str,", ");

	    strcat( str, stat[i].str );
	}
    }

    printf("status:\n'%ls'\n\n",str);

} // status2str()


VOID
main( LONG argc,char * argv[] )
{
    UBYTE		* portname;
    struct MsgPort	* port,* replyport;
    XLMESSAGE		* xlm,* reply;

    if ( (argc < 2) || !(portname = argv[1]) )
	exit( RETURN_ERROR );

    if ( replyport = CreatePort(0,0) ) {
	if (xlm = (XLMESSAGE *)AllocMem( sizeof (*xlm), MEMF_PUBLIC | MEMF_CLEAR)) {
	    xlm->msg.mn_Node.ln_Type = NT_MESSAGE;
	    xlm->msg.mn_Length = sizeof (*xlm);
	    xlm->msg.mn_ReplyPort = replyport;

	    xlm->command = XLM_CMD_QUERY;
	    Forbid();
	    if ( port = FindPort( portname ) ) {
		PutMsg( port, (struct Message *)xlm );
	    }
	    Permit();

	    if ( port ) {
		WaitPort( replyport );
		if ( reply = (XLMESSAGE *)GetMsg( replyport ) ) {
		    printf("status= %ld\n",reply->status);
		    status2str( reply->status );
		}
	    }

	    xlm->command = XLM_CMD_ABORT;
	    Forbid();
	    if ( port = FindPort( portname ) ) {
		PutMsg( port, (struct Message *)xlm );
	    }
	    Permit();

	    if ( port ) {
		WaitPort( replyport );
		if ( reply = (XLMESSAGE *)GetMsg( replyport ) ) {
		    printf("status= %ld\n",reply->status);
		    status2str( reply->status );
		}
	    }

	    FreeMem( xlm, sizeof (*xlm) );
	}
	DeletePort( replyport );
    }

    exit( RETURN_OK );

} // main()
