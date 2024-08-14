/* window.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

STRPTR DumpWindowFlags (struct GlobalData * gd, struct Window * win)
{
    ULONG flags = win->Flags;

    memset (gd->gd_FBuffer, 0, sizeof (gd->gd_FBuffer));

    /* Gadget information */
    if (flags & WFLG_SIZEGADGET)
	strcat (gd->gd_FBuffer, "size ");

    if (flags & WFLG_DRAGBAR)
	strcat (gd->gd_FBuffer, "drag ");

    if (flags & WFLG_DEPTHGADGET)
	strcat (gd->gd_FBuffer, "depth ");

    if (flags & WFLG_CLOSEGADGET)
	strcat (gd->gd_FBuffer, "close ");

    if (flags & WFLG_HASZOOM)
	strcat (gd->gd_FBuffer, "zoom ");

    /* Refresh */
    switch (flags & WFLG_REFRESHBITS)
    {
	case WFLG_SMART_REFRESH:
	    strcat (gd->gd_FBuffer, "smart ");
	    break;

	case WFLG_SIMPLE_REFRESH:
	    strcat (gd->gd_FBuffer, "simple ");
	    break;

	case WFLG_SUPER_BITMAP:
	    strcat (gd->gd_FBuffer, "super ");
	    break;

	case WFLG_OTHER_REFRESH:
	    strcat (gd->gd_FBuffer, "other ");
	    break;
    }

    /* Misc. */
    if (flags & WFLG_GIMMEZEROZERO)
	strcat (gd->gd_FBuffer, "gzz ");

    if (flags & WFLG_RMBTRAP)
	strcat (gd->gd_FBuffer, "rmb ");

    if (flags & WFLG_NOCAREREFRESH)
	strcat (gd->gd_FBuffer, "nocare ");

    if (flags & WFLG_NW_EXTENDED)
	strcat (gd->gd_FBuffer, "ext. ");

    if (flags & WFLG_NEWLOOKMENUS)
	strcat (gd->gd_FBuffer, "nlm ");

    if (flags & WFLG_VISITOR)
	strcat (gd->gd_FBuffer, "visitor ");

    if (flags & WFLG_ZOOMED)
	strcat (gd->gd_FBuffer, "zoomed ");

    return (gd->gd_FBuffer);
}

/*****************************************************************************/

void showwindow (struct GlobalData * gd, ULONG address)
{
    struct Screen *scr;
    struct Window *win;
    BOOL found = FALSE;
    ULONG lock;

    /* Just in case we don't find the screen */
    strcpy (gd->gd_Node, "@{b}window gone@{ub}\n");

    /* Search for the screen */
    lock = LockIBase (0);
    for (scr = IntuitionBase->FirstScreen; scr && !found; scr = scr->NextScreen)
    {
	for (win = scr->FirstWindow; win && !found; win = win->NextWindow)
	{
	    /* Is this the window we were looking for? */
	    if (win == (struct Window *) address)
	    {
		/* Build the screen information */
		strcpy (gd->gd_Node, "@{b}Window@{ub}\n\n");
		bprintf (gd, "        Address: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", (void *) address, (void *) address);
		bprintf (gd, "    Next Window: @{\"%08lx\" link HYPERNOZY.WINDOW.(%08lx)}\n", win->NextWindow, win->NextWindow);
		bprintf (gd, "            Box: %ld, %ld, %ld, %ld\n", (void *) win->LeftEdge, (void *) win->TopEdge, (void *) win->Width, (void *) win->Height);
		bprintf (gd, "   Minimum Size: %ld, %ld\n", (void *) win->MinWidth, (void *) win->MinHeight);
		bprintf (gd, "   Maximum Size: %ld, %ld\n", (void *) win->MaxWidth, (void *) win->MaxHeight);
		bprintf (gd, "          Flags: %s\n", DumpWindowFlags (gd, win));
		bprintf (gd, "           Menu: @{\"%08lx\" link HYPERNOZY.MENU.(%08lx)}\n", (void *) win->MenuStrip, (void *) win->MenuStrip);
		bprintf (gd, "          Title: %s\n", ((win->Title) ? win->Title : ""));
		bprintf (gd, "   FirstRequest: @{\"%08lx\" link HYPERNOZY.REQUESTER.(%08lx)}\n", win->FirstRequest, win->FirstRequest);
		bprintf (gd, "     DMRRequest: @{\"%08lx\" link HYPERNOZY.REQUESTER.(%08lx)}\n", win->DMRequest, win->DMRequest);
		bprintf (gd, "     Req. Count: %ld\n", (void *) win->ReqCount);
		bprintf (gd, "         Screen: @{\"%08lx\" link HYPERNOZY.SCREEN.(%08lx)}\n", win->WScreen, win->WScreen);
		bprintf (gd, "       RastPort: @{\"%08lx\" link HYPERNOZY.RASTPORT.(%08lx)}\n", win->RPort, win->RPort);
		bprintf (gd, "   Border Sizes: %ld, %ld, %ld, %ld\n", (void *) win->BorderLeft, (void *) win->BorderTop, (void *) win->BorderRight, (void *) win->BorderBottom);
		bprintf (gd, "Border RastPort: @{\"%08lx\" link HYPERNOZY.RASTPORT.(%08lx)}\n", win->BorderRPort, win->BorderRPort);
		bprintf (gd, "   First Gadget: @{\"%08lx\" link HYPERNOZY.GADGETLIST.(%08lx)}\n", win->FirstGadget, win->FirstGadget);
		bprintf (gd, "  Parent Window: @{\"%08lx\" link HYPERNOZY.WINDOW.(%08lx)}\n", win->Parent, win->Parent);
		bprintf (gd, "   Desc. Window: @{\"%08lx\" link HYPERNOZY.WINDOW.(%08lx)}\n", win->Descendant, win->Descendant);
		bprintf (gd, "   Pointer Data: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", win->Pointer, win->Pointer);
		bprintf (gd, "    Pointer Box: %ld, %ld, %ld, %ld\n", (void *)win->XOffset, (void *)win->YOffset, (void *)win->PtrWidth, (void *) win->PtrHeight);
		bprintf (gd, "    IDCMP Flags: %08lx\n", (void *) win->IDCMPFlags);
		bprintf (gd, "      User Port: @{\"%08lx\" link HYPERNOZY.MSGPORT.(%08lx)}\n", win->UserPort, win->UserPort);
		bprintf (gd, "    Window Port: @{\"%08lx\" link HYPERNOZY.MSGPORT.(%08lx)}\n", win->WindowPort, win->WindowPort);
		bprintf (gd, "    Message Key: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", win->MessageKey, win->MessageKey);
		bprintf (gd, "  Detail, Block: %ld, %ld\n", (void *) win->DetailPen, (void *) win->BlockPen);
		bprintf (gd, "     Check Mark: @{\"%08lx\" link HYPERNOZY.IMAGE.(%08lx)}\n", win->CheckMark, win->CheckMark);
		bprintf (gd, "   Screen Title: %s\n", (win->ScreenTitle) ? win->ScreenTitle : "");
		bprintf (gd, "      Ext. Data: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", win->ExtData, win->ExtData);
		bprintf (gd, "      User Data: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", win->UserData, win->UserData);
		bprintf (gd, "          Layer: @{\"%08lx\" link HYPERNOZY.LAYER.(%08lx)}\n", win->WLayer, &win->WLayer);
		bprintf (gd, "           Font: @{\"%08lx\" link HYPERNOZY.FONT.(%08lx)}\n", win->IFont, win->IFont);
		bprintf (gd, "     More Flags: %08lx\n", (void *) win->MoreFlags);

		/* Show that we found it */
		found = TRUE;
	    }
	}
    }
    UnlockIBase (lock);

}
