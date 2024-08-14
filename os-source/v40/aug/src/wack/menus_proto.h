/*
 * Amiga Grand Wack
 *
 * $Id: menus_proto.h,v 39.3 93/05/21 17:33:48 peter Exp $
 *
 */

long init_Menus( void );

void exit_Menus( void );

STRPTR bindMenuHeader( STRPTR arg );

long createMenuHeader( STRPTR header_name, STRPTR arg );

STRPTR bindMenuItem( STRPTR arg );

long createMenuItem( STRPTR header_name, STRPTR item_name, STRPTR arg );

STRPTR bindMenuSub( STRPTR arg );

long findLastMenu( void );

long findLastItem( STRPTR header_name );

long findLastSub( STRPTR header_name, STRPTR item_name );

long findHeader( STRPTR header_name );

long findItem( STRPTR header_name, STRPTR item_name );

long addMenuEntry( unsigned long index, UBYTE type, STRPTR label, char *arg );

void undoMenus( void );

long redoMenus( void );

ULONG EvalMenus( UWORD menucode );
