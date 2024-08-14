/*
 * Amiga Grand Wack
 *
 * $Id: rexxhandler_proto.h,v 39.9 93/11/05 15:03:11 jesup Exp $
 *
 */

typedef char * (*rexxfunc)( char *arg );

ULONG ARexx_Init( void );

void ARexx_Exit( void );

ULONG ARexx_NumPending( void );

void ARexx_Handler( void );

void BindRexxPrim( char *name, rexxfunc function );

char * RexxPrintfResult( char *fmt, ... );

STRPTR ARexx_Execute( char *arg1, char *arg2 );

STRPTR ARexx_ExecuteString( char *arg );

void ARexx_Bind( void );

char * rxcmd_CurrentAddr( char *arg );

char * rxcmd_SpareAddr( char *arg );

char * rxcmd_Version( char *arg );

char * rxcmd_Print( char *arg );

long parseReadTemplate( char *arg, long **address );

char * rxcmd_ReadByte( char *arg );

char * rxcmd_ReadWord( char *arg );

char * rxcmd_ReadLong( char *arg );

long parseWriteTemplate( char *arg, long **address, long *data );

char * rxcmd_WriteByte( char *arg );

char * rxcmd_WriteWord( char *arg );

char * rxcmd_WriteLong( char *arg );

char * rxcmd_ReadString( char *arg );

char * rxcmd_ReadBSTR( char *arg );

char * rxcmd_FindLibrary( char *arg );

char * rxcmd_Connection( char *arg );

char * rxcmd_Context( char *arg );

char * rxcmd_ReadBlock( char *arg );

char * rxcmd_WriteBlock( char *arg );

long parseArrayTemplate( char *arg, UBYTE **addressp, UBYTE **data, ULONG *size );

char * rxcmd_Call( char *arg );
