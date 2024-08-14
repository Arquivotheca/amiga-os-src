#include "appobjectsbase.h"
#include "mtextgclass.h"
#include <clib/macros.h>
#include <string.h>

void kprintf(void *, ...);
#define	DB(x)	;

#define	G(o)	((struct Gadget *)(o))

/* find the point at which the line wraps */
WORD TextWrap (UBYTE * s, struct TextFont * f, WORD pixels, WORD * flag)
{
    WORD c, d;			/* character to testq */
    WORD length = 0;		/* length of line */
    WORD line_pixels = 0;	/* pixel width of line */
    UBYTE *start = s;		/* save initial pointer */

    /* font size tables */
    WORD *space = (WORD *) (f->tf_CharSpace);
    WORD *kern = (WORD *) (f->tf_CharKern);

    /* mark line as not end line */
    *flag = FALSE;

    /* The following conditional tests are in carefully tweaked order */
    for (;;)
    {
	/* get next character */
	c = d = *s++;

	/* if prop font */
	if (f->tf_Flags & FPF_PROPORTIONAL)
	{
	    /* NULL counts as space for size */
	    if (d == '\0')
	    {
		d = ' ';
	    }

	    /* clip character number */
	    if (d > f->tf_HiChar || d < f->tf_LoChar)
	    {
		d = f->tf_HiChar;
	    }

	    /* offset to start */
	    d -= f->tf_LoChar;

	    /* size of character */
	    line_pixels += space[d] + kern[d];
	}
	else
	{
	    /* character position */
	    line_pixels += f->tf_XSize;
	}

	/* if line big enough, break it */
	if (line_pixels >= pixels)
	{
	    /* if no spaces were encountered */
	    if (length == 0)
	    {
		/* then just return this character */
		return (s - start - 1);
	    }

	    /* else return the actual length */
	    return (length);
	}

	/* if it's a carriage return */
	if (c == RETURN_KEY)
	{
	    /* then just print as is */
	    return (s - start);
	}

	/* if it's a NULL character */
	if (c == '\0')
	{
	    /* then we reached the end */
	    *flag = TRUE;

	    /* then just print as is */
	    return (s - start);
	}

	/* insert test for word-wrap mode... */
	if (c == ' ')
	{
	    /* mark last wrap position */
	    length = s - start;
	}
    }
}

/* locate the current cursor index based on an xy value */
WORD xyfind (WORD cx, WORD cy, struct Gadget * g)
{
    struct TextInfo *ti = (struct TextInfo *) g->SpecialInfo;
    struct StringExtend *se = ti->Extension;
    struct IBox *box = &(ti->Domain);
    struct TextFont *f = se->Font;	/* text font to use */
    UBYTE *s = ti->Buffer;	/* string to search				 */
    WORD cpos = 0;		/* cursor position				 */
    WORD *space = (WORD *) (f->tf_CharSpace);	/* space table */
    WORD *kern = (WORD *) (f->tf_CharKern);	/* kern table */
    WORD c, d;			/* character to test */
    UWORD length = 0;		/* length of wrapped line */
    UWORD line_pixels = 0;	/* pixel width of line */
    UWORD csize;		/* character size */
    WORD endflag;		/* indicated end of string */

    /* Search until correct line */
    while (cy--)
    {
	/* Find length of line */
	length = TextWrap (s, f, box->Width, &endflag);

	/* See if end of string */
	if (endflag)
	{
	    cpos += (length - 1);
	    return cpos;
	}

	/* Increment string to next line */
	s += length;

	/* Bump the index */
	cpos += length;
    }

    /* get length of current line */
    length = TextWrap (s, f, box->Width, &endflag) - 1;

    /* default character size */
    csize = f->tf_XSize;

    /* get each character in line */
    while ((c = d = *s++) && length > 0)
    {
	/* characters remaining */
	length--;

	/* figure out char width */
	if (f->tf_Flags & FPF_PROPORTIONAL)
	{
	    if (c > f->tf_HiChar || c < f->tf_LoChar)
		c = f->tf_HiChar;
	    d -= f->tf_LoChar;
	    csize = space[d] + kern[d];
	}

	/* add to pixel width */
	line_pixels += csize;

	/* if found x position, break (added = sign) */
	if (line_pixels >= cx)
	    break;

	/* add to character position */
	cpos++;
    }

    return cpos;
}

ULONG renderMTextG (Class * cl, Object * o, struct gpRender * gpr)
{
    struct RastPort *rp;
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;
    struct Gadget *g = G (o);
    struct TextInfo *ti = INST_DATA (cl, o);
    struct StringExtend *se = ti->Extension;
    WORD displines;

    DB (kprintf("renderMTextG start 0x%lx\n", renderMTextG));

    /* Figure out the number of characters */
    ti->NumChars = strlen (ti->Buffer);

    /* Compute number of lines in the display */
    displines = ti->Domain.Height / se->Font->tf_YSize;

    /* Count the lines. This is slow. */
    {
	WORD maxlines = 0, index = 0, length;
	UBYTE *s = ti->Buffer;
	WORD endflag = 0;

	/* Count the number of lines */
	while (endflag == 0)
	{
	    if (index > ti->NumChars)
	    {
		/* Found end */
		length = 0;
		endflag = 1;
	    }
	    else
	    {
		/* Get length of line to display */
		length = TextWrap (s, se->Font, ti->Domain.Width, &endflag);
	    }

	    /* Increment counters */
	    s += length;
	    index += length;
	    maxlines++;
	}

	/* Does the gadget think the same thing? */
	if ((maxlines != ti->LinesInBuffer) ||
	    (displines != ti->LinesInDisplay))
	{
	    /* Adjust it */
	    ti->LinesInBuffer = maxlines;
	    ti->LinesInDisplay = displines;

	    /* Announce the change to whole world */
	    notifyAttrChanges (o, gpr->gpr_GInfo, 0,
			       GA_ID, g->GadgetID,
			       CGTA_Visible, (LONG) ti->LinesInDisplay,
			       CGTA_Total, (LONG) maxlines,
			       CGTA_Top, (LONG) ti->ScrollPosition,
			       TAG_END);
	}
	else
	{
	    /* Tell the current top position */
	    notifyAttrChanges (o, gpr->gpr_GInfo, OPUF_INTERIM,
			       GA_ID, g->GadgetID,
			       CGTA_Top, (LONG) ti->ScrollPosition,
			       TAG_END);
	}
    }

    /* Get the rastport that we're going to write into */
    if (rp = ObtainGIRPort (gpr->gpr_GInfo))
    {
	UBYTE *s = ti->Buffer;
	WORD y = ti->Domain.Top;
	WORD index = 0;
	WORD lastindex;
	WORD line = 0;
	UWORD visible = FALSE;
	UBYTE textpen = se->Pens[0];
	UBYTE backpen = se->Pens[1];
	WORD return_key = 0;
	WORD length;
	WORD endflag = 0;

	if (g->Flags & SELECTED)
	{
	    textpen = se->ActivePens[0];
	    backpen = se->ActivePens[1];
	}

	/* Get ready to draw the text */
	SetFont (rp, se->Font);
	SetDrMd (rp, JAM2);
	SetAPen (rp, ti->CursorPens[0]);
	SetBPen (rp, ti->CursorPens[1]);

	for (;;)
	{

	    if (index > ti->NumChars)
	    {
		/* Found end */
		length = 0;
		endflag = FALSE;
		return_key = 0;
	    }
	    else
	    {
		/* Get length of line to display */
		length = TextWrap (s, se->Font, ti->Domain.Width, &endflag);
		return_key = (s[length - 1]== RETURN_KEY) ? 1 : 0;
	    }

	    /* Render a single line */
	    if (line >= ti->ScrollPosition)
	    {
		WORD p1, p2;

		if (!visible)
		{
		    ti->FirstVisible = index;
		    visible = TRUE;
		}

		if (g->Flags & SELECTED)
		{
		    if ((ti->Flags & TXT_HIGHLIGHT) &&
			(ti->BufferPos != ti->AnchorPos))
		    {
			if (ti->BufferPos > ti->AnchorPos)
			{
			    p1 = ti->AnchorPos;
			    p2 = ti->BufferPos;
			}
			else
			{
			    p1 = ti->BufferPos;
			    p2 = ti->AnchorPos;
			}
		    }
		    else
		    {
			p1 = p2 = ti->BufferPos;
		    }
		}
		else
		{
		    p1 = p2 = -1;
		}

		lastindex = index + length - return_key;

		if (endflag)
		{
		    lastindex--;
		}

		Move (rp, ti->Domain.Left, y + rp->TxBaseline);

		/* handle the cursor case first. */
		if ((p1 == p2) &&
		    (p1 >= index) &&
		    (p1 < (index + length)))	/* case 4a */
		{
		    WORD old_x;

		    /* Normal colors */
		    SetDrMd (rp, JAM2);
		    SetAPen (rp, textpen);
		    SetBPen (rp, backpen);

		    /* Draw first part of line */
		    Text (rp, (char *)s, (p1 - index));

		    /* Cursor colors */
		    SetAPen (rp, ti->CursorPens[0]);
		    SetBPen (rp, ti->CursorPens[1]);

		    /* Save cursor position */
		    old_x = rp->cp_x;

		    /* Draw the cursor */
		    if (ti->BufferPos == ti->NumChars)
			Text (rp, " ", 1);
		    else
			Text (rp, (char *) s + (p1 - index), 1L);

		    /* Calculate new cursor pixel position */
		    mt->tgWork.CurX = (rp->cp_x + old_x) / 2 - ti->Domain.Left;
		    mt->tgWork.CurY = line;

		    if (p1 + 1 < lastindex)	/* if any chars left			 */
		    {
			/* Normal colors */
			SetDrMd (rp, JAM2);
			SetAPen (rp, textpen);
			SetBPen (rp, backpen);

			/* draw rest of line */
			Text (rp, (char *)s + p1 + 1 - index, lastindex - p1 - 1);
		    }

		    /* For erasing... */
		    SetAPen (rp, backpen);
		}
		else if (p2 <= index || p1 >= lastindex)	/* cases 1 and 6		 */
		{
		    SetDrMd (rp, JAM2);
		    SetAPen (rp, textpen);
		    SetBPen (rp, backpen);
		    Text (rp, (char *) s, lastindex - index);
		    SetAPen (rp, backpen);
		}
		else if (p1 >= index && p2 < lastindex)	/* case 4				 */
		{
		    SetDrMd (rp, JAM2);
		    SetAPen (rp, textpen);
		    SetBPen (rp, backpen);
		    Text (rp, (char *) s, p1 - index);
		    if (p1 == ti->BufferPos)
		    {
			mt->tgWork.CurX = rp->cp_x - ti->Domain.Left;
			mt->tgWork.CurY = line;
		    }

		    SetAPen (rp, ti->HighPens[0]);
		    SetBPen (rp, ti->HighPens[1]);
		    Text (rp, (char *) s + (p1 - index), p2 - p1);
		    if (p2 == ti->BufferPos)
		    {
			mt->tgWork.CurX = rp->cp_x - ti->Domain.Left;
			mt->tgWork.CurY = line;
		    }

		    SetDrMd (rp, JAM2);
		    SetAPen (rp, textpen);
		    SetBPen (rp, backpen);
		    Text (rp, (char *) s + (p2 - index), lastindex - p2);

		    SetAPen (rp, backpen);
		}
		else if (p1 <= index)	/* case 2 and 3				 */
		{
		    SetAPen (rp, ti->HighPens[0]);
		    SetBPen (rp, ti->HighPens[1]);

		    if (p2 < lastindex)	/* case 2					 */
		    {
			Text (rp, (char *) s, p2 - index);
			if (p2 == ti->BufferPos)
			{
			    mt->tgWork.CurX = rp->cp_x - ti->Domain.Left;
			    mt->tgWork.CurY = line;
			}

			SetAPen (rp, textpen);
			SetBPen (rp, backpen);
			Text (rp, (char *) s + (p2 - index), lastindex - p2);

			SetAPen (rp, backpen);
		    }
		    else
			/* case 3					 */
		    {
			Text (rp, (char *) s, lastindex - index);
			SetAPen (rp, ti->HighPens[1]);
		    }
		}
		else if (p2 >= index)	/* case 5					 */
		{
		    SetDrMd (rp, JAM2);
		    SetAPen (rp, textpen);
		    SetBPen (rp, backpen);
		    Text (rp, (char *) s, p1 - index);

		    if (p1 == ti->BufferPos)
		    {
			mt->tgWork.CurX = rp->cp_x - ti->Domain.Left;
			mt->tgWork.CurY = line;
		    }

		    SetAPen (rp, ti->HighPens[0]);
		    SetBPen (rp, ti->HighPens[1]);
		    Text (rp, (char *) s + (p1 - index), lastindex - p1);
		    SetAPen (rp, ti->HighPens[1]);
		}

		SetDrMd (rp, JAM2);
		if (endflag)
		{
		    SetAPen (rp, backpen);
		}

		RectFill (rp,
			  (rp->cp_x), y,
			  (ti->Domain.Left + ti->Domain.Width - 1),
			  (y + rp->TxHeight - 1));

		y += rp->TxHeight;

		ti->LastVisible = index + length;

		if (y + rp->TxHeight > ti->Domain.Top + ti->Domain.Height)
		{
		    WORD bx = (ti->Domain.Left + ti->Domain.Width - 1);
		    WORD by = (ti->Domain.Top + ti->Domain.Height);

		    if (by > y)
		    {
			RectFill (rp, ti->Domain.Left, y, bx, by);
		    }

		    break;
		}
	    }

	    /* Increment counters */
	    s += length;
	    index += length;
	    line++;
	}

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    DB (kprintf("renderMTextG exit\n"));

    return (1L);
}
