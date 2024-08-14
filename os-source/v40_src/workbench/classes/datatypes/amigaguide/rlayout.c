/* rlayout.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	MULTI_FONT	FALSE

/*****************************************************************************/

static char *amigaguide = "AmigaGuide®";

/*****************************************************************************/

enum
{
    RL_AMIGAGUIDE,
    RL_APEN,
    RL_BOLD,
    RL_BACKGROUND,
    RL_BODY,
    RL_BPEN,
    RL_CLEARTABS,
    RL_CODE,
    RL_FOREGROUND,
    RL_FONTNUMBER,
    RL_HEADING,
    RL_ITALIC,
    RL_CENTER,
    RL_LEFTJUSTIFY,
    RL_RIGHTJUSTIFY,
    RL_LINDENT,
    RL_LINE,
    RL_MONO,
    RL_PARAGRAPH,
    RL_DEFAULTPARAGRAPH,
    RL_PARINDENT,
    RL_PLAIN,
    RL_PROP,
    RL_SETTABS,
    RL_TAB,
    RL_UNDERLINE,
    RL_UNBOLD,
    RL_UNITALIC,
    RL_UNUNDERLINE,
};

/*****************************************************************************/

static const struct Keyword rl_Keywords[] =
{
    {"AMIGAGUIDE",		RL_AMIGAGUIDE},
    {"APEN",			RL_APEN},
    {"B",			RL_BOLD},
    {"BG",			RL_BACKGROUND},
    {"BODY",			RL_BODY},
    {"BPEN",			RL_BPEN},
    {"CLEARTABS",		RL_CLEARTABS},
    {"CODE",			RL_CODE},
    {"FG",			RL_FOREGROUND},
    {"FONTN",			RL_FONTNUMBER},
    {"HEADING",			RL_HEADING},
    {"I",			RL_ITALIC},
    {"JCENTER",			RL_CENTER},
    {"JLEFT",			RL_LEFTJUSTIFY},
    {"JRIGHT",			RL_RIGHTJUSTIFY},
    {"LINDENT",			RL_LINDENT},
    {"LINE",			RL_LINE},
    {"MONO",			RL_MONO},
    {"PAR",			RL_PARAGRAPH},
    {"PARD",			RL_DEFAULTPARAGRAPH},
    {"PARI",			RL_PARINDENT},
    {"PLAIN",			RL_PLAIN},
    {"PROP",			RL_PROP},
    {"SETTABS",			RL_SETTABS},
    {"TAB",			RL_TAB},
    {"U",			RL_UNDERLINE},
    {"UB",			RL_UNBOLD},
    {"UI",			RL_UNITALIC},
    {"UU",			RL_UNUNDERLINE},
};

#define	RL_TOTAL_KEYWORDS	(sizeof (rl_Keywords) / sizeof (struct Keyword))

/*****************************************************************************/

static void rl_expandTabs (struct AGLib *ag, struct WorkLayout *wl, struct NodeData *nd)
{
    register LONG j, k;

    /* Find the tab spot */
    for (j = 0, k = -1; (j < MAX_TABS) && (k == -1); j++)
    {
	if (wl->wl_XOffset < (ULONG) ((ULONG) nd->nd_Tabs[j] * wl->wl_SpaceWidth))
	    k = j;
    }

    /* Increment */
    while (--wl->wl_NumTabs)
	k++;

    /* Jump to the next tab stop */
    wl->wl_XOffset = (ULONG) nd->nd_Tabs[k] * wl->wl_SpaceWidth;

    /* Clear the tabs */
    wl->wl_NumTabs = 0;
}

/*****************************************************************************/

void rl_layoutRecurse (struct AGLib *ag,
		       struct ClientData *cd, struct DatabaseData *dbd, struct NodeData *nd,
		       struct WorkLayout *wl, STRPTR buffer, LONG size)
{
    struct Line *lnSpace = NULL;
    LONG lastspace = -1;

    register LONG i, j;
    UBYTE ch;

    WORD minwidth, mw;
    ULONG lheight;
    ULONG lwidth;
    ULONG lbase;
    ULONG bbase;

    LONG value, tmp;
    ULONG cmdi = 0;
    ULONG cmda = 0;

    struct Line *anchorLine = NULL;
    ULONG newanchor, oldanchor;
    ULONG loopCnt;
    ULONG anchor;
    ULONG mspace;
    ULONG num;

    BOOL inBackslash = FALSE;
#if MULTI_FONT
    BOOL newfont;
#endif

    struct MacroNode *mn;

    /* Argument parsing */
    BOOL space = FALSE;
    STRPTR argv[64];
    UWORD token;
    LONG argc;
    UWORD nt;

    ULONG lnflags = NULL;

    /* Set the height and baseline */
    lheight = wl->wl_Font->tf_YSize + 2;
    lbase   = wl->wl_Font->tf_Baseline;
    bbase   = lheight - lbase;
    wl->wl_LineHeight = MAX (wl->wl_LineHeight, lheight);
    wl->wl_Baseline   = MAX (wl->wl_Baseline, lbase);
    wl->wl_BelowBase  = MAX (wl->wl_BelowBase, bbase);

    mspace = TextLength (&wl->wl_RPort, "m", 1);
    mw = nd->nd_MinWidth * mspace;
    minwidth = MAX (mw, wl->wl_View.Width);

#if MULTI_FONT
    newfont = FALSE;
#endif

    /* Step through the buffer */
    for (i = argc = num = anchor = oldanchor = newanchor = loopCnt = 0; i < size; i++)
    {
	/* Get the current character */
	ch = buffer[i];

	/* Are we in an attribute command */
	if (wl->wl_InAttr)
	{
	    /* End of a command? */
	    if (((ch == ECURLY) || (ch=='\n') || (ch==0)) && !wl->wl_InQuote)
	    {
		/* Terminate the lines */
		wl->wl_CmdBuffer[cmdi] = 0;
		wl->wl_ArgBuffer[cmda] = 0;

		/* Get the value */
		value = -1;
		if (argc)
		    StrToLong (argv[1], &value);

		/* Uppercase the command */
		StrToUpper (ag, wl->wl_CmdBuffer);

		/* Turn off the switches */
		wl->wl_InAttr = wl->wl_InCmnd = FALSE;

		/* See if we are inside a link command */
		if (wl->wl_InLink && (argc > 0))
		{
		    /* Expand any tabs that are left */
		    if (wl->wl_NumTabs)
			rl_expandTabs (ag, wl, nd);

		    num = strlen (wl->wl_CmdBuffer);
		    tmp = strlen (wl->wl_ArgBuffer);
		    SetSoftStyle (&wl->wl_RPort, FS_NORMAL, 0xFF);
		    lwidth = TextLength (&wl->wl_RPort, &buffer[i - tmp + 1], num) + 4;

		    /* Force a line feed if needed */
		    if (wl->wl_Wrap && (wl->wl_XOffset + lwidth > minwidth))
		    {
			lastspace = -1;
			wl->wl_Line->ln_Flags |= LNF_LF;
			wl->wl_XOffset = wl->wl_LeftMargin;
			wl->wl_YOffset += wl->wl_LineHeight;
#if MULTI_FONT
			wl->wl_LineHeight = wl->wl_Baseline = 0;
#endif
			anchorLine = NULL;
			wl->wl_NumLines++;
		    }

		    wl->wl_NumColumns = MAX (wl->wl_NumColumns, wl->wl_XOffset + lwidth);

		    if (wl->wl_Line = allocpvec (ag, cd->cd_Pool, sizeof (struct Line) + tmp + 1, MEMF_PUBLIC))
		    {
			/* Set the active link */
			wl->wl_LinkNum++;
			if (cd->cd_ActiveNum == wl->wl_LinkNum)
			    cd->cd_ActiveLine = wl->wl_Line;

			/* Fill in all the information */
			wl->wl_Line->ln_Text		= &buffer[i - tmp + 1];
			wl->wl_Line->ln_TextLen		= num;
			wl->wl_Line->ln_Flags		= LNF_LINK;
			wl->wl_Line->ln_Baseline	= lbase;
			wl->wl_Line->ln_Box.Left	= (WORD) wl->wl_XOffset;
			wl->wl_Line->ln_Box.Top		= (WORD) wl->wl_YOffset;
			wl->wl_Line->ln_Box.Width	= (WORD) lwidth;
			wl->wl_Line->ln_Box.Height	= (WORD) lheight;
			wl->wl_Line->ln_Style		= FS_NORMAL;
			wl->wl_Line->ln_FgPen		= wl->wl_FgPen;
			wl->wl_Line->ln_BgPen		= wl->wl_BgPen;
			wl->wl_Line->ln_Font		= wl->wl_Font;
			wl->wl_Line->ln_Data		= MEMORY_FOLLOWING (wl->wl_Line);
			wl->wl_Line->ln_LinkNum		= wl->wl_LinkNum;
			wl->wl_XOffset += lwidth;
			strcpy (wl->wl_Line->ln_Data, wl->wl_ArgBuffer);

			/* Add the link node to line list */
			AddTail ((struct List *) &cd->cd_LineList, (struct Node *) wl->wl_Line);
		    }
		}
		/* Is this a macro? */
		else if (mn = (struct MacroNode *) FindName ((struct List *) &dbd->dbd_MacroList, wl->wl_CmdBuffer))
		{
		    UWORD idx, idj, idk, cnt, len;
		    struct Line *ln;
		    STRPTR buffer;
		    ULONG msize;
		    WORD var;

		    wl->wl_Num++;

		    len = strlen (mn->mn_Macro);
		    msize = len + 1;
		    for (idx = cnt = 0; idx < len; idx++)
		    {
			if (mn->mn_Macro[idx]=='$')
			{
			    var = (WORD) (mn->mn_Macro[idx+1] - '0');
			    if ((var > 0) && (var <= argc))
			    {
				msize += strlen (argv[var]) + 1;
				cnt++;
			    }
			    idx++;
			}
		    }

		    if (cnt)
		    {
			msize += sizeof (struct Line);

                        if (ln = allocpvec (ag, cd->cd_Pool, msize, MEMF_CLEAR | MEMF_PUBLIC))
                        {
			    ln->ln_Style = wl->wl_Style;
			    ln->ln_FgPen = wl->wl_FgPen;
			    ln->ln_BgPen = wl->wl_BgPen;
			    ln->ln_Font  = wl->wl_Font;
			    buffer = ln->ln_Data = MEMORY_FOLLOWING (ln);
			    AddHead ((struct List *) & cd->cd_LineList, (struct Node *) ln);

                            for (idx = idj = 0; idx < len; idx++)
                            {
                                if (mn->mn_Macro[idx]=='$')
                                {
                                    var = (WORD) (mn->mn_Macro[idx+1] - '0');
                                    if ((var > 0) && (var <= argc))
                                    {
                                        for (idk = 0; idk < strlen (argv[var]); idk++)
                                            buffer[idj++]=argv[var][idk];
                                    }
                                    idx++;
                                }
                                else
                                    buffer[idj++]=mn->mn_Macro[idx];
                            }
                            rl_layoutRecurse (ag, cd, dbd, nd, wl, buffer, strlen (buffer) + 1);
                        }
		    }
		    else
			rl_layoutRecurse (ag, cd, dbd, nd, wl, mn->mn_Macro, strlen (mn->mn_Macro) + 1);

		    wl->wl_InAttr = wl->wl_InCmnd = FALSE;
		    wl->wl_Num--;
		}
		else
		{
		    switch (token = m_binSearch (wl->wl_CmdBuffer, strlen (wl->wl_CmdBuffer), rl_Keywords, RL_TOTAL_KEYWORDS))
		    {
			case RL_AMIGAGUIDE:
			    /* Expand any tabs that are left */
			    if (wl->wl_NumTabs)
				rl_expandTabs (ag, wl, nd);

			    /* Get the lengths for this segment */
			    num = strlen (amigaguide);
			    lwidth = TextLength (&wl->wl_RPort, amigaguide, num) + 3;

			    /* Force a line feed if needed */
			    if (wl->wl_Wrap && (wl->wl_XOffset + lwidth > minwidth))
			    {
				lastspace = -1;
				wl->wl_Line->ln_Flags |= LNF_LF;
				wl->wl_XOffset = wl->wl_LeftMargin;
				wl->wl_YOffset += wl->wl_LineHeight;
#if MULTI_FONT
				wl->wl_LineHeight = wl->wl_Baseline = 0;
#endif
				anchorLine = NULL;
				wl->wl_NumLines++;
			    }

			    wl->wl_NumColumns = MAX (wl->wl_NumColumns, wl->wl_XOffset + lwidth);

			    if (wl->wl_Line = allocpvec (ag, cd->cd_Pool, sizeof (struct Line), MEMF_PUBLIC))
			    {
				wl->wl_Line->ln_Text = amigaguide;
				wl->wl_Line->ln_TextLen = num;
				wl->wl_Line->ln_Flags = lnflags;
				wl->wl_Line->ln_Baseline = lbase;
				wl->wl_Line->ln_Box.Left = (WORD) wl->wl_XOffset;
				wl->wl_Line->ln_Box.Top = (WORD) wl->wl_YOffset;
				wl->wl_Line->ln_Box.Width = (WORD) lwidth;
				wl->wl_Line->ln_Box.Height = (WORD) lheight;
				wl->wl_Line->ln_Style = FSF_BOLD;
				wl->wl_Line->ln_FgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN];
				wl->wl_Line->ln_BgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN];
				wl->wl_Line->ln_Font = wl->wl_Font;
				AddTail ((struct List *) &cd->cd_LineList, (struct Node *) wl->wl_Line);
				wl->wl_XOffset += lwidth;
			    }
			    break;

			case RL_TAB:
			    wl->wl_NumTabs++;
			    lnSpace = wl->wl_Line;
			    lastspace = i;
			    break;

			case RL_SETTABS:
			    for (j = 0, nt = nd->nd_DefTabs; j < MAX_TABS; j++, nt += (UWORD) nd->nd_DefTabs)
			    {
				if (j < argc)
				{
				    StrToLong (argv[j + 1], &tmp);
				    nt = nd->nd_Tabs[j] = (UWORD) tmp;
				}
				else
				    nd->nd_Tabs[j] = nt;
			    }
			    break;

			case RL_CLEARTABS:
			    /* Restore the default tab table */
			    for (j = 0, nt = nd->nd_DefTabs; j < MAX_TABS; j++, nt += nd->nd_DefTabs)
				nd->nd_Tabs[j] = nt;
			    break;

			case RL_PARAGRAPH:
			    /* Paragraph */
			    wl->wl_LineFeed = wl->wl_NewSeg = TRUE;
			    wl->wl_NumTabs = 0;
			    wl->wl_ParXAdjust = wl->wl_ParIndent;
			    lastspace = -1;
			    break;

			case RL_LINE:
			    /* Line feed */
			    wl->wl_LineFeed = wl->wl_NewSeg = TRUE;
			    wl->wl_NumTabs = 0;
			    lastspace = -1;
			    break;

			case RL_APEN:
			    StrToLong (argv[1], &tmp);
			    wl->wl_FgPen = tmp;
			    break;

			case RL_BPEN:
			    StrToLong (argv[1], &tmp);
			    wl->wl_BgPen = tmp;
			    break;

			case RL_BOLD:
			    wl->wl_Style |= FSF_BOLD;
			    break;

			case RL_UNBOLD:
			    wl->wl_Style &= ~FSF_BOLD;
			    break;

			case RL_UNDERLINE:
			    wl->wl_Style |= FSF_UNDERLINED;
			    break;

			case RL_UNUNDERLINE:
			    wl->wl_Style &= ~FSF_UNDERLINED;
			    break;

			case RL_ITALIC:
			    wl->wl_Style |= FSF_ITALIC;
			    break;

			case RL_UNITALIC:
			    wl->wl_Style &= ~FSF_ITALIC;
			    break;

			case RL_PLAIN:
			    wl->wl_Style = FS_NORMAL;
			    break;

			case RL_DEFAULTPARAGRAPH:
			    wl->wl_FgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN];
			    wl->wl_BgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN];
			    wl->wl_LeftMargin = 0;
			    wl->wl_ParIndent = 0;
			    wl->wl_Style = FS_NORMAL;
#if MULTI_FONT
			    wl->wl_Font = dbd->dbd_Fonts[nd->nd_DefFont];
			    newfont = TRUE;
#endif
			    wl->wl_Wrap = wl->wl_DefWrap;
			    wl->wl_SmartWrap = wl->wl_DefSmartWrap;
			    break;

			case RL_HEADING:
#if MULTI_FONT
			    tmp = 2 + (3 - value);
			    if ((value >= 1) && (value <= 3) && (dbd->dbd_Fonts[tmp]))
			    {
				wl->wl_Font = dbd->dbd_Fonts[tmp];
				newfont = TRUE;
			    }
#endif
			    wl->wl_Wrap = wl->wl_DefWrap;
			    wl->wl_SmartWrap = wl->wl_DefSmartWrap;
			    break;

			case RL_BODY:
#if MULTI_FONT
			    wl->wl_Font = dbd->dbd_Fonts[nd->nd_DefFont];
			    newfont = TRUE;
#endif
			    wl->wl_Wrap = wl->wl_DefWrap;
			    wl->wl_SmartWrap = wl->wl_DefSmartWrap;
			    break;

			case RL_CODE:
#if MULTI_FONT
			    wl->wl_Font = dbd->dbd_Fonts[0];
			    newfont = TRUE;
#endif
			    wl->wl_Wrap = wl->wl_SmartWrap = FALSE;
			    break;

			case RL_MONO:
			    break;
#if MULTI_FONT
			case RL_MONO:
			    wl->wl_Font = dbd->dbd_Fonts[0];
			    newfont = TRUE;
			    break;

			case RL_PROP:
			    wl->wl_Font = dbd->dbd_Fonts[1];
			    newfont = TRUE;
			    break;

			case RL_FONTN:
			    StrToLong (argv[1], &tmp);
			    tmp = tmp + 5;

			    if ((tmp > 5) && (tmp < MAX_FONTS) && dbd->dbd_Fonts[tmp])
			    {
				wl->wl_Font = dbd->dbd_Fonts[tmp];
				wl->wl_Wrap = wl->wl_DefWrap;
				wl->wl_SmartWrap = wl->wl_DefSmartWrap;
				newfont = TRUE;
			    }
			    break;
#endif
			case RL_LEFTJUSTIFY:
			    wl->wl_Justify = JLEFT;
			    break;

			case RL_CENTER:
			    wl->wl_Justify = JCENTER;
			    break;

			case RL_RIGHTJUSTIFY:
			    wl->wl_Justify = JRIGHT;
			    break;

			case RL_LINDENT:
			    /* Set the left margin */
			    wl->wl_LeftMargin = value * wl->wl_SpaceWidth;

			    /* Stop those nasty layout loops */
			    if (wl->wl_ParIndent > 0)
				minwidth = MAX ((WORD) (wl->wl_LeftMargin + wl->wl_ParIndent) + mw, wl->wl_View.Width);
			    else
				minwidth = MAX ((WORD) wl->wl_LeftMargin + mw, wl->wl_View.Width);

			    /* Paragraph indent? */
			    if (num == 0 && wl->wl_NumLines == 0)
				wl->wl_XOffset = wl->wl_LeftMargin + wl->wl_ParIndent;
			    break;

			case RL_PARINDENT:
			    /* Set the first line indent */
			    wl->wl_ParIndent = value * wl->wl_SpaceWidth;

			    /* Stop those nasty layout loops */
			    if (wl->wl_ParIndent > 0)
				minwidth = MAX ((WORD) (wl->wl_LeftMargin + wl->wl_ParIndent) + mw, wl->wl_View.Width);
			    else
				minwidth = MAX ((WORD) wl->wl_LeftMargin + mw, wl->wl_View.Width);

			    /* Paragraph indent? */
			    if (num == 0 && wl->wl_NumLines == 0)
				wl->wl_XOffset = wl->wl_LeftMargin + wl->wl_ParIndent;
			    break;

			case RL_FOREGROUND:
			case RL_BACKGROUND:
			    if (argc == 1)
			    {
				BYTE pen;

				StrToUpper (ag, argv[1]);
				if (strcmp (argv[1], "TEXT") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN];
				else if (strcmp (argv[1], "SHINE") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[SHINEPEN];
				else if (strcmp (argv[1], "SHADOW") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[SHADOWPEN];
				else if (strcmp (argv[1], "FILL") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[FILLPEN];
				else if (strcmp (argv[1], "FILLTEXT") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[FILLTEXTPEN];
				else if (strcmp (argv[1], "BACK") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN];
				else if (strcmp (argv[1], "BACKGROUND") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN];
				else if (strcmp (argv[1], "HIGHLIGHT") == 0)
				    pen = cd->cd_GInfo.gi_DrInfo->dri_Pens[HIGHLIGHTTEXTPEN];

				if (token == RL_FOREGROUND)
				    wl->wl_FgPen = pen;
				else
				    wl->wl_BgPen = pen;
			    }
			    break;
		    }
		}

		oldanchor = anchor;
		newanchor = anchor = i + 1;
		space = wl->wl_InLink = FALSE;
		num = argc = 0;
	    }
	    /* See if we are inside quotes */
	    else if (ch == QUOTE)
	    {
		/* Save this character */
		wl->wl_ArgBuffer[cmda++] = ch;

		/* Toggle the quote flag */
		wl->wl_InQuote ^= 1;
	    }
	    else if (((ch == ' ') || (ch == '\t')) && !wl->wl_InQuote)
	    {
		/* Save this character */
		wl->wl_ArgBuffer[cmda++] = ch;

		if (!space)
		{
		    wl->wl_CmdBuffer[cmdi++] = 0;
		    argv[++argc] = &wl->wl_CmdBuffer[cmdi];
		}
		space = TRUE;
	    }
	    else
	    {
		/* Save this character */
		wl->wl_ArgBuffer[cmda++] = ch;
		wl->wl_CmdBuffer[cmdi++] = ch;
		space = FALSE;
	    }
	}
	else if (wl->wl_InCmnd)
	{
	    if ((i == newanchor) && (ch == SCURLY))
	    {
		wl->wl_InAttr = TRUE;
	    }
	    else if ((ch == '\n') || (ch == 0))
	    {
		wl->wl_InCmnd = FALSE;
		oldanchor = anchor;
		newanchor = anchor = i + 1;
	    }
	}
	else if ((ch == BACKSLASH) && !inBackslash)
	{
	    newanchor = i + 1;
	    wl->wl_NewSeg = inBackslash = TRUE;
	}
	else if ((ch=='@') && (buffer[i+1]==SCURLY) && !inBackslash)
	{
	    space = FALSE;
	    cmda = cmdi = 0;
	    newanchor = i + 1;
	    wl->wl_NewSeg = wl->wl_InCmnd = TRUE;

	    /* See if we are inside a button */
	    if (buffer[i + 2] == QUOTE)
		wl->wl_InLink = TRUE;
	}
#if 1
	else if ((ch=='@') && (buffer[i-1]=='\n') && !inBackslash)
	{
	    register LONG k;

	    if (!wl->wl_SmartWrap || (buffer[i - 1] == 10))
	    {
		wl->wl_NumTabs = 0;
	    }

	    if (wl->wl_SmartWrap)
	    {
		lnSpace = wl->wl_Line;
		lastspace = i - 1;
	    }

	    for (k = i; k < size; k++)
	    {
		if (buffer[k] =='\n')
		{
		    i = k;
		    break;
		}
	    }

	    newanchor = i + 1;
	    wl->wl_NewSeg = TRUE;
	    wl->wl_XAdjust = (num) ? wl->wl_SpaceWidth : 0;
	}
#endif
	/* End of buffer */
	else if (ch == 0)
	{
	    inBackslash = FALSE;
	    wl->wl_NewSeg = TRUE;
	    newanchor = i + 1;
	}
	/* Tab */
	else if ((ch == 9) && !inBackslash)
	{
	    /* See if we need to terminate a line segment */
	    if (wl->wl_NumTabs == 0)
		wl->wl_NewSeg = TRUE;
	    wl->wl_NumTabs++;
	    lnSpace = wl->wl_Line;
	    lastspace = i;
	}
	/* Line feed */
	else if ((ch == 10) && !inBackslash)
	{
	    if (!wl->wl_SmartWrap || (buffer[i - 1] == 10))
	    {
		wl->wl_LineFeed = TRUE;
		wl->wl_NumTabs = 0;
	    }

	    if (wl->wl_SmartWrap)
	    {
		lnSpace = wl->wl_Line;
		lastspace = i;
	    }

	    newanchor = i + 1;
	    wl->wl_NewSeg = TRUE;
	    wl->wl_XAdjust = (num) ? wl->wl_SpaceWidth : 0;
	}
	else
	{
	    /* No longer in a backslash */
	    inBackslash = FALSE;

	    /* See if we have any TABs that we need to finish out */
	    if (wl->wl_NumTabs)
	    {
		/* Expand the tabs */
		rl_expandTabs (ag, wl, nd);

		num = 0;
		oldanchor = anchor;
		anchor = i;
	    }

	    if (wl->wl_Wrap)
	    {
		/* Do word wrapping */
		lwidth = TextLength (&wl->wl_RPort, &buffer[anchor], num + 1);
		if (wl->wl_XOffset + lwidth > minwidth)
		{
		    BOOL drain = FALSE;
		    struct Line *nln;
		    struct Line *ln;

		    if (lastspace >= 0)
		    {
			if (lastspace >= anchor)
			{
			    wl->wl_NewSeg = TRUE;
			    num -= (i - lastspace);
			    i = lastspace + 1;
			    DB (kprintf ("%ld: i=%ld\n", __LINE__, lastspace));
			}
			else
			{
			    wl->wl_ParXAdjust = 0;
			    i = lastspace + 1;
			    DB (kprintf ("%ld: i=%ld\n", __LINE__, lastspace));
			    drain = TRUE;
			    num = 0;
			}
		    }
		    else
		    {
			wl->wl_ParXAdjust = 0;
			drain = TRUE;
			i = anchor;
			DB (kprintf ("%ld : anchor=%ld\n", __LINE__, anchor));
			num = 0;

			/* Increment the loop counter */
			if (anchor == oldanchor)
			    loopCnt++;

			/* Error control */
			if (loopCnt > 2)
			{
			    nd->nd_MinWidth++;
			    mw = nd->nd_MinWidth * mspace;
			    minwidth += mspace;
			    loopCnt = 0;
			}
		    }

		    if (drain && lnSpace &&
			(ln = (struct Line *)lnSpace->ln_Link.mln_Succ) && (ln->ln_Link.mln_Succ))
		    {
			nln = (struct Line *) ln->ln_Link.mln_Succ;
			while (nln->ln_Link.mln_Succ)
			{
			    Remove ((struct Node *) nln);
			    freepvec (ag, cd->cd_Pool, nln);
			    nln = (struct Line *) ln->ln_Link.mln_Succ;
			}
			wl->wl_Line = lnSpace;
		    }

		    wl->wl_NewSeg = wl->wl_LineFeed = TRUE;
		    newanchor = i;
		    i--;
		}
		else
		    num++;
	    }
	    else
		num++;

	    /* Remember this as the last space */
	    if (ch == ' ')
	    {
		lnSpace = wl->wl_Line;
		lastspace = i;
	    }
	}

#if MULTI_FONT
	/* Did the font change? */
	if (newfont)
	{
	    /* Set the font */
	    SetFont (&wl->wl_RPort, wl->wl_Font);

	    /* Calculate the amount of space a tab takes up */
	    wl->wl_SpaceWidth = TextLength (&wl->wl_RPort, " ", 1);
	    wl->wl_TabSpace = wl->wl_SpaceWidth * 8;

	    /* Set the height and baseline */
	    lheight = wl->wl_Font->tf_YSize + 2;
	    lbase   = wl->wl_Font->tf_Baseline;
	    bbase   = lheight - lbase;

	    wl->wl_LineHeight = MAX (wl->wl_LineHeight, lheight);
	    wl->wl_Baseline   = MAX (wl->wl_Baseline, lbase);
	    wl->wl_BelowBase  = MAX (wl->wl_BelowBase, bbase);

	    mw = nd->nd_MinWidth * mspace;
	    minwidth = MAX (mw, wl->wl_View.Width);
	    newfont = FALSE;
	}
#endif

	/* Time for a new line segment? */
	if (wl->wl_NewSeg)
	{
	    if (wl->wl_Line = allocpvec (ag, cd->cd_Pool, sizeof (struct Line), MEMF_PUBLIC))
	    {
		if (num)
		{
		    SetSoftStyle (&wl->wl_RPort, wl->wl_Style, 0xFF);
		    lwidth = TextLength (&wl->wl_RPort, &buffer[anchor], num);
#if MULTI_FONT
		    lheight = wl->wl_Font->tf_YSize + 2;
		    lbase = wl->wl_Font->tf_Baseline;
#endif
		}
		else
		{
		    lwidth = 0;
#if MULTI_FONT
		    lheight = dbd->dbd_Fonts[nd->nd_DefFont]->tf_YSize;
		    lbase = dbd->dbd_Fonts[nd->nd_DefFont]->tf_Baseline;
#endif
		}
#if MULTI_FONT
		bbase = lheight - lbase;
#endif

		if (wl->wl_LineFeed)
		{
		    wl->wl_Line->ln_Flags |= LNF_LF;
		    lastspace = -1;
		}

		wl->wl_Line->ln_Flags |= lnflags;
		wl->wl_Line->ln_Text = &buffer[anchor];
		wl->wl_Line->ln_TextLen = num;
		wl->wl_Line->ln_Baseline = lbase;
		wl->wl_Line->ln_Box.Left = (WORD) wl->wl_XOffset;
		wl->wl_Line->ln_Box.Top = (WORD) wl->wl_YOffset;
		wl->wl_Line->ln_Box.Width = (WORD) lwidth;
		wl->wl_Line->ln_Box.Height = (WORD) lheight;
		wl->wl_Line->ln_Style = wl->wl_Style;
		wl->wl_Line->ln_FgPen = wl->wl_FgPen;
		wl->wl_Line->ln_BgPen = wl->wl_BgPen;
		wl->wl_Line->ln_Font = wl->wl_Font;
		AddTail ((struct List *) &cd->cd_LineList, (struct Node *) wl->wl_Line);

#if MULTI_FONT
		wl->wl_LineHeight = MAX (wl->wl_LineHeight, lheight);
		wl->wl_Baseline   = MAX (wl->wl_Baseline, lbase);
#endif
		wl->wl_NumColumns = MAX (wl->wl_NumColumns, wl->wl_XOffset + lwidth);

		if (wl->wl_LineFeed)
		{
		    /* Make sure we have an anchor */
		    if (anchorLine)
		    {
			struct Line *jln1 = NULL, *jln2 = NULL;
			struct Line *tln;
			WORD left, width;

#if MULTI_FONT
			/* Set the correct line height */
			wl->wl_LineHeight = wl->wl_Baseline + wl->wl_BelowBase;
#endif

			/* Step through the list */
			tln = anchorLine;
			while (tln->ln_Link.mln_Succ)
			{
#if MULTI_FONT
			    /* Adjust the wl->wl_Baseline and the height */
			    tln->ln_Baseline   = wl->wl_Baseline;
			    tln->ln_Box.Height = wl->wl_LineHeight;
#endif

			    if (wl->wl_Justify != JLEFT)
			    {
				/* Find the first text */
				if (!jln1 && tln->ln_TextLen)
				    jln1 = jln2 = tln;
				else if (jln1 && tln->ln_TextLen)
				    jln2 = tln;
			    }

			    /* Get the next segment */
			    tln = (struct Line *) tln->ln_Link.mln_Succ;
			}

			/* We have to modify the Left edges of all the boxes */
			if ((wl->wl_Justify != JLEFT) && jln1 && jln2)
			{
			    width = jln2->ln_Box.Left + jln2->ln_Box.Width - jln1->ln_Box.Left;

			    if (wl->wl_Justify == JCENTER)
				left = (minwidth - width) / 2;
			    else
				left = minwidth - width;

			    if (left < 0)
				left = 0;

			    /* Step through the list */
			    tln = anchorLine;
			    while (tln->ln_Link.mln_Succ)
			    {
				/* Adjust the left edge */
				tln->ln_Box.Left += left;

				/* Get the next segment */
				tln = (struct Line *) tln->ln_Link.mln_Succ;
			    }
			}
		    }
		    else if (wl->wl_Justify == JCENTER)
			wl->wl_Line->ln_Box.Left = (minwidth - wl->wl_Line->ln_Box.Width) / 2;
		    else if (wl->wl_Justify == JRIGHT)
			wl->wl_Line->ln_Box.Left = minwidth - wl->wl_Line->ln_Box.Width;

		    if (wl->wl_Line->ln_Box.Left < 0)
			wl->wl_Line->ln_Box.Left = 0;

		    /* Adjust the left and top offset */
		    wl->wl_XOffset = wl->wl_LeftMargin + wl->wl_ParXAdjust;
		    wl->wl_YOffset += wl->wl_LineHeight;

#if MULTI_FONT
		    wl->wl_LineHeight = wl->wl_Baseline = 0;
#endif
		    anchorLine = NULL;

		    wl->wl_NumLines++;
		}
		else
		{
		    wl->wl_XOffset += wl->wl_XAdjust + lwidth;
		    if (!anchorLine)
			anchorLine = wl->wl_Line;
		}
	    }

	    num = 0;
	    oldanchor = anchor;
	    anchor = newanchor;
	    wl->wl_NewSeg = wl->wl_LineFeed = FALSE;
	    wl->wl_ParXAdjust = wl->wl_XAdjust = wl->wl_YAdjust = 0;
	}
    }

    /* Set the number of lines */
    cd->cd_TotVert  = wl->wl_YOffset;
    cd->cd_TotHoriz = wl->wl_NumColumns;
}
