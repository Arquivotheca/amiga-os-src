/*** menu.c *****************************************************************
 *
 *  File
 *
 *  $Id: menu.c,v 38.23 93/02/15 19:01:55 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#ifndef EXEC_ALERTS_H
#include <exec/alerts.h>
#endif

#include <intuition/imageclass.h>

#define TEXTPAD 4

#define DEBUG 0
#define D(x)	;
#define DOOM(x)	;
#define DB(x)	;

#define SWAP_OFF_TO_SCREEN	1
#define SWAP_SCREEN_TO_OFF	0

#define swapMenuOut( sc, cr ) menuBlit( sc, (cr)->BitMap, &(cr)->bounds, SWAP_OFF_TO_SCREEN )
#define freeMenuBitMap( bm ) FreeBitMap( bm )

/* = declarations ======================================================== */

/* This function should really take a bitmap pointer instead of
 * effectively getting it out of IBase->MenuRPort.  However, that
 * cost ROM space and we can't afford it for 3.01.  Next time.
 */

menuImage(firstitem, menurect, realrect, disabled)
struct MenuItem *firstitem;
struct Rectangle *menurect;
struct Rectangle *realrect;
BOOL disabled;
{
    struct MenuItem *item;

    extern USHORT	BPattern[];
    int left, top, i, topoffset;
    int xedge;
    struct Window *AWindow;
    struct IntuiText itext;	/* used for command key only */
    UBYTE charbuf[2];

    /* jimm's stuff	*/
    struct	TextFont	*tmpfp;
    struct	TextFont	*commseqfont;
    int 			commseqindent = 0;
    int 			commseqbaseline = 0;
    struct TTextAttr		plainttattr;
    struct TextAttr		*commseqtattr;

    struct IntuitionBase *IBase = fetchIBase();
    struct RastPort *rport = &IBase->MenuRPort;

    struct	TextFont	*OpenFont();

    /* ClipRect bitmaps have to be word-aligned.  This is the
     * offset into that BitMap caused by the fact that the real
     * rectangle isn't necessarily word-aligned.
     */
    xedge = realrect->MinX & 0xF;

    D( printf("\nmenuImage:item%lx,x/yoff %ld/%ld,\n\t width/height %ld/%ld\n",
	item, xoffset, yoffset, rectWidth( realrect ), rectHeight( realrect ) ) );
    D( printf("\txedge %ld, disabled %ld\n", xedge, disabled) );

    AWindow = IBase->ActiveWindow;
    tmpfp = ISetFont(rport, AWindow->WScreen->Font);

    rport->Mask = -1;
    SetDrMd(rport, JAM2);
    SetRast(rport, XWINDOW(AWindow)->MenuBlockPen);
    drawBox(rport, xedge, 0,
	rectWidth( realrect ), rectHeight( realrect ),
	XWINDOW(AWindow)->MenuDetailPen, JAM1, 2, 1);
    itext.FrontPen = XWINDOW(AWindow)->MenuDetailPen;
    itext.DrawMode = JAM1;
    itext.ITextFont = NULL;
    itext.NextText = NULL;
    itext.IText = charbuf;

    charbuf[1] = '\0';

    /* jimm: 12/16/89: two-passes: first figure out
     * maximum command-key indentation.
     */
    for ( item = firstitem; item; item = item->NextItem )
    {
	if (item->ItemFill)
	{
	    /* use (measure) in the item's specified font */
	    itext.ITextFont = (item->Flags & ITEMTEXT)?
		    ((struct IntuiText *)item->ItemFill)->ITextFont: NULL;

	    /* could let non-text items with commseq's use previous
	     * (or "leftover") font, I suppose.
	     */

	    if (item->Flags & COMMSEQ)
	    {
		/* get the largets inset value for comsseq chars */
		charbuf[0] = item->Command;
		commseqindent = imax( commseqindent,
		    RastPortITextLength(rport, &itext) );
	    }
	}
    }

    /* now, with indentation 'commseqindent' calculated,
     * render the menu items
     */
    for ( item = firstitem; item; item = item->NextItem )
    {
	left = item->LeftEdge - menurect->MinX + xedge;
	top = item->TopEdge - menurect->MinY;
	commseqtattr = NULL;

	if (item->ItemFill)
	    {
	    if (item->Flags & ITEMTEXT)
		{
		topoffset = top+((struct IntuiText *)item->ItemFill)->TopEdge;

		DB( printf("menuImage text <%s>\n",
			((struct IntuiText *)item->ItemFill)->IText ) );

		PrintIText(rport, item->ItemFill, left, top);

		/** use the same font as (most recent) IntuiText **/
		if ( ((struct IntuiText *)item->ItemFill)->ITextFont )
		    {
		    commseqtattr = ((struct IntuiText *)item->ItemFill)->ITextFont;
		    }
		}
	    else
		{
		topoffset = top + ((struct Image *)item->ItemFill)->TopEdge;
		/* Peter 26-Mar-91: Think of this as DrawImageState(IDS_NORMAL) */
		DrawImage(rport, item->ItemFill, left, top);
		}
	    }

	/* add the checkmark as needed */
	if ((item->Flags & CHECKIT) && (item->Flags & CHECKED))
	    /* render the checkmark */
	    DrawImage(rport, AWindow->CheckMark, left, topoffset);

	/* has this baby got an equivalent command-key sequence? */
	if (item->Flags & COMMSEQ)
	{
	    /* ok, draw the amiga-glyph, and then the character */
	    /* jimm: 3/13/89 * use text length for prop. font */
	    charbuf[0] = item->Command;
	    i = left + item->Width - commseqindent - 2;

	    /*  Did we find a suitable font to render the command-key in? */
	    if ( commseqtattr )
	    {
		/* Let's try to get the same font, in plain style.
		 * First, copy the non-extended TextAttr.  Then,
		 * if the TextAttr was tagged, copy tta_Tags too.
		 * Then, clear all the style bits.
		 */
 		*((struct TextAttr *)&plainttattr) = *commseqtattr;

		if ( TESTFLAG( commseqtattr->ta_Style, FSF_TAGGED ) )
		{
		    plainttattr.tta_Tags =
			((struct TTextAttr *)commseqtattr)->tta_Tags;
		}
		/* Clear all style bits */
		plainttattr.tta_Style &= ~(FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);

		if  ( commseqfont = OpenFont( &plainttattr ) )
		{
		    commseqbaseline = commseqfont->tf_Baseline;
		    ICloseFont( commseqfont );
		    commseqtattr = &plainttattr;
		}
		else
		{
		    commseqbaseline = rport->TxBaseline;
		    commseqtattr = NULL;
		}
	    }

	    /* jimm 5/3/90: align amiga glyph at baseline	*/
	    /* Peter 16-Aug-90: Don't let it be higher than it was in 1.3 */
	    DrawImage( rport, XWINDOW(AWindow)->AmigaIcon,
		i - XWINDOW(AWindow)->AmigaIcon->Width - 2,
		topoffset + imax( 0,
		commseqbaseline - (XWINDOW(AWindow)->AmigaIcon->Height-1) ) );
	    itext.LeftEdge = i;
	    itext.TopEdge = topoffset;
	    itext.ITextFont = commseqtattr;
	    PrintIText(rport, &itext, 0, 0);
	}

	/* if disabled is true, every item is ghosted */
	if ( disabled || ((item->Flags & ITEMENABLED) == NULL) )
	    /* render the "ghosting" */
	    BlastPattern( rport, left, top, 
		    left + item->Width - 1, top + item->Height - 1,
		    BPattern, 1, XWINDOW(AWindow)->MenuBlockPen, 0, JAM1);

	/** using for(;;) loop item = item->NextItem; */
    }
    ICloseFont(tmpfp);	/* jimm: 6/26/86: */
}


#if 0
/* Nice try, but PhotonPaint II puts IntuiText underneath the
 * checkmark, so the kludge that is here is needed, but Tax-Break
 * uses XOR text in their menus, so the same kludge breaks them.
 * Someday I hope to return here and make everything well again...
 */

/* checkmarkMenu()
 * Re-render checkmarks of a displayed menu panel.
 * Pass it the first MenuItem of an itempanel,
 * or pass it the MenuItem that springs a submenu.
 */

checkmarkMenu(menu, firstitem, sub)
struct Menu *menu;
struct MenuItem *firstitem;
BOOL		sub;
{
    int xoffset, yoffset;
    struct MenuItem *item;
    int left, top, topoffset;
    struct Rectangle rect;

    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *AScreen = IBase->ActiveScreen;
    struct Window *AWindow = IBase->ActiveWindow;
    struct RastPort *rport;

    /* These equations were stolen from highGrunt */
    xoffset = AScreen->BarHBorder + menu->LeftEdge;
    yoffset = (AScreen->BarHeight - 1) + AScreen->MenuVBorder;

    if (sub)
    {
	xoffset += firstitem->LeftEdge;
	yoffset += firstitem->TopEdge;
	firstitem = firstitem->SubItem;
    }

    rport = obtainRP( XSC( AScreen )->ClipLayer->rp, XSC( AScreen )->ClipLayer );

    for ( item = firstitem; item; item = item->NextItem )
    {
	if (item->Flags & CHECKIT)
	{
	    left = xoffset + item->LeftEdge;
	    top = yoffset + item->TopEdge;

	    if (item->ItemFill)
	    {
		if (item->Flags & ITEMTEXT)
		{
		    topoffset = top + ((struct IntuiText *)item->ItemFill)->TopEdge;
		}
		else
		{
		    topoffset = top + ((struct Image *)item->ItemFill)->TopEdge;
		}
	    }
	    /* add the checkmark as needed */
	    if (item->Flags & CHECKED)
	    {
		/* render the checkmark */
		DrawImage(rport, AWindow->CheckMark, left, topoffset);
	    }
	    else
	    {
		boxToRect( (struct Box *)&AWindow->CheckMark->LeftEdge, &rect );
		offsetRect( &rect, left, topoffset );

		/* ZZZ: This used to call FillRect(), which is now deleted.
		 * the new code is untested
		 */
		/* erase the checkmark's spot */
		SetAPen( rport, XWINDOW(AWindow)->MenuBlockPen );
		SafeRectFill( rport, rect.MinX, rect.MinY, rect.MaxX, rect.MaxY );

		/* For people like PhotonPaintII, who use an image
		 * to illustrate the "unchecked" state.
		 * We die on any XOR-mode text.
		 */
		if (item->Flags & ITEMTEXT)
		{
		    PrintIText(rport, item->ItemFill, left, top);
		}
		else
		{
		    DrawImage(rport, item->ItemFill, left, top);
		}
	    }
	}
    }
    freeRP( rport );
}
#endif

itemExtent( window, item, rect )
struct Window *window;
struct MenuItem *item;
struct Rectangle *rect;
{
    int i;
    APTR fill;
    struct RastPort *rp = &window->WScreen->RastPort;

    /* Item select box, as programmer provides */
    boxToRect( &item->LeftEdge, rect );

    fill = item->ItemFill;
    for ( i = 0; i < 2; i++ )
    {
	if ( fill )
	{
	    /* Let's get the extent of the imagery: */
	    struct IBox itembox;
	    WORD imheight;

	    if ( TESTFLAG( item->Flags, ITEMTEXT ) )
	    {
		struct TextAttr	*tattr;
		tattr = ((struct IntuiText *) fill)->ITextFont;

		itembox.Left = ((struct IntuiText *) fill)->LeftEdge;
		itembox.Top = ((struct IntuiText *) fill)->TopEdge;
		itembox.Width = RastPortITextLength( rp, fill );
		itembox.Height = rp->TxHeight;
		if ( tattr )
		{
		    itembox.Height = tattr->ta_YSize;
		}
	    }
	    else
	    {
		itembox = *(struct IBox *)(&((struct Image *)fill)->LeftEdge);
	    }

	    imheight = XWINDOW(window)->AmigaIcon->Height;
	    if ( TESTFLAG( item->Flags, COMMSEQ ) &&
		( itembox.Height < imheight ) )
	    {
		itembox.Height = imheight;
	    }
	    imheight = window->CheckMark->Height;
	    if ( TESTFLAG( item->Flags, CHECKIT ) && 
		( itembox.Height < imheight ) )
	    {
		itembox.Height = imheight;
	    }

	    /* After this, itembox is actually a struct Rectangle */
	    boxToRect( &itembox, &itembox );
	    /* Offset the imagery relative to the menu item,
	     * and stretch our result to include it
	     */
	    offsetRect( &itembox, item->LeftEdge, item->TopEdge );
	    rectHull( rect, &itembox );
	}
	fill = item->SelectFill;
    }
}

/* this routine feels through the item list and fills in the variables with
 *  data describing:
 *	- the leftmost position specified by any of the items
 *	- the topmost position described by the items
 *	- the exact rightmost position
 *	- the exact bottommost position.  This includes subtracting one from
 *	  the height of text characters (for the blank line)
 * These are derived by examining ItemFills (Text or Image), the
 * select box of the items, and SelectFill if any.
 *
 * Then, we include the menu border padding stuff, then enforcer certain
 * spatial relationships with respect to the parent item or menu.
 */
boxer( window, firstitem, rect, hullrect, issubmenu )
struct Window *window;
struct MenuItem *firstitem;
struct Rectangle *rect;
int issubmenu;
{
    struct Rectangle work; /* temporary rectangle for each item */
    struct MenuItem *item;
    struct Screen *screen = window->WScreen;

    degenerateRect( rect );	/* creates maximally inside out rect */

    for ( item = firstitem; item; item = item->NextItem )
    {
	itemExtent( window, item, &work );

	/* fit item into item's menu */
	rectHull( rect, &work );	/* expand total extent */
	DB( dumpRect("work", &work) );
	DB( dumpRect("rect", rect) );
    }

    if ( !issubmenu )
    {
#if 0
	if ( rect->MinY < 0 )
	{
	    Alert( AN_ItemBoxTop );	/* Why bother? */
	}
#endif
	rect->MinY = 0;
    }
    /* Add the menu panel trim */
    rect->MinX -= screen->MenuHBorder;
    rect->MinY -= screen->MenuVBorder;
    rect->MaxX += screen->MenuHBorder;
    rect->MaxY += screen->MenuVBorder;

    /* Here, we enforce certain supplied criteria:
     * A menu-panel must span the width of the menu header.
     * A submenu-panel must touch at least a corner of its parent item.
     */
    rectHull( rect, hullrect );

    DB( printf("boxer: X: %ld-%ld Y: %ld-%ld\n",
	rect->MinX, rect->MaxX, rect->MinY, rect->MaxY) );
}

struct Menu *grabMenu(menu, menuNum)
register struct Menu *menu;
register USHORT menuNum;
/* returns the address of the n-th menu in the list starting from the arg */
{
    SHORT menupick;

    if ((menupick = MENUNUM(menuNum)) == NOMENU) return(NULL);

    while (menupick-- > 0) menu = menu->NextMenu;
    return(menu);
}


struct MenuItem *grabItem(menu, menuNum)
register struct Menu *menu;
register SHORT menuNum;
/* returns the adress of the n-th item of the argument menu */
{
    SHORT itempick;
    struct MenuItem *item;

    item = menu->FirstItem;

    if ((itempick = ITEMNUM(menuNum)) == NOITEM) return(NULL);

    while (itempick-- > 0) item = item->NextItem;
    return(item);
}


struct MenuItem *grabSub(item, menuNum)
register struct MenuItem *item;
register SHORT menuNum;
/* returns the address of the n-th subitem of the argument item */
{
    SHORT subpick;
    struct MenuItem *sub;

    sub = item->SubItem;

    if ((subpick = SUBNUM(menuNum)) == NOSUB) return(NULL);

    while (subpick-- > 0) sub = sub->NextItem;
    return(sub);
}


eraseItem()
{
    struct Menu *menu;
    struct IntuitionBase *IBase = fetchIBase();

    /* swap our menu rectangle out of the Active RastPort */

    if (IBase->Flags & GOODITEMDRAWN)
    {
	swapMenuOut( IBase->ActiveScreen, &IBase->ItemCRect );
    }

    if ( IBase->ItemCRect.BitMap )
    {
	WaitBlit();
	freeMenuBitMap( IBase->ItemCRect.BitMap );
    }

    IBase->Flags &= ~ITEMDRAWN;

    if (menu = grabMenu(IBase->ActiveWindow->MenuStrip, IBase->MenuDrawn))
	menu->Flags &= ~MIDRAWN;

    SETDMENU(NOMENU);
}


eraseSub()
{
    int i;
    struct MenuItem *item;
    struct Menu *menu;
    struct IntuitionBase *IBase = fetchIBase();

    /* swap our menu rectangle out of the Active RastPort */
    if (IBase->Flags & GOODSUBDRAWN)
    {
	D( printf("eraseSub, plane 0 at %lx\n",
		IBase->SubCRect.BitMap->Planes[0] ) );
	D( printf( "rastport %lx, rpbitmap %lx, plane 0 %lx\n",
		&IBase->ActiveScreen->RastPort,
		IBase->ActiveScreen->RastPort.BitMap,
		IBase->ActiveScreen->RastPort.BitMap->Planes[0] ) );
	D( kpause("--") );
	swapMenuOut( IBase->ActiveScreen, &IBase->SubCRect );
    }
    D( else printf("eraseSub: won't do it: no goodsubdrawn\n" ) );

    if ( IBase->SubCRect.BitMap )
    {
	WaitBlit();
	freeMenuBitMap( IBase->SubCRect.BitMap );
    }

    IBase->Flags &= ~SUBDRAWN;

    i = IBase->MenuDrawn;
    if (menu = grabMenu(IBase->ActiveWindow->MenuStrip, i))
	if (item = grabItem(menu, i))
	    item->Flags &= ~ISDRAWN;

    SETDITEM(NOITEM);
}


SHORT hitMenu()
/* returns the menu number (nonshifted) or NOMENU from the current mouse
 *  position trying to collide with the menu bar
 */
{
    register struct Menu *menu;
    register int num;
    struct Point pt;
    struct IBox menubox;
    struct Screen *AScreen;
    struct IntuitionBase *IBase = fetchIBase();

    AScreen = IBase->ActiveScreen;

    /* This should have been BarHBorder, but we leave it as VBorder,
     * since it gets rendered this same wrong way
     */
    pt.X = AScreen->MouseX - AScreen->BarVBorder;
    pt.Y = AScreen->MouseY;
    menubox.Top = 0;
    menubox.Height = AScreen->BarHeight;
    num = 0;
    menu = IBase->ActiveWindow->MenuStrip;

    while (menu)
	{
	menubox.Left = menu->LeftEdge;
	menubox.Width = menu->Width;
	if ( ptInBox( pt, &menubox ) )
	    return(num);
	num++;
	menu = menu->NextMenu;
	}

    return(NOMENU);
}


USHORT hitItem()
/* returns the item number (nonshifted) or NOITEM from the current mouse
 *  position trying to collide with MenuDrawn
 */
{
    return(hitGrunt(FALSE));
}



SHORT hitSub()
/* returns the subitem number (nonshifted) or NOSUB from the current mouse
 *  position trying to collide with MenuDrawn
 */
{
    register SHORT num;

    if ((num = hitGrunt(TRUE)) == NOITEM) return (NOSUB);
    else return(num);
}


SHORT hitGrunt(subsearch)
BOOL subsearch;
/* returns the subitem number (nonshifted) or NOSUB from the current mouse
 *  position trying to collide with MenuDrawn
 */
{
    register struct Menu *menu;
    register struct MenuItem *item;
    register int num;
    struct Point rel;
    struct Screen *AScreen;
    struct IntuitionBase *IBase = fetchIBase();

    AScreen = IBase->ActiveScreen;

    num = 0;
    menu = grabMenu(IBase->ActiveWindow->MenuStrip, IBase->MenuDrawn);

    /* Peter 9-Aug-90: Fixed that off-by-four error in selecting
     * menu items:
     */
    rel.X = AScreen->MouseX - AScreen->BarHBorder - menu->LeftEdge;
    rel.Y = AScreen->MouseY - AScreen->BarHeight - AScreen->MenuVBorder + 1;

    if (subsearch)
	{
	item = grabItem(menu, IBase->MenuDrawn);
	rel.X -= item->LeftEdge;
	rel.Y -= item->TopEdge;
	item = item->SubItem;
	}
    else item = menu->FirstItem;

    while (item)
	{
	if ( ptInBox( rel, &item->LeftEdge ) )
	    return(num);
	num++;
	item = item->NextItem;
	}

    return(NOITEM);
}


highMenu()
/* highlights the menu of MenuSelected
 * presumes that the menubar is already drawn
 */
{
    struct Menu *menu;
    struct Screen *AScreen;
    struct RastPort *RP;
    struct IntuitionBase *IBase = fetchIBase();

    AScreen = IBase->ActiveScreen;

    menu = grabMenu(IBase->ActiveWindow->MenuStrip, IBase->MenuSelected);

    if (menu->Flags & MENUENABLED)
    {
	RP = obtainRP(XSC(AScreen)->ClipLayer->rp, XSC(AScreen)->ClipLayer);

    /* This should have been BarHBorder, but we leave it as VBorder,
     * since we shouldn't move the rendering, for compatibility
     */
	setMenuRPMask(RP);
	xorboxmask(RP, AScreen->BarVBorder + menu->LeftEdge, 
	    0, menu->Width, AScreen->BarHeight);
	freeRP(RP);
    }
}

/* setMenuRPMask() checks if the window has newlook-menus.  If so,
 * it sets up the Mask that will allow complement-mode operations
 * to interchange MenuDetailPen with MenuBlockPen.
 */
setMenuRPMask( rport )
struct RastPort *rport;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Window *AWindow = IBase->ActiveWindow;
    rport->Mask = -1;
    if ( TESTFLAG( AWindow->Flags, WFLG_NEWLOOKMENUS ) )
    {
	rport->Mask = XWINDOW(AWindow)->MenuDetailPen ^ XWINDOW(AWindow)->MenuBlockPen;
    }
}

highGrunt(item, left, top, AScreen)
struct MenuItem *item;
int left, top;
struct Screen *AScreen;
/* highlights the item of MenuSelected
 * presumes that the itemlist is already drawn
 */
{
    struct RastPort *RP = obtainRP(XSC(AScreen)->ClipLayer->rp, XSC(AScreen)->ClipLayer);
    APTR fill;
    ULONG state;
    struct TextFont	*tmpfp;

    switch (item->Flags & HIGHFLAGS)
	{
	case HIGHCOMP:
		setMenuRPMask(RP);
		xorboxmask(RP, left, top, item->Width, item->Height);
		RP->Mask = -1;
		break;

	case HIGHBOX:
		setMenuRPMask(RP);
		outbox(RP, left, top, item->Width, item->Height);
		RP->Mask = -1;
		break;

	case HIGHIMAGE:

	    /* Assume item is in selected-state */
	    fill = item->SelectFill;
	    state = IDS_SELECTED;
	    if (item->Flags & HIGHITEM)
	    {
		/* Item is not selected (note that HIGHITEM gets inverted below) */
		fill = item->ItemFill;
		state = IDS_NORMAL;
	    }
	    /* jimm: 7/20/86: alternate text didn't work!! */
	    if (item->Flags & ITEMTEXT)
	    {
		/* Peter 2-Apr-91: Set up default font for this RastPort */
		tmpfp = ISetFont(RP, AScreen->Font);
		PrintIText(RP, fill, left, top);
		ICloseFont(tmpfp);
	    }
	    else
	    {
		/* Peter 26-Mar-91: Pass the state to the image */
		DrawImageState(RP, fill, left, top, state, NULL);
	    }

		break;

	case HIGHNONE:
		break;
	}

    /* lastly, invert the HIGHITEM bit */
    item->Flags ^= HIGHITEM;
    freeRP(RP);
}


highItem()
/* highlights the item of MenuSelected
 * presumes that the itemlist is already drawn
 */
{
    /* conditional jimm: 7/8/86 */
    if (fetchIBase()->Flags & GOODITEMDRAWN) highPrimeGrunt(FALSE);
}


highSub()
/* highlights the subitem of MenuSelected
 * presumes that the sublist is already drawn
 */
{
    /* conditional jimm: 7/8/86 */
    if (fetchIBase()->Flags & GOODSUBDRAWN) highPrimeGrunt(TRUE);
}


highPrimeGrunt(subsearch)
BOOL subsearch;
{
    struct Menu *menu;
    struct MenuItem *item, *sub;
    int num, left, top;
    struct Screen *AScreen;
    struct IntuitionBase *IBase = fetchIBase();

    AScreen = IBase->ActiveScreen;

    num = IBase->MenuSelected;

    menu = grabMenu(IBase->ActiveWindow->MenuStrip, num);
    if ((menu->Flags & MENUENABLED) == NULL) return;

    item = grabItem(menu, num);
    if ((item->Flags & ITEMENABLED) == NULL) return;
    left = AScreen->BarHBorder + menu->LeftEdge + item->LeftEdge;
    top = (AScreen->BarHeight - 1) + AScreen->MenuVBorder + item->TopEdge;

    if (subsearch)
	{
	sub = grabSub(item, num);
	if ((sub->Flags & ITEMENABLED) == NULL) return;
	left += sub->LeftEdge;
	top += sub->TopEdge;
	item = sub;
	}

    highGrunt(item, left, top, AScreen);
}




showMStrip(w)
register struct Window *w;
/*    Displays the menu bar of the window in its screen title bar */
{
    extern USHORT	BPattern[];
    register struct Screen *s;
    register struct Menu *m;
    struct IntuiText itext;
    struct RastPort *rp;

    /** ZZZ: why? **/
    LockLayerInfo(&w->WScreen->LayerInfo);

    resetMenu(w, ~MIDRAWN, ~ISDRAWN & ~HIGHITEM & ~MENUTOGGLED, 
	    ~HIGHITEM & ~MENUTOGGLED);

    s = w->WScreen;

    forceBarFront(s);

    rp = obtainRP(s->BarLayer->rp, NULL);

    m = w->MenuStrip;

    itext.FrontPen = XWINDOW(w)->MenuDetailPen;
    itext.BackPen = XWINDOW(w)->MenuBlockPen;

    if ( !m )
    {
	/* Menuless windows always get new-look colors */
	itext.FrontPen = XSC(s)->SDrawInfo.dri_Pens[ BARDETAILPEN ];
	itext.BackPen = XSC(s)->SDrawInfo.dri_Pens[ BARBLOCKPEN ];
    }

#if 1
    drawScreenBar( rp, s, itext.BackPen );
#else
    drawBlock(rp, 0, 0, s->Width - 1, s->BarHeight-1, itext.BackPen);
    drawBlock(rp, 0, s->BarHeight, s->Width - 1, s->BarHeight,
	XSC(s)->SDrawInfo.dri_Pens[ BARTRIMPEN ]);
#endif

    itext.DrawMode = JAM2;
    itext.TopEdge = s->BarVBorder;
    itext.LeftEdge = s->BarHBorder;
    itext.ITextFont = s->Font;
    itext.NextText = NULL;

    while (m)
	{
	if (m->MenuName)
	    {
	    itext.IText = m->MenuName;
	    PrintIText(s->BarLayer->rp, &itext, m->LeftEdge, 0);
	    if (!(m->Flags & MENUENABLED))
		BlastPattern(s->BarLayer->rp,
			s->BarHBorder + m->LeftEdge, 0,
			s->BarHBorder + m->LeftEdge + m->Width - 1, 
			s->BarHeight - 1,
			BPattern, 1,
			itext.BackPen, 0, JAM1);
	    }
	m = m->NextMenu;
	}

    /** ZZZ: why? **/
    freeRP(rp);
    UnlockLayerInfo(&w->WScreen->LayerInfo);
}


removeMStrip( s )
struct Screen *s;
{
    if (s->Flags & SHOWTITLE) forceBarCenter(s);
    else forceBarBack(s);
    screenbar( s );
    BorderPatrol( s );
    /* no more MENULOCK */
    /* IBase->Flags &= ~INMENUSTATE; */
}


resetMenu(w, mflags, iflags, sflags)
struct Window *w;
USHORT mflags, iflags, sflags;
/* clears all the DRAWN flags in the entire MenuStrip */
{
    struct Menu *menu;
    struct MenuItem *item, *sub;

    if (w)
	{
	menu = w->MenuStrip;

	while (menu)
	    {
	    item = menu->FirstItem;
 	    while (item)
		{
		item->Flags &=  iflags;
		sub = item->SubItem;
 		while (sub)
		    {
		    sub->Flags &= sflags;
		    sub = sub->NextItem;
		    }
		item = item->NextItem;
		}
	    menu->Flags &= mflags;
	    menu = menu->NextMenu;
	    }
	}
}


/* Allocates a menu bitmap, given the screen, and the four extremities
 * of the rectangle.  Fails if the rectangle is degenerate vertically.
 */

struct BitMap *allocMenuBitMap( ascreen, rect )
struct Screen *ascreen;
struct Rectangle *rect;
{
    struct BitMap *bm = NULL;
    long height = rectHeight( rect );
    /* The Menu BitMap needs the same relative alignment as
     * the screen.
     */
    long width = ( rect->MaxX & (~0xF)) - (rect->MinX & (~0xF)) + 16;

    if ( height > 0 )
    {
	bm = (struct BitMap *)AllocBitMap( width, height,
	    ascreen->RastPort.BitMap->Depth, NULL,
	    ascreen->RastPort.BitMap );
    }
    return( bm );
}



/* drawItem **************************************************************
*
*NAME
*   drawItem  --  draws the item box that contains the specified item
*
*SYNOPSIS
*   drawItem(menuNumber);
*
*FUNCTION
*   Draws the box containing the item of the MenuStrip of the ActiveWindow
*   as specified by the menuNumber argument.   Wants MenuNumber to have 
*   valid menu and item components, doesn't care about subitem
*   Sets menu component of IBase->MenuDrawn to show that this menu's
*   itemlist was drawn.
*   Sets the menu's MIDRAWN flag that the menu's items were drawn
*   WARNING:  this routine does not check whether or not the menu number is
*	out of bounds!
*
*INPUTS
*   menuNumber = an Intuition menu number
*
*RESULT
*   None
*
*BUGS
*   None
*
*SEE ALSO
*   None
*/
drawItem(num)
USHORT num;
{
    struct Menu *menu;
    struct Rectangle screenrect;
    struct Screen *AScreen;
    struct IntuitionBase *IBase = fetchIBase();

    IBase->Flags &= ~GOODITEMDRAWN;

    AScreen = IBase->ActiveScreen;

    menu = grabMenu(IBase->ActiveWindow->MenuStrip, num);
    /* JazzX/Y,BeatX/Y is nothing other than a struct Rectangle
     * describing the menu's extent, calculated at SetMenuStrip()
     * time.  So let's copy that into screenrect, which must
     * be shifted by the menu trim-border and the bar-height.
     */
    screenrect = *( (struct Rectangle *) &menu->JazzX );
    offsetRect( &screenrect, menu->LeftEdge + AScreen->BarHBorder,
	AScreen->BarHeight - 1 + AScreen->MenuVBorder );

    /* make sure the menu box is onscreen */
    if (screenrect.MinX < 0)
    {
	screenrect.MaxX -= screenrect.MinX;	/* move it right some more */
	screenrect.MinX = 0;
    }

    /* Let's allocate the Item ClipRect's BitMap, complete with planes */

    if ( IBase->ItemCRect.BitMap = allocMenuBitMap( AScreen, &screenrect ) )
    {
	IBase->MenuRPort.BitMap = IBase->ItemCRect.BitMap;

	/* Render the MenuItem panel whose menu extent is in Jazz/Beat,
	 * and whose on-screen position is screenrect.
	 */
	menuImage( menu->FirstItem, (struct Rectangle *)&menu->JazzX, &screenrect,
	    !TESTFLAG( menu->Flags, MENUENABLED ) );

	IBase->ItemCRect.bounds = screenrect;

	/* swap our menu rectangle into the Active RastPort */
	if ( swapMenuIn( AScreen, &IBase->ItemCRect ) )
	{
	    IBase->Flags |= GOODITEMDRAWN;
	}
    }

    IBase->Flags |= ITEMDRAWN;
    menu->Flags |= MIDRAWN;
    SETDMENU(MENUNUM(num));
}

/* drawSub **************************************************************
*
*NAME
*   drawSub  --  draws all menus specified by the MenuNumber
*
*SYNOPSIS
*   drawSub(menuNumber);
*
*FUNCTION
*   Draws the menu piece of the MenuStrip of the ActiveWindow
*   as specified by the menuNumber argument.   Wants MenuNumber to have 
*   valid menu, item and sub components, doesn't care about subitem
*   Sets item component of IBase->MenuDrawn  to show that this item's
*   sublist was drawn
*   Sets the item's ISDRAWN flag that the item's subitems were drawn
*   WARNING:  this routine does not check whether or not the menu number is
*	out of bounds!
*
*INPUTS
*   menuNumber = an Intuition menu number
*
*RESULT
*   None
*
*BUGS
*   None
*
*SEE ALSO
*   None
*/
drawSub(num)
USHORT num;
{
    struct Menu *menu;
    struct MenuItem *item;

    struct Rectangle menurect;
    struct Rectangle screenrect;
    struct Rectangle hullrect;

    struct Screen *AScreen;
    struct IntuitionBase *IBase = fetchIBase();

    AScreen = IBase->ActiveScreen;

    IBase->Flags &= ~GOODSUBDRAWN;

    D( printf("---\ndrawSub enter, screen %lx\n", AScreen) );

    menu = grabMenu(IBase->ActiveWindow->MenuStrip, num);
    item = grabItem(menu, num);

    /* The subitem box must touch at least the corner of the parent item: */
    hullrect.MinX = item->Width - 1;
    hullrect.MinY = item->Height - 1;
    hullrect.MaxX = 0;
    hullrect.MaxY = 0;
    boxer( IBase->ActiveWindow, item->SubItem, &menurect, &hullrect, 1 );

    D( dumpRect("subitem menurect", &menurect) );

    /* The screen rectangle is the menu's rectangle, offset by the
     * item's position plus the menu trim-border and the bar-height.
     */
    screenrect = menurect;
    offsetRect( &screenrect,
	item->LeftEdge + menu->LeftEdge + AScreen->BarHBorder,
	item->TopEdge + ( AScreen->BarHeight - 1 + AScreen->MenuVBorder ) );

    D( dumpRect("offset screenrect", &screenrect) );

    D( printf("BX/YOffset %ld/%ld\n", BXOffset, BYOffset) );

    /* Let's allocate the SubItem ClipRect's BitMap, complete with planes */

    if ( IBase->SubCRect.BitMap = allocMenuBitMap( AScreen, &screenrect ) )
    {
	IBase->MenuRPort.BitMap = IBase->SubCRect.BitMap;

	/* Render the SubItem panel whose menu extent is in menurect
	 * and whose on-screen position is screenrect.
	 */
	menuImage( item->SubItem, &menurect, &screenrect,
	    !( ( menu->Flags & MENUENABLED ) && ( item->Flags & ITEMENABLED ) ) );

	IBase->SubCRect.bounds = screenrect;

	D( dumpRect("subitem SubCRect.bounds", &screenrect ) );

	if ( swapMenuIn( AScreen, &IBase->SubCRect ) )
	{
	    IBase->Flags |= GOODSUBDRAWN;
	}
    }

    IBase->Flags |= SUBDRAWN;
    item->Flags |= ISDRAWN;

    SETDITEM(ITEMNUM(num));
}
/*** intuition.library/ResetMenuStrip ***/
/*** intuition.library/SetMenuStrip ***/

BOOL
doSetMenuStrip(window, menu, recalc)
struct Window *window;
struct Menu *menu;
LONG recalc;		/* TRUE for SetMenuStrip, FALSE for ResetMenuStrip */
{
    doISM( itSETMENU, window, menu, recalc );

    return ( TRUE );
}

/*
 * called by ISM
 */
realSetMenuStrip(window, menu, precalc_sizes )
struct Window *window;
struct Menu *menu;
{
    struct Rectangle hullrect;
    struct Menu *workmenu;
    /* not used ... struct RastPort *RP; */
    int HBorder, VBorder;

    if ( precalc_sizes )
    {
	/* The item box must span at least the extent of the menu header,
	 * but we don't care what happens in the y-direction:
	 */
	hullrect.MinX = 0;
	hullrect.MinY = 32767;
	hullrect.MaxY = -32767;
	/* precalc/build this menu OK */
	workmenu = menu;
	while (workmenu)
	{
	    hullrect.MaxX = workmenu->Width - 1;

	    /* look!  it's RJ's little secret.	JaxxX/JazzY/BeatX/BeatY
	     * are none other than the menu-panel rectangle.
	     */
	    boxer( window, workmenu->FirstItem,
		((struct Rectangle *)&workmenu->JazzX), &hullrect, 0 );

	    workmenu = workmenu->NextMenu;
	}
    }

    /* link it into the window */
    window->MenuStrip = menu;
}

/*** intuition.library/ClearMenuStrip ***/

ClearMenuStrip(window)
struct Window *window;
{

    /* will delink when safe */
    doISM( itCLEARMENU, window );
}


/* called by ISM */
BOOL IOnOffMenu(window, menuNum, onOrOff)
struct Window *window;
USHORT menuNum;
BOOL onOrOff;
{
    int menupick;
    register struct Menu *menu;
    register struct MenuItem *item, *subitem;
    int  menuAnd, menuOr;
    register int  itemAnd, itemOr;

    DOOM( printf("IOOM: window %lx menustrip %lx menunum %lx onoff %lx\n",
	window, window->MenuStrip, menuNum, onOrOff ));

    if ((menupick = MENUNUM(menuNum)) == NOMENU) return;

    assertLock( "IOnOffMenu", ISTATELOCK );

    /* get the menu, item, and subitem numbers */
    menu = grabMenu(window->MenuStrip, menuNum);
    if (item = grabItem(menu, menuNum))
	subitem = grabSub(item, menuNum);

    if (onOrOff)
	{ /* we're trying to turn on this menu */
	menuOr = MENUENABLED; /* if the flag is off, then set it on */
 	menuAnd = -1; /* we want to turn the bit on , not mask it off */
	itemOr = ITEMENABLED;
	itemAnd = -1;
	}
    else
	{ /* we're trying to turn off this menu */
	menuOr = 0;		/* if the flag is on, then set it off */
 	menuAnd = ~MENUENABLED;/* want to turn the bit off, not mask it on */
	itemOr = 0;
	itemAnd = ~ITEMENABLED;
	}


    if (item == NULL) /* we want to enable the entire menu */
    {
	menu->Flags |= menuOr; /* OR in a one-bit if we want to enable */
	menu->Flags &= menuAnd; /* AND in a zero-bit to disable */
    }
    else /* we've got a menu AND an item */
    {
	/* we want to enable this item AND ALL sub-menus if any */
	if (subitem == NULL)
	{
	    item->Flags |= itemOr; /* OR in a one-bit to enable */ 
	    item->Flags &= itemAnd; /* AND in a zero-bit to disable */ 
	}
	else /* finally, we've got a subitem pick too */
	{
	    subitem->Flags |= itemOr; /* OR in a one-bit to enable */  
	    subitem->Flags &= itemAnd; /* AND in a zero-bit to disable */ 
	}
    }
}


/*** intuition.library/OnMenu ***/

BOOL OnMenu(window, menuNum)
struct Window *window;
UWORD menuNum;
{
    DOOM( printf("OnMenu %lx %lx\n", window, menuNum ) );
    return ( doISM( itONOFFMENU, window, menuNum, TRUE ) );
}
/*** intuition.library/OffMenu ***/

BOOL OffMenu(window, menuNum)
struct Window *window;
UWORD menuNum;
{
    DOOM( printf("OffMenu %lx %lx\n", window, menuNum ) );
    return ( doISM( itONOFFMENU, window, menuNum, FALSE ) );
}



/*
 * getMenu() figures out which menu, item, and subitem we're currently
 * over.
 * Maintains MenuDrawn and MenuSelected, and draws the menu display with
 * highlighting.
 *
 * NB:  MenuDrawn and MenuSelected should be set to MENUNULL before first call
 */

getMenu()
{
    UWORD subpanel_of, itempanel_of;
    UWORD sel_item, sel_sub, sel_menu;
    UWORD newsel_item, newsel_sub, newsel_menu;
    register struct MenuItem *iptr;
    struct IntuitionBase *IBase = fetchIBase();

    /* Which menu, if any, begat the itempanel? */
    itempanel_of = MENUNUM(IBase->MenuDrawn);
    /* Which item, if any, begat the subpanel? */
    subpanel_of = ITEMNUM(IBase->MenuDrawn);

    /* Which, menu, item, and subitem are selected? */
    sel_menu = MENUNUM(IBase->MenuSelected);
    sel_item = ITEMNUM(IBase->MenuSelected);
    sel_sub = SUBNUM(IBase->MenuSelected);

    /* Is there a submenu panel? */
    if (subpanel_of != NOITEM)
    {
	/* Did we hit a subitem? */
	if ((newsel_sub = hitSub()) != NOSUB)
	{
	    /* Is this a different subitem from the last one? */
	    if (newsel_sub != sel_sub)
	    {
		 /* Unhighlight the previously selected subitem, if any */
		if (sel_sub != NOSUB)
		{
		    highSub();
		}
		/* Select and highlight the new subitem, and we're done */
		SETSSUB(newsel_sub);
		highSub();
	    }
	    /* Same subitem as before, so do nothing */
	    /* we can leave, having hit a subitem and dealt with it */
	    return;
	}


	/* ELSE We didn't hit any subitems */
	{
	    /* Unhighlight the previously selected subitem, if any */
	    if (sel_sub != NOSUB)
	    {
		highSub();
		SETSSUB(NOSUB);
	    }

	    /* If we are nevertheless inside the subpanel, then we are done */
	    if ( inrect(IBase->ActiveScreen->MouseX, IBase->ActiveScreen->MouseY,
		 &IBase->SubCRect.bounds) )
	    {
		return;
	    }
	}
    }

    /* Either there was no submenu panel, or the mouse is not over it */

    /* Is there an itempanel? */
    if (itempanel_of != NOMENU)
    {
	/* Did we hit an item? */
	if ((newsel_item = hitItem()) != NOITEM)
	{
	    /* Is this a different item from the last one? */
	    if (newsel_item != sel_item)
	    {
		/* Remove the subpanel of and unhighlight the previously
		 * selected item */
		if (sel_item != NOITEM)
		{
		    if (subpanel_of != NOITEM)
		    {
			eraseSub();
		    }
		    highItem();
		}
		/* Select and highlight the new item */
		SETSITEM(newsel_item);
		highItem();
		/* Find the item itself */
		iptr = grabItem( grabMenu( IBase->ActiveWindow->MenuStrip, 
			SHIFTMENU(sel_menu) ), SHIFTITEM(newsel_item) );
		/* Does the item have a subpanel? */
		if (iptr->SubItem)
		{
		    drawSub(SHIFTITEM(newsel_item) | SHIFTMENU(sel_menu));
		    /* Did we hit something in the subpanel? */
		    if ((newsel_sub = hitSub()) != NOSUB)
		    {
			/* select and highlight this subitem */
			SETSSUB(newsel_sub);
			highSub();
		    }
		}
	    }
	    /* Same item as before, so do nothing */
	    /* we can leave, having hit an item and dealt with it */
	    return;
	}
	/* If the previously selected item had no subpanel,
	 * unhighlight it
	 */
 	if ( (sel_item != NOITEM) && (subpanel_of == NOITEM) )
	{
	    highItem();
	    SETSITEM(NOITEM);
	}
    }

    /* Either there was no itempanel, or the mouse is not over it */

    /* Which menu, if any, was hit? */
    if ((newsel_menu = hitMenu()) != NOMENU)
    {
	/* Remove its subpanel, if any */
	if (subpanel_of != NOITEM)
	{
	    eraseSub();
	    highItem();
	}
	/* Is this a different menu? */
	if (newsel_menu != sel_menu)
	{
	    /* Remove the old itempanel, if any */
	    if (itempanel_of != NOMENU)
	    {
		eraseItem();
		highMenu();
	    }

	    /* select and highlight this one and draw its itemlist */
	    SETSMENU(newsel_menu);
	    highMenu();
	    drawItem(SHIFTMENU(newsel_menu));
	}
	/* else no, this menu is the same as the last time */
	SETSSUB(NOSUB);
	SETSITEM(NOITEM);
    }
}

/* Given a screen and a ClipRect whose bounds are the desired on-screen
 * location of the menu, and whose bitmap points to a buffer holding
 * the menu, bring that menu on-screen.
 *
 * This routine attempts to allocate a spare buffer so that the menu
 * may be brought up using two simple blits.  If no memory is available,
 * it uses the SwapBitsRastPortClipRect() function, which was the
 * (slower) traditional way of bringing up menus.
 */
long
swapMenuIn( sc, cr )
struct Screen *sc;
struct ClipRect *cr;
{
    struct BitMap *swapbm;
    long result = 0;
    long width;
    long height;

    /* To prevent crashing if the menu is off-screen, contrain menu
     * ClipRect to be on-screen, and only SwapBits if not degenerate.
     */
    if ( cr->bounds.MaxX >= sc->Width )
    {
	cr->bounds.MaxX = sc->Width - 1;
    }
    if ( cr->bounds.MaxY >= sc->Height )
    {
	cr->bounds.MaxY = sc->Height - 1;
    }

    width = rectWidth( &cr->bounds );
    height = rectHeight( &cr->bounds );
    if ( ( width > 0 ) && ( height > 0 ) )
    {
	/* SwapBits() is memory efficient, but slow.  For speed,
	 * we attempt to allocate a second menu buffer, and do
	 * two straight-copy blits instead (i.e. instead of the three
	 * XOR blits inside SwapBits()).  If we can't get the memory
	 * for the second buffer, we fall back to SwapBits().
	 */
	if ( swapbm = allocMenuBitMap( sc, &cr->bounds ) )
	{
	    struct BitMap *menubm = cr->BitMap;
	    menuBlit( sc, swapbm, &cr->bounds, SWAP_SCREEN_TO_OFF );

	    menuBlit( sc, menubm, &cr->bounds, SWAP_OFF_TO_SCREEN );

	    WaitBlit();
	    freeMenuBitMap( menubm );
	    cr->BitMap = swapbm;
	}
	else
	{
	    SwapBits( &sc->RastPort, cr );
	}
	result = 1;
    }
    return( result );
}


/* Blit between off-screen bitmap and screen.  'bounds' describes
 * the bounds of the on-screen area to affect.  'toscreen' selects
 * whether the blit goes to the screen or to the off-screen bitmap.
 */

menuBlit( sc, savebm, bounds, toscreen )
struct Screen *sc;
struct BitMap *savebm;
struct Rectangle *bounds;
long toscreen;
{
    LONG sourceleft;
    LONG sourcetop;
    LONG destleft;
    LONG desttop;
    struct BitMap *sourcebm;
    struct BitMap *destbm;

    sourceleft = destleft = bounds->MinX;
    sourcetop = desttop = bounds->MinY;
    if ( toscreen )
    {
	sourcebm = savebm;
	destbm = sc->RastPort.BitMap;
	sourceleft &= 0xF;
	sourcetop = 0;
    }
    else
    {
	sourcebm = sc->RastPort.BitMap;
	destbm = savebm;
	destleft &= 0xF;
	desttop = 0;
    }
    BltBitMap( sourcebm, sourceleft, sourcetop,
	destbm, destleft, desttop,
	rectWidth( bounds ), rectHeight( bounds ), 0xC0, ~0, 0 );
}
