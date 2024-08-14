/*** listview.c ************************************************************
*
*   listview.c	- Scrolling ListView routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: listview.c,v 39.21 93/05/24 11:59:08 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"
#include <intuition/cghooks.h>


/*****************************************************************************/


BOOL HandleListView(struct ExtGadget *gad, struct IntuiMessage *imsg);
VOID SetListViewAttrsA(struct ExtGadget *gad, struct Window *win,
                       struct TagItem *taglist);


/*****************************************************************************/


#define RENDER_FULL      1
#define RENDER_UPDATE    2
#define RENDER_SELECT    3
#define RENDER_DESELECT  4


/*****************************************************************************/


static VOID ASM DefaultCallBack(REG(a1) struct LVDrawMsg *msg,
                                REG(a2) struct Node *node)
{
struct RastPort   *rp    = msg->lvdm_RastPort;
UBYTE              state = msg->lvdm_State;
struct TextExtent  extent;
ULONG              fit;
WORD               x,y;
WORD               slack;
ULONG              apen;
ULONG              bpen;
UWORD             *pens = msg->lvdm_DrawInfo->dri_Pens;
STRPTR             name;

    apen = pens[FILLTEXTPEN];
    bpen = pens[FILLPEN];
    if ((state == LVR_NORMAL) || (state == LVR_NORMALDISABLED))
    {
        apen = pens[TEXTPEN];
        bpen = pens[BACKGROUNDPEN];
    }
    SetABPenDrMd(rp,apen,bpen,JAM2);

    name = node->ln_Name;

    fit = TextFit(rp,name,strlen(name),&extent,NULL,1,
                  msg->lvdm_Bounds.MaxX-msg->lvdm_Bounds.MinX-3,
                  msg->lvdm_Bounds.MaxY-msg->lvdm_Bounds.MinY+1);

    slack = (msg->lvdm_Bounds.MaxY - msg->lvdm_Bounds.MinY) - (extent.te_Extent.MaxY - extent.te_Extent.MinY);

    x = msg->lvdm_Bounds.MinX - extent.te_Extent.MinX + 2;
    y = msg->lvdm_Bounds.MinY - extent.te_Extent.MinY + ((slack+1) / 2);

    extent.te_Extent.MinX += x;
    extent.te_Extent.MaxX += x;
    extent.te_Extent.MinY += y;
    extent.te_Extent.MaxY += y;

    Move(rp,x,y);
    Text(rp,name,fit);

    SetAPen(rp,bpen);
    FillOldExtent(rp,&msg->lvdm_Bounds,&extent.te_Extent);

    if ((state == LVR_NORMALDISABLED) || (state == LVR_SELECTEDDISABLED))
    {
        Ghost(rp,pens[BLOCKPEN],msg->lvdm_Bounds.MinX, msg->lvdm_Bounds.MinY,
                                msg->lvdm_Bounds.MaxX, msg->lvdm_Bounds.MaxY);
    }
}


/*****************************************************************************/


static VOID RenderLV(struct ExtGadget *lv, struct Window *wp, UWORD render)
{
struct Node      *node;
struct LVDrawMsg  dm;
ULONG             itemHeight = LVID(lv)->lvid_ItemHeight;
UWORD             yPos;
UWORD             i,start,end;
LONG              scrCnt,dy;
struct RastPort   clonerp;
struct RastPort  *rp;
struct ExtGadget *gad = LVID(lv)->lvid_ListGad;
struct Hook      *oldHook;
BOOL              damage;
struct Layer     *layer;
LONG              maxY;

    if (LVID(lv)->lvid_Labels == (struct List *)~0)
        return;

    if (rp = GetRP(lv,wp))
    {
        dm.lvdm_MethodID    = LV_DRAW;
        dm.lvdm_RastPort    = &clonerp;
        dm.lvdm_DrawInfo    = LVID(lv)->lvid_DrawInfo;
        dm.lvdm_Bounds.MinX = gad->LeftEdge;
        dm.lvdm_Bounds.MinY = gad->TopEdge;
        dm.lvdm_Bounds.MaxX = gad->LeftEdge + gad->Width - 1;

        cloneRastPort(&clonerp,rp);
        SetFont(&clonerp,LVID(lv)->lvid_Font);
        SetMaxPen(&clonerp,LVID(lv)->lvid_MaxPen);

        start = 0;

        if (render == RENDER_FULL)
        {
            end = LVID(lv)->lvid_Visible;
        }
        else if ((render == RENDER_SELECT) || (render == RENDER_DESELECT))
        {
            if ((LVID(lv)->lvid_Selected < LVID(lv)->lvid_Top)
            ||  (LVID(lv)->lvid_Selected >= LVID(lv)->lvid_Top + LVID(lv)->lvid_Visible))
                return;

            start = LVID(lv)->lvid_Selected - LVID(lv)->lvid_Top;
            end = start + 1;
        }
        else if (LVID(lv)->lvid_Top != LVID(lv)->lvid_OldTop)
        {
            scrCnt = LVID(lv)->lvid_Top - LVID(lv)->lvid_OldTop;

            if (abs(scrCnt)+1 < LVID(lv)->lvid_Visible)
            {
                dy = scrCnt * itemHeight;

                if (dy<0)
                {
                    end = -scrCnt;
                }
                else
                {
                    end   = LVID(lv)->lvid_Visible;
                    start = end - scrCnt;
                }

		layer = wp->WLayer;
                LockLayerInfo(layer->LayerInfo);
                oldHook = InstallLayerHook(layer,LAYERS_NOBACKFILL);

                damage = (LAYERREFRESH & layer->Flags);

	        ScrollRasterBF(&clonerp,0,dy,dm.lvdm_Bounds.MinX,
                                             dm.lvdm_Bounds.MinY,
                                             dm.lvdm_Bounds.MaxX,
                                             dm.lvdm_Bounds.MinY + UMult32(itemHeight,LVID(lv)->lvid_Visible) - 1);

                if (!damage && (LAYERREFRESH & layer->Flags))
                {
                    BeginRefresh(wp);
                    RenderLV(lv,wp,RENDER_FULL);
                    EndRefresh(wp,TRUE);
                }

                InstallLayerHook(layer,oldHook);
                UnlockLayerInfo(layer->LayerInfo);
            }
            else
            {
                end = LVID(lv)->lvid_Visible;
            }
        }
        else
        {
            return;
        }

        yPos = dm.lvdm_Bounds.MinY + start*itemHeight;

        if (LVID(lv)->lvid_Labels)
        {
            end   -= start;
            start += LVID(lv)->lvid_Top;
            i      = start;

            node = LVID(lv)->lvid_Labels->lh_Head;
            while (start-- && node->ln_Succ)
                node = node->ln_Succ;

            while (end-- && node->ln_Succ)
            {
                dm.lvdm_Bounds.MinY = yPos;
                dm.lvdm_Bounds.MaxY = yPos + itemHeight - 1;
                dm.lvdm_State       = LVR_NORMAL;

                if ((i == LVID(lv)->lvid_Selected) && (render != RENDER_DESELECT))
                    dm.lvdm_State = LVR_SELECTED;

                if (gad->Flags & GFLG_DISABLED)
                    dm.lvdm_State = dm.lvdm_State * 6 + 2;   /* don't ask... */

                CallHookPkt(LVID(lv)->lvid_CallBack,node,&dm);

                node = node->ln_Succ;

                yPos += itemHeight;
                i++;
            }
        }

        maxY = gad->TopEdge + gad->Height - 1;
        if ((render == RENDER_FULL) && (yPos <= maxY))
        {
            SetAPen(&clonerp,LVID(lv)->lvid_DrawInfo->dri_Pens[BACKGROUNDPEN]);
            RectFill(&clonerp,dm.lvdm_Bounds.MinX,yPos,dm.lvdm_Bounds.MaxX,maxY);

            if (gad->Flags & GFLG_DISABLED)
                Ghost(&clonerp,LVID(lv)->lvid_DrawInfo->dri_Pens[BLOCKPEN],
                      dm.lvdm_Bounds.MinX,
                      yPos,
                      dm.lvdm_Bounds.MaxX,
                      maxY);
        }
    }

    LVID(lv)->lvid_OldTop = LVID(lv)->lvid_Top;
}


/*****************************************************************************/


static VOID SetLVAttrs(struct ExtGadget *lv,
                       struct Window *wp,
                       struct TagItem *taglist)
{
struct TagItem   *tag;
struct Node      *node;
WORD              render = 0;
WORD              oldTop;
WORD              newTop;
WORD              oldSelected;
WORD              newSelected;
WORD              makeVis;
WORD              i;
WORD              maxTop;
struct ExtGadget *gad = LVID(lv)->lvid_ListGad;
BOOL              disabled;

    disabled = (gad->Flags & GFLG_DISABLED);
    TagAbleGadget(gad,wp,taglist);
    if (disabled != (gad->Flags & GFLG_DISABLED))
        render = RENDER_FULL;

    if (tag = findGTTagItem(GTLV_Labels,taglist))
    {
	LVID(lv)->lvid_Labels = (struct List *)tag->ti_Data;
	LVID(lv)->lvid_Total  = 0;

	if (LVID(lv)->lvid_Labels && (LVID(lv)->lvid_Labels != (struct List *)~0))
	{
            node = LVID(lv)->lvid_Labels->lh_Head;
            while (node->ln_Succ)
            {
                LVID(lv)->lvid_Total++;
                node = node->ln_Succ;
            }
	}
	render = RENDER_FULL;
    }

    /* don't do anything else if we have detached labels! */
    if (LVID(lv)->lvid_Labels == (struct List *)~0)
        return;

    oldTop = LVID(lv)->lvid_Top;
    newTop = getGTTagData(GTLV_Top,oldTop,taglist);

    if ((maxTop = LVID(lv)->lvid_Total - LVID(lv)->lvid_Visible) < 0)
        maxTop = 0;

    if (tag = findGTTagItem(GTLV_MakeVisible,taglist))
    {
        makeVis = tag->ti_Data;
        if (makeVis < 0)
            makeVis = 0;

        if (makeVis < oldTop)
            newTop = makeVis;

        if (makeVis >= oldTop + LVID(lv)->lvid_Visible)
            newTop = makeVis - LVID(lv)->lvid_Visible + 1;
    }

    if (newTop > maxTop)
        newTop = maxTop;

    oldSelected = LVID(lv)->lvid_Selected;
    if (LVID(lv)->lvid_AllowSelections)
        newSelected = getGTTagData(GTLV_Selected,oldSelected,taglist);
    else
        newSelected = ~0;

    if (newSelected >= LVID(lv)->lvid_Total)   /* fails if newSelected == ~0 */
    {
        /* NOTE! If LVID(lv)->lvid_Total is 0, newSelected will become
         * -1, or in other words ~0
         */
        newSelected = LVID(lv)->lvid_Total - 1;
    }

    if ((newSelected != oldSelected) && (!LVID(lv)->lvid_ReadOnly))
    {
        if (render == 0)
        {
            if (oldSelected != ~0)
                RenderLV(lv,wp,RENDER_DESELECT);
            render = RENDER_SELECT;
        }
        LVID(lv)->lvid_Selected = newSelected;
    }

    LVID(lv)->lvid_SelectedName = NULL;
    if (LVID(lv)->lvid_Labels)
    {
        i    = LVID(lv)->lvid_Selected;
	node = LVID(lv)->lvid_Labels->lh_Head;
	while (node->ln_Succ)
	{
	    if (i == 0)
		LVID(lv)->lvid_SelectedName = node->ln_Name;

            i--;
	    node = node->ln_Succ;
	}
    }

    if (oldTop != newTop)
    {
        LVID(lv)->lvid_Top = newTop;

        if (render == 0)
            render = RENDER_UPDATE;
        else if (render == RENDER_SELECT)
            render = RENDER_FULL;
    }

    if (render)
        RenderLV(lv,wp,render);
}


/*****************************************************************************/


VOID RefreshLV(struct ExtGadget *lv, struct Window *wp, BOOL refresh)
{
    RenderLV(lv,wp,RENDER_FULL);
}


/*****************************************************************************/


WORD MouseY(struct Window *wp)
{
    if (wp->Flags & WFLG_GIMMEZEROZERO)
        return(wp->GZZMouseY);

    return(wp->MouseY);
}


/*****************************************************************************/


static WORD GetMaxPen(struct DrawInfo *di, LONG pen, ...)
{
WORD  maxPen;
LONG *ptr;

    maxPen = 0;
    ptr    = &pen;
    while (*ptr != -1)
    {
        if (di->dri_Pens[*ptr] > maxPen)
            maxPen = di->dri_Pens[*ptr];

        ptr++;
    }

    return(maxPen);
}


/*****************************************************************************/


typedef unsigned long (*HOOKFUNC)();


struct ExtGadget *CreateListViewA(struct ExtGadget *gad, struct NewGadget *ng,
                                  struct TagItem *taglist)
{
struct NewGadget  mod_ng;
struct ExtGadget *preLV, *lv;
struct TagItem   *tag;
struct ExtGadget *displaygad = NULL;
long             frametype;
LONG             lvheight;
UWORD            scrollwidth;
UWORD            itemHeight;
UWORD            visible;
struct TagItem   tags[2];

    mod_ng               = *ng;
    mod_ng.ng_GadgetText = NULL;

    tags[0].ti_Tag  = GA_Disabled;   /* overide setting in taglist */
    tags[0].ti_Data = FALSE;
    tags[1].ti_Tag  = TAG_MORE;
    tags[1].ti_Data = (ULONG)taglist;
    if (!(lv = CreateGenericBase(gad, ng, LISTVIEW_IDATA_SIZE,tags)))
    	return(NULL);

    if (tag = findGTTagItem(GTLV_ShowSelected,taglist))
    {
        LVID(lv)->lvid_AllowSelections = TRUE;
	if (displaygad = (struct ExtGadget *)tag->ti_Data)    /* not NULL means caller has a string gadget */
	{
	    /* The visual Width had better match since we can't adjust it */
	    if (displaygad->Width + (LRTRIM + 2 * BEVELXSIZE) != ng->ng_Width)
		return(NULL);

	    displaygad->LeftEdge = ng->ng_LeftEdge + LEFTTRIM + BEVELXSIZE;
	    displaygad->TopEdge  = ng->ng_TopEdge + TOPTRIM + BEVELYSIZE;
            displaygad->MoreFlags &= ~(GMORE_GADGETHELP);   /* no help on the display gadget, the listview dummy gadget takes care of that */
	}
    }

    scrollwidth = getGTTagData(GTLV_ScrollWidth, 16, taglist);
    itemHeight  = getGTTagData(GTLV_ItemHeight,ng->ng_TextAttr->ta_YSize,taglist)
                  + getTagData(LAYOUTA_SPACING, 0, taglist);

    if ((visible = (ng->ng_Height - TBTRIM - (displaygad ? displaygad->BoundsHeight : 0)) / itemHeight) == 0)
	return(NULL);

    lvheight = visible * itemHeight + TBTRIM;

    if (displaygad)
	displaygad->TopEdge += lvheight;

    preLV = gad;

    SGAD(lv)->sg_SetAttrs = SetListViewAttrsA;
    SGAD(lv)->sg_GetTable = Listview_GetTable;
    SGAD(lv)->sg_Flags    = SG_EXTRAFREE_DISPOSE | SG_EXTRAFREE_DISPOSE_LV;
    SGAD(lv)->sg_Refresh  = RefreshLV;

    LVID(lv)->lvid_DefaultCallBack.h_SubEntry = (HOOKFUNC)DefaultCallBack;
    LVID(lv)->lvid_DefaultCallBack.h_Entry    = callCHook;
    LVID(lv)->lvid_DefaultCallBack.h_Data     = GadToolsBase;
    LVID(lv)->lvid_CallBack                   = (struct Hook *)getGTTagData(GTLV_CallBack,(ULONG)&LVID(lv)->lvid_DefaultCallBack,taglist);
    LVID(lv)->lvid_ItemHeight                 = itemHeight;
    LVID(lv)->lvid_Visible                    = visible;
    LVID(lv)->lvid_DisplayGad                 = displaygad;
    LVID(lv)->lvid_Selected                   = (UWORD)~0;
    LVID(lv)->lvid_Font                       = OpenFont(ng->ng_TextAttr);
    LVID(lv)->lvid_DrawInfo                   = VI(ng->ng_VisualInfo)->vi_DrawInfo;
    LVID(lv)->lvid_MaxPen                     = getGTTagData(GTLV_MaxPen,
                                                             GetMaxPen(LVID(lv)->lvid_DrawInfo,
                                                                       TEXTPEN,BACKGROUNDPEN,
                                                                       FILLPEN,FILLTEXTPEN,
                                                                       BLOCKPEN,
                                                                      -1),
                                                             taglist);

    frametype = FRAME_BUTTON;
    if (LVID(lv)->lvid_ReadOnly = getGTTagData(GTLV_ReadOnly, FALSE, taglist))
	frametype = FRAME_BUTTON|FRAMETYPE_RECESSED;

    if (!(lv->GadgetRender = getBevelImage(0,0,ng->ng_Width - scrollwidth,lvheight,frametype)))
	return(NULL);

    /* Well, doesn't this just suck?  The dummy gadget's height
     * should really be lvheight + displaygad->BoundsHeight,
     * but V37 forgets to add the bounds height of the display gadget.
     * Applications are depending on this, unfortunately.
     * The extra height is needed for correct label positioning,
     * so watch me add it for placeGadgetText(), then uncorrect
     * it for compatibility.
     * I'm not a happy guy, but it's a self-inflicted wound :-(
     */
    /* Set the true height of the gadget: */
    lv->Height = lvheight;
    if (displaygad)
	lv->Height += displaygad->BoundsHeight;

    lv->BoundsHeight = lv->Height;
    placeGadgetText(lv, ng->ng_Flags, PLACETEXT_ABOVE, NULL );
    lv->Height = lvheight;
    lv->Flags  |= GADGIMAGE | GADGHNONE;

    mod_ng.ng_LeftEdge = ng->ng_LeftEdge + 2;
    mod_ng.ng_TopEdge  = ng->ng_TopEdge + 2;
    mod_ng.ng_Width    = lv->Width - scrollwidth - 4;
    mod_ng.ng_Height   = lvheight - TBTRIM;
    if (!(gad = LVID(lv)->lvid_ListGad = (struct ExtGadget *)CreateGadget(GENERIC_KIND, lv, &mod_ng,
                                                             TAG_MORE, taglist)))
    {
        return(NULL);
    }

    gad->Flags                 = GADGHNONE | GADGIMAGE;  /* clears away GFLG_EXTENDED */
    gad->Activation            = GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE;
    gad->GadgetType           |= BOOLGADGET;
    SGAD(gad)->sg_Parent       = lv;
    SGAD(gad)->sg_EventHandler = HandleListView;
    SGAD(gad)->sg_Flags        = SG_MOUSEMOVE | SG_MOUSEBUTTONS | SG_INTUITICKS;

    /* get us a scroller! */
    mod_ng.ng_LeftEdge = ng->ng_LeftEdge + ng->ng_Width - scrollwidth;
    mod_ng.ng_TopEdge  = ng->ng_TopEdge;
    mod_ng.ng_Width    = scrollwidth;
    mod_ng.ng_Height   = lvheight;

    /* !!! Would need some tagitem filtering here: */
    if (!(gad = (struct ExtGadget *)CreateGadget(SCROLLER_KIND,gad,&mod_ng,
                             GTSC_Visible, visible,
                             PGA_FREEDOM,  LORIENT_VERT,
                             /* Arrows should be the same height as a single line, but no bigger
                              * than 1/4 the height of the whole scroller:
                              */
                             GTSC_Arrows,  min(itemHeight,mod_ng.ng_Height >> 2),
                             GA_Disabled,  FALSE,   /* override setting in user tag list */
                             TAG_MORE,     taglist)))
    {
	return(NULL);
    }

    LVID(lv)->lvid_Scroller  = gad;
    SCID(gad)->scid_ListView = lv;         /* Install back pointer into Scroller */
    gad->MoreFlags &= ~(GMORE_GADGETHELP); /* no help on the scroller, the dummy gadget takes care of that */

    SetListViewAttrsA(lv,NULL,taglist);

    preLV->NextGadget = lv->NextGadget;
    gad->NextGadget   = lv;
    lv->NextGadget    = NULL;

    return(lv);
}


/*****************************************************************************/


BOOL HandleListView(struct ExtGadget *gad, struct IntuiMessage *imsg)
{
struct ExtGadget *lv = SGAD(gad)->sg_Parent;
WORD              newSelected;
struct TagItem    tags[3];
BOOL              result;
ULONG             class = imsg->Class;
struct Window    *wp    = imsg->IDCMPWindow;
BOOL              allowSelections;

    if (MouseY(wp) < gad->TopEdge)
    {
        if (class == IDCMP_MOUSEMOVE)
            return(FALSE);

        if (newSelected = LVID(lv)->lvid_Top)  /* assignment! */
            newSelected--;
    }
    else if (MouseY(wp) >= gad->TopEdge + gad->Height)
    {
        if (class == IDCMP_MOUSEMOVE)
            return(FALSE);

        newSelected = LVID(lv)->lvid_Top + LVID(lv)->lvid_Visible;
    }
    else
    {
        newSelected = (MouseY(wp) - gad->TopEdge) / LVID(lv)->lvid_ItemHeight + LVID(lv)->lvid_Top;
    }

    if (newSelected >= LVID(lv)->lvid_Total)
    {
        /* NOTE! If LVID(lv)->lvid_Total is 0, newSelected will become
         * -1, or in other words ~0
         */
        newSelected = LVID(lv)->lvid_Total - 1;
    }

    if (newSelected != LVID(lv)->lvid_Selected)
    {
        tags[0].ti_Tag  = GTLV_MakeVisible;
        tags[0].ti_Data = newSelected;
        tags[1].ti_Tag  = GTLV_Selected;
        tags[1].ti_Data = newSelected;
        tags[2].ti_Tag  = TAG_DONE;

        allowSelections = LVID(lv)->lvid_AllowSelections;
        LVID(lv)->lvid_AllowSelections = TRUE;

        SetListViewAttrsA(lv,wp,tags);

        LVID(lv)->lvid_AllowSelections = allowSelections;
    }

    if ((class == IDCMP_GADGETUP) || ((class == IDCMP_MOUSEBUTTONS) && (imsg->Code == SELECTUP)))
    {
        imsg->Class    = IDCMP_GADGETUP;
        imsg->Code     = LVID(lv)->lvid_Selected;
        imsg->IAddress = (APTR)lv;

        result = (newSelected == LVID(lv)->lvid_Selected) && (newSelected != ~0);

        if (!LVID(lv)->lvid_AllowSelections)
        {
            RenderLV(lv,wp,RENDER_DESELECT);
            LVID(lv)->lvid_Selected = (UWORD)~0;
        }

        return(result);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID SetListViewAttrsA(struct ExtGadget *lv, struct Window *win,
                       struct TagItem *taglist)
{
WORD         oldTop;
WORD         oldTotal;
WORD         oldSelected;
struct List *oldLabels;

    oldTop      = LVID(lv)->lvid_Top;
    oldTotal    = LVID(lv)->lvid_Total;
    oldSelected = LVID(lv)->lvid_Selected;
    oldLabels   = LVID(lv)->lvid_Labels;

    SetLVAttrs(lv,win,taglist);

    if (LVID(lv)->lvid_Labels != (struct List *)~0)
    {
        if ((oldLabels == (struct List *)~0) || (LVID(lv)->lvid_Selected != oldSelected) || findGTTagItem(GTLV_Labels,taglist))
            GT_SetGadgetAttrs(LVID(lv)->lvid_DisplayGad,win,NULL,GTST_String, LVID(lv)->lvid_SelectedName,
                                                                 TAG_MORE,    taglist);
                                                                 /* lets GA_Disabled through */

        if ((oldLabels == (struct List *)~0) || (LVID(lv)->lvid_Total != oldTotal) || (LVID(lv)->lvid_Top != oldTop))
            GT_SetGadgetAttrs(LVID(lv)->lvid_Scroller,win,NULL,GTSC_Top,   LVID(lv)->lvid_Top,
                                                               GTSC_Total, LVID(lv)->lvid_Total,
                                                               TAG_DONE);
    }
}


/*****************************************************************************/


/* for private use by SCROLLER_KIND */
VOID SetListViewTop(struct ExtGadget *lv, struct Window *win,
                    WORD top)
{
struct TagItem tags[2];

    tags[0].ti_Tag  = GTLV_Top;
    tags[0].ti_Data = top;
    tags[1].ti_Tag  = TAG_DONE;

    SetLVAttrs(lv,win,tags);
}
