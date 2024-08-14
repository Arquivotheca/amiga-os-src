/* bitmap.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

STRPTR DumpBitMapFlags (struct GlobalData *gd, struct Screen *scr)
{
    UBYTE buffer[256];
    ULONG value;

    memset (buffer, 0, sizeof (buffer));

    value = GetBitMapAttr (scr->RastPort.BitMap, BMA_FLAGS);
    if (value & BMF_DISPLAYABLE)
	strcat (buffer, "displayable ");
    if (value & BMF_INTERLEAVED)
	strcat (buffer, "interleaved ");
    if (value & BMF_STANDARD)
	strcat (buffer, "standard ");

    return (buffer);
}

