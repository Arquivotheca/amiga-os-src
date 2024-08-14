/*
 * Amiga Grand Wack
 *
 * $Id: define_proto.h,v 39.7 93/07/16 18:24:17 peter Exp $
 *
 */

typedef STRPTR ( *CMDFUNC )();

char * UnCtrl( char c, char *s );

void BindMacro( char *name, short *type, char *body );

struct Symbol * NewSymbol( char *name, struct SymbolMap *symMap );

struct Symbol * FindSymbol( char *name, struct SymbolMap *symMap );

static void ShowSymbol( struct Symbol *sp );

static void ShowSymbolMap( struct SymbolMap *symMap );

STRPTR ShowTopSymMap( void );

void UnBindSymbols( void );

void UnBindSymbolMap( struct SymbolMap *symMap );

void dumpNearestSymbol( ULONG addr );

STRPTR ApproxSym( char *arg );

STRPTR ApproxSymIndirect( char *arg );

struct Symbol *BindValue( char *name, short vtype, long value );

struct Symbol *BindValueQuick( char *name, short vtype, long value );

void BindCommand( char *name, CMDFUNC function );

void BindCommand2( char *name, CMDFUNC function, char *arg1 );

void TryIt( void );

long processLine( char *line );

STRPTR bindAlias( char *arg );

STRPTR bindXWack( char *arg );

STRPTR bindSystem( char *arg );

STRPTR bindRexx( char *arg );

STRPTR bindConstant( char *arg );
