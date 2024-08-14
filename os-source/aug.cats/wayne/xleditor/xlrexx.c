/***********

    XlRexx.c

    W.D.L 930615

************/


// Tab size is 8!
#include <exec/types.h>

#include <clib/exec_protos.h>
#include <clib/rexxsyslib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>

#include "rexx/rxslib.h"
#include "rexx/rexxio.h"
#include "rexx/errors.h"


#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"


#define		BASE_PORT_NAME	"XlRexxPort"
#define		REXX_LIBNAME	"rexxsyslib.library"
#define		MAX_PORTNAME_LEN	15

STATIC UBYTE	RxPortName[MAX_PORTNAME_LEN+2] = BASE_PORT_NAME;

struct Library * RexxSysBase;
struct MsgPort * ARexxPort;


VOID
RxTerm( VOID )
{
    struct RexxMsg * msg;

    if ( ARexxPort ) {
	Forbid();

	while ( msg = (struct RexxMsg *)GetMsg( ARexxPort ) ) {
	    msg->rm_Result1 = RC_FATAL;
	    msg->rm_Result2 = NULL;
	    ReplyMsg( (struct Message *)msg );
	}

	DeletePort( ARexxPort );
	ARexxPort = NULL;
	Permit();
    }

    if ( RexxSysBase ) {
	RexxSysBase = NULL;
	CloseLibrary( RexxSysBase );
    }

} // RxTerm();

/*
 * Opens the rexx library and creates our port. Returns the 
 * ports 1<<mp_SigBit if succesful or NULL if not.
 *
 */
ULONG
RxInit( VOID )
{
    int	cnt = 1;

    if ( !(RexxSysBase = OpenLibrary( REXX_LIBNAME, 0l )) )
	return( FALSE );

    while ( strlen( RxPortName ) < MAX_PORTNAME_LEN ) {
	Forbid();
	    if ( !FindPort( RxPortName ) ) {
		ARexxPort = CreatePort( RxPortName, 0 );

		Permit();
		if ( ARexxPort ) {
		    return( 1L << ARexxPort->mp_SigBit );
		} else {
		    RxTerm();
		    return( NULL );
		}
	    }
	Permit();
	sprintf( RxPortName,"%ls%ld",BASE_PORT_NAME,++cnt);
    }

    RxTerm();
    return( NULL );

} // RcInit()


VOID
ProcessRexxMsgs( VOID )
{
    struct RexxMsg	* msg;
    ULONG		  action;
    int			  ret;

    while ( msg = (struct RexxMsg *)GetMsg( ARexxPort ) ) {
	action = msg->rm_Action & RXCODEMASK;

	if ( action == RXCOMM ) {
	    kprintf("RXCOMM: '%ls'\n",msg->rm_Args[0] );
	    ret = msg->rm_Result1 = RC_OK;

	    if ( msg->rm_Action & RXFF_RESULT ) {
		UBYTE * str = ErrorString( ret );

		msg->rm_Result2 = (LONG)CreateArgstring( str, strlen( str ) );
	    }
	} else {
	    kprintf("ProcessRexxMsg() action == %ld\n",action);
	}

	ReplyMsg( (struct RexxMsg *)msg );
    }

} // ProcessRexxMsg()

