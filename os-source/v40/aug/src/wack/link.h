/*
 * Amiga Grand Wack
 *
 * link.h -- Definitions for generic connection functions for the target machine.
 *
 * $Id: link.h,v 39.2 93/11/05 15:02:57 jesup Exp $
 *
 */

#include <exec/types.h>

#define LINK_CUSTOMENTRIES	23

/* Skip 5 vectors: Open, Close, Expunge, Reserved, and ARexx */
#define LINK_SKIPENTRIES 5

#define LINK_ENTRIES ( LINK_SKIPENTRIES + LINK_CUSTOMENTRIES )
#define PAD ( LINK_ENTRIES%2 ? 2 : 0 )

#define LINKTABLESIZE (6 * LINK_ENTRIES + PAD)

struct LinkLibBase
{
    struct MinList *ll_ValidMem;
};

struct LinkFakeLibrary
{
    UBYTE lfl_JumpTable[ LINKTABLESIZE ];
    struct LinkLibBase lfl_LibBase;
};
