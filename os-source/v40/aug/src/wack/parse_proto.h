/*
 * Amiga Grand Wack
 *
 * $Id: parse_proto.h,v 39.2 93/08/23 19:32:22 peter Exp $
 *
 */

STRPTR parseRemainder( STRPTR arg );

STRPTR parseToken( STRPTR arg, STRPTR token );

STRPTR parseHexNum( STRPTR arg, ULONG *value, ULONG *inputsize );

STRPTR parseAddress( STRPTR arg, ULONG *value );

STRPTR parseDecNum( STRPTR arg, ULONG *value );
