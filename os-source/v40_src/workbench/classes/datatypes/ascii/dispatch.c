/* dispatch.c
 *
 */

#include "asciibase.h"

/*****************************************************************************/

#define	G(o)	((struct Gadget *)(o))

/*****************************************************************************/

void NewList (struct List *);

/*****************************************************************************/

struct localData
{
    VOID	*lod_Pool;
    ULONG	 lod_Flags;
    LONG	 lod_MWidth;

    /* ANSI Decoding */
    BOOL	 lod_InSequence;
    BYTE	 lod_CharsInBuffer;
    BYTE	 lod_NumGoodChars;
    UBYTE	 lod_SaveBuffer[30];
    BYTE	 lod_ScanStep;
    SHORT	 lod_WhichANSI;
};

/*****************************************************************************/

#define	LDF_EUC		(1L<<0)

/*****************************************************************************/

static ULONG setdtattrs (struct ASCIILib * ascii, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

static ULONG getdtattrs (struct ASCIILib * ascii, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

static ULONG notifyAttrChanges (struct ASCIILib * ascii, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

#define	ATTR_UNDERLINE	1
#define	ATTR_HIGHLIGHT	2
#define ATTR_BLINK	4
#define	ATTR_INVERSE	8

/*****************************************************************************/

#define	CSI	0x9B

/* External comparison routines. */
extern BYTE __stdargs Num1 (WORD Char);
extern BYTE __stdargs Num2 (WORD Char);
extern BYTE __stdargs Num3 (WORD Char);
extern BYTE __stdargs Num4 (WORD Char);

 /* This structure describes an ANSI control sequence. */
struct ControlCode
{
    UBYTE FirstChar;
    BYTE (*__stdargs Match) (WORD Char);
    UBYTE LastChar;

    BYTE ExactSize;
    WORD Func;
};

/*****************************************************************************/

/* At present 55 different control sequences are supported/trapped. */
#define NUM_CODES 55

/* This follows the control code information. */
static struct ControlCode ANSICode[] =
{
 /* Single Character Sequences. */

    'D', NULL, 0, 1, 0,		/* CursorScrollDown, */
    'M', NULL, 0, 1, 0,		/* CursorScrollUp, */
    'E', NULL, 0, 1, 0,		/* NextLine, */
    '7', NULL, 0, 1, 0,		/* SaveCursor, */
    '8', NULL, 0, 1, 0,		/* LoadCursor, */
    '=', NULL, 0, 1, 0,		/* NumericAppMode, */
    '>', NULL, 0, 1, 0,		/* NumericAppMode, */
    'N', NULL, 0, 1, 0,		/* Ignore, */
    'O', NULL, 0, 1, 0,		/* Ignore, */
    'H', NULL, 0, 1, 0,		/* SetTab, */
    'Z', NULL, 0, 1, 0,		/* RequestTerminal, */
    'c', NULL, 0, 1, 0,		/* Reset, */
    '<', NULL, 0, 1, 0,		/* Ignore, */
    '~', NULL, 0, 1, 0,		/* Ignore, */
    'n', NULL, 0, 1, 0,		/* Ignore, */
    '}', NULL, 0, 1, 0,		/* Ignore, */
    'o', NULL, 0, 1, 0,		/* Ignore, */
    '|', NULL, 0, 1, 0,		/* Ignore, */

 /* Double Character Sequences. */

    '[', NULL, 's', 2, 0,	/* SaveCursor, */
    '[', NULL, 'u', 2, 0,	/* LoadCursor, */
    '(', NULL, 'A', 2, 0,	/* FontStuff, */
    '(', NULL, 'B', 2, 0,	/* FontStuff, */
    '(', NULL, '0', 2, 0,	/* FontStuff, */
    ')', NULL, 'A', 2, 0,	/* FontStuff, */
    ')', NULL, 'B', 2, 0,	/* FontStuff, */
    ')', NULL, '0', 2, 0,	/* FontStuff, */
    '#', NULL, '3', 2, 0,	/* ScaleFont, */
    '#', NULL, '4', 2, 0,	/* ScaleFont, */
    '#', NULL, '5', 2, 0,	/* ScaleFont, */
    '#', NULL, '6', 2, 0,	/* ScaleFont, */
    '#', NULL, '8', 2, 0,	/* Ignore, */
    ' ', NULL, 'F', 2, 0,	/* Ignore, */
    ' ', NULL, 'G', 2, 0,	/* Ignore, */

 /* Multiple Character Sequence. */

    '[', Num3, 'i', 0, 0,	/* Ignore, */
    '[', Num3, 'n', 0, 0,	/* RequestInformation, */
    '[', Num3, 'c', 0, 0,	/* RequestTerminal, */
    '[', Num3, 'h', 0, 0,	/* SetSomething, */
    '[', Num3, 'l', 0, 0,	/* SetSomething, */

    '[', Num4, 'h', 0, 0,	/* Ignore, */

    '[', Num1, 'A', 0, 0,	/* MoveCursor, */
    '[', Num1, 'B', 0, 0,	/* MoveCursor, */
    '[', Num1, 'C', 0, 0,	/* MoveCursor, */
    '[', Num1, 'D', 0, 0,	/* MoveCursor, */
    '[', Num1, 'K', 0, 0,	/* EraseLine, */
    '[', Num1, 'J', 0, 0,	/* EraseScreen, */
    '[', Num1, 'P', 0, 0,	/* EraseCharacters, */
    '[', Num1, 'L', 0, 0,	/* InsertLine, */
    '[', Num1, 'M', 0, 0,	/* ClearLine, */
    '[', Num1, 'g', 0, 0,	/* SetTabs, */
    '[', Num1, 'q', 0, 0,	/* Ignore, */

    '[', Num2, 'H', 0, 0,	/* SetPosition, */
    '[', Num2, 'f', 0, 0,	/* SetPosition, */
    '[', Num2, 'm', 0, 1,	/* SetAttributes, */
    '[', Num2, 'y', 0, 0,	/* Ignore, */
    '[', Num2, 'r', 0, 0,	/* SetRegion, */
};

/*****************************************************************************/

static void DoCancel (Class * cl, Object * o)
{
    struct localData *lod = INST_DATA (cl, o);

    lod->lod_InSequence = FALSE;
    lod->lod_NumGoodChars = lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
}

/*****************************************************************************/

static BOOL ParseCode (Class * cl, Object * o, UBYTE c)
{
    struct localData *lod = INST_DATA (cl, o);
    SHORT i;

    /* ScanStep = 0:	This is the first character to introduce a control sequence. */
    if (!lod->lod_ScanStep)
    {
	/* Scan all available codes and try to find a match. */
	for (i = 0; i < NUM_CODES; i++)
	{
	    /* This character may introduce a control sequence. */
	    if (ANSICode[i].FirstChar == c)
	    {
		/*
		 * If this is a single character control sequence call the approriate function and exit immediately.
		 */
		if (ANSICode[i].ExactSize == 1)
		{
		    lod->lod_SaveBuffer[lod->lod_CharsInBuffer++] = c;
		    lod->lod_SaveBuffer[lod->lod_CharsInBuffer] = 0;
		    lod->lod_NumGoodChars = lod->lod_CharsInBuffer;
		    lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
		    lod->lod_WhichANSI = i;
		    return (FALSE);
		}

		/*
		 * The length of this control sequence is greater than a single character. Save the input character and return.
		 */
		lod->lod_ScanStep = i;
		lod->lod_SaveBuffer[lod->lod_CharsInBuffer++] = c;
		lod->lod_SaveBuffer[lod->lod_CharsInBuffer] = 0;
		return (TRUE);
	    }
	}

	/* No control sequence introducing character was found. */
	/* No Match */
	lod->lod_NumGoodChars = lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
	return (FALSE);
    }
    else
    {
	/* Length of the control sequence overrides the boundary, exit immediately! */
	if (lod->lod_CharsInBuffer == 21)
	{
	    /* No Match */
	    lod->lod_NumGoodChars = lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
	    return (FALSE);
	}

	/* Scan the remaining codes for a match. */
	for (i = lod->lod_ScanStep; i < NUM_CODES; i++)
	{

	    /*
	     * This sequence begins with the same character the parser was initialized with, so let's take a look at it.
	     */
	    if (ANSICode[i].FirstChar == lod->lod_SaveBuffer[0])
	    {
		/* This character is supposed to terminate the sequence. So let's exit. */
		if (ANSICode[i].LastChar == c)
		{
		    lod->lod_SaveBuffer[lod->lod_CharsInBuffer++] = c;
		    lod->lod_SaveBuffer[lod->lod_CharsInBuffer] = 0;
		    lod->lod_NumGoodChars = lod->lod_CharsInBuffer;
		    lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
		    lod->lod_WhichANSI = i;
		    return (FALSE);
		}
		else
		{
		    if (ANSICode[i].Match)
		    {
			/* This character is part of a legal sequence. Store it and return. */
			if ((*(ANSICode[i].Match)) (c))
			{
			    lod->lod_ScanStep = i;
			    lod->lod_SaveBuffer[lod->lod_CharsInBuffer++] = c;
			    return (TRUE);
			}
		    }
		}
	    }
	}

	/* This character is not part of a valid ANSI control sequence, so exit. */
	/* No Match */
	lod->lod_NumGoodChars = lod->lod_CharsInBuffer = lod->lod_ScanStep = 0;
	return (FALSE);
    }
}

/*****************************************************************************/

static void CSIFake (Class * cl, Object * o)
{
    struct localData *lod = INST_DATA (cl, o);

    /* Reset scanner */
    DoCancel (cl, o);

    /* Perform as if ESC [ had been transmitted. */
    lod->lod_InSequence = ParseCode (cl, o, '[');
}

/*****************************************************************************/

static UBYTE *ReadValue (UBYTE * Buffer, SHORT * Value)
{
    while ((*Buffer < '0' || *Buffer > '9') && *Buffer)
	Buffer++;

    if (*Buffer)
    {
	*Value = 0;
	while (*Buffer >= '0' && *Buffer <= '9')
	    *Value = (*Value * 10) + (*Buffer++ - '0');
    }
    else
	*Value = -1;

    if (*Buffer == ';' || *Buffer == ' ')
	return (&Buffer[1]);
    else
	return (NULL);
}

/*****************************************************************************/

ULONG layoutMethod (struct ASCIILib * ascii, Class * cl, Object * o, struct gpLayout * gpl)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    ULONG style = FS_NORMAL;
    struct TextFont *font;
    BOOL noprint = FALSE;
    struct RastPort trp;
    struct IBox *domain;
    ULONG hunit, hwidth;
    struct Line *line;
    STRPTR escbuffer;
    STRPTR title;
    LONG tspace;
    LONG offset;		/* Segment offset */
    LONG twidth;		/* Width of tab	 */
    LONG swidth;		/* Segment width */
    LONG lwidth;		/* Total line width */
    LONG anchor;		/* Beginning of	current	segment	 */
    WORD *space;
    WORD lspace;
    LONG width;			/* Width of view area */
    WORD *kern;
    BOOL split;
    ULONG bsig;
    LONG tabs;			/* Number of tabs */
    LONG chs;			/* Number of characters	 */
    LONG len;
    LONG num;
    LONG i;
    BOOL lf;
    WORD ch;

    BYTE fgpen = 1, bgpen = 0;
    BYTE attributes = 0;
    BYTE tmpen;
    WORD value;

    struct TextAttr *tattr;
    struct List *linelist;
    UBYTE smallbuffer[1];
    ULONG wrap = FALSE;
    BOOL send = FALSE;
    ULONG bufferlen;
    STRPTR buffer;
    ULONG visible;
    ULONG total;

    if (gpl->gpl_GInfo)
    {
	fgpen = gpl->gpl_GInfo->gi_DrInfo->dri_Pens[TEXTPEN];
	bgpen = gpl->gpl_GInfo->gi_DrInfo->dri_Pens[BACKGROUNDPEN];
    }

    /* Get the information that is required */
    if ((i = getdtattrs (ascii, o,
			 DTA_TextAttr,	(ULONG) &tattr,
			 DTA_TextFont,	(ULONG) &font,
			 TDTA_Buffer,	(ULONG) &buffer,
			 TDTA_BufferLen,(ULONG) &bufferlen,
			 TDTA_LineList,	(ULONG) &linelist,
			 TDTA_WordWrap,	(ULONG) &wrap,
			 DTA_Domain,	(ULONG) &domain,
			 DTA_ObjName,	(ULONG) &title,
			 TAG_DONE)) >= 7)
    {
	/* Obtain a lock on our data */
	ObtainSemaphore (&(si->si_Lock));

	/* Cache the information that we need */
	width = (LONG) domain->Width;
	space = (WORD *) (font->tf_CharSpace);
	kern  = (WORD *) (font->tf_CharKern);
	total = si->si_TotVert;

	/* Make sure we have a buffer to operate on */
	if (buffer)
	{
	    /* Make sure we are supposed to layout the text */
	    if (wrap || gpl->gpl_Initial)
	    {
		InitRastPort (&trp);
		SetFont (&trp, font);

		split = FALSE;
		send = TRUE;
		lf = FALSE;
		offset = 0;
		swidth = 0;
		lwidth = 0;
		tabs = 0;
		chs = 0;
		num = 0;
		len = 0;
		lspace = -1;

#if 1
		tspace = TextLength (&trp, " ", 1) * 8;
#else
		tspace = (space && kern) ? 64 : (font->tf_XSize * 8);
#endif

		DoCancel (cl, o);

		/* Delete the old line list */
		while (line = (struct Line *) RemHead (linelist))
		    FreePooled (lod->lod_Pool, line, sizeof (struct Line));

		for (anchor = i = bsig = 0; (i <= bufferlen) && (bsig == 0); i++)
		{
		    len++;
		    ch = buffer[i];

		    if (lod->lod_InSequence)
		    {
			if (ch == 27)
			{
			    num++;
			    DoCancel (cl, o);
			    lod->lod_InSequence = TRUE;
			}
			else if (ch == CSI)
			{
			    num++;
			    CSIFake (cl, o);
			}
			else if (lod->lod_InSequence = ParseCode (cl, o, ch))
			{
			    num++;
			}
			else
			{
			    if (lod->lod_NumGoodChars)
			    {
				escbuffer = &lod->lod_SaveBuffer[0];
				switch (ANSICode[lod->lod_WhichANSI].Func)
				{
				    case 1:
					do
					{
					    escbuffer = ReadValue (escbuffer, &value);

					    if (value == -1)
						value = 0;

					    switch (value)
					    {
						case 0:
						    attributes = 0;
						    style = FS_NORMAL;
						    fgpen = 1;
						    bgpen = 0;
						    if (gpl->gpl_GInfo)
						    {
							fgpen = gpl->gpl_GInfo->gi_DrInfo->dri_Pens[TEXTPEN];
							bgpen = gpl->gpl_GInfo->gi_DrInfo->dri_Pens[BACKGROUNDPEN];
						    }
						    break;

						case 1:
						    attributes |= ATTR_HIGHLIGHT;
						    style |= FSF_BOLD;
						    break;

						case 3:
						    style |= FSF_ITALIC;
						    break;

						case 4:
						    attributes |= ATTR_UNDERLINE;
						    style |= FSF_UNDERLINED;
						    break;

						case 5:
						    attributes |= ATTR_BLINK;
						    break;

						case 7:
						    attributes |= ATTR_INVERSE;
						    tmpen = fgpen;
						    fgpen = bgpen;
						    bgpen = tmpen;
						    break;

						default:
						    if (value >= 30)
						    {
							if (value <= 37)
							{
							    if (attributes & ATTR_INVERSE)
								bgpen = value - 30;
							    else
								fgpen = value - 30;
							}
							else
							{
							    if (value >= 40 && value <= 47)
							    {
								if (attributes & ATTR_INVERSE)
								    fgpen = value - 40;
								else
								    bgpen = value - 40;
							    }
							}
						    }
						    else if (value >= 20)
						    {
							switch (value - 20)
							{
							    case 2:
								attributes &= ~ATTR_HIGHLIGHT;
								style &= ~FSF_BOLD;
								break;

							    case 3:
								style &= ~FSF_ITALIC;
								break;

							    case 4:
								attributes &= ~ATTR_UNDERLINE;
								style &= ~FSF_UNDERLINED;
								break;
							}
						    }

						    break;
					    }
					}
					while (escbuffer);
					break;
				}
			    }

			    DoCancel (cl, o);
			    noprint = split = TRUE;
			}
		    }
		    else if ((ch == 27) || (ch == CSI))
		    {
			split = lod->lod_InSequence = TRUE;
		    }
		    else if (ch == '\t')
		    {
			if (chs)
			{
			    split = TRUE;
			}
			else
			{
			    if (tabs++ || !lwidth)
				twidth = tspace;
			    else
				twidth = (((lwidth / tspace) + 1) * tspace) - lwidth;

			    noprint = TRUE;
			    if (!wrap || (swidth + twidth <= width - offset))
			    {
				offset += twidth;
				swidth += twidth;
				lwidth += twidth;
				num++;
			    }
			    else
			    {
				lf = TRUE;
			    }
			}
		    }
		    else if (ch == '\r')
		    {
			split = TRUE;
		    }
		    else if ((ch == '\n') || (ch == 0) || (ch == 12))
		    {
			lf = TRUE;
		    }
		    else
		    {
			if (tabs)
			{
			    num--;
			    noprint = split = TRUE;
			}
			else
			{
			    if ((lod->lod_Flags & LDF_EUC) || (space && kern))
			    {
				smallbuffer[0] = ch;
				twidth = TextLength (&trp, smallbuffer, 1);
			    }
			    else
				twidth = font->tf_XSize;

			    if ((num == 0) && (style & FSF_ITALIC))
				offset += font->tf_XSize / 3;

			    if (!wrap || (swidth + twidth <= width - offset))
			    {
				swidth += twidth;
				lwidth += twidth;
				num++;
			    }
			    else
			    {
				lf = TRUE;
				if (lspace >= 0)
				    num -= (i - lspace);
			    }

			    chs++;
			    if (ch == ' ')
				lspace = i;
			}
		    }

		    if (lf || split || (wrap && (swidth > (width - offset))))
		    {
			if (noprint)
			{
			    if (line && lf)
			    {
				line->ln_Flags |= LNF_LF;
				lwidth = 0;
				offset = 0;
			    }
			}
			else
			{
			    if (line = AllocPooled (lod->lod_Pool, sizeof (struct Line)))
			    {
				SetSoftStyle (&trp, style, 0xFF);
				swidth = TextLength (&trp, &buffer[anchor], num);

				line->ln_Text    = &buffer[anchor];
				line->ln_TextLen = num;
				line->ln_XOffset = offset;
				line->ln_Width   = swidth;
				line->ln_Flags   = NULL;
				line->ln_FgPen   = fgpen;
				line->ln_BgPen   = bgpen;
				line->ln_Style   = style;
				line->ln_Data    = NULL;
				AddTail (linelist, (struct Node *) line);

				if (lf || (wrap && (swidth > (width - offset))))
				{
				    line->ln_Flags |= LNF_LF;
				    lod->lod_MWidth = MAX (lod->lod_MWidth, lwidth);
				    lwidth = 0;
				    offset = 0;
				}
				else
				{
				    offset += swidth;
				}
			    }

			    if (!lf && num && (num < len))
			    {
				num--;
			    }
			    else if ((ch == CSI) && (num == 0))
			    {
				num = -1;
			    }
			}

			i = anchor + num;
			anchor = i + 1;

			lspace = -1;
			chs = tabs = num = len = swidth = 0;
			split = lf = noprint = FALSE;

			/* Aborted? */
			bsig = CheckSignal (SIGBREAKF_CTRL_C);
		    }
		}

		/* Force a recount, because some of the partial lines may not have been added to the list. */
		total = 0;
		line = (struct Line *) linelist;
		while (line->ln_Link.mln_Succ && i)
		{
		    if (line->ln_Flags & LNF_LF)
			total++;
		    line = (struct Line *) line->ln_Link.mln_Succ;
		}
	    }
	}

	if ((space && kern) || (lod->lod_Flags & LDF_EUC))
	{
	    si->si_HorizUnit = hunit = 1;
	    hwidth = lod->lod_MWidth;
	}
	else
	{
	    si->si_HorizUnit = hunit = font->tf_XSize;
	    hwidth = lod->lod_MWidth / font->tf_XSize;
	}

	si->si_VisVert = visible = domain->Height / si->si_VertUnit;
	si->si_TotVert = total;

	si->si_VisHoriz = (LONG) domain->Width / hunit;
	si->si_TotHoriz = hwidth;

	si->si_VertUnit = font->tf_YSize;

	/* Check for abortion */
	if (CheckSignal (SIGBREAKF_CTRL_C))
	    bsig++;

	/* Were we aborted? */
	if (send && (bsig == 0))
	{
	    /* Tell everyone about our changes */
	    notifyAttrChanges (ascii, o, gpl->gpl_GInfo, NULL,
			       GA_ID,		G (o)->GadgetID,
			       DTA_Title,	title,
			       TAG_DONE);
	}

	/* Release the lock */
	ReleaseSemaphore (&(si->si_Lock));
    }

    return (total);
}

/*****************************************************************************/

static ULONG ASM dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ASCIILib *ascii = (struct ASCIILib *) cl->cl_UserData;
    struct localData *lod;
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		ULONG len, estlines, poolsize;
		struct DataTypeHeader *dth;
		struct DataType *dtn;
		UWORD dtype;

		lod = INST_DATA (cl, newobj);

		/* Get the handles */
		getdtattrs (ascii, newobj,
			    DTA_DataType, (ULONG) &dtn,
			    TDTA_BufferLen, (ULONG)&len,
			    TAG_DONE);

		/* Estimate the pool size that we will need */
		estlines = (len / 80) + 1;
		estlines = (estlines > 200) ? 200 : estlines;
		poolsize = sizeof (struct Line) * estlines;

		/* Create a message for the lines */
		if (lod->lod_Pool = CreatePool (MEMF_CLEAR | MEMF_PUBLIC, poolsize, poolsize))
		{
		    /* See if we are EUC or not */
		    if (dtn)
		    {
			dth = dtn->dtn_Header;
			dtype = dth->dth_Flags & DTF_TYPE_MASK;
			if ((dtype == DTF_BINARY) && (Stricmp (dth->dth_Name, "EUC") == 0))
			    lod->lod_Flags |= LDF_EUC;
		    }
		}
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_UPDATE:
	case OM_SET:
	    if ((retval = DoSuperMethodA (cl, o, msg)) && (OCLASS (o) == cl))
	    {
		struct RastPort *rp;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr;

		    /* Force a redraw */
		    gpr.MethodID   = GM_RENDER;
		    gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort  = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}
		retval = 0;
	    }
	    break;

	case GM_LAYOUT:
	    notifyAttrChanges (ascii, o, ((struct gpLayout *) msg)->gpl_GInfo, NULL,
			       GA_ID, G (o)->GadgetID,
			       DTA_Busy, TRUE,
			       TAG_DONE);

	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += DoAsyncLayout (o, (struct gpLayout *) msg);
	    break;

	case DTM_PROCLAYOUT:
	    notifyAttrChanges (ascii, o, ((struct gpLayout *) msg)->gpl_GInfo, NULL,
			       GA_ID, G (o)->GadgetID,
			       DTA_Busy, TRUE,
			       TAG_DONE);
	    DoSuperMethodA (cl, o, msg);

	case DTM_ASYNCLAYOUT:
	    retval = layoutMethod (ascii, cl, o, (struct gpLayout *) msg);
	    break;

	case OM_DISPOSE:
	    {
		struct List *linelist;

		lod = INST_DATA (cl, o);

		/* Don't let the super class free the line list */
		if (getdtattrs (ascii, o, TDTA_LineList, (ULONG) &linelist, TAG_DONE) && linelist)
		    NewList (linelist);

		/* Delete the line list */
		DeletePool (lod->lod_Pool);
	    }

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

Class *initClass (struct ASCIILib * ascii)
{
    Class *cl;

    if (cl = MakeClass (ASCIIDTCLASS, TEXTDTCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatch;
	cl->cl_UserData = (ULONG) ascii;
	AddClass (cl);
    }

    return (cl);
}
