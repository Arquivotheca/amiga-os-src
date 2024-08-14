/*** slider.c *************************************************************
*
*   slider.c	- Slider gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: slider.c,v 39.12 92/10/16 18:23:38 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* !!!Note:  In general, the way the labels are positioned presumes that
 * spaces are the same width as numbers in the particular font.
 * Also, two BLTN304 warnings are generated in -rr mode (even though
 * the function that it uses __stdargs for is declared EXPLICITLY
 * that way)
 */

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreateSliderA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void RefreshSlider (struct ExtGadget *gad, struct Window *win, BOOL refresh);
BOOL HandleSlider (struct ExtGadget *gad, struct IntuiMessage *imsg);
void SetSliderAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);

static void FindSliderValues (UWORD numlevels, UWORD level,
    UWORD *body, UWORD *pot, BOOL invert);
static UWORD FindSliderLevel (UWORD numlevels, UWORD pot, BOOL invert);

/*------------------------------------------------------------------------*/

/* CreateSliderA() */

struct TagItem to_prop_map[] =
{
    GA_IMMEDIATE, GTPROP_GADGETDOWN,
    GA_RELVERIFY, GTPROP_GADGETUP,
    TAG_DONE, 0,
};


/*****************************************************************************/


struct ExtGadget *CreateSliderA(struct ExtGadget *gad, struct NewGadget *ng,
                             struct TagItem *taglist )
{
struct ExtGadget    *preSL, *slgad;
struct NewGadget  mod_ng;
struct TagItem   *ti;
BOOL              displaylevel;
struct Rectangle  rect, gadgetrect, result;
WORD              maxLen;
UWORD             levelPlace;
Point             textsize;
struct TagItem   *maxPixTag;
struct IntuiText *itext;
struct TextFont  *font;

    /* We must create the dummy-gadget first, but we would like to
     * use it as the return value, so later we move it to last
     * in this piece of gadget-list.  We have to save the value of
     * gad before the slider was allocated:
     */
    preSL = gad;

    if (!(slgad = CreateGenericBase(preSL, ng, SLIDER_IDATA_SIZE, taglist)))
    	return(NULL);

    slgad->Flags |= GADGIMAGE|GADGHNONE;

    SGAD(slgad)->sg_SetAttrs = SetSliderAttrsA;
    SGAD(slgad)->sg_GetTable = Slider_GetTable;
    SGAD(slgad)->sg_Refresh  = RefreshSlider;
    SGAD(slgad)->sg_Flags    = SG_EXTRAFREE_DISPOSE;

    if (!(slgad->GadgetRender = getBevelImage(0,0,slgad->Width,slgad->Height,FRAME_BUTTON)))
	return(NULL);

    mod_ng = *ng;
    mod_ng.ng_LeftEdge  += LEFTTRIM;
    mod_ng.ng_TopEdge   += TOPTRIM;
    mod_ng.ng_Width     -= LRTRIM;
    mod_ng.ng_Height    -= TBTRIM;
    mod_ng.ng_GadgetText = NULL;

    displaylevel = (findGTTagItem(GTSL_MaxLevelLen, taglist) &&
	            findGTTagItem(GTSL_LevelFormat, taglist));

    maxLen = getGTTagData(GTSL_MaxLevelLen, 2, taglist);

    if (gad = CreateGenericBase(slgad,&mod_ng, (((displaylevel) ? sizeof(struct IntuiText) +
                                ALIGNSIZE(maxLen + 1) : 0) ),
                                taglist))
    {
	DP(("CS:  gadget at $%lx\n", gad));

	SGAD(gad)->sg_Parent       = slgad;
	SGAD(gad)->sg_EventHandler = HandleSlider;
	SGAD(gad)->sg_Flags        = SG_MOUSEMOVE;

	/* Find and fill out the instance data for the slider: */
	SLID(slgad)->slid_Prop        = gad;
	SLID(slgad)->slid_Flags       = PackBoolTags(NULL, taglist, to_prop_map);
	SLID(slgad)->slid_Max         = 15;
        SLID(slgad)->slid_Extent.MinX = 1;      /* causes an invalid Extent, so first EraseOldExtent() call won't happen */
	if (ti = findTagItem(PGA_FREEDOM, taglist))
	{
	    if (ti->ti_Data == LORIENT_VERT)
	    {
		SLID(slgad)->slid_Flags |= GTPROP_VERTICAL;
	    }
	}

	/* Install required blank image structure: */
	gad->GadgetRender  = (APTR) &SLID(slgad)->slid_PropImage;
	gad->Flags         = GADGHNONE | GADGIMAGE;  /* clears away GFLG_EXTENDED flag */
	gad->Activation    = GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE;
	gad->GadgetType   |= PROPGADGET;
	gad->SpecialInfo   = (APTR) &SLID(slgad)->slid_PropInfo;

	if (displaylevel)
	{
	    SLID(slgad)->slid_IText            = MEMORY_FOLLOWING(SGAD(gad));
	    SLID(slgad)->slid_IText->FrontPen  = VI(ng->ng_VisualInfo)->vi_textPen;
	    SLID(slgad)->slid_IText->BackPen   = VI(ng->ng_VisualInfo)->vi_backgroundPen;
	    SLID(slgad)->slid_IText->DrawMode  = JAM2;
	    SLID(slgad)->slid_IText->ITextFont = ng->ng_TextAttr;
	    SLID(slgad)->slid_IText->IText     = (STRPTR)MEMORY_FOLLOWING(SLID(slgad)->slid_IText);
	}

	if (SLID(slgad)->slid_Flags & GTPROP_VERTICAL)
	{
	    SLID(slgad)->slid_PropInfo.Flags = AUTOKNOB | PROPBORDERLESS | FREEVERT;
	    SLID(slgad)->slid_Pot            = &SLID(slgad)->slid_PropInfo.VertPot;
	    SLID(slgad)->slid_Body           = &SLID(slgad)->slid_PropInfo.VertBody;
	}
	else
	{
	    SLID(slgad)->slid_PropInfo.Flags = AUTOKNOB | PROPBORDERLESS | FREEHORIZ;
	    SLID(slgad)->slid_Pot            = &SLID(slgad)->slid_PropInfo.HorizPot;
	    SLID(slgad)->slid_Body           = &SLID(slgad)->slid_PropInfo.HorizBody;
	}

	/* Move the dummy-gadget to the end of this list, so that
	 * it is the one that is returned:
	 */
	preSL->NextGadget = slgad->NextGadget;
	gad->NextGadget = slgad;
	slgad->NextGadget = NULL;
	MP(("CSL: Done.\n"));
	gad = slgad;

	SetSliderAttrsA(slgad,NULL,taglist);

        levelPlace = getGTTagData(GTSL_LevelPlace,PLACETEXT_LEFT,taglist);

        placeGadgetText(slgad,ng->ng_Flags,PLACETEXT_LEFT,&rect);

        if (itext = SLID(slgad)->slid_IText)
        {
            if (maxPixTag = findGTTagItem(GTSL_MaxPixelLen,taglist))
            {
                SLID(slgad)->slid_MaxPixelLen = maxPixTag->ti_Data;
                textsize.x = SLID(slgad)->slid_MaxPixelLen;
                textsize.y = ng->ng_TextAttr->ta_YSize;
            }
            else
            {
                if (font = OpenFont(itext->ITextFont))
                {
                    SLID(slgad)->slid_MaxPixelLen = maxLen * font->tf_XSize;
                    CloseFont(font);
                }
                else
                {
                    return(NULL);
                }
                intuiTextSize(itext,&textsize);
            }
            PlaceSizedIntuiText(itext,&rect,levelPlace,&textsize);

            gadgetrect.MinX = slgad->BoundsLeftEdge;
            gadgetrect.MinY = slgad->BoundsTopEdge;
            gadgetrect.MaxX = slgad->BoundsLeftEdge + slgad->BoundsWidth - 1;
            gadgetrect.MaxY = slgad->BoundsTopEdge + slgad->BoundsHeight - 1;

            rect.MinX = slgad->LeftEdge + itext->LeftEdge;
            rect.MinY = slgad->TopEdge + itext->TopEdge;
            rect.MaxX = rect.MinX + textsize.x;
            rect.MaxY = rect.MinY + textsize.y;

            CombineRects(&gadgetrect,&rect,&result);

            slgad->BoundsLeftEdge = result.MinX;
            slgad->BoundsTopEdge  = result.MinY;
            slgad->BoundsWidth    = result.MaxX - result.MinX + 1;
            slgad->BoundsHeight   = result.MaxY - result.MinY + 1;
        }
    }

    return(gad);
}


/*****************************************************************************/


VOID SetSliderAttrsA(struct ExtGadget *slgad, struct Window *win,
                     struct TagItem *taglist)
{
struct ExtGadget *gad;

    SLID(slgad)->slid_DispFunc      = (LONG (*)())getGTTagData(GTSL_DispFunc, (ULONG)SLID(slgad)->slid_DispFunc, taglist);
    SLID(slgad)->slid_Min           = (WORD)getGTTagData(GTSL_Min,SLID(slgad)->slid_Min,taglist);
    SLID(slgad)->slid_Max           = (WORD)getGTTagData(GTSL_Max,SLID(slgad)->slid_Max,taglist);
    SLID(slgad)->slid_Level         = (WORD)getGTTagData(GTSL_Level,SLID(slgad)->slid_Level,taglist);
    SLID(slgad)->slid_LevelFormat   = (STRPTR) getGTTagData(GTSL_LevelFormat,(ULONG)SLID(slgad)->slid_LevelFormat,taglist);
    SLID(slgad)->slid_Justification = getGTTagData(GTSL_Justification,(ULONG)SLID(slgad)->slid_Justification,taglist);

    if (SLID(slgad)->slid_Level > SLID(slgad)->slid_Max)
	SLID(slgad)->slid_Level = SLID(slgad)->slid_Max;

    else if (SLID(slgad)->slid_Level < SLID(slgad)->slid_Min)
	SLID(slgad)->slid_Level = SLID(slgad)->slid_Min;

    FindSliderValues((WORD)(SLID(slgad)->slid_Max - SLID(slgad)->slid_Min + 1),
                     (WORD)(SLID(slgad)->slid_Level - SLID(slgad)->slid_Min),
                     SLID(slgad)->slid_Body, SLID(slgad)->slid_Pot,
                     (BOOL)(SLID(slgad)->slid_Flags & GTPROP_VERTICAL));

    gad = SLID(slgad)->slid_Prop;

    if (win)
        NewModifyProp(gad, win, NULL,
                      SLID(slgad)->slid_PropInfo.Flags,
                      SLID(slgad)->slid_PropInfo.HorizPot,
                      SLID(slgad)->slid_PropInfo.VertPot,
                      SLID(slgad)->slid_PropInfo.HorizBody,
                      SLID(slgad)->slid_PropInfo.VertBody, 1);

    RefreshSlider(slgad, win, FALSE);
    TagAbleGadget(gad, win, taglist);
}


/*****************************************************************************/


/*/ RefreshSlider()
 *
 * Function to refresh the level indicator (if any) beside the slider.
 */

VOID RefreshSlider(struct ExtGadget *slgad, struct Window *win,
                   BOOL refresh )
{
LONG              num;
struct Rectangle  oldExtent;
struct RastPort  *rp;

    if (SLID(slgad)->slid_IText)
    {
	if (SLID(slgad)->slid_DispFunc)
	{
	    LONG __stdargs (*dispfunc)(struct ExtGadget *, WORD);

	    dispfunc = SLID(slgad)->slid_DispFunc;
	    num = (*dispfunc)(slgad, SLID(slgad)->slid_Level);
	    DP(("RL(sl):  level %ld, converted num %ld\n",
		(LONG)SLID(slgad)->slid_Level, num));
	}
	else
	{
	    num = (LONG) SLID(slgad)->slid_Level;
	}
	DP(("RS:  Num = %ld\n", num));
	sprintf(SLID(slgad)->slid_IText->IText, SLID(slgad)->slid_LevelFormat, num);
	DP(("RS:  Formatted label: '%s'\n", SLID(slgad)->slid_IText->IText));

	if (rp = GetRP(slgad,win))
	{
	    oldExtent = SLID(slgad)->slid_Extent;
            printITextToFit(rp,SLID(slgad)->slid_IText,
                            slgad->LeftEdge,slgad->TopEdge,
                            SLID(slgad)->slid_MaxPixelLen,
                            SLID(slgad)->slid_Justification,
                            &SLID(slgad)->slid_Extent);

            if (oldExtent.MaxX > oldExtent.MinX)
                EraseOldExtent(rp,&oldExtent,&SLID(slgad)->slid_Extent);
        }
    }
}


/*****************************************************************************/


/*/ HandleSlider()
 *
 * Function to handle IntuiMessages that relate to sliders.
 *
 * Created:  30-Aug-89, Peter Cherna
 * Modified: 08-Nov-89, Peter Cherna
 *
 */

BOOL HandleSlider(struct ExtGadget *gad, struct IntuiMessage *imsg)
{
    UWORD newlevel;
    BOOL hearupdown = FALSE;
    struct ExtGadget *slgad = SGAD(gad)->sg_Parent;

    DP(("HS: Gadget at $%lx\n", gad));

    /* IAddress for IDCMP_MOUSEMOVE's points to the window even if it
     * was a gadget being FOLLOWMOUSE'd.  We are ok to 'fix' it
     * here since it is our gadget class.
     */
    imsg->IAddress = (APTR) slgad;

    /* Find a new value of level based on the new value of Pot: */
    newlevel = FindSliderLevel((WORD)(SLID(slgad)->slid_Max - SLID(slgad)->slid_Min + 1),
	*SLID(slgad)->slid_Pot, (BOOL)(SLID(slgad)->slid_Flags & GTPROP_VERTICAL)) + SLID(slgad)->slid_Min;

    if (imsg->Class == IDCMP_GADGETDOWN)
    {
	DP(("HS: IDCMP_GADGETDOWN\n"));
	if (SLID(slgad)->slid_Flags & GTPROP_GADGETDOWN)
	{
	    /* The client wants to hear this always: */
	    hearupdown = TRUE;
	}
    }
    else if (imsg->Class == IDCMP_GADGETUP)
    {
	DP(("HS: IDCMP_GADGETUP\n"));
	if (SLID(slgad)->slid_Flags & GTPROP_GADGETUP)
	{
	    /* The client wants to hear this always: */
	    hearupdown = TRUE;
	}

	/* When we get a IDCMP_GADGETUP event, we want the knob to jump to
	 * the nearest integral level
	 */
	FindSliderValues((WORD)(SLID(slgad)->slid_Max - SLID(slgad)->slid_Min + 1),
                         (WORD)(newlevel - SLID(slgad)->slid_Min),
                         SLID(slgad)->slid_Body, SLID(slgad)->slid_Pot,
                         (BOOL)(SLID(slgad)->slid_Flags & GTPROP_VERTICAL));

	NewModifyProp(gad, imsg->IDCMPWindow, NULL,
                      SLID(slgad)->slid_PropInfo.Flags,
	              SLID(slgad)->slid_PropInfo.HorizPot,
                      SLID(slgad)->slid_PropInfo.VertPot,
                      SLID(slgad)->slid_PropInfo.HorizBody,
                      SLID(slgad)->slid_PropInfo.VertBody, 1);
    }
    D(else
    {
	DP(("HS: IDCMP_MOUSEMOVE\n"));
    });

    /* If we cause a message, we want the code field equal to the
     * new level.
     * We just stuffed a WORD into a UWORD, but it's just a carrier.
     * Later if the client copies imsg->Code into a WORD, he'll
     * get his signs back
     */
    imsg->Code = newlevel;

    /* Always react if the value of level actually changed: */
    if (newlevel != SLID(slgad)->slid_Level)
    {
	SLID(slgad)->slid_Level = newlevel;
	RefreshSlider(slgad, imsg->IDCMPWindow, FALSE);

	/* If this is a IDCMP_GADGETUP or IDCMP_GADGETDOWN and our client doesn't
	 * want to hear this type, we satisfy him by converting it
	 * to a IDCMP_MOUSEMOVE, since he MUST hear of every event in which
	 * the level changed:
	 */
	if (!hearupdown)
	    imsg->Class = IDCMP_MOUSEMOVE;
	DP(("HS: Level Changed\n"));
	D(
	    if ((imsg->Class == IDCMP_MOUSEMOVE) &&
		(hearupdown) &&
		(imsg->IAddress != (APTR)slgad))
		DP(("ERROR1: slgad $%lx, win $%lx\n",slgad, imsg->IDCMPWindow));
	);
	return(TRUE);
    }

    /* The level did not change, so only let the message reach
     * the client if it was a IDCMP_GADGETUP or IDCMP_GADGETDOWN and the client
     * was interested in that - this condition is reflected by
     * the value of hearupdown:
     */
    DP((hearupdown ? "HS: Sending message even though level unchanged\n" :
	"HS: Message dropped since level unchanged.\n"));
    D(
	if ((imsg->Class == IDCMP_MOUSEMOVE) &&
	    (hearupdown) &&
	    (imsg->IAddress != (APTR)slgad))
	    DP(("ERROR2: slgad $%lx, win $%lx\n",slgad, imsg->IDCMPWindow));
    );
    return(hearupdown);
}


/*****************************************************************************/


/*/ FindSliderValues()
 *
 * Function to calculate the Body and Pot values of a slider gadget
 * given the two values numlevels and level, representing the
 * number of levels available in the slider, and the current level.
 * For example, a Red, Green, or Blue slider would have (currently)
 * numlevels = 16, level = level of the color (0-15).
 *
 * Created:  13-Sep-89, Peter Cherna
 * Modified: 15-May-90, Peter Cherna
 *
 */

static void FindSliderValues( UWORD numlevels, UWORD level,
                              UWORD *body, UWORD *pot, BOOL invert )
{
    /* body is the relative size of the proportional gadget's body.
     * Clearly, this proportion should be 1 / numlevels.
     */

    if (numlevels > 0)
	(*body) = (MAXBODY) / numlevels;
    else
	(*body) = MAXBODY;

    /* pot is the position of the proportional gadget body, with zero
     *        meaning that the slider is all the way up (or left),
     *        and full (MAXPOT) meaning that the slider is all the way
     *        down (or right).
     *
     * For slider gadgets the derivation is a bit ugly:
     *
     * We illustrate a slider of four levels (0, 1, 2, 3) with the slider
     * at level 2.  The key observation is that pot refers the the leading
     * edge of the knob, and as such MAXPOT is not all the way to the
     * right, but is one body's width left of that.
     *
     * Level:	    0       1       2       3
     *		---------------------------------
     *		|       |       |*******|       |
     *	 	|       |       |*******|       |
     *		|       |       |*******|       |
     *		|       |       |*******|       |
     *		---------------------------------
     *		|               |       |
     *		0              pot    MAXPOT
     *
     * From which we observe that pot = MAXPOT * (level/(numlevels-1))
     *
     */

    if (numlevels > 1)
    {
	(*pot) = (((ULONG)MAXPOT) * level)/(numlevels-1);
    }
    else
    {
	(*pot) = 0;
    }

    if (invert)
    {
	(*pot) = MAXPOT - (*pot);
    }
}


/*------------------------------------------------------------------------*/

/*/ FindSliderLevel()
 *
 * Function to calculate the level of a slider gadget given the
 * total number of levels as well as the HorizPot or VertPot value.
 *
 * Created:  13-Sep-89, Peter Cherna
 * Modified: 15-May-90, Peter Cherna
 */

static UWORD FindSliderLevel( UWORD numlevels, UWORD pot, BOOL invert )
{
    UWORD level;

    /* If you thought the explanation for calculating pot from level was
     * bad, you ain't seen the reverse:
     *
     * We illustrate a 4-level slider (0, 1, 2, 3) with the knob on
     * the transition point between calling it at levels 1 and 2.
     *
     * Level:	    0       1       2       3
     *		---------------------------------
     *	 	|       |    ***|***    |       |
     *		|       |    ***|***    |       |
     *		|       |    ***|***    |       |
     *		|       |    ***|***    |       |
     *		---------------------------------
     *		|           |           |
     *		0          pot        MAXPOT
     *
     * We've already shown that the vertical lines (which represent the
     * natural position of the knob for a given level are:
     *
     * 	pot = MAXPOT * (level/(numlevels-1))
     *
     * and we see that the threshold between level and level-1
     * is half-way between pot(level) and pot(level-1),
     * from which we get
     *
     * 	level = (numlevels-1) * (pot/MAXPOT) + 1/2
     *
     */

    if (numlevels > 1)
    {
	level = (((ULONG)pot) * (numlevels-1) + MAXPOT/2) / MAXPOT;
    }
    else
    {
	level = 0;
    }

    if (invert)
    {
	/* Vertical sliders are inverted wrt. Intuition's idea of
	 * min and max:
	 */
	level = numlevels - 1 - level;
    }
    return(level);
}
