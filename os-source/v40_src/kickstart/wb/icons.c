/*
 * $Id: icons.c,v 38.3 92/04/09 07:10:42 mks Exp $
 *
 * $Log:	icons.c,v $
 * Revision 38.3  92/04/09  07:10:42  mks
 * Quicky cleanup of some code/function calls
 * 
 * Revision 38.2  92/02/13  14:22:32  mks
 * Code cleanup
 *
 * Revision 38.1  91/06/24  11:35:55  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "graphics/rastport.h"
#include "graphics/layers.h"
#include "graphics/regions.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "graphics/gfxmacros.h"
#include "quotes.h"

#define COPY_MODE	0xC0
#define MASKCOPY_MODE	0xE0
#define INVERT_MODE	0x30
#define MASKINVERT_MODE	0x20

initgadget2(obj)
struct WBObject *obj;
{
    struct DiskObject *dobj;
    int result = 0; /* assume failure */

    MP(("initgadget2: enter, obj=$%lx (%s), type=%ld\n",obj, obj->wo_Name, obj->wo_Type));

    if (dobj = GetDefDiskObject(obj->wo_Type))
    {
	if (AssignDOtoWBO(obj, dobj))
	{
		obj->wo_DiskObject = dobj;
		result = PrepareIcon(obj);
	}
	else FreeDiskObject(dobj);
    }

    MP(("initgadget2: exit, returning %ld\n", result));
    return(result);
}

/* call before doing any updates in the window */
void BeginIconUpdate(obj, win, ClearDamage_P)
struct WBObject *obj;
struct Window *win;
int ClearDamage_P;
{
    struct WorkbenchBase *wb = getWbBase();
    struct RastPort *rp;
    struct Layer *layer;
    struct Region *region;
    struct Rectangle rect;

#ifdef DEBUGGING
    if (obj == NULL) {
	MP(("BeginIconUpdate: null object\n"));
	Debug(0L);
    }
#endif DEBUGGING

    MP(("BeginIconUpdate: enter, ClearDamage_P=%lx, NestCount=%ld\n",
	ClearDamage_P, wb->wb_UpdateNestCnt));
    MP(("\tobj=%lx (%s)\n", obj, obj->wo_Name));
    MP(("\twin=%lx (%s)\n", win, win->Title));

    if ((wb->wb_UpdateNestCnt)++ != 0) {
	return;
    }

    rp = win->RPort;
    layer = rp->Layer;

    /* jimm - 5/21/90: make sure Intuition isn't trying to protect
     * the damage list between the time the damage list is
     * created and when Intuition refreshes it.
     */
    LockLayerInfo(layer->LayerInfo); /* jimm added */

    /* jimm: note: layer info lock before layerrom */
    MP(("BeginIconUpdate: LOCKLAYERROM\n"));
    LockLayerRom(layer);

    rect.MinX = win->BorderLeft;
    rect.MinY = win->BorderTop;
    rect.MaxX = win->Width - win->BorderRight - 1;
    rect.MaxY = win->Height - win->BorderBottom - 1;
#ifdef DEBUGGING
/*    dumpRect("BeginIconUpdate", &rect); */
#endif DEBUGGING

    win->UserData = 0;

    if (ClearDamage_P) {
	MP(("BeginIconUpdate: inside ClearDamage_P\n"));
	/* try and save the old damage list */
	region = (struct Region *)NewRegion();
	if (region) {
	    MP(("BeginIconUpdate: inside region\n"));
	    win->UserData = (BYTE *) layer->DamageList;
	    layer->DamageList = region;
	}
	/* prevent us from trashing the borders */
	ClearRegion(layer->DamageList);
	OrRectRegion(layer->DamageList, &rect);
    }
    else {
	MP(("BeginIconUpdate: at AndRectRegion\n"));
	AndRectRegion(layer->DamageList, &rect);
    }
#ifdef DEBUGGING
/*    dumpRegion("BeginIconUpdate", layer->DamageList); */
#endif DEBUGGING

    /* jimm - 5/21/90: note: layer info before implicit layerrom lock
     * in BeginUpdate().  the difference between BeginUpdate and
     * BeginRefresh, if you havn't got a gimme00 window, is the LockLayerInfo.
     *
     * That's why people who "trash" the damage list by calling
     * BeginRefresh(w); EndRefresh(w, TRUE) don't collide with
     * Intuition:  Intuition holding the LayerInfo lock blocks
     * am application calling BeginRefresh().
     */

    BeginUpdate(layer);
    MP(("BeginIconUpdate: exit\n"));
}

void EndIconUpdate(win) /* call when done changing the window */
struct Window *win;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Layer *layer;
    int updatemode = TRUE;

    MP(("EndIconUpdate: enter, win=%lx (%s), UserData=%lx, NestCount=%ld\n",
	win, win->Title, win->UserData, wb->wb_UpdateNestCnt - 1));
    if (--wb->wb_UpdateNestCnt != 0) {
	return;
    }
    layer = win->RPort->Layer;
    if (win->UserData) {
	DisposeRegion(layer->DamageList);
	layer->DamageList = (struct Region *) win->UserData;
	win->UserData = NULL;
	updatemode = FALSE;
    }
    else {
	layer->Flags &= ~LAYERREFRESH;
    }
    EndUpdate(layer, updatemode);
    MP(("EndIconUpdate: UNLOCKLAYERROM\n"));
    UnlockLayerRom(layer);

    UnlockLayerInfo(layer->LayerInfo);/* jimm - 5/21/90 - added */

    MP(("EndIconUpdate: exit\n"));
}

int RenderIcon(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd;
struct Image *image;
struct Window *win;
APTR mask;
struct WBObject *next;
struct Node *nextnode;
int mode;

    dd=obj->wo_Parent->wo_DrawerData;

    MP(("RenderIcon: enter\n"));
    MP(("\tobj=%lx (%s), par=%lx (%s), dd=%lx\n",obj, obj->wo_Name, obj->wo_Parent,obj->wo_Parent ? obj->wo_Parent->wo_Name : "NULL", dd));

    if (obj != wb->wb_RootObject) if (obj->wo_Parent) if (dd) if (dd->dd_DrawerWin) if (win = obj->wo_IconWin)
    {
        MP(("\tViewModes=%ld, %s\n",dd->dd_ViewModes, dd->dd_ViewModes <= DDVM_BYICON ? "BYICON" : "BYTEXT"));
        MP(("\t%sselected, %svisible\n", obj->wo_Selected ? "" : "not ",obj->wo_Invisible ? "in" : ""));

        if (dd->dd_ViewModes > DDVM_BYICON)
        { /* we are in view by text mode */

            if (!obj->wo_ImageSize)
            {
                MP(("RenderIcon: calling growNameImage for $%lx (%s)\n",obj, obj->wo_Name));
                growNameImage(obj);
                MakeNameGadget(obj, dd->dd_ViewModes);
            }

            /* make sure the icon's in the right place */
            nextnode = obj->wo_Siblings.ln_Succ;
            obj->wo_CurrentX = 0;
            MP(("\tobj=%lx (%s), nextnode=%lx, nextnode->ln_Succ=%lx\n",obj, obj->wo_Name, nextnode, nextnode->ln_Succ));
            if (!nextnode->ln_Succ)
            {
                /* we are the top of the list */
                obj->wo_CurrentY = 0;
                MP(("RenderIcon: text. obj %lx/%s, HEAD OF LIST\n",obj, obj->wo_Name));
            }
            else
            {
                next = NodeToObj(nextnode);
                MP(("RenderIcon: text. obj %lx/%s, next %lx/%s, next y=%ld\n",obj, obj->wo_Name, next, next->wo_Name, next->wo_CurrentY));
                obj->wo_CurrentY = next->wo_CurrentY + wb->wb_TextFont->tf_YSize + 1;
            }
            obj->wo_Gadget.LeftEdge = win->BorderLeft - dd->dd_CurrentX;
            obj->wo_Gadget.TopEdge = obj->wo_CurrentY - dd->dd_CurrentY + win->BorderTop;

            MP(("RenderIcon: text %lx/%s current %ld/%ld, gad %ld/%ld, %ld/%ld\n",obj, obj->wo_Name,obj->wo_CurrentX, obj->wo_CurrentY,obj->wo_Gadget.LeftEdge, obj->wo_Gadget.TopEdge,obj->wo_Gadget.Width, obj->wo_Gadget.Height));
        }
	else
        {
            obj->wo_Gadget.LeftEdge = obj->wo_CurrentX - dd->dd_CurrentX + win->BorderLeft;
            obj->wo_Gadget.TopEdge = obj->wo_CurrentY - dd->dd_CurrentY + win->BorderTop;
	}

	/*
	 * New check for invisible icons...  This is to prune the
	 * the icons that are rendered...
	 */
	{
	long tmp;

		tmp=obj->wo_Gadget.TopEdge;

		if (tmp>win->Height) win=NULL;
		else if ((tmp+obj->wo_Gadget.Height+EMBOSEHEIGHT+wb->wb_IconFont->tf_YSize+1)<0) win=NULL;
		else
		{
			tmp=obj->wo_Gadget.LeftEdge;

			if (tmp>win->Width) win=NULL;
			else
			{
				if (dd->dd_ViewModes > DDVM_BYICON) tmp+=wb->wb_TextFont->tf_XSize*strlen(obj->wo_NameBuffer);
				if ((tmp+obj->wo_Gadget.Width+EMBOSEWIDTH)<0) win=NULL;
			}
		}
	}

	if (win)
	{
            GetImageMode(obj, (APTR *)&image, &mode, &mask);

            BeginIconUpdate(obj->wo_Parent, win, TRUE);

            if (dd->dd_ViewModes <= DDVM_BYICON)
            {
                EmboseIcon(win->RPort, obj);
                RenderImage(win->RPort, image,
                    obj->wo_Gadget.LeftEdge + wb->wb_EmboseBorderLeft,
                    obj->wo_Gadget.TopEdge + wb->wb_EmboseBorderTop, mode, mask);
            }
            else
            {
                RenderImage(win->RPort, image,
                    obj->wo_Gadget.LeftEdge + VIEWBYTEXTLEFTOFFSET, obj->wo_Gadget.TopEdge, mode, mask);
            }

            /* now render the text */
            RenderName(obj);
            EndIconUpdate( win );
	}
    }
    MP(("RenderIcon: exit\n"));
    return(NULL);
}

void EmboseIcon(rp, obj)
struct RastPort *rp;
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase(); /* Added for calls */
    struct Gadget *gad = &obj->wo_Gadget;
    int x = gad->LeftEdge;
    int y = gad->TopEdge;
    int width = gad->Width + EMBOSEWIDTH;
    int height = gad->Height + EMBOSEHEIGHT - 1;
    UBYTE wbShinePen = wb->wb_shinePen;
    UBYTE wbShadowPen = wb->wb_shadowPen;

    SetDrMd(rp, JAM2);
    /* draw right line and bottom line */
    SetAPen(rp, obj->wo_Selected ? wbShinePen : wbShadowPen);
    Move(rp, x + width - 1, y + 1);
    Draw(rp, x + width - 1, y + 1);
    Draw(rp, x + width - 1, y + height - 1);
    Draw(rp, x, y + height - 1);
    /* draw left and top line */
    SetAPen(rp, obj->wo_Selected ? wbShadowPen : wbShinePen);
    Draw(rp, x, y);
    Draw(rp, x + width - 1, y);
    /* clear inner area */
    SetAfPt(rp, NULL, 0);
    SetAPen(rp, 0);
    RectFill(rp, x + 1, y + 1, x + width - 2, y + height - 2);
}

/* stolen with impunity from RJ.  Thanks much... */

void RenderImage( drp, image, xoff, yoff, mode, maskplane )
struct RastPort *drp;		/* dest raster port */
struct Image *image;
int xoff, yoff;
LONG mode;
APTR maskplane;
{
    struct BitMap BMap;
    int pick, onoff, oldmask;
    int depthmask;
    void (*call)();
    int bmode = mode;
    int left, top;

    MP(("RenderImage: enter\n    drp=$%lx, image=$%lx, xoff=%ld, yoff=%ld, mode=%ld, maskplace=$%lx\n",
	drp, image, xoff, yoff, mode, maskplane));

    /* don't bother if there's no image */
    if( ! image ) return;

    /* don't bother with empty images */
    if( image->Width == 0 || image->Height == 0 ) return;

    /* we will do this in three passes.  The first pass will copy the
     * existing portions of the image.  The second pass will clear
     * those planes that need to be cleared.  The final pass will
     * set those planes that need to be set.
     */

    ImageToBitMap( image, &BMap, (UBYTE *) image->ImageData );
    pick = image->PlanePick;
    onoff = image->PlaneOnOff;

    oldmask = drp->Mask;
    depthmask = ~(~0 << BMap.Depth);

    if( drp->Mask = (depthmask & pick) ) {

	if( maskplane ) {
	    call = BltMaskBitMapRastPort;
	    if( mode == COPY_MODE ) bmode = MASKCOPY_MODE;
	    else bmode = MASKINVERT_MODE;
	} else {
	    call = BltBitMapRastPort;
	}

	MP(("RenderImage: calling blit with bmode 0x%lx, plane 0x%lx\n",
	    bmode, maskplane ));

	(*call)( &BMap, 0, 0, drp, xoff + image->LeftEdge,
	   yoff + image->TopEdge, image->Width, image->Height,
	   bmode, maskplane );
    }


    /* now deal with the planes that were not set */
    if( drp->Mask = (depthmask & ~pick) ) {
	MP(( "RenderImage: null pick. image 0x%lx, maskplanes %lx\n",
	    image, maskplane ));

	left = xoff + image->LeftEdge;
	top = yoff + image->TopEdge;
	drp->Mask = (depthmask & ~pick);

	if( mode == COPY_MODE ) {
	    SetAPen( drp, (onoff & drp->Mask) );
	} else {
	    SetAPen( drp, (~onoff & drp->Mask) );
	}

	BltPattern( drp, maskplane, left, top,
	    left + image->Width, top + image->Height,
	    BMap.BytesPerRow );

    }

    /* restore the write mask */
    drp->Mask = oldmask;

    MP(("RenderImage: exit\n"));
}

void ClearDrawerWindow(dd)
struct NewDD *dd;
{
	struct Window *win = dd->dd_DrawerWin;
	struct RastPort *rp = win->RPort;

	MP(("ClearDrawerWindow: enter\n"));
 	EraseRect(rp, 0, 0, win->Width - 1, win->Height - 1); /* do it */
	MP(("ClearDrawerWindow: exit\n"));
}


IconBoxBackdrop(icon, parent)
struct WBObject *icon;
struct WBObject *parent;
{
struct WorkbenchBase *wb = getWbBase();
struct Window *win = wb->wb_BackWindow;
int max;
int flag=FALSE;

    MP(("IconBoxBackdrop: enter, icon=$%lx (%s), parent=$%lx (%s)\n",icon, icon->wo_Name, parent, parent->wo_Name));
    MP(("\tBackdrop=%ld, CurrentX=%ld, CurrentY=%ld\n",	wb->wb_Backdrop, icon->wo_CurrentX, icon->wo_CurrentY));
    if ((parent == wb->wb_RootObject) && wb->wb_Backdrop)
    {
	/* keep us from going off the backdrop */
	/* legal range is x:0-RightEdge, y:0-LeftEdge */
	/* be sure to account for height */

	/* clip to top of window */
	if (icon->wo_CurrentX < 0)
	{
	    icon->wo_CurrentX = 0;
	    flag=TRUE;
	}

	if (icon->wo_CurrentY < 0)
	{
	    icon->wo_CurrentY = 0;
	    flag=TRUE;
	}

	max = win->Width - icon->wo_Gadget.Width - EMBOSEWIDTH;
	if (icon->wo_CurrentX > max)
	{
	    icon->wo_CurrentX = max;
	    flag=TRUE;
	}

	max = win->Height - icon->wo_Gadget.Height - wb->wb_IconFontHeight - EMBOSEHEIGHT;
	if (icon->wo_CurrentY > max)
	{
	    icon->wo_CurrentY = max;
	    flag=TRUE;
	}

	if (flag) PlaceObj(icon, parent);
    }

    MP(("IconBoxBackdrop: exit\n"));
    return(0);
}


AddIcon( icon, drawer )
struct WBObject *icon, *drawer;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *dd;

    MP(("AddIcon: icon=%lx, drawer=%lx, name='%s', image=%lx\n",
	icon, drawer, icon->wo_Name, icon->wo_Gadget.GadgetRender ));

    dd = drawer->wo_DrawerData;

#ifdef DEBUGGING
    if( ! dd ) {
	MP(( "AddIcon: Null NewDD!\n" ));
	Debug(0L);
	return( 1 );
    }
#endif DEBUGGING

    icon->wo_Parent = drawer;
    drawer->wo_UseCount++;
    icon->wo_UseCount++;

    icon->wo_IconWin = dd->dd_DrawerWin;

    if (dd->dd_ViewModes <= DDVM_BYICON)
    {
	icon->wo_SaveX = icon->wo_CurrentX; /* init SaveX */
	MP(("AddIcon: setting SaveX to %ld ($%lx) for %lx (%s)\n",
	    icon->wo_SaveX, icon->wo_SaveX, icon, icon->wo_Name));
	icon->wo_SaveY = icon->wo_CurrentY; /* init SaveY */
    }

    ChangeChildView(icon, dd, dd->dd_ViewModes, drawer);

    if( dd->dd_ViewModes <= DDVM_BYICON )
    {
	/* if obj has no position or is on the backdrop then place it somewhere */
	if (icon->wo_CurrentX == NO_ICON_POSITION || drawer == wb->wb_RootObject)
        {
	    MP(( "AddIcon: placing %lx/%s\n", icon, icon->wo_Name ));
	    PlaceObj( icon, drawer);
	}

	ADDHEAD( &dd->dd_Children, &icon->wo_Siblings );

	IconBoxBackdrop(icon,drawer);	/* Make sure it is on the screen */
        RenderIcon(icon);
    }
    else
    {
	InsertByText( icon, drawer );
    }


    MP(("AddIcon: exit\n"));

    return( NULL );
}


void SetIconNames(icon)
struct WBObject *icon;
{
    icon->wo_MasterNode.ln_Name =
    icon->wo_Siblings.ln_Name =
    icon->wo_SelectNode.ln_Name =
    icon->wo_UtilityNode.ln_Name = icon->wo_Name;

    icon->wo_MasterNode.ln_Type = 0;
    icon->wo_Siblings.ln_Type = sizeof(struct Node);
    icon->wo_SelectNode.ln_Type = 2 * sizeof(struct Node);
    icon->wo_UtilityNode.ln_Type = 3 * sizeof(struct Node);
}


RenderName(icon)
struct WBObject *icon;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Window *win;
    struct RastPort *rp;
    struct NewDD *dd;

    if (icon == wb->wb_RootObject) return(0);

    if (!icon) goto nullerr;
    if (!icon->wo_Parent) goto nullerr;
    dd = icon->wo_Parent->wo_DrawerData;
    if (!dd) goto nullerr;
    win = dd->dd_DrawerWin;

    if (!win) {
	MP(("RenderName: bypassing rendering $%lx (%s) since it has no window\n",
	    icon, icon->wo_Name));
	return(0);
    }

    rp = win->RPort;
    if (!rp) goto nullerr;
    SetAPen(rp, wb->wb_APen);
    SetBPen(rp, wb->wb_BPen);
    SetDrMd(rp, wb->wb_DrawMode);

    if (dd->dd_ViewModes <= DDVM_BYICON) {
	MP(("RenderName: calling SetFont($%lx, $%lx)...", rp, wb->wb_IconFont));
	SetFont(rp, wb->wb_IconFont);
	MP(("ok\n"));
	MP(("RenderName: BYICON\n"));

	Move(rp, icon->wo_Gadget.LeftEdge + wb->wb_EmboseBorderLeft + icon->wo_NameXOffset,
	    icon->wo_Gadget.TopEdge + (EMBOSEHEIGHT) + icon->wo_NameYOffset);

	MP(("RenderName: calling Text($%lx, %s, %ld)...",
	    rp, icon->wo_Name, strlen(icon->wo_Name)));
	Text(rp, icon->wo_Name, strlen(icon->wo_Name));
	MP(("ok\n"));
    }
    else {
	MP(("RenderName: calling SetFont($%lx, $%lx)...", rp, wb->wb_TextFont));
	SetFont(rp, wb->wb_TextFont);
	MP(("ok\n"));
	MP(("RenderName: BYTEXT: (%s), LE=%ld, W=%ld, NXO=%ld\n\t'%s'",
	    icon->wo_Name, icon->wo_Gadget.LeftEdge,
	    icon->wo_Gadget.Width, icon->wo_NameXOffset,
	    icon->wo_NameBuffer));
	Move(rp, icon->wo_Gadget.LeftEdge + icon->wo_Gadget.Width +
	    icon->wo_NameXOffset + VIEWBYTEXTLEFTOFFSET,
	    icon->wo_Gadget.TopEdge + icon->wo_NameYOffset);
	MP(("RenderName: calling Text($%lx, %s, %ld)...",
	    rp, icon->wo_NameBuffer, strlen(icon->wo_NameBuffer)));
	Text(rp, icon->wo_NameBuffer, strlen(icon->wo_NameBuffer));
	MP(("ok\n"));
    }
    MP(("RenderName: end\n"));
    return(0);
nullerr:
    return(-1);
}

/*
** PrepareIcons's purpose in life is to deal with BACKFILLed icons.
**
** if this is not a BACKFILL icon then it just returns.  If it is
** then it will compute a mask of all the significant bits of
** the icon's image, and assign it to the unused SelectRender
** pointer of the gadget.
**
*/

PrepareIcon(obj)
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct RastPort *rp;
    struct Image *im = (struct Image *) obj->wo_Gadget.GadgetRender;
    UWORD *maskplane;		/* the mask of where to blit */
    UBYTE *tmpdata;		/* needed for the TmpRas */
    UWORD *data;		/* a second image */
    UBYTE compmask;		/* The complement of colour 0 */
    LONG planesize;
    int i, maxx, maxy, size, result = 1, j, k;
    struct BitMap BMap;
    struct TmpRas TR;		/* tempras needed to FloodFill */

    MP(("PI: enter, obj=%lx (%s), im=%lx, Flags=%lx\n",
	obj, obj->wo_Name, im, obj->wo_Gadget.Flags & GADGHIGHBITS));
    if (im && (obj->wo_Gadget.Flags & GADGHIGHBITS) == GADGBACKFILL) {
	planesize = RASSIZE(im->Width, im->Height);
	size = planesize * im->Depth;
	/* This is the colour we need to look for in backfill... */
	compmask = (1 << (im->Depth)) - 1;
	MP(("calling ObjAlloc..."));
	/* !!! this breaks for extended images */
	maskplane = ObjAllocChip(obj, planesize);
	MP(("ok\n"));
	if (!maskplane) {
	    result = 0;
	    goto end;
	}
	/* cheat and use this otherwise unused rast port */
	rp = &wb->wb_TextRast;
	/* allocate a TmpRas and extra image region */
	if (!(tmpdata = (UBYTE *)ALLOCVEC(planesize + size, MEMF_CHIP|MEMF_PUBLIC))) {
	    result = 0;
	    goto end;
	}
	data = (UWORD *) (tmpdata + planesize);
	MP(("PI: calling SetRastPort..."));
	SetRastPort(rp, im, &BMap, data);
	MP(("ok\ncalling InitTmpRas..."));
	rp->TmpRas = (struct TmpRas *)InitTmpRas(&TR, tmpdata, planesize);
	MP(("ok\ncalling RenderImages..."));
	RenderImage(rp, im, 0, 0, INVERT_MODE, 0);
	MP(("ok\n"));
	/* the rast port now has an inverted image of the original
	** icon.  Go ahead and fill in all the colour (compmask) to colour 0.
	*/
	/* now fill in the outsides */
	maxx = im->Width - 1;
	maxy = im->Height - 1;
	SetAPen(rp, 0);
	MP(("PI: Mask=%lx, DrawMode=%lx, AreaPtrn=%lx, APen=%ld, BPen=%ld\n",
	    rp->Mask, rp->DrawMode, rp->AreaPtrn, rp->FgPen, rp->BgPen));
	MP(("maxy=%ld...", maxy));
	for (i = 0; i <= maxy; i++) {
	    if (ReadPixel(rp, 0, i) == compmask) {
		MP(("f %ld %ld", 0, i));
		Flood(rp, 1, 0, i);
		MP(("\n"));
	    }
	    if (ReadPixel(rp, maxx, i) == compmask) {
		MP(("f %ld %ld", maxx, i));
		Flood(rp, 1, maxx, i);
		MP(("\n"));
	    }
	}
	MP(("maxx=%ld...", maxx));
	for (i = 0; i <= maxx; i++) {
	    if (ReadPixel(rp, i, 0) == compmask) {
		MP(("f %ld %ld", i, 0));
		Flood(rp, 1, i, 0);
		MP(("\n"));
	    }
	    if (ReadPixel(rp, i, maxy) == compmask) {
		MP(("f %ld %ld", i, maxy));
		Flood(rp, 1, i, maxy);
		MP(("\n"));
	    }
	}
	MP(("ok\ncalling BltClear..."));
	/* now make the mask.  It will be an OR of each of the original
	** image and the new image, with all the bit planes OR'ed together
	*/
	/* clear the mask to begin with */
	BltClear((APTR) maskplane, planesize, 1);
	MP(("ok\n"));
	{
	    UWORD *src = im->ImageData;
	    UWORD *dst = data;
	    UWORD endOfLineMask;

	    /* mask for unused bits at the end of a line */
	    if (!(endOfLineMask = ~(0xffff >> (im->Width & 0xf)))) {
		endOfLineMask = 0xffff; /* im->Width was divisible by 16 */
	    }
	    i = im->Depth;
#ifdef DEBUGGING
	    j = im->Height;
	    k = BMap.BytesPerRow >> 1;
#endif DEBUGGING
	    MP(("i=%ld, j=%ld, k=%ld\n", i, j, k));
	    do {
		UWORD *mask = maskplane - 1;
		j = im->Height;
		do {
		    k = BMap.BytesPerRow >> 1;
		    do {
			*++mask = *src++ | *dst++;
		    } while (--k);
		    *mask &= endOfLineMask; /* and off the final bits */
		} while (--j);
	    } while (--i);
	}
	MP(("ok\n"));
	/* set up the SelectRender stuff */
	obj->wo_Gadget.SelectRender = (APTR) maskplane;
	obj->wo_IOGadget.SelectRender = (APTR) maskplane;
	rp->TmpRas = NULL;
	/* now free up the temporary stuff */
	FREEVEC(tmpdata);
    }
end:
    MP(("PI: exit, result=%lx\n", result));
    return(result);
}


void SetRastPort( newrp, im, bitmap, data )
struct RastPort *newrp;
struct Image *im;
struct BitMap *bitmap;
USHORT *data;
{
    newrp->Mask = -1;
    newrp->Layer = NULL;
    newrp->BitMap = bitmap;
    ImageToBitMap( im, bitmap, (UBYTE *) data );
}


void GetImageMode( icon, imagep, modep, maskp )
struct WBObject *icon;
APTR *imagep;
ULONG *modep;
APTR *maskp;
{
    int highlite = icon->wo_Gadget.Flags & GADGHIGHBITS;

    *imagep = icon->wo_Gadget.GadgetRender;
    *maskp = NULL;
    *modep = COPY_MODE;

    if( highlite == GADGBACKFILL ) {
	*maskp = icon->wo_Gadget.SelectRender;
    }

    if( icon->wo_Selected ) {
	if( highlite == GADGHIMAGE && icon->wo_Gadget.SelectRender ) {
	    *imagep = icon->wo_Gadget.SelectRender;
	} else {
	    *modep = INVERT_MODE;
	}
    }

}


void ImageToBitMap( im, bm, idata )
struct Image *im;
struct BitMap *bm;
UBYTE *idata;
{
    ULONG pick;
    ULONG offset;
    ULONG i;

    InitBitMap(bm,im->Depth,im->Width,im->Height);

    pick = im->PlanePick;

    /* assign to the plane fields */
    offset = bm->Rows * bm->BytesPerRow;
    for( i = 0; i < bm->Depth; i++ ) {
	if( pick & (1 << i) ) {
	    bm->Planes[i] = (PLANEPTR) idata;
	    idata += offset;
	} else {
	    bm->Planes[i] = 0;
	}
	MP(("ImageToBitMap: bm->Planes[%ld] = $%lx\n",
	    i, bm->Planes[i]));
    }
}

/* called for each object in a drawer to redo its icon */
ChangeChildView(obj, dd, newmode, drawer)
struct WBObject *obj;
struct NewDD *dd;
int newmode;
struct WBObject *drawer;
{
    int oldmode = dd->dd_ViewModes;

    MP(("CCV: obj=%lx (%s), dd=%lx, Flags=%lx, ViewModes=%lx, newmode=%lx\n",
	obj, obj->wo_Name, dd, dd->dd_Flags, dd->dd_ViewModes, newmode));

    if (newmode <= DDVM_BYICON) {
	obj->wo_Gadget = obj->wo_IOGadget; /* structure assignment */
	if (oldmode > DDVM_BYICON) { /* switching from text to icon */
	    /* restore graphical icon position */
	    obj->wo_CurrentY = obj->wo_SaveY;
	    /* icon has no position in drawer, give it one */
	    if ((obj->wo_CurrentX = obj->wo_SaveX) == NO_ICON_POSITION) PlaceObj(obj, drawer);
	}
    }
    else { /* switching to a ViewByText mode */
	obj->wo_Gadget.GadgetRender = (APTR) &obj->wo_NameImage;
	/* if switching from ViewByIcon to a ViewByText mode */
	if (oldmode <= DDVM_BYICON) {
	    /* save graphical icon position */
	    obj->wo_SaveX = obj->wo_CurrentX;
	    obj->wo_SaveY = obj->wo_CurrentY;
	    MP(("\tsaving graphical position %ld,%ld ($%lx,$%lx)\n",obj->wo_SaveX, obj->wo_SaveY, obj->wo_SaveX, obj->wo_SaveY));
	}
    }
    MakeNameGadget(obj, newmode);
    return(0);
}

/*
 * The following is needed to kill all of the children of a
 * soon-to-be removed workbench object.
 */
RemoveIconsLoop(struct WBObject *obj)
{
    /*
     * Check if there is a lower drawer and kill it off...
     */
    if (obj->wo_DrawerData)
    {
    	SiblingSuccSearch(obj->wo_DrawerData->dd_Children.lh_Head,RemoveIconsLoop);
    }

    MP(("RFI: removing %lx (%s)\n", obj, obj->wo_Name));
    RemoveObjectCommon(obj);
    return(0);
}

RemoveFakeIcons(struct WBObject *obj)
{
    if (obj->wo_FakeIcon) RemoveIconsLoop(obj);
    return(0);
}

ChangeShowAll(drawer, newmode)
struct WBObject *drawer;
int newmode;
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd = drawer->wo_DrawerData;
int result=-1;

    MP(("ChangeShowAll: enter, drawer=%lx (%s)\n",drawer, drawer->wo_Name));
    MP(("\tdd=%lx, newmode=%ld, oldmode=%ld\n",dd, newmode, dd->dd_Flags & DDFLAGS_SHOWALL));
    MP(("\tDrawerOpen=%lx, DrawerWin=%lx\n",drawer->wo_DrawerOpen, dd->dd_DrawerWin));

    if (drawer != wb->wb_RootObject)
    {
	if (CheckForType(drawer,ISDRAWER))
	{
            result=0;	/* ZZZ:  It may be good to move up one... */
            /* see if we need to do anything */
            if ((dd->dd_Flags & DDFLAGS_SHOWALL) != newmode)
            {
                dd->dd_Flags &= ~DDFLAGS_SHOWALL; /* clear previous mode */
                dd->dd_Flags |= newmode; /* set newmode */
                BeginDiskIO();
                if (newmode & DDFLAGS_SHOWALL)
                {
                    MP(("CSA: calling OpenCommon\n"));
                    if (!OpenCommon(drawer, DDFLAGS_SHOWALL, FALSE))
		    {
			ChangeShowAll(drawer,DDFLAGS_SHOWICONS);
		    }
                }
                else
                {
                    MP(("CSA: calling SSS(RemoveFakeIcons)\n"));
                    SiblingSuccSearch(dd->dd_Children.lh_Head, RemoveFakeIcons);
                }
                EndDiskIO();
                RefreshDrawer(drawer);
            }
        }
    }
    MP(("CSA: exit\n"));
    return(result);
}

/* called with a drawer that need's it's view changed */
ChangeViewBy(drawer, newmode)
struct WBObject *drawer;
int newmode;
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd = drawer->wo_DrawerData;
int result=-1;

    MP(("ChangeViewBy: drawer=%lx (%s), dd=%lx, DrawerWin=%lx (%s)\n",drawer, drawer->wo_Name, dd, dd->dd_DrawerWin));

    if (drawer != wb->wb_RootObject)
    {
    	result=0;

        if (CheckForType(drawer,ISDRAWER))
        {
            /* see if we need to do anything */
            if (newmode != dd->dd_ViewModes)
            {

                dd->dd_CurrentX = 0;
                dd->dd_CurrentY = 0;

                /* first change the icons */
                if (SiblingSuccSearch(dd->dd_Children.lh_Head, ChangeChildView, dd, newmode, drawer))
                {
                    /* we ran out of memory */
/* MKS:  I don't think this bail-out works exactly right... */
                    result=-1;
                }
		else if (newmode <= DDVM_BYICON)
                { /* new mode is ViewByIcon */
                    /* old mode was ViewByText (thus it has no backfill hook) */
                    if (dd->dd_ViewModes > DDVM_BYICON)
                    {
                        InstallBackFill(dd->dd_DrawerWin); /* install backfill hook */
                    }
                    dd->dd_ViewModes = newmode; /* set new mode for drawer */
                    RefreshDrawer(drawer); /* reorder the window */
                    MP(("ChangeViewBy: calling RefreshDrawer, "));
                }
                else
                {
                    MP(("ChangeViewBy: calling SortByText, "));
                    /* old mode was ViewByIcon (thus it has a backfill hook) */
                    if (dd->dd_ViewModes <= DDVM_BYICON)
                    {
			InstallLayerHook(dd->dd_DrawerWin->WLayer, NULL); /* remove backfill hook */
                    }
                    dd->dd_ViewModes = newmode; /* set new mode for drawer */
                    SortByText(drawer); /* reorder the window */
                }
	    }
	}
    }
    MinMaxDrawer(drawer);
    return(result);
}
