/*
 * Amiga Grand Wack
 *
 * envoylink.c -- Routines to handle target connected via Envoy.
 *
 * $Id: envoylink.c,v 39.8 93/11/05 15:02:22 jesup Exp $
 *
 * Routines to perform remote communication via Envoy networking.
 * These routines are called by the generic versions in link.c.
 *
 */

#include <exec/memory.h>
#include <exec/semaphores.h>
#include <utility/tagitem.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

#include "link.h"
#include "envoylink.h"
#include "validmem.h"
#include "asm_proto.h"
#include "link_proto.h"
#include "envoylink_proto.h"
#include "io_proto.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern struct LinkLibBase *LinkLibBase;

extern struct Library *SysBase;

extern long remote_error;
extern struct Task *WackTask;

/*ZZZ move to main loop */LONG __asm Envoy_Reconnect( void );
void __asm Envoy_ReleaseValidMem( register __a0 struct MinList *validlist );

static char envoy_connection[100];
static STRPTR remote_machine;
static struct Library *NIPCBase = NULL;
static struct Entity *entity = NULL;
static struct Entity *remoteentity = NULL;
static struct Transaction *trans = NULL;
static struct MsgPort *transactionport = NULL;
static struct SignalSemaphore transaction_sem;
static struct RemoteWackCommand rwc;

static struct TagItem cetags[] =
{
    ENT_AllocSignal, 0,
    TAG_END, 0
};

struct EnvoyValidMem
{
    struct MinList eml_List;
    struct MemoryBlock *eml_Array;
};


ULONG
doEnvoyTransaction( ULONG action, APTR address, ULONG data, void *response,
    ULONG responselength )
{
    ULONG alien = 0;
    struct MsgPort *port;

    if ( !remote_error )
    {
	Envoy_Reconnect();	/* ZZZ Move to main loop */
	if ( remoteentity )
	{
	    ObtainSemaphore( &transaction_sem );
	    if ( FindTask( NULL ) == WackTask )
	    {
		/* This is Wack itself running, so use its transaction MsgPort */
		port = transactionport;
	    }
	    else
	    {
		/* Someone's called me through the fake library feature,
		 * and they're not my task.  Therefore, I have to allocate
		 * my own MsgPort and use that for replying to.
		 */
		alien = 1;
		if ( !( port = CreateMsgPort() ) )
		{
		    remote_error = MYENVOYERR_ALLOCFAIL;
		}
	    }
	    if ( !remote_error )
	    {
		rwc.rwc_Action = action;
		rwc.rwc_Address = address;
		rwc.rwc_Data = data;
		trans->trans_ResponseData = response;
		trans->trans_RespDataLength = responselength;
		trans->trans_Timeout = 3;
	
		trans->trans_Msg.mn_ReplyPort = port;
		BeginTransaction( remoteentity, entity, trans );
		WaitPort( port );
		GetMsg( port );

		if ( trans->trans_Error )
		{
		    PPrintf("Connection to machine %s broken\n", remote_machine );
		    remote_error = trans->trans_Error;
		    LoseEntity( remoteentity );
		    remoteentity = 0;
		    Envoy_ReleaseValidMem( LinkLibBase->ll_ValidMem );
		    LinkLibBase->ll_ValidMem = 0;
		}
	    }
	    if ( alien )
	    {
		if ( port )
		{
		    DeleteMsgPort( port );
		}
	    }
	    ReleaseSemaphore( &transaction_sem );
	}
	else
	{
	    remote_error = MYENVOYERR_LOSTCONTACT;
	}
    }
    return( trans->trans_RespDataActual );
}


struct MinList * __asm
Envoy_ObtainValidMem( void )
{
    ULONG count;
    struct MemoryBlock *marray;
    struct EnvoyValidMem *validlist = NULL;

    doEnvoyTransaction( RWCA_GETVALIDMEMCOUNT, 0, 0, &count, 4 );

    if ( !remote_error )
    {
	marray = AllocVec( count * sizeof( struct MemoryBlock ), MEMF_CLEAR );
	doEnvoyTransaction( RWCA_GETVALIDMEM, 0, 0, marray,
	    count * sizeof( struct MemoryBlock ) );

	if ( !remote_error )
	{
	    if ( validlist = AllocMem( sizeof( struct EnvoyValidMem ), MEMF_ANY ) )
	    {
		NewList( (struct List *)validlist );
		validlist->eml_Array = marray;

		while ( count-- )	
		{
		    AddTail( (struct List *)validlist, (struct Node *)&marray[ count ] );
		}
	    }
	}
    }
    if ( !validlist )
    {
	FreeVec( marray );
    }
    return( (struct MinList *)validlist );
}


void __asm
Envoy_ReleaseValidMem( register __a0 struct MinList *validlist )
{
    if ( validlist )
    {
	FreeVec( ((struct EnvoyValidMem *)validlist)->eml_Array );
	FreeMem( validlist, sizeof( struct EnvoyValidMem ) );
    }
}


STRPTR __asm
Envoy_Connect( register __a0 STRPTR machine )
{
    STRPTR errorstr = "Failed to open nipc.library";

    remote_machine = machine;
    sprintf( envoy_connection, "Envoy %s", remote_machine);
    InitSemaphore( &transaction_sem );

    if (NIPCBase = OpenLibrary("nipc.library", 0L))
    {
	errorstr = "Envoy allocation failure";
	if ( entity = CreateEntityA( (struct TagItem *) &cetags ) )
	{
	    if ( trans = AllocTransactionA( NULL ) )
	    {
		trans->trans_RequestData = &rwc;
		trans->trans_ReqDataLength = 
		    trans->trans_ReqDataActual = sizeof( struct RemoteWackCommand );

		if ( transactionport = CreateMsgPort() )
		{
		    errorstr = "Remote Wack server not found";
		    if ( remoteentity = FindEntity( remote_machine, "RemoteWack", entity, NULL))
		    {
			errorstr = "Failed to build remote valid memory list";
			if ( LinkLibBase->ll_ValidMem = Envoy_ObtainValidMem() )
			{
			    errorstr = NULL;
			}
		    }
		}
	    }
	}
    }
    return( errorstr );
}


void __asm
Envoy_Disconnect( void )
{
    Envoy_ReleaseValidMem( LinkLibBase->ll_ValidMem );

    if ( remoteentity )
    {
	LoseEntity( remoteentity );
    }
    if ( trans )
    {
	FreeTransaction( trans );
    }
    if ( entity )
    {
	DeleteEntity( entity );
    }
    if ( transactionport )
    {
	DeleteMsgPort( transactionport );
    }
    if ( NIPCBase )
    {
	CloseLibrary( NIPCBase );
    }
}

/* Result is:
 * 0 = was not disconnected in the first place
 * 1 = was disconnected, successfully reconnected
 * -1 = was disconnected, failed to reconnect.
 */
LONG __asm
Envoy_Reconnect( void )
{
    LONG result = 0;	/* Assume no need to reconnect */

    if ( !remoteentity )
    {
	result = -1;	/* Assume failed to reconnect */
	/* attempt to reconnect */
	if ( remoteentity = FindEntity( remote_machine, "RemoteWack", entity, NULL ) )
	{
	    if ( LinkLibBase->ll_ValidMem = Envoy_ObtainValidMem() )
	    {
		result = 1;
	    }
	    else
	    {
		LoseEntity( remoteentity );
		remoteentity = 0;
	    }
	}
    }
    return( result );
}


ULONG __asm
Envoy_Resume( void )
{
    return( 0 );	/* Not supported */
}


ULONG __asm
Envoy_Lock( void )
{
    return( 0 );	/* Not supported */
}


ULONG __asm
Envoy_Unlock( void )
{
    return( 0 );	/* Not supported */
}


BYTE __asm
Envoy_ReadByte( register __a0 APTR addr )
{
    LONG response = 0;

    doEnvoyTransaction( RWCA_READBYTE, addr, 0, &response, 4 );
    return( (BYTE) response );
}


WORD __asm
Envoy_ReadWord( register __a0 APTR addr )
{
    LONG response = 0;

    doEnvoyTransaction( RWCA_READWORD, addr, 0, &response, 4 );
    return( (WORD) response );
}


LONG __asm
Envoy_ReadLong( register __a0 APTR addr )
{
    LONG response = 0;

    doEnvoyTransaction( RWCA_READLONG, addr, 0, &response, 4 );
    return( (LONG) response );
}


void __asm
Envoy_ReadBlock( register __a0 APTR addr, register __a1 APTR buffer,
    register __d0 ULONG size )
{
    doEnvoyTransaction( RWCA_READBLOCK, addr, size, buffer, size );
}


ULONG __asm
Envoy_ReadString( register __a0 APTR addr, register __a1 STRPTR buffer,
    register __d0 ULONG maxlen )
{
    return( doEnvoyTransaction( RWCA_READSTRING, addr, maxlen, buffer, maxlen ) );
}


void __asm
Envoy_WriteByte( register __a0 APTR addr, register __d0 BYTE data )
{
    doEnvoyTransaction( RWCA_WRITEBYTE, addr, data, NULL, 0 );
}


void __asm
Envoy_WriteWord( register __a0 APTR addr, register __d0 WORD data )
{
    doEnvoyTransaction( RWCA_WRITEWORD, addr, data, NULL, 0 );
}


void __asm
Envoy_WriteLong( register __a0 APTR addr, register __d0 LONG data )
{
    doEnvoyTransaction( RWCA_WRITELONG, addr, data, NULL, 0 );
}


ULONG __asm
Envoy_Context( void )
{
    return( 0 );	/* Not supported */
}

ULONG __asm
Envoy_ShowContext( void )
{
    return( 0 );	/* Not supported */
}


STRPTR __asm
Envoy_Connection( void )
{
    return( envoy_connection );
}


/* Perform a protected (i.e. Disable()d) read of an Exec list, filling
 * the supplied array with pointers to the Nodes.
 * The array runs from 0..maxcount inclusive.
 * Return values:
 *	 0 = everything is OK
 *	<0 = an error occurred
 *	>0 = the array is not big enough, feel free to try again.
 */
LONG __asm
Envoy_ReadNodeArray( register __a0 struct List *addr,
    register __a1 struct Node **array, register __d0 ULONG maxcount )
{
    LONG result = 0;

    doEnvoyTransaction( RWCA_READNODEARRAY, addr, maxcount, array, (maxcount+1)*sizeof(long) );
    if ( remote_error )
    {
	result = -1;
    }
    else if ( array[ 0 ] == RWCA_RNA_TOOSMALL )
    {
	array[ 0 ] = 0;
	result = 1;
    }
    return( result );
}


/* We are called with an error number (possibly zero).  If we recognize
 * the error as ours, return an error string, else return NULL.
 */

STRPTR __asm
Envoy_Error( register __d0 ULONG error )
{
    STRPTR errorstr = NULL;

    /* Envoy errors run from 1-1000. 1001-1010 are for my own custom
     * networking errors.
     */
    if ( ( error >= 1 ) && ( error < 1010 ) )
    {
	switch ( error )
	{
	case ENVOYERR_CANTDELIVER:
	    errorstr = "Envoy Error: can't deliver transaction";
	    break;

	case ENVOYERR_TIMEOUT:
	    errorstr = "Envoy Error: transaction timed out";
	    break;

	case MYENVOYERR_ALLOCFAIL:
	    errorstr = "Envoy Error: allocation failure";
	    break;

	case MYENVOYERR_LOSTCONTACT:
	    errorstr = "Envoy Error: connection broken";
	    break;

	default:
	    errorstr = "Envoy Error: unknown error %ld";
	    break;
	}
    }
    return( errorstr );
}


ULONG __asm
Envoy_Symbol( register __a0 APTR addr )
{
    return( 0 );	/* Not supported */
}


void
Envoy_InitLib( APTR lfl )
{
    long entry = LINK_SKIPENTRIES;

    initLinkLVO( lfl, entry++, Envoy_Connect );
    initLinkLVO( lfl, entry++, Envoy_Disconnect );
    initLinkLVO( lfl, entry++, Envoy_Reconnect );
    initLinkLVO( lfl, entry++, Envoy_Resume );

    initLinkLVO( lfl, entry++, Envoy_Lock );
    initLinkLVO( lfl, entry++, Envoy_Unlock );

    initLinkLVO( lfl, entry++, Envoy_ReadByte );
    initLinkLVO( lfl, entry++, Envoy_ReadWord );
    initLinkLVO( lfl, entry++, Envoy_ReadLong );
    initLinkLVO( lfl, entry++, Envoy_ReadBlock );
    initLinkLVO( lfl, entry++, Envoy_ReadString );

    initLinkLVO( lfl, entry++, Envoy_WriteByte );
    initLinkLVO( lfl, entry++, Envoy_WriteWord );
    initLinkLVO( lfl, entry++, Envoy_WriteLong );
    initLinkLVO( lfl, entry++, lib_stubReturn/*Envoy_WriteBlock*/ );
    initLinkLVO( lfl, entry++, lib_stubReturn/*Envoy_WriteString*/ );

    initLinkLVO( lfl, entry++, Envoy_Context );
    initLinkLVO( lfl, entry++, Envoy_ShowContext );
    initLinkLVO( lfl, entry++, Envoy_Connection );
    initLinkLVO( lfl, entry++, Envoy_Error );
    initLinkLVO( lfl, entry++, Envoy_Symbol );
    initLinkLVO( lfl, entry++, Envoy_ReadNodeArray );
    initLinkLVO( lfl, entry++, lib_stubReturn/*Envoy_Call*/ );
}
