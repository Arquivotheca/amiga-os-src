/*
 * Amiga Grand Wack
 *
 * $Id: wackbase_proto.h,v 39.7 93/11/05 15:04:33 jesup Exp $
 *
 */

STRPTR runCommand( char *arg1, char *arg2 );

STRPTR runWackCommand( char *arg1, char *arg2 );

struct WackLibBase * InitWackBase( void );

void FreeWackBase( struct WackLibBase *wackbase );

STRPTR WackPortName( void );

void initFunction( struct WackFakeLibrary *wfl, ULONG count, APTR funcptr );

ULONG wacklib_stubReturn( void );

void __asm lib_VPrintf( register __a0 STRPTR fmt, register __a1 ULONG *arglist );

APTR __asm lib_ReadCurrAddr( void );

APTR __asm lib_WriteCurrAddr( register __a0 APTR address );

APTR __asm lib_WriteSpareAddr( register __a0 APTR address );

APTR __asm lib_ReadSpareAddr( void );

ULONG __asm lib_ReadByte( register __a0 APTR address );

ULONG __asm lib_ReadWord( register __a0 APTR address );

ULONG __asm lib_ReadLong( register __a0 APTR address );

VOID __asm lib_ReadBlock( register __a0 APTR address, register __a1 APTR buffer,
	register __d0 ULONG size );

void __asm lib_WriteByte( register __a0 APTR address, register __d0 UBYTE data );

void __asm lib_WriteWord( register __a0 APTR address, register __d0 UWORD data );

void __asm lib_WriteLong( register __a0 APTR address, register __d0 ULONG data );

VOID __asm lib_WriteBlock( register __a0 APTR address, register __a1 APTR buffer,
			   register __d0 ULONG size );

struct Library * __asm lib_FindLibrary( register __a0 STRPTR library );

ULONG __asm lib_ReadString( register __a0 APTR address, register __a1 APTR buffer, register __d0 ULONG maxlen );

ULONG __asm lib_ReadBSTR( register __a0 APTR address, register __a1 APTR buffer, register __d0 ULONG maxlen );

APTR __asm lib_ReadContextAddr( void );

ULONG __asm lib_Call( register __a0 APTR address);
