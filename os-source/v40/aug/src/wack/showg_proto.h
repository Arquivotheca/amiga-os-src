/*
 * Amiga Grand Wack
 *
 * $Id: showg_proto.h,v 39.1 93/05/21 17:35:10 peter Exp $
 *
 */

ULONG dumpViewPort( APTR addr, struct ViewPort *vp );

ULONG dumpView( APTR addr, struct View *v );

ULONG dumpViewExtra( APTR addr, struct ViewExtra *ve );

ULONG dumpLayer_Info( APTR addr, struct Layer_Info *li );

ULONG dumpLayer( APTR addr, struct Layer *l );

STRPTR ShowGfxBase( void );

STRPTR ShowViewPort( char *arg );
STRPTR ShowViewPortList( char *arg );
STRPTR ShowView( char *arg );
STRPTR ShowViewExtra( char *arg );
STRPTR ShowLayer_Info( char *arg );
STRPTR ShowLayer( char *arg );
