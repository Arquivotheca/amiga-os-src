/* layout.c
 *
 * You must copy the template to the appropriate requester structure.  Zero
 * the LOGroup of any gadget that is conditional.
 *
 * The variables are:
 *
 *    struct ASLGadget *fr_Template[MAX_FR_GADGETS];
 *    struct Gadget *fr_Gadgets[MAX_FR_GADGETS];
 *
 * Creation attributes are declared statically in the template.  A SetAttr
 * pass for the dynamic attributes must be done once the gadgets are attached
 * to the window.
 *
 * The gadget array should start with objects that have a fixed height, and
 * end with a group that has a relative height (the listview at the top of
 * the window).
 *
 */

/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <dos/datetime.h>
#include <utility/tagitem.h>
#include <graphics/regions.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <string.h>
#include <math.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "aslutils.h"
#include "filereq.h"
#include "fontreq.h"
#include "asllists.h"
#include "layout.h"


/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	DEF_WIDTH  8L
#define	DEF_HEIGHT 8L

/*
 *  a = default font size (width or height)
 *  b = current font size (width or height)
 *  c = default object size (width or height)
 *
 */

#define SCALE(a,b,c)	(((b) * (c)) / (a))


/*****************************************************************************/


#define	MAX_ARRAY	20

#define	CA_COUNT	0	/* Number of members (row count) */
#define	CA_CNT		1	/* Elements */
#define	CA_MWIDTH	2	/* Maximum label width */
#define	CA_GWIDTH	3	/* Maximum gadget width */
#define	CA_START	4	/* First element */
#define	CA_WIDTH	5
#define	CA_MAX		6


/*****************************************************************************/


#define	MAX_LEVELS 2


/*****************************************************************************/


VOID GadgetBox (struct ReqInfo * ri, struct IBox * src, struct IBox * dst,
		 WORD fwidth, WORD fheight)
{

    /* Set the upper left coordinates */
    dst->Left = src->Left;
    dst->Top = src->Top;
    dst->Width = (src->Width > 0) ? SCALE (DEF_WIDTH, fwidth, src->Width) : src->Width;
    dst->Height = SCALE (DEF_HEIGHT, fheight, src->Height);

    /* Adjust for relative coordinates */
    if (dst->Left < 0)
	dst->Left = ri->ri_Coords.Width + dst->Left;

    if (dst->Top < 0)
	dst->Top = ri->ri_Coords.Height + dst->Top;

    if (dst->Width < 0)
	dst->Width = ri->ri_Coords.Width + dst->Width;

    if (dst->Height < 0)
	dst->Height = ri->ri_Coords.Height + dst->Height;
}


/*****************************************************************************/


WORD LabelsWidth (struct ReqInfo * ri, struct RastPort * rp, ULONG tagt, struct TagItem * tags)
{
    struct TagItem *tag;
    WORD result = 0;
    STRPTR *labels;
    WORD i;

    if ((tag = FindTagItem (tagt, tags)) && (labels = (STRPTR *) tag->ti_Data))
    {
	for (i = 0; labels[i]; i++)
	    result = MAX (result, TextLength (rp, labels[i], strlen (labels[i])));
    }

    return (result);
}


/*****************************************************************************/


STRPTR GetGadgetLabel (struct ReqInfo * ri, AppStringsID stringID)
{
    STRPTR label;

    if ((stringID == MSG_ASL_OK_GAD) && (ri->ri_PositiveText))
	label = ri->ri_PositiveText;
    else if ((stringID == MSG_ASL_CANCEL_GAD) && (ri->ri_NegativeText))
	label = ri->ri_NegativeText;
    else if (stringID == MSG_ASL_FO_MODE_GAD)
	label = ((struct ExtFontReq *)ri)->fo_ModeLabels[0];
    else
	label = GetString (&ri->ri_LocaleInfo, stringID);

    return (label);
}


/*****************************************************************************/

struct Pals
{
    UWORD w, h;
};

struct Pals psa[] =
{
    {28, 14},			/* 2 */
    {48, 14},			/* 4 */
    {88, 24},			/* 8 */
    {128, 24},			/* 16 */
    {148, 24},			/* 32 */
    {188, 34},			/* 64 */
    {200, 46},			/* 128 */
    {320, 58},			/* 256 */
};

/*****************************************************************************/


VOID SetGadgetSize (struct ReqInfo * ri, struct RastPort * rp, WORD i,
		     WORD mspace, WORD fwidth, WORD fheight,
		     WORD * twidth, WORD * mwidth,
		     WORD * gwidth, WORD * gheight)
{
    struct ASLGadget *tgad = &ri->ri_Template[i];
    struct TagItem *tag;
    WORD m, d, nc;

    switch (tgad->ag_GadgetKind)
    {
	case GENERIC_KIND:
	    tgad->ag_MinWidth = tgad->ag_LOWork.Width;
	    tgad->ag_MinHeight = tgad->ag_LOWork.Height;

	    if (tgad->ag_LOSize.Width <= 0)
		tgad->ag_MinWidth = 30;

	    if (tgad->ag_LOSize.Height <= 0)
		tgad->ag_MinHeight = 19;

	    break;

	case LISTVIEW_KIND:
	    tgad->ag_MinWidth = 23 + (mspace * 2);
	    tgad->ag_MinHeight = (2 * (fheight + 1)) + 4;
	    break;

	case CHECKBOX_KIND:	/* checkmark : margin : label */
	    *mwidth = *gwidth = CHECKBOX_WIDTH;
	    *gheight = MAX (CHECKBOX_HEIGHT, fheight) + 2;
	    tgad->ag_MinWidth = *gwidth + INTERWIDTH + *twidth;
	    tgad->ag_MinHeight = *gheight;
	    break;

	case CYCLE_KIND:	/* label : margin : glyph : margin : labels : margin */
	    m = LabelsWidth (ri, rp, GTCY_Labels, tgad->ag_CreationTags);
	    *mwidth = *gwidth = 28 + mspace + m;
	    *gheight = MAX (14, fheight + 6);
	    tgad->ag_MinWidth = *mwidth;	/*  + INTERWIDTH + *twidth; */
	    tgad->ag_MinHeight = *gheight;
	    break;

	case SLIDER_KIND:	/* Just for the ASL requesters */
	    *gheight = fheight + 3;

	    if (*mwidth < 80)	/* pick a number, any number ! */
		*mwidth = 80;

	    tgad->ag_MinWidth = *mwidth;
	    tgad->ag_MinHeight = fheight;
	    break;

	case PALETTE_KIND:
	    m = GetTagData (GTPA_IndicatorWidth, 20, tgad->ag_CreationTags);
	    nc = GetTagData (GTPA_NumColors, 0, tgad->ag_CreationTags);

	    d = 1;
	    while ((1 << d) < nc)
	       d++;

	    *mwidth = m + INTERWIDTH + psa[d].w + 8;
	    *gheight = MAX (psa[d].h, fheight + 6);
	    break;

	case BUTTON_KIND:	/* margin : label : margin */
	    *mwidth = 4 + mspace + *twidth;
	    *twidth = 0;

	case NUMBER_KIND:
	case TEXT_KIND:
	    if (tag = FindTagItem (GTIN_MaxChars, tgad->ag_CreationTags))
	    {
		*mwidth = ((tag->ti_Data + 1) * fwidth) + 16;
		if (*gwidth > 0)
		    *gwidth = *mwidth;
	    }
	    *gheight = fheight + 6;
	    tgad->ag_MinHeight = fheight;
	    break;

	case INTEGER_KIND:
	    if (tag = FindTagItem (GTIN_MaxChars, tgad->ag_CreationTags))
	    {
		*mwidth = ((tag->ti_Data + 1) * fwidth) + 16;
		if (*gwidth > 0)
		    *gwidth = *mwidth;
	    }

	case STRING_KIND:
	    *gheight = fheight + 6;
	    tgad->ag_MinHeight = *gheight;
	    break;
    }
}


/*****************************************************************************/


VOID AslFreeGadgets (struct ReqInfo * ri)
{
    UWORD i;

    FreeGadgets (ri->ri_Gadgets);
    ri->ri_Gadgets = NULL;
    ri->ri_LastAdded = NULL;

    for (i = 0; ri->ri_Template[i].ag_GadgetKind != END_KIND; i++)
	ri->ri_Template[i].ag_Gadget = NULL;
}


/*****************************************************************************/


VOID FreeLayoutGadgets (struct ReqInfo * ri, BOOL cleanup)
{
struct Window *wp;

    wp = ri->ri_Window;

    if (ri->ri_Gadgets)
    {
	if (cleanup)
	{
	    RemoveGList(wp, ri->ri_Gadgets, -1);
	    RefreshWindowFrame(wp);

            SetAPen(wp->RPort, ri->ri_DrawInfo->dri_Pens[BACKGROUNDPEN]);
            RectFill(wp->RPort, wp->BorderLeft, wp->BorderTop,
                     wp->Width - wp->BorderRight - 1, wp->Height - wp->BorderBottom - 1);
	}

	AslFreeGadgets(ri);
    }
}


/*****************************************************************************/


BOOL LayoutGadgets (struct ReqInfo * ri, WORD mode)
{
    struct ASLGadget *tgad = ri->ri_Template;
    WORD carray[MAX_ARRAY][CA_MAX];	/* Column array */
    struct IBox msize[MAX_LEVELS];
    struct RastPort trp;
    struct NewGadget ng;	/* Used to create GadTools gadget */
    struct RastPort *rp;
    struct TagItem *tag;
    struct IBox member;		/* Member rectangle */
    BOOL failed;
    WORD gheight;		/* Height of gadget */
    WORD pheight;
    WORD columns;
    STRPTR label;		/* Label holder */
    WORD fheight;		/* Font height */
    WORD ogroup;		/* Previous group number */
    WORD fwidth;		/* Font width */
    WORD mspace;
    WORD gstart;		/* Start of current group */
    WORD twidth;		/* text width */
    WORD mwidth;		/* Total member width */
    WORD gwidth;		/* Width of gadget */
    WORD cwidth;		/* Width of columns */
    WORD pwidth;
    WORD vfill;			/* Spacing */
    WORD hfill;
    WORD lsub;
    WORD top;
    WORD cnt;
    WORD sub;
    WORD i;			/* Current template gadget */
    WORD j;			/* Current destination gadget */
    WORD k;			/* Work variable */
    WORD l;
    WORD m;
    WORD n;

    DB (kprintf ("\nasl.library: layout\n"));

    CalculatePaneSize (ri);

    pwidth = ri->ri_Coords.Width;
    pheight = ri->ri_Coords.Height;

    for (i = 0; i < MAX_LEVELS; i++)
    {
	msize[i].Width = msize[i].Height = 0;
    }

    if (mode == LGM_SETMIN)
    {
	ri->ri_Coords.Width = ri->ri_Coords.Height = 4096;
	InitRastPort (&trp);
	rp = &trp;
    }
    else
    {
	rp = ri->ri_Window->RPort;
	ri->ri_LastAdded = CreateContext (&ri->ri_Gadgets);
    }

    /* Different levels of checking */
    for (n = 0; n < MAX_LEVELS; n++)
    {
	failed = FALSE;

	if (n == 0)
	{
	    /* First time, try it with the better font */
	    ri->ri_Font = ri->ri_DFont;
	    ri->ri_TextAttr = ri->ri_DTextAttr;
	}
	else if (n == 1)
	{
	    /* Second time, try it with the default (topaz 8) font */
	    ri->ri_Font = AslBase->ASL_FFont;
	    ri->ri_TextAttr = AslBase->ASL_FTextAttr;
	}

	SetFont (rp, ri->ri_Font);

	/* Get the destination sizes */
	fwidth = (WORD) TextLength (rp, "n", 1);
	mspace = (WORD) TextLength (rp, "m", 1);
	fheight = (WORD) ri->ri_Font->tf_YSize;

	/* Step through the gadget array */
	top = ri->ri_Coords.Height;

	for (i = j = ogroup = 0;
	     ((tgad[i].ag_GadgetKind != END_KIND) && !failed);
	     i++)
	{
	    /* New group? */
	    if (tgad[i].ag_LOGroup && (tgad[i].ag_LOGroup != ogroup))
	    {
		/* Initialize the variables */
		ogroup = tgad[i].ag_LOGroup;

		/* See if we have a group header */
		gstart = i;
		if ((tgad[i].ag_GadgetKind == HGROUP_KIND) ||
		    (tgad[i].ag_GadgetKind == VGROUP_KIND) ||
		    (tgad[i].ag_GadgetKind == DGROUP_KIND))
		{
		    /* Skip the header */
		    i++;

		    /* Set the upper left coordinates */
		    GadgetBox (ri, &tgad[gstart].ag_LOSize, &tgad[gstart].ag_LOGSize, fwidth, fheight);

		    /* Clear the row/column array */
		    for (l = 0; l < MAX_ARRAY; l++)
		    {
			for (m = 0; m < CA_MAX; m++)
			{
			    carray[l][m] = 0;
			}
		    }

		    lsub = sub = -1;
		    cwidth = columns = 0;

		    /* Count the members */
		    for (k = l = cnt = i;
			 ((tgad[k].ag_GadgetKind != END_KIND) && ((tgad[k].ag_LOGroup == ogroup) || (tgad[k].ag_LOGroup == 0)));
			 k++, cnt++)
		    {
			if (tgad[k].ag_LOGroup)
			{
			    if ((tgad[k].ag_LOSub != lsub) && (tgad[k].ag_LOSub >= 0))
			    {
				if (sub >= 0)
				    cwidth += carray[sub][CA_MWIDTH] + INTERWIDTH + carray[sub][CA_GWIDTH];
				lsub = tgad[k].ag_LOSub;
				sub++;
				carray[sub][CA_START] = k;
				columns++;
			    }

			    /* Get the width of the label */
			    label = GetGadgetLabel (ri, tgad[k].ag_Label);
			    twidth = TextLength (rp, label, strlen (label));

			    if (tgad[k].ag_Label == MSG_ASL_FR_DRAWER_GAD)
				twidth += mspace * 3;

			    /* Get the width of the gadget, and any additional labels */
			    mwidth = gwidth = tgad[k].ag_LOSize.Width;
			    gheight = tgad[k].ag_LOSize.Height;

			    SetGadgetSize (ri, rp, k, mspace, fwidth, fheight, &twidth, &mwidth, &gwidth, &gheight);

			    tgad[k].ag_LOWork.Left = tgad[k].ag_LOSize.Left;
			    tgad[k].ag_LOWork.Top = tgad[k].ag_LOSize.Top;
			    tgad[k].ag_LOWork.Width = gwidth;
			    tgad[k].ag_LOWork.Height = gheight;
			    tgad[k].ag_LOGSize.Left = tgad[k].ag_LOGSize.Width = mwidth;

			    if (tgad[k].ag_LOSub < 0)
			    {
				/* Added the text width */
				mwidth += twidth + tgad[(k - 1)].ag_LOWork.Width + 4;
				if (twidth)
				    mwidth += 12;
			    }

			    carray[sub][CA_COUNT]++;
			    carray[sub][CA_MWIDTH] = MAX (carray[sub][CA_MWIDTH], twidth);
			    carray[sub][CA_GWIDTH] = MAX (carray[sub][CA_GWIDTH], mwidth);
			    carray[sub][CA_WIDTH] += mwidth;
			}
			carray[sub][CA_CNT]++;
		    }

		    if (sub >= 0)
		    {
			cwidth += carray[sub][CA_MWIDTH] + INTERWIDTH + carray[sub][CA_GWIDTH];
		    }
		}

		/* Handle the vertical group */
		if (tgad[gstart].ag_GadgetKind == VGROUP_KIND)
		{
		    BOOL neg = FALSE;
		    WORD ofs, lastw;

		    /* Compute the fill amount */
		    hfill = 2;

		    for (m = ofs = 0; m < columns; m++)
		    {
			/* Adjust the members */
			cnt = carray[m][CA_START] + carray[m][CA_CNT];
			for (k = carray[m][CA_START], lastw = 0; k < cnt; k++)
			{
			    if ((tgad[k].ag_LOGroup) && (tgad[k].ag_LOSub >= 0))
			    {
				lastw += tgad[k].ag_LOWork.Height + hfill;

				if (tgad[k].ag_LOWork.Width < 0)
				    neg = TRUE;
			    }
			}
			lastw -= hfill;
			ofs = MAX (ofs, lastw);
		    }

		    msize[n].Height += (ofs + 2);
		    top -= ofs + 2;
		    tgad[gstart].ag_LOGSize.Top = top;
		    tgad[gstart].ag_LOGSize.Height = ofs;

		    ofs = vfill = 0;
		    if (columns > 1)
		    {
			vfill = (tgad[gstart].ag_LOGSize.Width - cwidth) / (columns - 1);
			vfill = MAX (0, vfill);
			msize[n].Width = MAX (msize[n].Width, (cwidth + (INTERWIDTH * (columns - 1))));
		    }
		    else if (!neg)
		    {
			ofs = MAX (0, ((tgad[gstart].ag_LOGSize.Width - cwidth) / 2));
			msize[n].Width = MAX (msize[n].Width, cwidth);
		    }
		    else
		    {
			msize[n].Width = MAX (msize[n].Width, (carray[0][CA_MWIDTH] + INTERWIDTH + 24));
		    }

		    for (m = 0; m < columns; m++)
		    {
			/* Adjust the members */
			cnt = carray[m][CA_START] + carray[m][CA_CNT];
			for (k = carray[m][CA_START], member.Top = tgad[gstart].ag_LOGSize.Top;
			     k < cnt;
			     k++)
			{
			    if (tgad[k].ag_LOGroup)
			    {
				tgad[k].ag_LOWork.Left = tgad[gstart].ag_LOGSize.Left + ofs + ((tgad[k].ag_GadgetFlags & PLACETEXT_RIGHT) ? 0 : carray[m][CA_MWIDTH] + INTERWIDTH);
				tgad[k].ag_LOWork.Top = member.Top;

				if (tgad[k].ag_LOSub >= 0)
				{
				    if (tgad[k].ag_LOWork.Width < 0)
				    {
					tgad[k].ag_LOWork.Width = tgad[gstart].ag_LOGSize.Width - carray[m][CA_MWIDTH] - INTERWIDTH;
				    }
				    else if (tgad[k].ag_LOWork.Width == 0)
				    {
					tgad[k].ag_LOWork.Width = carray[m][CA_GWIDTH];
				    }

				    lastw = tgad[k].ag_LOWork.Width + 4 - 1;
				    member.Top += tgad[k].ag_LOWork.Height + hfill;
				}
				else
				{
				    /* Get the width of the label */
				    label = GetGadgetLabel (ri, tgad[k].ag_Label);
				    twidth = TextLength (rp, label, strlen (label));
				    if (twidth)
					twidth += 12;
				    tgad[k].ag_LOWork.Top -= tgad[(k - 1)].ag_LOWork.Height + hfill;
				    tgad[k].ag_LOWork.Left = tgad[(k - 1)].ag_LOWork.Left + lastw + twidth;
				    tgad[k].ag_LOWork.Width = carray[m][CA_GWIDTH] - lastw - twidth;
				    tgad[(k - 1)].ag_LOWork.Height = tgad[k].ag_LOWork.Height;
				}
			    }
			}

			tgad[gstart].ag_LOGSize.Width -= carray[m][CA_MWIDTH] + INTERWIDTH + carray[m][CA_GWIDTH] + vfill;
			ofs += carray[m][CA_MWIDTH] + INTERWIDTH + carray[m][CA_GWIDTH] + vfill;
		    }
		}
		/* Handle the horizontal group (ALWAYS buttons) */
		else if (tgad[gstart].ag_GadgetKind == HGROUP_KIND)
		{

		    /*
		     * Note that LOGSize.Left contains the minimum gadget width for buttons.
		     */
		    WORD need, pad, change;

		    msize[n].Height += tgad[i].ag_LOWork.Height + 4;
		    top -= tgad[i].ag_LOWork.Height + 2;
		    tgad[gstart].ag_LOGSize.Top = top;
		    tgad[gstart].ag_LOGSize.Height = tgad[i].ag_LOWork.Height;
		    top -= 2;

		    member.Width = carray[0][CA_GWIDTH];
		    mwidth = member.Width * carray[0][CA_COUNT];

		    /* Adjust the members */
		    for (k = carray[0][CA_START], cwidth = 0; k < cnt; k++)
		    {
			if (tgad[k].ag_LOGroup)
			{
			    cwidth += tgad[k].ag_LOGSize.Left;
			    tgad[k].ag_LOGSize.Width = member.Width;
			}
		    }

		    pad = (carray[0][CA_COUNT] - 1) * 4;
		    msize[n].Width = MAX (msize[n].Width, cwidth + pad);

		    if ((mwidth + pad) > tgad[gstart].ag_LOGSize.Width)
		    {
			change = need = tgad[gstart].ag_LOGSize.Width - mwidth - pad;

			while ((need < 0) && (change != 0))
			{
			    /* Adjust the members */
			    for (k = carray[0][CA_START], change = 0; (k < cnt) && (need < 0); k++)
			    {
				if (tgad[k].ag_LOGroup && (tgad[k].ag_LOGSize.Width > tgad[k].ag_LOGSize.Left))
				{
				    tgad[k].ag_LOGSize.Width--;
				    change++;
				    need++;
				}
			    }

			}
			mwidth = tgad[gstart].ag_LOGSize.Width - pad;
		    }

		    /* Compute the fill amount */
		    vfill = 0;
		    if (carray[0][CA_COUNT] > 1)
		    {
			vfill = (tgad[gstart].ag_LOGSize.Width - mwidth) / (carray[0][CA_COUNT] - 1);
			vfill = MAX (0, vfill);
		    }

		    /* Adjust the members */
		    for (k = carray[0][CA_START], member.Left = tgad[gstart].ag_LOGSize.Left; k < cnt; k++)
		    {
			if (tgad[k].ag_LOGroup)
			{
			    tgad[k].ag_LOWork.Left = member.Left;
			    tgad[k].ag_LOWork.Top = tgad[gstart].ag_LOGSize.Top;
			    tgad[k].ag_LOWork.Width = tgad[k].ag_LOGSize.Width;
			    tgad[k].ag_LOWork.Height = tgad[gstart].ag_LOGSize.Height;
			    member.Left += tgad[k].ag_LOWork.Width + vfill;
			}
		    }
		}
		/* Handle the diagonal group */
		else if (tgad[gstart].ag_GadgetKind == DGROUP_KIND)
		{
		    WORD lof, width;

		    msize[n].Height += 2;
		    tgad[gstart].ag_LOGSize.Top = 2;
		    tgad[gstart].ag_LOGSize.Height = top - 4;

		    /* Adjust the members */
		    for (k = carray[0][CA_START], lof = -1, twidth = cwidth = 0;
			 (k < cnt);
			 k++)
		    {
			if (tgad[k].ag_LOGroup)
			{
			    member.Left = tgad[k].ag_LOWork.Left;
			    member.Top = tgad[k].ag_LOWork.Top;
			    member.Width = tgad[k].ag_LOWork.Width;
			    member.Height = tgad[k].ag_LOWork.Height;

			    if (member.Width == 0)
				member.Width = tgad[gstart].ag_LOGSize.Width;
			    else if (member.Width < 0)
				member.Width = tgad[gstart].ag_LOGSize.Width + member.Width;

			    if (member.Height == 0)
				member.Height = tgad[gstart].ag_LOGSize.Height;
			    else if (member.Height < 0)
				member.Height = tgad[gstart].ag_LOGSize.Height + member.Height;

			    if (member.Left == 0)
			    {
				msize[n].Height += tgad[k].ag_MinHeight;
				member.Left = tgad[gstart].ag_LOGSize.Left;
			    }
			    else if (member.Left < 0)
				member.Left = tgad[gstart].ag_LOGSize.Left + tgad[gstart].ag_LOGSize.Width + member.Left;

			    if (member.Top == 0)
				member.Top = tgad[gstart].ag_LOGSize.Top;
			    else if (member.Top < 0)
				member.Top = tgad[gstart].ag_LOGSize.Top + tgad[gstart].ag_LOGSize.Height + member.Top;

			    if (member.Left > lof)
			    {
				lof = member.Left;
				cwidth += twidth;
			    }

			    width = (tgad[k].ag_LOSize.Width < 0) ? tgad[k].ag_MinWidth : member.Width;
			    twidth = MAX (twidth, width);

			    tgad[k].ag_LOWork.Left = member.Left;
			    tgad[k].ag_LOWork.Top = member.Top;
			    tgad[k].ag_LOWork.Width = member.Width;
			    tgad[k].ag_LOWork.Height = member.Height;
			}
		    }
		    cwidth += twidth;
		    msize[n].Width = MAX (msize[n].Width, cwidth);
		}
		/* Handle the no-group */
		else
		{
		    if (tgad[gstart].ag_LOSize.Height > 0)
		    {
			/* Set the upper left coordinates */
			GadgetBox (ri, &tgad[gstart].ag_LOSize, &tgad[gstart].ag_LOWork, fwidth, fheight);

			msize[n].Height += tgad[gstart].ag_LOWork.Height + 2;
			top -= tgad[gstart].ag_LOWork.Height + 2;
			tgad[gstart].ag_LOWork.Top = top;
		    }
		    else
		    {
			/* Set the upper left coordinates */
			GadgetBox (ri, &tgad[gstart].ag_LOSize, &tgad[gstart].ag_LOWork, fwidth, fheight);

			/* Get the width of the label */
			label = GetGadgetLabel (ri, tgad[gstart].ag_Label);
			twidth = TextLength (rp, label, strlen (label));

			/* Get the width of the gadget, and any additional labels */
			mwidth = gwidth = tgad[k].ag_LOSize.Width;
			gheight = tgad[k].ag_LOSize.Height;

			SetGadgetSize (ri, rp, gstart, mspace, fwidth, fheight, &twidth, &mwidth, &gwidth, &gheight);

			msize[n].Height += tgad[gstart].ag_MinHeight + 2;
			tgad[gstart].ag_LOWork.Top = 2;
			tgad[gstart].ag_LOWork.Height = top - 4;
		    }
		}

		if (top < 0)
		{
		    DB (kprintf ("failed: top < 0\n"));
		    failed = TRUE;
		}
	    }

	    /* Are we creating the gadgets yet? */
	    if (!failed)
	    {
		/* Is this gadget part of the whole? */
		if (tgad[i].ag_LOGroup)
		{
		    ng.ng_LeftEdge = tgad[i].ag_LOWork.Left + ri->ri_Screen->WBorLeft;
		    ng.ng_TopEdge = tgad[i].ag_LOWork.Top + ri->ri_Screen->BarHeight + 1;
		    ng.ng_Width = tgad[i].ag_LOWork.Width;
		    ng.ng_Height = tgad[i].ag_LOWork.Height;
		    ng.ng_TextAttr = ri->ri_TextAttr;
		    ng.ng_Flags = tgad[i].ag_GadgetFlags;
		    ng.ng_VisualInfo = ri->ri_VisualInfo;
		    ng.ng_GadgetID = tgad[i].ag_Command;
		    ng.ng_UserData = (APTR) ri;
		    ng.ng_GadgetText = NULL;
		    if (tgad[i].ag_Label != MSG_NOTHING)
			ng.ng_GadgetText = GetGadgetLabel (ri, tgad[i].ag_Label);

		    if (tgad[i].ag_GadgetKind == LISTVIEW_KIND)
		    {
			if (tag = FindTagItem (GTLV_ShowSelected, tgad[i].ag_CreationTags))
			{
			    tag->ti_Data = NULL;
			    if ((i > 0) && (
					       (tgad[(i - 1)].ag_GadgetKind == INTEGER_KIND) ||
					       (tgad[(i - 1)].ag_GadgetKind == STRING_KIND)))
			    {
				tag->ti_Data = (LONG) tgad[(i - 1)].ag_Gadget;
			    }
			}
		    }

		    if (tgad[i].ag_LOWork.Height < tgad[i].ag_MinHeight)
		    {
			if (n == 0)
			{
			    DB (kprintf ("failed: kind=%ld %ld < %ld\n",
				(LONG)tgad[i].ag_GadgetKind,
				(LONG)tgad[i].ag_LOWork.Height, (LONG)tgad[i].ag_MinHeight));
			    failed = TRUE;
			}
		    }
		    else if (ng.ng_LeftEdge + ng.ng_Width - 1 > ri->ri_Coords.Width)
		    {
			if (n == 0)
			{
			    DB (kprintf ("failed: kind=%ld left %ld + width %ld = %ld (%ld) : ",
				     (LONG)tgad[i].ag_GadgetKind,
				     (LONG)ng.ng_LeftEdge, (LONG)ng.ng_Width,
				     (LONG)(ng.ng_LeftEdge + ng.ng_Width - 1), (LONG)ri->ri_Coords.Width));
			    failed = TRUE;
			}
		    }
		    else if (ng.ng_LeftEdge + tgad[i].ag_MinWidth - 1 > ri->ri_Coords.Width)
		    {
			if (n == 0)
			{
			    DB (kprintf ("failed: kind=%ld left %ld + width %ld = %ld (%ld)\n",
				     (LONG)tgad[i].ag_GadgetKind,
				     (LONG)ng.ng_LeftEdge, (LONG)tgad[i].ag_MinWidth,
				     (LONG)(ng.ng_LeftEdge + tgad[i].ag_MinWidth - 1), (LONG)ri->ri_Coords.Width));
			    failed = TRUE;
			}
		    }

		    if (mode == LGM_CREATE)
		    {
#if 0
if (tgad[i].ag_GadgetKind == CHECKBOX_KIND)
{
    kprintf ("left %ld + width %ld = %ld (%ld) : ",
	     (LONG)ng.ng_LeftEdge, (LONG)ng.ng_Width,
	     (LONG)(ng.ng_LeftEdge + ng.ng_Width), (LONG)ri->ri_Coords.Width);

    kprintf ("left %ld + width %ld = %ld (%ld)\n",
	     (LONG)ng.ng_LeftEdge, (LONG)tgad[i].ag_MinWidth,
	     (LONG)(ng.ng_LeftEdge + tgad[i].ag_MinWidth), (LONG)ri->ri_Coords.Width);
}
#endif

			/* Create the gadget */
			ri->ri_LastAdded = CreateGadgetA (tgad[i].ag_GadgetKind, ri->ri_LastAdded, &ng, tgad[i].ag_CreationTags);
			tgad[i].ag_Gadget = ri->ri_LastAdded;

/*
if (tgad[i].ag_GadgetKind == CHECKBOX_KIND)
{
    kprintf("Gadget # = %ld\n",i);
    kprintf("LastAdded = %lx\n",ri->ri_LastAdded);
    kprintf("GadgetKind = %ld\n",tgad[i].ag_GadgetKind);
    kprintf("ng.LeftEdge = %ld\n",ng.ng_LeftEdge);
    kprintf("ng.TopEdge = %ld\n",ng.ng_TopEdge);
    kprintf("ng.Width = %ld\n",ng.ng_Width);
    kprintf("ng.Height = %ld\n",ng.ng_Height);
    kprintf("ng.TextAttr = %lx\n",ng.ng_TextAttr);
    kprintf("ng.TextAttr.ta_Name = %s\n",ng.ng_TextAttr->ta_Name);
    kprintf("ng.TextAttr.ta_YSize = %ld\n",ng.ng_TextAttr->ta_YSize);
    kprintf("ng.TextAttr.ta_Flags = %lx\n",ng.ng_TextAttr->ta_Flags);
    kprintf("ng.TextAttr.ta_Style = %lx\n",ng.ng_TextAttr->ta_Style);
    kprintf("ng.Flags = %lx\n",ng.ng_Flags);
    kprintf("ng.VisualInfo = %lx\n",ng.ng_VisualInfo);
    kprintf("ng.GadgetText = %s\n\n",ng.ng_GadgetText);

    {
    struct TagItem *tag;
    struct TagItem *tags = tgad[i].ag_CreationTags;

        while (tag = NextTagItem(&tags))
        {
            kprintf("tag = %lx, data = %lx\n",tag->ti_Tag, tag->ti_Data);
        }
    }
    kprintf("\n=======================\n");
}
*/
		    }
		}
	    }
	}

	if (!ri->ri_LastAdded)
	{
	    AslFreeGadgets (ri);
	}
	else if (failed)
	{
	    AslFreeGadgets (ri);
	    ri->ri_LastAdded = CreateContext (&ri->ri_Gadgets);
	}
	else if (mode == LGM_CREATE)
	{
	    /* Make sure we fall out when done */
	    n = MAX_LEVELS;
	}
    }

    ri->ri_Coords.Width = pwidth;
    ri->ri_Coords.Height = pheight;

    if (mode == LGM_SETMIN)
    {
	WORD mw, mh;

	mw = mh = 8096;
	for (n = i = 0; n < MAX_LEVELS; n++)
	{
	    if ((msize[n].Width < mw) && (msize[n].Height < mh))
	    {
		mw = msize[n].Width;
		mh = msize[n].Height;
		i = n;
	    }
	}

	msize[i].Width += 8;
	msize[i].Height += 4;

	ri->ri_MinWidth = msize[i].Width + 16;
	ri->ri_MinHeight = msize[i].Height + ri->ri_Screen->BarHeight + 19;

	if (ri->ri_Coords.Width < ri->ri_MinWidth)
	    ri->ri_Coords.Width = ri->ri_MinWidth;

	if (ri->ri_Coords.Height < ri->ri_MinHeight)
	    ri->ri_Coords.Height = ri->ri_MinHeight;
    }
    else if (failed)
    {
	FreeLayoutGadgets (ri, FALSE);
    }

    for (i = 0; ri->ri_Template[i].ag_GadgetKind != END_KIND; i++)
    {
	FreeTagItems (ri->ri_Template[i].ag_CreationTags);
	ri->ri_Template[i].ag_CreationTags = NULL;
    }

    return (ri->ri_LastAdded != NULL);
}


/*****************************************************************************/


VOID CalculatePaneSize (struct ReqInfo * ri)
{
    struct Window *window;

    if (window = ri->ri_Window)
    {
	ri->ri_Coords.Left = window->LeftEdge;
	ri->ri_Coords.Top = window->TopEdge;
	ri->ri_Coords.Width = window->Width - window->BorderLeft - window->BorderRight;
	ri->ri_Coords.Height = window->Height - window->BorderTop - window->BorderBottom;
    }
}


/*****************************************************************************/


VOID SetGadgetAttr (struct ReqInfo * ri, WORD id, ULONG tags,...)
{

    if (ri->ri_Template[id].ag_Gadget)
    {
	GT_SetGadgetAttrsA (ri->ri_Template[id].ag_Gadget, ri->ri_Window, NULL, (struct TagItem *) & tags);
    }
    else if (!ri->ri_Template[id].ag_CreationTags && ri->ri_Template[id].ag_LOGroup)
    {
	ri->ri_Template[id].ag_CreationTags = CloneTagItems ((struct TagItem *) & tags);
    }
}
