/* screen.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

STRPTR ScreenType (struct GlobalData * gd, struct Screen * scr)
{
    STRPTR retval = NULL;

    switch (scr->Flags & SCREENTYPE)
    {
	case WBENCHSCREEN:
	    retval = "Workbench";
	    break;

	case PUBLICSCREEN:
	    retval = "Public";
	    break;

	case CUSTOMSCREEN:
	    retval = "Custom";
	    break;
    }

    return (retval);
}

/*****************************************************************************/

STRPTR DumpScreenFlags (struct GlobalData * gd, struct Screen * scr)
{
    UWORD flags = scr->Flags;

    memset (gd->gd_FBuffer, 0, sizeof (gd->gd_FBuffer));
    if (flags & SHOWTITLE)
	strcat (gd->gd_FBuffer, "showtitle ");
    if (flags & CUSTOMBITMAP)
	strcat (gd->gd_FBuffer, "custombitmap ");
    if (flags & SCREENQUIET)
	strcat (gd->gd_FBuffer, "screenquiet ");
    if (flags & NS_EXTENDED)
	strcat (gd->gd_FBuffer, "extended ");
    if (flags & AUTOSCROLL)
	strcat (gd->gd_FBuffer, "autoscroll ");
    return (gd->gd_FBuffer);
}

/*****************************************************************************/

void showscreenlist (struct GlobalData * gd)
{
    struct Screen *scr;
    ULONG lock;

    /* Make the title */
    strcpy (gd->gd_Node, "@{b}Screen Name                                Address@{ub}\n");

    /* Build the screen list */
    lock = LockIBase (0);
    for (scr = IntuitionBase->FirstScreen; scr; scr = scr->NextScreen)
    {
	bprintf (gd, "@{\"%-40s\" link HYPERNOZY.SCREEN.(%08lx)} @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n",
		 scr->Title, scr, scr, scr);
    }
    UnlockIBase (lock);
}

/*****************************************************************************/

void showscreen (struct GlobalData * gd, ULONG address)
{
    struct Screen *scr;
    struct Window *win;
    ULONG lock;

    /* Just in case we don't find the screen */
    strcpy (gd->gd_Node, "@{b}screen gone@{ub}\n");

    /* Search for the screen */
    lock = LockIBase (0);
    for (scr = IntuitionBase->FirstScreen; scr; scr = scr->NextScreen)
    {
	/* Is this the screen we were looking for? */
	if (scr == (struct Screen *) address)
	{
	    /* Build the screen information */
	    strcpy (gd->gd_Node, "@{b}Screen@{ub}\n\n");
	    bprintf (gd, "        Address: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", (void *) address, (void *) address);
	    bprintf (gd, "    Next Screen: @{\"%08lx\" link HYPERNOZY.SCREEN.(%08lx)}\n", scr->NextScreen, scr->NextScreen);
	    bprintf (gd, "         Offset: %ld, %ld\n", (void *) scr->LeftEdge, (void *) scr->TopEdge);
	    bprintf (gd, "     Dimensions: %ld x %ld x %ld\n", (void *) scr->Width, (void *) scr->Height, (void *) scr->RastPort.BitMap->Depth);
	    bprintf (gd, "          Flags: %s\n", DumpScreenFlags (gd, scr));
	    bprintf (gd, "          Title: %s\n", ((scr->Title) ? scr->Title : ""));
	    bprintf (gd, "  Default Title: %s\n", ((scr->DefaultTitle) ? scr->DefaultTitle : ""));
	    bprintf (gd, "           Type: %s\n", ScreenType (gd, scr));
	    bprintf (gd, "           Font: @{\"%08lx\" link HYPERNOZY.FONT.(%08lx)}\n", scr->Font, scr->Font);
	    bprintf (gd, "       ViewPort: @{\"%08lx\" link HYPERNOZY.VIEWPORT.(%08lx)}\n", &scr->ViewPort, &scr->ViewPort);
	    bprintf (gd, "       RastPort: @{\"%08lx\" link HYPERNOZY.RASTPORT.(%08lx)}\n", &scr->RastPort, &scr->RastPort);
	    bprintf (gd, "         BitMap: @{\"%08lx\" link HYPERNOZY.BITMAP.(%08lx)}\n", scr->RastPort.BitMap);
#if 0
	    bprintf (gd, "   BitMap Flags: %s\n", DumpBitMapFlags (scr));
#endif
	    bprintf (gd, "      LayerInfo: @{\"%08lx\" link HYPERNOZY.LAYERINFO.(%08lx)}\n", &scr->LayerInfo, &scr->LayerInfo);
	    bprintf (gd, "   First Gadget: @{\"%08lx\" link HYPERNOZY.GADGETLIST.(%08lx)}\n", scr->FirstGadget, scr->FirstGadget);
	    bprintf (gd, "  Detail, Block: %ld, %ld\n", (void *) scr->DetailPen, (void *) scr->BlockPen);
	    bprintf (gd, "       BarLayer: @{\"%08lx\" link HYPERNOZY.LAYER.(%08lx)}\n", scr->BarLayer, scr->BarLayer);
	    bprintf (gd, "        ExtData: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", scr->ExtData, scr->ExtData);
	    bprintf (gd, "       UserData: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", scr->UserData, scr->UserData);

	    /* Build the window list */
	    strcat (gd->gd_Node, "\n@{b}Windows@{ub}\n\n");
	    for (win = scr->FirstWindow; win; win = win->NextWindow)
		bprintf (gd, "  @{\"%-40s\" link HYPERNOZY.WINDOW.(%08lx)} @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n",
			 win->Title, win, win, win);
	    break;
	}
    }
    UnlockIBase (lock);

}
