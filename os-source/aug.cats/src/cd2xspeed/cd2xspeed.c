Date; /*

SC LINK MODIFIED NOSTKCHK NOICONS CD2XSpeed
quit

*/

/*********************************
           CD2XSpeed.c

 Turn cd double read speed on/off.

          W.D.L 930420
**********************************/

// 930916    Now goes away quietly upon failure, so its always safe to use.

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
#include <exec/io.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>

#include "devices/cd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// for setmem()

#include "cd2xspeed_rev.h"

#define TEMPLATE	"ON/S,OFF/S" VERSTAG " Wayne D. Lutz"
#define	OPT_ON		0
#define	OPT_OFF		1
#define	OPT_COUNT	2

STATIC LONG		  opts[OPT_COUNT];
STATIC struct RDArgs	* rdargs;
STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;
STATIC struct IOStdReq	* CDDeviceMReq;

IMPORT struct Library * SysBase;

/*
 *  SendIOR -- asynchronously execute a device command
 */
BOOL
SendIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data   = data;

    SendIO( (struct IORequest *)req);

    if ( req->io_Error ) {
//	printf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()

/*
 * Send a CD_CONFIG command to cd.device.
 */
BOOL
CDConfig( ULONG tag, ... )
{
    SendIOR( CDDeviceMReq, CD_CONFIG, NULL, 0, &tag );

    if ( CDDeviceMReq->io_Error ) {
//	printf("\n!!!CD_CONFIG ERROR!!! io_Error= %ld\n\n",CDDeviceMReq->io_Error);
	return( FALSE );
    }

    WaitIO( (struct IORequest *)CDDeviceMReq );

    return( TRUE );

} // CDConfig()


/*
 * Close cd.device.
 */
VOID
CDDeviceTerm( VOID )
{
    if ( CDDeviceMReq ) {
	if ( CDDevice ) {

	    CloseDevice( (struct IORequest *)CDDeviceMReq );
	    CDDevice = NULL;
	}

	DeleteStdIO( CDDeviceMReq );
	CDDeviceMReq = NULL;
    }

    if ( CDPort ) {
	DeleteMsgPort( CDPort );
	CDPort = NULL;
    }

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
	    if ( CDDeviceMReq = CreateStdIO( CDPort ) ) {

		if ( !OpenDevice( "cd.device", 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
		    CDDevice = CDDeviceMReq->io_Device;
		}
	    }
	}

	if ( !CDDevice ) {
	    return( FALSE );
	}

	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // CDDeviceInit()


VOID
main( LONG argc,char * argv[] )
{
    int speed;

    // workbench
    if ( argc == 0 )
	exit( RETURN_OK );

    if ( SysBase->lib_Version < 36 )
	exit( RETURN_OK );

    setmem( opts, sizeof (opts) ,0 );

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_OK );
    }

    if ( !CDDeviceInit( NULL ) ) {
//	printf("Could NOT open cd.device Aborting\n");
	exit( RETURN_OK );
    }

    speed = opts[OPT_OFF] ? 75 : 150;

    CDConfig( TAGCD_READSPEED, speed, 0 );

    CDDeviceTerm();

    FreeArgs( rdargs );

    exit( RETURN_OK );

} // main()

