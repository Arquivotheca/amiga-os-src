/* findline.c
 *
 */

#include "textbase.h"

#define	DR(x)	;

/*****************************************************************************/

struct Line *FindLine (struct TextLib * txl, Class * cl, Object * o, struct gpInput * msg, struct RastPort * rp, WORD mode, struct IBox *box, LONG status)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct Line *line = NULL;
    struct TextExtent te;
    BOOL update = FALSE;
    BOOL inside = FALSE;
    BOOL good = FALSE;
    LONG sc, ec;
    LONG i, k;

    if ((msg->gpi_Mouse.X >= 0) && (msg->gpi_Mouse.X < box->Width) &&
	(msg->gpi_Mouse.Y >= 0) && (msg->gpi_Mouse.Y < box->Height))
	inside = TRUE;

    if ((msg->gpi_Mouse.X < 0) && (si->si_TopHoriz == 0))
    {
	msg->gpi_Mouse.X = 0;
	inside = TRUE;
    }
    if ((msg->gpi_Mouse.Y < 0) && (si->si_TopVert == 0))
    {
	msg->gpi_Mouse.X = msg->gpi_Mouse.Y = 0;
	inside = TRUE;
    }

    if ((msg->gpi_Mouse.X >= box->Width) && (si->si_TopHoriz >= si->si_TotHoriz - si->si_VisHoriz))
	inside = TRUE;
    if ((msg->gpi_Mouse.Y >= box->Height) && (si->si_TopVert >= si->si_TotVert - si->si_VisVert))
    {
	msg->gpi_Mouse.X = box->Width;
	inside = TRUE;
    }

    lod->lod_Word = NULL;
    lod->lod_WordBuffer[0] = 0;

    line = (struct Line *) lod->lod_LineList.mlh_Head;
    lod->lod_WY = i = si->si_TopVert + (msg->gpi_Mouse.Y / si->si_VertUnit);
    while (line->ln_Link.mln_Succ && i)
    {
	if (line->ln_Flags & LNF_LF)
	    i--;
	line = (struct Line *) line->ln_Link.mln_Succ;
    }

    /* Did we find the line that they selected? */
    if (i == 0)
    {
	k = 1;
	while (line && line->ln_Link.mln_Succ && k)
	{
	    if (msg->gpi_Mouse.X < (LONG)line->ln_XOffset - si->si_TopHoriz)
	    {
		if (msg->gpi_Mouse.X > 0)
		    msg->gpi_Mouse.X = line->ln_XOffset - si->si_TopHoriz;
		good = TRUE;
		k = 0;
	    }
	    /* Did we hit a segment? */
	    else if ((msg->gpi_Mouse.X >= (LONG)line->ln_XOffset - si->si_TopHoriz) &&
		     (msg->gpi_Mouse.X <= (LONG)(line->ln_XOffset - si->si_TopHoriz + line->ln_Width - 1)))
	    {
		good = TRUE;
		k = 0;
	    }
	    else if (line->ln_Flags & LNF_LF)
	    {
		if (mode == 0)
		{
		    line = NULL;
		}
		else
		{
		    if ((line->ln_TextLen == 0) && !(((struct Line *)line->ln_Link.mln_Pred)->ln_Flags & LNF_LF))
			line = (struct Line *) line->ln_Link.mln_Pred;
		    msg->gpi_Mouse.X = line->ln_XOffset - si->si_TopHoriz + line->ln_Width;
		    lod->lod_Word = (ULONG)&line->ln_Text[line->ln_TextLen];
		    good = TRUE;
		}

		/* Show that we are done */
		k = 0;
	    }
	    else
	    {
		line = (struct Line *) line->ln_Link.mln_Succ;
	    }
	}
    }

    if (good)
    {
	/* Find the character that was hit */
	DR (kprintf ("lx=%ld sx=%ld mx=%ld\n",
		     (LONG)line->ln_XOffset, si->si_TopHoriz, (LONG)msg->gpi_Mouse.X));

	i = TextFit (rp, line->ln_Text, line->ln_TextLen, &te, NULL, 1,
		     (msg->gpi_Mouse.X - (line->ln_XOffset - si->si_TopHoriz)),	/* Width */
		     si->si_VertUnit);						/* Height */

	/* Searching for a word? */
	if (mode == 0)
	{
	    /* Find the beginning of the word */
	    sc = i;
	    do
	    {
		if (stpchr (lod->lod_WordDelim, line->ln_Text[sc]))
		    break;
		else
		    sc--;
	    } while (sc >= 0);
	    sc++;

	    /* Find the end of the word */
	    ec = i;
	    do
	    {
		if (stpchr (lod->lod_WordDelim, line->ln_Text[ec]))
		    break;
		else
		    ec++;
	    } while (ec < line->ln_TextLen);

	    /* Have to have a word */
	    if (sc < ec)
	    {
		/* Copy the word into our buffer */
		strncpy (lod->lod_WordBuffer, &line->ln_Text[sc], ec - sc);
		lod->lod_WordBuffer[ec - sc] = 0;

		/* Set up the anchor and end points */
		lod->lod_Word  = lod->lod_AWord = (ULONG)&line->ln_Text[sc];
		lod->lod_EWord = (ULONG)&line->ln_Text[ec];
		lod->lod_Select.Top    = lod->lod_Select.Height = lod->lod_WY;
		lod->lod_Select.Left    = lod->lod_WX = line->ln_XOffset + TextLength (rp, line->ln_Text, sc);
		lod->lod_Select.Width    = lod->lod_Select.Left + TextLength (rp, lod->lod_WordBuffer, strlen (lod->lod_WordBuffer));
	    }
	    else
	    {
		line = NULL;
	    }
	}
	else
	{
	    lod->lod_WX      = line->ln_XOffset + te.te_Width;
	    lod->lod_Word    = (ULONG)&line->ln_Text[i];
	    msg->gpi_Mouse.X = lod->lod_WX - si->si_TopHoriz;

	    if (status == DBS_MOVE)
	    {
		if (((lod->lod_OY != lod->lod_WY) || (lod->lod_OX != lod->lod_WX)) && inside)
		{
		    /* erase old */
		    DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_MOVE);
		    update = TRUE;
		}
	    }

	    lod->lod_EWord   = lod->lod_Word;
	    lod->lod_Select.Height      = lod->lod_WY;
	    lod->lod_Select.Width      = lod->lod_WX + TextLength (rp, (STRPTR) lod->lod_Word, 1);
	    if (status == DBS_DOWN)
	    {
		lod->lod_AWord   = lod->lod_Word;
		lod->lod_Select.Top      = lod->lod_WY;
		lod->lod_Select.Left      = lod->lod_WX;
		update = TRUE;
	    }

	    if (update)
	    {
		/* draw new */
		DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_MOVE);
	    }
	}
    }
    return (line);
}
