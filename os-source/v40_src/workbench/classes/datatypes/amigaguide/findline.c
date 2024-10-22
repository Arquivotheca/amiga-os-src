/* findline.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/*****************************************************************************/

struct Line *FindLine (struct AGLib * ag, Class * cl, Object * o, struct gpInput * msg, struct RastPort * rp, WORD mode, struct IBox * box, LONG status)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    struct Line *line = NULL;
    struct TextExtent te;
    BOOL update = FALSE;
    BOOL good = FALSE;
    LONG sc, ec;
    LONG i, k;

    /* Clear work variables */
    cd->cd_Word = NULL;
    cd->cd_WordBuffer[0] = 0;

    /* Find the line that was hit */
    cd->cd_WY = i = si->si_TopVert + ((msg->gpi_Mouse.Y - (cd->cd_Top - box->Top)) / si->si_VertUnit);

    /* Was it beyond the last line */
    if (i >= si->si_TotVert)
	return (NULL);

    /* Skip past the lines that it can't be on */
    line = (struct Line *) cd->cd_LineList.mlh_Head;
    while (i && line->ln_Link.mln_Succ)
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
#if 1
	    /* Did we hit a segment? */
	    if (((LONG)msg->gpi_Mouse.X >= (LONG) (line->ln_Box.Left - si->si_TopHoriz)) &&
		((LONG)msg->gpi_Mouse.X <= (LONG) (line->ln_Box.Left - si->si_TopHoriz + line->ln_Box.Width - 1)))
	    {
		good = TRUE;
		k = 0;
	    }
#else
	    if ((LONG)msg->gpi_Mouse.X < (LONG) (line->ln_Box.Left - si->si_TopHoriz))
	    {
		if (msg->gpi_Mouse.X > 0)
		    msg->gpi_Mouse.X = line->ln_Box.Left - si->si_TopHoriz;
		good = TRUE;
		k = 0;
	    }
	    /* Did we hit a segment? */
	    else if (((LONG)msg->gpi_Mouse.X >= (LONG) (line->ln_Box.Left - si->si_TopHoriz)) &&
		     ((LONG)msg->gpi_Mouse.X <= (LONG) (line->ln_Box.Left - si->si_TopHoriz + line->ln_Box.Width - 1)))
	    {
		good = TRUE;
		k = 0;
	    }
#endif
	    else if (line->ln_Flags & LNF_LF)
	    {
		if (mode == 0)
		{
		    line = NULL;
		}
		else
		{
		    if ((line->ln_TextLen == 0) && !(((struct Line *) line->ln_Link.mln_Pred)->ln_Flags & LNF_LF))
			line = (struct Line *) line->ln_Link.mln_Pred;
		    msg->gpi_Mouse.X = line->ln_Box.Left - si->si_TopHoriz + line->ln_Box.Width;
		    cd->cd_Word = (ULONG) & line->ln_Text[line->ln_TextLen];
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

    /* Disable the selection stuff for now */
    if ((mode != 0) && ((line == NULL) || !(line->ln_Flags & LNF_LINK)))
    {
	good = FALSE;
	line = NULL;
    }

    if (good)
    {
	/* Find the character that was hit */
	i = TextFit (rp, line->ln_Text, line->ln_TextLen, &te, NULL, 1,
		     (msg->gpi_Mouse.X - (line->ln_Box.Left - si->si_TopHoriz)),	/* Width */
		     si->si_VertUnit);	/* Height */

	/* Searching for a word? */
	if ((mode == 0) && !(line->ln_Flags & LNF_LINK))
	{
	    /* Find the beginning of the word */
	    sc = i;
	    do
	    {
		if (stpchr (cd->cd_WordDelim, line->ln_Text[sc]))
		    break;
		else
		    sc--;
	    } while (sc >= 0);
	    sc++;

	    /* Find the end of the word */
	    ec = i;
	    do
	    {
		if (stpchr (cd->cd_WordDelim, line->ln_Text[ec]))
		    break;
		else
		    ec++;
	    } while (ec < line->ln_TextLen);

	    /* Have to have a word */
	    if (sc < ec)
	    {
		/* Copy the word into our buffer */
		strncpy (cd->cd_WordBuffer, &line->ln_Text[sc], ec - sc);
		cd->cd_WordBuffer[ec - sc] = 0;

		/* Set up the anchor and end points */
		cd->cd_Word = cd->cd_AWord = (ULONG) & line->ln_Text[sc];
		cd->cd_EWord = (ULONG) & line->ln_Text[ec];
		cd->cd_Select.Top = cd->cd_Select.Height = cd->cd_WY;
		cd->cd_Select.Left = cd->cd_WX = line->ln_Box.Left + TextLength (rp, line->ln_Text, sc);
		cd->cd_Select.Width = cd->cd_Select.Left + TextLength (rp, cd->cd_WordBuffer, strlen (cd->cd_WordBuffer));
	    }
	    else
	    {
		line = NULL;
	    }
	}
	else
	{
	    cd->cd_WX = line->ln_Box.Left + te.te_Width;
	    cd->cd_Word = (ULONG) & line->ln_Text[i];
	    msg->gpi_Mouse.X = cd->cd_WX - si->si_TopHoriz;

	    if (status == DBS_MOVE)
	    {
		if ((cd->cd_OY != cd->cd_WY) || (cd->cd_OX != cd->cd_WX))
		{
		    /* erase old */
#if 0
		    DrawBox (ag, cl, o, rp, box, &cd->cd_Select, DBS_MOVE);
#endif
		    update = TRUE;
		}
	    }

	    cd->cd_EWord = cd->cd_Word;
	    cd->cd_Select.Height = cd->cd_WY;
	    cd->cd_Select.Width = cd->cd_WX + TextLength (rp, (STRPTR) cd->cd_Word, 1);
	    if (status == DBS_DOWN)
	    {
		cd->cd_AWord = cd->cd_Word;
		cd->cd_Select.Top = cd->cd_WY;
		cd->cd_Select.Left = cd->cd_WX;
		update = TRUE;
	    }

	    if (update)
	    {
		/* draw new */
#if 0
		DrawBox (ag, cl, o, rp, box, &cd->cd_Select, DBS_MOVE);
#endif
	    }
	}
    }

    return (line);
}
