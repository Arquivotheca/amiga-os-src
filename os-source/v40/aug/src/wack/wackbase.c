/*
 * Amiga Grand Wack
 *
 * wackbase.c -- Wack "fake" library handling.
 *
 * $Id: wackbase.c,v 39.11 93/11/05 14:57:27 jesup Exp $
 *
 * Wack's ARexx port resembles an Exec Library in that it has LVO's
 * located at negative offsets from it.  Those LVO's are usable by
 * external Wack-extension programs (bound with bindxwack).  Such
 * programs receive the name of the Wack ARexx port as their first
 * argument.  They need to FindPort() that port and place the address
 * in WackBase.  They then can use the pragma functions in wack_pragmas.h
 * to call the Wack host's functions.
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dostags.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "wackbase.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern APTR LinkLibBase;

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern BPTR inputfh;
extern BPTR outputfh;

extern APTR CurrAddr;
extern APTR SpareAddr;

#include "symbols.h"
#include "simplerexx.h"
#include "parse.h"

#include "parse_proto.h"
#include "sys_proto.h"
#include "asm_proto.h"
#include "wackbase_proto.h"
#include "define_proto.h"
#include "show_proto.h"
#include "io_proto.h"
#include "link_proto.h"
#include "special_proto.h"
#include "ops_proto.h"

extern struct ARexxContext *RexxContext;
extern struct WackLibBase *WackBase;

LONG
SystemTags( STRPTR command, ULONG tag, ... )
{
    return( SystemTagList( command, (struct TagItem *) &tag ) );
}

#define CMDBUFFERSIZE 512
char cmdbuffer[ CMDBUFFERSIZE ];

STRPTR
runCommand( char *arg1, char *arg2 )
{
    LONG error;

    strcpy( cmdbuffer, arg1 );
    if ( ( arg2 ) && ( *arg2 ) )
    {
	strcat( cmdbuffer, " " );
	strcat( cmdbuffer, arg2 );
    }
    error = SystemTags( cmdbuffer,
	SYS_Input, inputfh,
	SYS_Output, outputfh,
	SYS_UserShell, TRUE,
	TAG_DONE );

    if ( error )
    {
	PPrintf("Command returned %ld\n", error );
    }

    return( NULL );
}


/* Runs the specified command, with the ARexx port name as the
 * first argument.  For example, if the command was:
 * "wacksystem intuition window arg", the line passed to
 * System() will be "intuition wack.N window arg"
 */
STRPTR
runWackCommand( char *arg1, char *arg2 )
{
    LONG error;
    char *remainder;

    if ( remainder = parseToken( arg1, cmdbuffer ) )
    {
	strcat( cmdbuffer, " WACKPORT=" );
	strcat( cmdbuffer, WackPortName() );

	if ( remainder = parseRemainder( remainder ) )
	{
	    strcat( cmdbuffer, " ");
	    strcat( cmdbuffer, remainder );
	}
	if ( remainder = parseRemainder( arg2 ) )
	{
	    strcat( cmdbuffer, " ");
	    strcat( cmdbuffer, remainder );
	}
	error = SystemTags( cmdbuffer,
	    SYS_Input, inputfh,
	    SYS_Output, outputfh,
	    SYS_UserShell, TRUE,
	    TAG_DONE );

	if ( error )
	{
	    PPrintf("Command returned %ld\n", error );
	}
    }
    else
    {
	PPrintf("Invalid command\n");
    }

    return( NULL );
}


struct WackLibBase *
InitWackBase( void )
{
    struct WackLibBase *wackbase = NULL;
    struct WackFakeLibrary *wfl;
    ULONG sig;
    char appname[] = "wack";
    ULONG loop;
    ULONG count;
    char *tmp;
    struct MsgPort *foundport;

    sig = AllocSignal(-1);
    if ( sig >= 0 )
    {
	if ( wfl = AllocMem( sizeof( struct WackFakeLibrary ), MEMF_CLEAR ) )
	{
	    long entry = 0;

	    wackbase = &wfl->wfl_LibBase;
	    /*
	     * Set up a port name...
	     */
	    tmp = wackbase->wl_PortNameBuffer;
	    for ( loop = 0; ( loop < 16 ) && ( appname[ loop ] ); loop++ )
	    {
		    *tmp++ = appname[ loop ];
	    }
	    *tmp++ = '.';

	    wackbase->wl_MsgPort.mp_Node.ln_Name = wackbase->wl_PortNameBuffer;
	    wackbase->wl_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	    wackbase->wl_MsgPort.mp_Flags        = PA_SIGNAL;
	    wackbase->wl_MsgPort.mp_SigBit       = sig;
	    wackbase->wl_MsgPort.mp_SigTask      = FindTask(NULL);

	    /* We need to make a unique port name... */
	    Forbid();
	    count = 1;
	    do
	    {
		sprintf(tmp,"%ld",count);
		foundport = FindPort( wackbase->wl_PortNameBuffer );
		count++;
	    }
	    while ( foundport );

	    AddPort( &wackbase->wl_MsgPort );
	    Permit();

	    initFunction( wfl, entry++, wacklib_stubReturn );	/* Open */
	    initFunction( wfl, entry++, wacklib_stubReturn );	/* Close */
	    initFunction( wfl, entry++, wacklib_stubReturn );	/* Expunge */
	    initFunction( wfl, entry++, wacklib_stubReturn );	/* Reserved */
	    initFunction( wfl, entry++, wacklib_stubReturn );	/* ARexx */
	    initFunction( wfl, entry++, lib_VPrintf );
	    initFunction( wfl, entry++, lib_ReadCurrAddr );
	    initFunction( wfl, entry++, lib_WriteCurrAddr );
	    initFunction( wfl, entry++, lib_ReadSpareAddr );
	    initFunction( wfl, entry++, lib_WriteSpareAddr );
	    initFunction( wfl, entry++, lib_ReadByte );
	    initFunction( wfl, entry++, lib_ReadWord );
	    initFunction( wfl, entry++, lib_ReadLong );
	    initFunction( wfl, entry++, lib_ReadBlock );
	    initFunction( wfl, entry++, lib_WriteByte );
	    initFunction( wfl, entry++, lib_WriteWord );
	    initFunction( wfl, entry++, lib_WriteLong );
	    initFunction( wfl, entry++, lib_FindLibrary );
	    initFunction( wfl, entry++, lib_ReadString );
	    initFunction( wfl, entry++, lib_ReadBSTR );
	    initFunction( wfl, entry++, lib_ReadContextAddr );
	    initFunction( wfl, entry++, lib_WriteBlock );
	    initFunction( wfl, entry++, lib_Call );
	    CacheClearU();
	}
	else
	{
	    FreeSignal( sig );
	}
    }
    return( wackbase );
}

void
FreeWackBase( struct WackLibBase *wackbase )
{
    struct WackFakeLibrary *wfl;

    if ( wackbase )
    {
	FreeSignal( wackbase->wl_MsgPort.mp_SigBit );
	wfl = (struct WackFakeLibrary *)
	    (((ULONG)wackbase) - ((ULONG)&((struct WackFakeLibrary *)0)->wfl_LibBase ));
	RemPort( &wackbase->wl_MsgPort );
	FreeMem( wfl, sizeof( struct WackFakeLibrary ) );
    }
}

STRPTR
WackPortName( void )
{
    return( WackBase->wl_MsgPort.mp_Node.ln_Name );
}

void
initFunction( struct WackFakeLibrary *wfl, ULONG count, APTR funcptr )
{
    ULONG lvo = WACKTABLESIZE - 6*(count+1);
    *((UWORD *)&wfl->wfl_JumpTable[ lvo ]) = 0x4EF9;	/* jump instruction */
    *((ULONG *)&wfl->wfl_JumpTable[ lvo+2 ]) = (ULONG) funcptr;
}


ULONG
wacklib_stubReturn( void )
{
    return( 0 );
}

void __asm
lib_VPrintf( register __a0 STRPTR fmt, register __a1 ULONG *arglist )
{
    VPPrintf( fmt, arglist );
}

APTR __asm
lib_ReadCurrAddr( void )
{
    return( CurrAddr );
}

APTR __asm
lib_WriteCurrAddr( register __a0 APTR address )
{
    APTR oldaddress = CurrAddr;
    CurrAddr = address;
    return( oldaddress );
}

APTR __asm
lib_ReadSpareAddr( void )
{
    return( SpareAddr );
}

APTR __asm
lib_WriteSpareAddr( register __a0 APTR address )
{
    APTR oldaddress = SpareAddr;
    SpareAddr = address;
    return( oldaddress );
}

ULONG __asm
lib_ReadByte( register __a0 APTR address )
{
    return( (ULONG)ReadByte( address ) );
}

ULONG __asm
lib_ReadWord( register __a0 APTR address )
{
    return( (ULONG)ReadWord( address ) );
}

ULONG __asm
lib_ReadLong( register __a0 APTR address )
{
    return( (ULONG)ReadLong( address ) );
}

VOID __asm
lib_ReadBlock( register __a0 APTR address,
	register __a1 APTR buffer,
	register __d0 ULONG size )
{
    ReadBlock( address, buffer, size );
}

void __asm
lib_WriteByte( register __a0 APTR address, register __d0 UBYTE data )
{
    WriteByte( address, data );
}

void __asm
lib_WriteWord( register __a0 APTR address, register __d0 UWORD data )
{
    WriteWord( address, data );
}

void __asm
lib_WriteLong( register __a0 APTR address, register __d0 ULONG data )
{
    WriteLong( address, data );
}

VOID __asm
lib_WriteBlock( register __a0 APTR address,
	register __a1 APTR buffer,
	register __d0 ULONG size )
{
    WriteBlock( address, buffer, size );
}

struct Library * __asm
lib_FindLibrary( register __a0 STRPTR library )
{
    return( FindBase( library ) );
}

ULONG __asm
lib_ReadString( register __a0 APTR address,
	register __a1 APTR buffer,
	register __d0 ULONG maxlen )
{
    return( GetString( address, buffer, maxlen ) );
}

ULONG __asm
lib_ReadBSTR( register __a0 APTR address,
	register __a1 APTR buffer,
	register __d0 ULONG maxlen )
{
    return( GetBSTR( address, buffer, maxlen ) );
}

APTR __asm
lib_ReadContextAddr( void )
{
    return( (APTR)LinkContext() );
}

ULONG __asm
lib_Call( register __a0 APTR address)
{
	return( CallFunction(address) );
}
