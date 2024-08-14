/*
 * Amiga Grand Wack
 *
 * $Id: io_proto.h,v 39.6 93/05/21 17:33:06 peter Exp $
 *
 */

STRPTR openEnvironment( void );

void closeEnvironment( STRPTR errorstr );

STRPTR openWin( void );

void closeWin( void );

STRPTR getTarget( void );

void executeStartupFile( void );

void NewLine( void );

void VPPrintf( char *fmt, ULONG *arglist );

void PPrintf( char *fmt, ... );

void Putchar( char c );

void resetError( void );

void showError( void );

void showCursor( BOOL turnoff );

void processInput( void );

APTR malloc( ULONG size );

void free( APTR addr );

STRPTR Capture( char *arg );

STRPTR SetWackPrompt( char *arg );
