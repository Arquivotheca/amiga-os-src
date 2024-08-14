/*
 * Amiga Grand Wack
 *
 * $Id: showi_proto.h,v 39.3 93/08/19 16:34:48 peter Exp $
 *
 */

void dumpBox( UBYTE *s, struct IBox *b );

void ShowISems( void );

STRPTR ShowLayerInfoSems( void );

void dumpRect( UBYTE *s, struct Rectangle *r );

STRPTR ShowAScreen( void );

STRPTR ShowAWindow( void );

STRPTR ShowFirstGad( char *arg );

STRPTR ShowScreen( char *arg );
STRPTR ShowExtGadget( char *arg );
STRPTR ShowExtGadgetList( char *arg );
STRPTR ShowMenuItem( char *arg );
STRPTR ShowMenuItemList( char *arg );
STRPTR ShowMenu( char *arg );
STRPTR ShowMenuList( char *arg );
STRPTR ShowWindow( char *arg );
STRPTR ShowRequester( char *arg );
STRPTR ShowPropInfo( char *arg );
STRPTR ShowImage( char *arg );
STRPTR ShowIntuiText( char *arg );

STRPTR ShowWindowFlags( char *arg );

STRPTR ShowIClass( char *arg );

STRPTR ShowObject( char *arg );

STRPTR ShowIBase( void );
