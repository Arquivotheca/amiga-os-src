/* ParsePathStringLVO.c
 *
 */

#include "amigaguidebase.h"

ULONG ASM LVOParsePathString (REG (a6) struct AGLib *ag, REG (d0) STRPTR line, REG (a0) STRPTR * argv, REG (d1) ULONG max)
{
    int inesc = 0;
    STRPTR *pargv;
    ULONG argc;
    int i;

    /* clear the work areas */
    argc = 0L;
    for (i = 0; i < max; i++)
    {
	argv[i] = NULL;
    }

    /* parse the arguments */
    while (argc < max)
    {
	/* skip past all the spaces */
	while (isspace (*line))
	{
	    line++;
	}

	/* see if end of line */
	if (*line == '\0')
	{
	    break;
	}

	/* indicate a boundery point */
	pargv = &argv[(UWORD) (argc++)];

	/* see if quotes have started */
	if ((*line == QUOTE) || (*line == SQUOTE))
	{
	    BOOL inquote;
	    int ns, nd;

	    /* indicate that we're inside quotes */
	    inquote = TRUE;

	    /* clear the quote counters */
	    ns = nd = 0;

	    /* count the quote type */
	    if (*line == SQUOTE)
	    {
		/* increment the single quote counter */
		ns++;
	    }
	    else if (*line == QUOTE)
	    {
		/* increment the double quote counter */
		nd++;
	    }

	    /* ptr inside quoted string */
	    *pargv = ++line;

	    /* step until end of quoting */
	    while ((*line != 0L) && inquote)
	    {
		if (*line == BACKSLASH)
		{
		    inesc = 1;
		}
		else if (inesc > 0)
		{
		    inesc++;
		}

		/* count the quote type */
		if ((inesc == 0) && (*line == SQUOTE))
		{
		    ns++;
		}
		else if ((inesc == 0) && (*line == QUOTE))
		{
		    nd++;
		}

		/* see if end of quotes */
		if (EVEN (ns) && EVEN (nd))
		{
		    /* indicate end of quotes */
		    inquote = FALSE;
		}
		else
		{
		    /* increment position */
		    line++;
		}

		if (inesc > 1)
		{
		    inesc = 0;
		}
	    }			/* end of while inside quotes */

	    /* see if found end of string */
	    if (*line == '\0')
		return (argc);
	    else
		*line++ = '\0';	/* terminate arg */
	}
	else
	    /* non-quoted arg */
	{
	    *pargv = line;
	    while ((*line != '\0') && (!isspace (*line)))
		line++;
	    if (*line == '\0')
		break;
	    else
		*line++ = '\0';	/* terminate arg */
	}
    }				/* while */

    /* Make sure the list is terminated properly */
    argv[argc] = NULL;

    return (argc);
}
