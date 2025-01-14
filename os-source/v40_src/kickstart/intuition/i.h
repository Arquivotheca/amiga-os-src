
/*** i.h ******************************************************************
 *
 *  i.h internal version of intuition.h
 *
 *  $Id: i.h,v 38.16 93/01/08 14:45:24 peter Exp $
 *
 ****************************************************************************
 * CONFIDENTIAL and PROPRIETARY
 * Copyright (C) 1985, COMMODORE-AMIGA, INC.
 * All Rights Reserved
 ****************************************************************************/

/******* INCLUDE PUBLIC COPY OF INTUITION.H ******/
#include "intuition.h"	/* Include from local directory */

#define XWINDOW(w)	((struct XWindow *) (w))
#define XGAD(g)		((struct ExtGadget *) (g))

/* window structure with private extension */
struct XWindow
{
    /* good old public window structure	*/
    struct Window	xw_Window;

    /**** Private extension fields ****/

    SHORT LastMouseY;	/* last values sent in MOUSEMOVE report	*/
    SHORT LastMouseX;

    struct IBox	UnzoomBox;	/* previous stuff	*/
    struct IBox	ZoomBox;

    SHORT	MouseQueueLimit;
    SHORT	MousePending;

    SHORT	RptQueueLimit;
    SHORT	RptPending;

    WORD	MenuBlockPen;	/* BlockPen or Pens[BARBLOCKPEN] */
    WORD	MenuDetailPen;	/* DetailPen or Pens[BARDETAILPEN] */

    struct Image *AmigaIcon;	/* Symbol for Amiga-key equiv */

    struct MsgPort WinPort;	/* We now embed the WindowPort, eliminating
				 * the prime cause of ModifyIDCMP() failure
				 */
    UWORD	WinTitleLength;	/* We cache the window title length
				 * to avoid TextFit() during redraw.
				 */

    /* This points to this window's MousePointer.  For old-style windows,
     * we end up with:
     *
     *	XWINDOW(w)->MousePointer = &w->Pointer;
     */
    struct MousePointer *MousePointer;	/* Points to this window's MousePointer */

    /* If non-NULL, then this window wants to lend its menu-action
     * to the specified window.
     */
    struct Window *MenuLend;

    /* WA_PointerDelay requests stash the info here */
    struct MousePointer *DeferredPointer;
    WORD DeferredCounter;

    /* When the active window has WMF_GADGETHELP set, all windows
     * of the same group will get GadgetHelp.
     */
    ULONG HelpGroup;

    struct Rectangle TitleExtent;
};

/* --- V36 Private MoreFlags to be used only by Intuition -----------	*/
#define WMF_NEEDMENUCLEAR	0x00000001 /* was sent a menuwaiting event */
#define WMF_USEMENUHELP		0x00000002 /* enable Menu Help key	*/
/* --- V39 Private MoreFlags to be used only by Intuition -----------	*/
#define WMF_NOTIFYDEPTH		0x00000008 /* depth-arrange to send CHANGEWINDOW? */
#define WMF_TABLETMESSAGES	0x00000010 /* Window wants TabletData in IDCMP messages */
#define WMF_GADGETHELP		0x00000020 /* Window wants GADGETHELP */
#define WMF_DEFERREDPOINTER	0x00000040 /* Deferred pointer change pending */
#define WMF_BEGINUPDATE		0x00000100 /* BeginUpdate succeeded */
#define WMF_GADGETSCROLLRASTER	0x00000200 /* A gadget has GMORE_SCROLLRASTER */

/* 0x00000004, 0x00000080 unused */

/* --- Other Window Values ---------------------------------------------- */
#define DEFAULTMOUSEQUEUE	(5)	/* no more mouse messages	*/
#define DEFAULTRPTQUEUE		(3)	/* no more repeat key messages	*/

/* new/replacement system gadget type */
#define WZOOM WDOWNBACK		/* new 		*/
