/*
 * Amiga Grand Wack
 *
 * sadlink.c -- Routines to handle link connected via SAD.
 *
 * $Id: sadlink.c,v 1.11 93/11/05 15:00:09 jesup Exp $
 *
 * Routines to perform remote communication via SAD.  These routines
 * are called by the generic versions in link.c.
 *
 */

/* #define SAD_PARALLEL to get the parallel.device version we're using
 * internally to debug prototype hardware without a working serial port.
 */

#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <utility/tagitem.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

#include "link.h"
#include "sadlink.h"
#include "validmem.h"

#include "sys_proto.h"
#include "asm_proto.h"
#include "link_proto.h"
#include "sadlink_proto.h"
#include "sadapi_proto.h"
#include "io_proto.h"
#include "validmem_proto.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern struct LinkLibBase *LinkLibBase;

extern long remote_error;
/*ZZZ - move */extern APTR SpareAddr;

static char sad_connection[100];
static struct SADAPIVars *sadness = NULL;
static LONG sadconnected = 0;
static ULONG pingresult;


struct MinList * __asm
SAD_ObtainValidMem( void )
{
#if 0
    struct Library *ExpansionBase;
#endif
    struct MinList *validlist;

    remote_error = 0;
    validlist = allocValidMem();

#ifdef SAD_PARALLEL
    validlist = addMemoryBlock( validlist,0x00000000,0xFFFFFFFC );
    validlist = addMemoryBlock( validlist,0xFFFFFFFC,4 );

    /* since this is called from PingSAD, we must eat the prompt */
    SADWaitForPrompt( sadness, 3 );
#else
    /*
     * Now for the control areas...
     */
    validlist = addMemoryBlock( validlist,0x00BC0000,0x00040000 );
    validlist = addMemoryBlock( validlist,0x00D80000,0x00080000 );

    /*
     * Now for F-Space...
     */
    validlist = addMemoryBlock( validlist,0x00F00000,0x00080000 );

    /*
     * Now for the ROM...
     */
    validlist = addMemoryBlock( validlist, 0x00F80000,0x00080000 );

#if 0
    /*
     * If the credit card resource, make the addresses valid...
     */
    if (OpenResource("card.resource"))
    {
	validlist = addMemoryBlock( validlist,0x00600000,0x00440002 );
    }

    /*
     * If CDTV, make CDTV hardware valid...
     */
    if (FindResident("cdstrap"))
    {
	validlist = addMemoryBlock( validlist,0x00E00000,0x00080000 );
    }
#else
    /* Assume CDTV! */
    validlist = addMemoryBlock( validlist,0x00E00000,0x00080000 );

#endif

    if ( SADWaitForPrompt( sadness, 3 ) )
    {
	/*
	 * Check for ReKick/ZKick/KickIt
	 */
	struct ExecBase *remoteSysBase;
	ULONG execname;
	struct MemHeader *mem, *succ;

	if ( !SADReadLong( sadness, (void *)4, (ULONG *) &remoteSysBase ) )
	{
	    remote_error = MYSADERR_LOSTCONTACT;
	}
	SADWaitForPrompt( sadness, 3 );
	if ( !SADReadLong( sadness, &remoteSysBase->LibNode.lib_Node.ln_Name, (ULONG *) &execname ) )
	{
	    remote_error = MYSADERR_LOSTCONTACT;
	}

	if (((execname) >> 16) == 0x20)
	{
	    validlist = addMemoryBlock( validlist,0x00200000,0x00080000 );
	}

	/*
	 * Now, put in the free memory
	 * Note: The first K of chip RAM isn't in the memory list, so
	 * it'll be automatically excluded from readability.  However,
	 * location 4 itself needs to be readable (up to a longword's
	 * worth, but locations 5-7 are not legitimate addresses to _start_
	 * reading from.  So we'll special case that up in validAddresses().
	 */
	SADWaitForPrompt( sadness, 3 );
	if ( !SADReadLong( sadness, &remoteSysBase->MemList.lh_Head, (ULONG *) &mem ) )
	{
	    remote_error = MYSADERR_LOSTCONTACT;
	}
	SADWaitForPrompt( sadness, 3 );
	if ( !SADReadLong( sadness, &mem->mh_Node.ln_Succ, (ULONG *)&succ ) )
	{
	    remote_error = MYSADERR_LOSTCONTACT;
	}
	while ( ( succ ) && ( remote_error == 0 ) )
	{
	    ULONG lower;
	    ULONG upper;
	    SADWaitForPrompt( sadness, 3 );
	    if ( !SADReadLong( sadness, &mem->mh_Lower, (ULONG *)&lower ) )
	    {
		remote_error = MYSADERR_LOSTCONTACT;
	    }
	    SADWaitForPrompt( sadness, 3 );
	    if ( !SADReadLong( sadness, &mem->mh_Upper, (ULONG *)&upper ) )
	    {
		remote_error = MYSADERR_LOSTCONTACT;
	    }
	    validlist = addMemoryBlock( validlist, lower, upper-lower );
	    mem = succ;
	    SADWaitForPrompt( sadness, 3 );
	    if ( !SADReadLong( sadness, &mem->mh_Node.ln_Succ, (ULONG *)&succ ) )
	    {
		remote_error = MYSADERR_LOSTCONTACT;
	    }
	}
	/* since this is called from PingSAD, we must eat the prompt */
	SADWaitForPrompt( sadness, 3 );
#if 0
	/*
	 * Map in the autoconfig boards
	 */
	if (ExpansionBase=OpenLibrary("expansion.library",0))
	{
	    struct ConfigDev *cd=NULL;

	    while (cd=FindConfigDev(cd,-1L,-1L))
	    {
		/* Skip memory boards... */
		if (!(cd->cd_Rom.er_Type & ERTF_MEMLIST))
		{
		    validlist = addMemoryBlock( validlist,(ULONG)(cd->cd_BoardAddr),cd->cd_BoardSize );
		}
	    }
	    CloseLibrary(ExpansionBase);
	}
#endif
    }
    else
    {
	remote_error = MYSADERR_LOSTCONTACT;
    }
#endif

    if ( !validlist )
    {
	remote_error = MYSADERR_ALLOCFAIL;
    }

    if ( remote_error )
    {
	freeValidMem( validlist );
	validlist = NULL;
    }
    else
    {
	sadconnected = 1;
    }
    return( validlist );
}

void __asm
SAD_ReleaseValidMem( register __a0 struct MinList *validlist )
{
    freeValidMem( validlist );
}


void
sadbroken( void )
{
    remote_error = MYSADERR_LOSTCONTACT;
    if ( sadconnected )
    {
	sadconnected = 0;
	SAD_ReleaseValidMem( LinkLibBase->ll_ValidMem );
	LinkLibBase->ll_ValidMem = 0;
    }
}


/* Ensure that SAD is alive and well on the other side */
unsigned long
pingSAD( struct SADAPIVars *sav )
{
    if ( !remote_error )
    {
	if ( !( pingresult = SADWaitForPrompt( sav, 3 ) ) )
	{
	    sadbroken();
	}
	else
	{
	    if ( !sadconnected )
	    {
		SADSetVersion( sav );
		/* SAD_ObtainValidMem starts with SADWaitForPrompt */
		LinkLibBase->ll_ValidMem = SAD_ObtainValidMem();
	    }
	}
    }
    return( (unsigned long )( remote_error == 0 ) );
}


STRPTR __asm
SAD_Connect( register __a0 STRPTR machine )
{
    ULONG unit = 0;
#ifdef SAD_PARALLEL
    STRPTR errorstr = "Invalid parallel unit";
#else
    STRPTR errorstr = "Invalid serial unit";
#endif

    if ( IsDecNum( machine, &unit ) )
    {
#ifdef SAD_PARALLEL
	if ( sadness = SADInit( "parallel.device", 9600, unit ) )
#else
	if ( sadness = SADInit( "serial.device", 9600, unit ) )
#endif
	{
	    /* eat prompt if it was waiting.  Failure ok. */
	    SADWaitForPrompt( sadness, 1 );
	    SADSetVersion( sadness );
	    sprintf( sad_connection, "SAD %ld", unit );
	    errorstr = NULL;
	}
    }
    return( errorstr );
}


void __asm
SAD_Disconnect( void )
{
    if ( sadness )
    {
	SADDeinit( sadness );
    }
}


/* Result is:
 * 0 = was not disconnected in the first place
 * 1 = was disconnected, successfully reconnected
 * -1 = was disconnected, failed to reconnect.
 */
LONG __asm
SAD_Reconnect( void )
{
    LONG result = 0;
    if ( pingSAD( sadness ) )
    {
	result = 1;
    }
    return( result );
}


ULONG __asm
SAD_Resume( void )
{
    ULONG result = 0;

    if ( ( sadness ) && ( pingSAD( sadness ) ) )
    {
	if ( !SADReturnToSystem( sadness ) )
	{
	    sadbroken();
	}
	else
	{
	    result = 1;
	}
    }
    return( result );
}


ULONG __asm
SAD_Call( register __a0 APTR addr )
{
    ULONG result = 0;

    if ( ( sadness ) && ( pingSAD( sadness ) ) )
    {
	if ( !SADCallFunction( sadness, addr ) )
	{
	    sadbroken();
	}
	else
	{
	    result = 1;
	}
    }
    return( result );
}


ULONG __asm
SAD_Lock( void )
{
    /* Link is always locked when in SAD */
    return( 1 );
}


ULONG __asm
SAD_Unlock( void )
{
    /* Link is always locked when in SAD */
    return( 1 );
}


BYTE __asm
SAD_ReadByte( register __a0 APTR addr )
{
    UBYTE response = 0;

    if ( pingSAD( sadness ) )
    {
	if ( !SADReadByte( sadness, addr, &response ) )
	{
	    sadbroken();
	}
    }
    return( (char) response );
}


WORD __asm
SAD_ReadWord( register __a0 APTR addr )
{
    UWORD response = 0;

    if ( pingSAD( sadness ) )
    {
	if ( !SADReadWord( sadness, addr, &response ) )
	{
	    sadbroken();
	}
    }
    return( (short) response );
}


LONG __asm
SAD_ReadLong( register __a0 APTR addr )
{
    ULONG response = 0;

    if ( pingSAD( sadness ) )
    {
	if ( !SADReadLong( sadness, addr, &response ) )
	{
	    sadbroken();
	}
    }
    return( (long) response );
}


void __asm
SAD_ReadBlock( register __a0 APTR addr, register __a1 APTR buffer,
    register __d0 ULONG size )
{
    if ( pingSAD( sadness ) )
    {
	if ( !SADReadArray( sadness, addr, buffer, size ) )
	{
	    sadbroken();
	}
    }
}


ULONG __asm
SAD_ReadString( register __a0 APTR addr, register __a1 STRPTR buffer,
    register __d0 ULONG maxlen )
{
    SAD_ReadBlock( addr, buffer, maxlen );
    buffer[ maxlen - 1 ] = 0;
    return( ( unsigned long ) ( strlen( buffer )+1 ) );
}


void __asm
SAD_WriteByte( register __a0 APTR addr, register __d0 BYTE data )
{
    if ( pingSAD( sadness ) )
    {
	if ( !SADWriteByte( sadness, addr, data ) )
	{
	    sadbroken();
	}
    }
}

void __asm
SAD_WriteWord( register __a0 APTR addr, register __d0 WORD data )
{
    if ( pingSAD( sadness ) )
    {
	if ( !SADWriteWord( sadness, addr, data ) )
	{
	    sadbroken();
	}
    }
}

void __asm
SAD_WriteLong( register __a0 APTR addr, register __d0 LONG data )
{
    if ( pingSAD( sadness ) )
    {
	if ( !SADWriteLong( sadness, addr, data ) )
	{
	    sadbroken();
	}
    }
}

void __asm
SAD_WriteBlock( register __a0 APTR addr, register __a1 APTR buffer,
    register __d0 ULONG size )
{
    if ( pingSAD( sadness ) )
    {
	if ( !SADWriteArray( sadness, addr, buffer, size ) )
	{
	    sadbroken();
	}
    }
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
SAD_ReadNodeArray( register __a0 struct List *addr,
    register __a1 struct Node **array, register __d0 ULONG maxcount )
{
    struct Node *node;
    struct Node *succ;
    long count = 0;
    long result;

    /* SAD is implicitly Forbid()den, so we don't need to worry
     * about that aspect.
     * We assume everything is cozy to start.
     */
    result = 0;

    node = (struct Node *)SAD_ReadLong( &addr->lh_Head );
    if ( remote_error )
    {
	/* Denotes failure */
	result = -1;
    }
    else
    {
	succ = (struct Node *)SAD_ReadLong( &node->ln_Succ );
	if ( remote_error )
	{
	    /* Denotes failure */
	    result = -1;
	}
    }

    while ( ( result == 0 ) && ( succ ) )
    {
	count++;
	if ( count <= maxcount )
	{
	    array[count] = node;
	    node = succ;
	    succ = (struct Node *)SAD_ReadLong( &node->ln_Succ );
	    if ( remote_error )
	    {
		/* Denotes failure */
		result = -1;
	    }
	}
	else
	{
	    /* Denote that the array was not big enough */
	    result = 1;
	}
    }
    /* Pardonez the cast... */
    array[ 0 ] = (struct Node *)count;
    return( result );
}


/* We are called with an error number (possibly zero).  If we recognize
 * the error as ours, return an error string, else return NULL.
 */

STRPTR __asm
SAD_Error( register __d0 ULONG error )
{
    STRPTR errorstr = NULL;
    if ( ( error >= 5000 ) && ( error < 5010 ) )
    {
	switch ( error )
	{
	case MYSADERR_ALLOCFAIL:
	    errorstr = "SAD Error: allocation failure";
	    break;

	case MYSADERR_LOSTCONTACT:
	    errorstr = "SAD Error: connection broken";
	    break;

	default:
	    errorstr = "SAD Error: unknown error %ld";
	    break;
	}
    }
    return( errorstr );
}


ULONG __asm
SAD_Context( void )
{
    ULONG contextaddr;
    ULONG result = 0;

    if ( sadness )
    {
	if ( pingSAD( sadness ) )
	{
	    if ( SADGetContextFrame( sadness, &contextaddr ) )
	    {
		result = contextaddr;
	    }
	}
    }
    return( result );
}


ULONG
SAD_ShowContext( void )
{
    struct SADContextFrame context;
    ULONG contextaddr;

    if ( sadness )
    {
	if ( SADSendDelete( sadness, 1 ) )
	{
	    if ( ( contextaddr = SAD_Context() ) && 
		SADReadArray( sadness, (void *)contextaddr,
		    (UBYTE *)&context, sizeof( struct SADContextFrame ) ) )
	    {
		if ( pingresult == 63 )
		{
		    PPrintf("SAD: Entered via Debug()\n");
		}
		else if ( pingresult == 33 )
		{
		    PPrintf("SAD: Entered via crash\n");
		}
		else if ( pingresult == 191 )
		{
		    PPrintf("SAD: Entered via NMI trap\n");
		}
		else
		{
		    PPrintf("SAD: Entered via unknown method\n");
		}


		SpareAddr = (APTR) context.sad_PC;
		PPrintf("PC:   %08lx VBR: %08lx\n", context.sad_PC, context.sad_VBR );
		PPrintf("USP:  %08lx SR:  %04lx\n", context.sad_USP, context.sad_SR );
		PPrintf("Data: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
		    context.sad_D0, context.sad_D1, context.sad_D2, context.sad_D3,
		    context.sad_D4, context.sad_D5, context.sad_D6, context.sad_D7 );
		PPrintf("Addr: %08lx %08lx %08lx %08lx %08lx %08lx %08lx --------\n",
		    context.sad_A0, context.sad_A1, context.sad_A2, context.sad_A3,
		    context.sad_A4, context.sad_A5, context.sad_A6 );
	    }
	    else
	    {
		sadbroken();
	    }
	}
    }
    return( 1 );
}


STRPTR __asm
SAD_Connection( void )
{
    return( sad_connection );
}


ULONG __asm
SAD_Symbol( register __a0 APTR addr )
{
    return( 0 );
}


void
SAD_InitLib( APTR lfl )
{
    long entry = LINK_SKIPENTRIES;

    initLinkLVO( lfl, entry++, SAD_Connect );
    initLinkLVO( lfl, entry++, SAD_Disconnect );
    initLinkLVO( lfl, entry++, SAD_Reconnect );
    initLinkLVO( lfl, entry++, SAD_Resume );

    initLinkLVO( lfl, entry++, SAD_Lock );
    initLinkLVO( lfl, entry++, SAD_Unlock );

    initLinkLVO( lfl, entry++, SAD_ReadByte );
    initLinkLVO( lfl, entry++, SAD_ReadWord );
    initLinkLVO( lfl, entry++, SAD_ReadLong );
    initLinkLVO( lfl, entry++, SAD_ReadBlock );
    initLinkLVO( lfl, entry++, SAD_ReadString );

    initLinkLVO( lfl, entry++, SAD_WriteByte );
    initLinkLVO( lfl, entry++, SAD_WriteWord );
    initLinkLVO( lfl, entry++, SAD_WriteLong );
    initLinkLVO( lfl, entry++, SAD_WriteBlock );
    initLinkLVO( lfl, entry++, lib_stubReturn/*SAD_WriteString*/ );

    initLinkLVO( lfl, entry++, SAD_Context );
    initLinkLVO( lfl, entry++, SAD_ShowContext );
    initLinkLVO( lfl, entry++, SAD_Connection );
    initLinkLVO( lfl, entry++, SAD_Error );
    initLinkLVO( lfl, entry++, SAD_Symbol );
    initLinkLVO( lfl, entry++, SAD_ReadNodeArray );
    initLinkLVO( lfl, entry++, SAD_Call );
}
