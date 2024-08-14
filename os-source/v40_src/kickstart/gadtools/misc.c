/*** misc.c ***************************************************************
*
*   misc.c	- Gadget Toolkit Miscellaneous routines
*
*   Copyright 1989, Commodore-Amiga, Inc.
*
*   $Id: misc.c,v 39.5 92/07/31 18:24:43 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

struct Image *getSysImage(struct VisualInfo *vi, LONG width, LONG height,
                          ULONG kind)
{
struct ImageLink *il;

    il = vi->vi_Images;
    while (il)
    {
        if ((il->il_Image->Width == width) && (il->il_Image->Height == height) && (il->il_Type == kind))
            return(il->il_Image);

        il = il->il_Next;
    }

    if (il = AllocVec(sizeof(struct ImageLink),MEMF_ANY))
    {

	 /* Note that the only place this image is disposed
	 * is in FreeVisualInfo().
	 */
	if (il->il_Image = NewObject(NULL,"sysiclass",
                                     SYSIA_DrawInfo, vi->vi_DrawInfo,
                                     SYSIA_Which,    kind,
                                     IA_Width,       width,
                                     IA_Height,      height,
                                     TAG_DONE))
        {
            il->il_Type   = kind;
            il->il_Next   = vi->vi_Images;
            vi->vi_Images = il;
            return(il->il_Image);
        }
        FreeVec(il);
    }

    return(NULL);
}
