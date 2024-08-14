/******************************************************************************
    RastPortTools.c

    Mitchell/Ware Systems           Version 1.01            2/18/90

-------------------------------------------------------------------------------
    NOTE: Libraries required:
        graphics.library
        layers.library

    BOOL ClipRastPort(rp, left, top, width, height)
        rp      -   pointer to RastPort to be cliped
        left    -
        top
        width
        height  -   location and dimension of window

        ClipRastPort() will clip the RastPort work area to the specified
        dimensions and Clip all operations there. Also, the origin will
        be moved to the upper left corner of the area. All rendering

        ClipRastPort() will return TRUE if sucessful, otherwise FALSE.
        One possible reason for failure is a low memory condition.

        WARN: ClipRastPort() also stores information in the rp->RP_User
              field, so beware!!!

        WARN: You MUST do an UnclipRastPort() before relinquishing this
              RastPort!

    void UnclipRastPort(rp, left, top)
        struct RastPort *rp;   -   RastPort to unclip
        short left, top;       -   The same left, top passed to ClipRastPort()

        Undoes the Operation by ClipRastPort. You MUST call this before
        relinquishing the RastPort!


******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/regions.h>

BOOL ClipRastPort(rp, left, top, width, height)
struct RastPort *rp;
short left, top, width, height;
{
    struct Region *reg;
    struct Rectangle rect;

    if (reg = NewRegion())
    {

        rect.MinX = left;
        rect.MinY = top;
        rect.MaxX = left + width - 1;
        rect.MaxY = top + height - 1;
        OrRectRegion(reg, &rect);

        rp->RP_User = InstallClipRegion(rp->Layer, reg);
        ScrollLayer(NULL, rp->Layer, -left, -top);
        return TRUE;
    }
    else
        return FALSE;
}

void UnclipRastPort(rp, left, top)
struct RastPort *rp;
short left, top;    /* same as the left/top passed to ClipRastPort() */
{
    ScrollLayer(NULL, rp->Layer, left, top);
    DisposeRegion(InstallClipRegion(rp->Layer, rp->RP_User));
}
