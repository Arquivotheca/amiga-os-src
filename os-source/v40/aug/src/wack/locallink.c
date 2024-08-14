/*
 * Amiga Grand Wack
 *
 * locallink.c -- Routines to handle local target.
 *
 * $Id: locallink.c,v 39.8 93/11/05 14:57:57 jesup Exp $
 *
 * Routines to communicate with the system Wack is running on.  These
 * routines are called by generic versions in link.c.
 *
 */

#include "link.h"
#include "sys_proto.h"
#include "link_proto.h"
#include "locallink_proto.h"
#include "validmem_proto.h"

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
extern struct Library *SysBase;

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern struct LinkLibBase *LinkLibBase;


struct MinList * __asm
Local_ObtainValidMem( void )
{
    return( buildValidMem() );
}


void __asm
Local_ReleaseValidMem( register __a0 struct MinList *validlist )
{
    freeValidMem( validlist );
}


STRPTR __asm
Local_Connect( register __a0 STRPTR machine )
{
    STRPTR errorstr = "Allocation failure";
    if ( LinkLibBase->ll_ValidMem = Local_ObtainValidMem() )
    {
	errorstr = NULL;
    }
    return( errorstr );
}


void __asm
Local_Disconnect( void )
{
    Local_ReleaseValidMem( LinkLibBase->ll_ValidMem );
    LinkLibBase->ll_ValidMem = NULL;
}


/* Result is:
 * 0 = was not disconnected in the first place
 * 1 = was disconnected, successfully reconnected
 * -1 = was disconnected, failed to reconnect.
 */
LONG __asm
Local_Reconnect( void )
{
    return( 0 );
}


ULONG __asm
Local_Resume( void )
{
    /* Not supported */
    return( 0 );
}


ULONG __asm
Local_Call( register __a0 APTR addr )
{
    void (* __far func)(void) = (void *) addr;

    (*func)();

    return( 1 );
}


ULONG __asm
Local_Lock( void )
{
    Forbid();
    return( 1 );
}


ULONG __asm
Local_Unlock( void )
{
    Permit();
    return( 1 );
}


BYTE __asm
Local_ReadByte( register __a0 APTR addr )
{
    return( *((char *)addr) );
}


WORD __asm
Local_ReadWord( register __a0 APTR addr )
{
    return( *((short *)addr) );
}


LONG __asm
Local_ReadLong( register __a0 APTR addr )
{
    return( *((long *)addr) );
}


void __asm
Local_ReadBlock( register __a0 APTR addr, register __a1 APTR buffer,
    register __d0 ULONG size )
{
    CopyMem( addr, buffer, size );
}


ULONG __asm
Local_ReadString( register __a0 APTR addr, register __a1 STRPTR buffer,
    register __d0 ULONG maxlen )
{
    unsigned long len = strlen( addr )+1;
    if ( len > maxlen )
    {
    	len = maxlen;
    }
    Local_ReadBlock( addr, buffer, len );
    buffer[ maxlen-1 ] = 0;
    return( len );
}


void __asm
Local_WriteByte( register __a0 APTR addr, register __d0 BYTE data )
{
    *((UBYTE *)addr) = data;
}


void __asm
Local_WriteWord( register __a0 APTR addr, register __d0 WORD data )
{
    *((UWORD *)addr) = data;
}


void __asm
Local_WriteLong( register __a0 APTR addr, register __d0 LONG data )
{
    *((ULONG *)addr) = data;
}


void __asm
Local_WriteBlock( register __a0 APTR addr, register __a1 APTR buffer,
    register __d0 ULONG size )
{
    CopyMem( buffer, addr, size );
}


ULONG __asm
Local_Context( void )
{
    /* Not supported */
    return( 0 );
}


ULONG __asm
Local_ShowContext( void )
{
    /* Not supported */
    return( 0 );
}


STRPTR __asm
Local_Connection( void )
{
    return( (STRPTR) "Local" );
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
Local_ReadNodeArray( register __a0 struct List *addr,
    register __a1 struct Node **array, register __d0 ULONG maxcount )
{
    struct Node *node;
    long count = 0;
    long result;

    Disable();
    /* Assume that everything will be OK */
    result = 0;
    node = addr->lh_Head;
    while ( ( result == 0 ) && ( node->ln_Succ ) )
    {
	count++;
	if ( count <= maxcount )
	{
	    array[count] = node;
	    node = node->ln_Succ;
	}
	else
	{
	    /* Denote that the array was not big enough */
	    result = 1;
	}
    }
    /* Pardonez the cast... */
    array[ 0 ] = (struct Node *)count;
    Enable();
    return( result );
}


/* We are called with an error number (possibly zero).  If we recognize
 * the error as ours, return an error string, else return NULL.
 *
 * We have no errors specific to local memory handling.
 *
 */
STRPTR __asm
Local_Error( register __d0 ULONG error )
{
    return( NULL );
}

ULONG __asm
Local_Symbol( register __a0 APTR addr )
{
    /* Not supported */
    return( 0 );
}


void
Local_InitLib( APTR lfl )
{
    long entry = LINK_SKIPENTRIES;

    initLinkLVO( lfl, entry++, Local_Connect );
    initLinkLVO( lfl, entry++, Local_Disconnect );
    initLinkLVO( lfl, entry++, Local_Reconnect );
    initLinkLVO( lfl, entry++, Local_Resume );

    initLinkLVO( lfl, entry++, Local_Lock );
    initLinkLVO( lfl, entry++, Local_Unlock );

    initLinkLVO( lfl, entry++, Local_ReadByte );
    initLinkLVO( lfl, entry++, Local_ReadWord );
    initLinkLVO( lfl, entry++, Local_ReadLong );
    initLinkLVO( lfl, entry++, Local_ReadBlock );
    initLinkLVO( lfl, entry++, Local_ReadString );

    initLinkLVO( lfl, entry++, Local_WriteByte );
    initLinkLVO( lfl, entry++, Local_WriteWord );
    initLinkLVO( lfl, entry++, Local_WriteLong );
    initLinkLVO( lfl, entry++, Local_WriteBlock );
    initLinkLVO( lfl, entry++, lib_stubReturn/*Local_WriteString*/ );

    initLinkLVO( lfl, entry++, Local_Context );
    initLinkLVO( lfl, entry++, Local_ShowContext );
    initLinkLVO( lfl, entry++, Local_Connection );
    initLinkLVO( lfl, entry++, Local_Error );
    initLinkLVO( lfl, entry++, Local_Symbol );
    initLinkLVO( lfl, entry++, Local_ReadNodeArray );
    initLinkLVO( lfl, entry++, Local_Call );
}
