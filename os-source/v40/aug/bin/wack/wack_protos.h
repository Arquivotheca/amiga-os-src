#ifndef  CLIB_WACK_PROTOS_H
#define  CLIB_WACK_PROTOS_H

/*
**	$Id: wack_protos.h,v 39.5 93/11/05 15:12:04 jesup Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

/* WackBase is a pointer to the Wack ARexx port obtained via FindPort(). */

void Wack_VPrintf( STRPTR fmt, ULONG *arglist );
void Wack_Printf( STRPTR fmt, ... );
APTR Wack_ReadCurrAddr( void );
APTR Wack_WriteCurrAddr( APTR newaddr );
APTR Wack_ReadSpareAddr( void );
APTR Wack_WriteSpareAddr( APTR newaddr );
UBYTE Wack_ReadByte( APTR address );
UWORD Wack_ReadWord( APTR address );
ULONG Wack_ReadLong( APTR address );
APTR Wack_ReadPointer( APTR address );
void Wack_ReadBlock( APTR address, APTR buffer, unsigned long size );
void Wack_WriteByte( APTR address, unsigned long data );
void Wack_WriteWord( APTR address, unsigned long data );
void Wack_WriteLong( APTR address, unsigned long data );
void Wack_WritePointer( APTR address, APTR data );
struct Library *Wack_FindLibrary( STRPTR library );
ULONG Wack_ReadString( STRPTR address, APTR buffer, unsigned long maxlen );
ULONG Wack_ReadBSTR( STRPTR address, APTR buffer, unsigned long maxlen );
APTR Wack_ReadContextAddr( void );
void Wack_WriteBlock( APTR address, APTR buffer, unsigned long size );
ULONG Wack_Call( APTR addr );
#endif   /* CLIB_WACK_PROTOS_H */
