/*
 * Amiga Grand Wack
 *
 * validmem.h - Structures used to keep lists of valid memory.
 *
 * $Id: validmem.h,v 39.0 92/10/30 15:26:49 peter Exp $
 *
 * This is the MemoryBlock structure.  A MinList of these is used
 * to determine valid memory ranges to read.
 *
 */

#include <exec/lists.h>

struct MemoryBlock
{
    struct MinNode mb_Node;
    unsigned long mb_Start;
    unsigned long mb_End;
};
