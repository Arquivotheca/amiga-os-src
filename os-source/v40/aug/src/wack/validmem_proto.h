/*
 * Amiga Grand Wack
 *
 * $Id: validmem_proto.h,v 39.1 93/04/27 14:42:08 peter Exp $
 *
 */

#include <exec/types.h>

long validAddresses( struct MinList *memf, void *address, unsigned long length );

long validAddress( struct MinList *memf, void *address );

struct MinList * allocValidMem( void );

void freeValidMem( struct MinList *memf );

struct MinList * addMemoryBlock( struct MinList *memf, ULONG start, unsigned long length );

struct MinList * buildValidMem( void );

