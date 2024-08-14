/*
 * Amiga Grand Wack
 *
 * $Id: ops_proto.h,v 39.5 93/11/05 15:04:20 jesup Exp $
 *
 */

STRPTR NextWord( char *arg );

STRPTR BackWord( char *arg );

STRPTR NextFrame( char *arg );

STRPTR BackFrame( char *arg );

STRPTR Disassemble( char *arg );

STRPTR ShiftBPtr( char *argStr );

STRPTR ToDec( char *argStr );

STRPTR ToHex( char *argStr );

STRPTR ClearScreen( void );

void stackCurrent( void );

STRPTR Indirect( STRPTR arg );

STRPTR IndirectBptr( STRPTR arg );

STRPTR Exdirect( STRPTR arg );

STRPTR swapSpareAddr( STRPTR arg );

STRPTR evaluateSymbol( struct Symbol *sp, char *argStr );

STRPTR SizeFrame( char *arg );

STRPTR NextCount( char *arg );

STRPTR BackCount( char *arg );

STRPTR Resume( void );

STRPTR Call( char *arg );

STRPTR SetBreakPoint( void );

STRPTR AssignMem( char *argStr );

STRPTR Nothing1( void );
STRPTR Nothing2( void );
STRPTR Nothing3( void );

void BadSyntax( void );

STRPTR setDump( char *argStr );

void ShowFrame(void);
