date ; /* execute to compile with SASC 6.x

;bumprev 1 cdaudio_rev

SC LINK MODIFIED NOSTKCHK NOICONS CDAudio
quit

*/

/*******************

    CDAudio.c

    W.D.L 931108

********************/

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
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/lowlevel.h>
#include <devices/cd.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "cdaudio_rev.h"


#define TEMPLATE    "TRACK/A/N" VERSTAG " Wayne D. Lutz"
#define	OPT_TRACK	0
#define	OPT_COUNT	1


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif


STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;
STATIC struct IOStdReq	* CDReq;
STATIC LONG		  CDPortSignal	= -1;


/*
 * Close cd.device if opened.
 */
VOID
CDDeviceTerm( VOID )
{
    if ( CDReq ) {

	if ( CDDevice ) {
	    CloseDevice( (struct IORequest *)CDReq );
	    CDDevice = NULL;
	}

	if ( CDPort ) {
	    while( GetMsg( CDPort ) );
	}

	DeleteStdIO( CDReq );
	CDReq = NULL;
    }

    if ( CDPort ) {
	while( GetMsg( CDPort ) );
	DeleteMsgPort( CDPort );
	CDPort = NULL;
    }

    CDPortSignal = -1;

} // CDDeviceTerm()


/*
 * Attempts to open cd.device if not already opened.
 * Returns:
 *	retcode: (BOOL) reflects device's open state.
 *  *opened: (BOOL) TRUE if opened by this call.
 */
BOOL
CDDeviceInit( ULONG * opened )
{
    if ( opened )
	*opened = FALSE;

    if ( !CDDevice ) {

	if ( CDPort = CreateMsgPort() ) {
	    if ( CDReq = CreateStdIO( CDPort ) ) {
		if ( !OpenDevice( "cd.device", 0, (struct IORequest *)CDReq, 0 ) ) {
		    CDDevice = CDReq->io_Device;
		}
	    }
	}

	if ( !CDDevice ) {
	    printf("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
		CDPort, CDReq, CDDevice );

	    CDDeviceTerm();
	    return( FALSE );
	}

	CDPortSignal = ( 1 << CDPort->mp_SigBit );

	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // CDDeviceInit()


VOID
CDAudioTerm( VOID )
{
    if( CDReq ) {

	if ( !CheckIO( (struct IORequest *)CDReq ) ) {
	    AbortIO( (struct IORequest *)CDReq );
	}

	WaitIO(  (struct IORequest *)CDReq );
    }

} /* CDAudioTerm() */


PlayTrack( ULONG track )
{
    CDReq->io_Command = CD_PLAYTRACK;
    CDReq->io_Offset = track;
    CDReq->io_Length = 1;
    CDReq->io_Data   = NULL;

    SendIO( (struct IORequest *)CDReq);

    if ( CDReq->io_Error ) {
	printf("PlayTrack io_Error= %ld\n",CDReq->io_Error);
	return( FALSE );
    }

    return( TRUE );

} // PlayTrack()


VOID
main( LONG argc,char * argv[] )
{
    ULONG	track,mask;
    int		ret;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }

    if ( ret = CDDeviceInit( NULL ) ) {
	track = *(ULONG *)opts[OPT_TRACK];

	if ( ret = PlayTrack( track ) ) {
	    mask = CDPortSignal|SIGBREAKF_CTRL_C;
	    Wait( mask );
	}

	CDAudioTerm();
	CDDeviceTerm();
    }

    FreeArgs( rdargs );

    if ( ret ) {
	ret = RETURN_OK;
    } else {
	ret = RETURN_FAIL;
    }

    exit( ret );

} // main()

