/*
 * Amiga Grand Wack
 *
 * $Id: copper_proto.h,v 39.1 93/05/21 17:32:08 peter Exp $
 *
 */

char * regname( int reg );

STRPTR dumpCops( void );

STRPTR dumpAllCops( void );

STRPTR dumpCopHeader( void );

void doDumpCops( int listtype );

void dumpCprList( struct cprlist *list, int type );

int copDisasm( APTR addr, UWORD op, UWORD w, int all );

void prComment( char *regname, int w );
