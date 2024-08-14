/*
 * Amiga Grand Wack
 *
 * $Id: asm_proto.h,v 39.3 93/07/16 18:23:35 peter Exp $
 *
 */

BOOL IsHexNum( char *string, ULONG *number );

BOOL IsDecNum( char *string, ULONG *number );

char UpperCase( char ch );

LONG Hash( char *string );

void sprintf( char *buffer, char *fmt, ... );

void sprintfA( char *buffer, char *fmt, ULONG * );
