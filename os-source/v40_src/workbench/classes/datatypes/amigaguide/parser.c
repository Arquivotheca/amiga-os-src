/* parser.c
 *
 */

#include "amigaguidebase.h"


/*****************************************************************************/

#define	USE_SQUOTE	FALSE

/*****************************************************************************/

ULONG ParseString (struct AGLib * ag, STRPTR line, STRPTR *argv, ULONG max)
{
    int inesc = 0;
    STRPTR *pargv;
    BOOL inquote;
    int ns, nd;
    ULONG argc;
    int i;

    argc = 0L;
    for (i = 0; i < max; i++)
	argv[i] = NULL;

    while (argc < max)
    {
	while (isspace (*line))
	    line++;
	if (*line == '\0')
	    break;

	pargv = &argv[(UWORD) (argc++)];
#if USE_SQUOTE
	if ((*line == QUOTE) || (*line == SQUOTE))
#else
	if (*line == QUOTE)
#endif
	{
	    inquote = TRUE;
	    ns = nd = 0;

	    if (*line == QUOTE)
		nd++;
#if USE_SQUOTE
	    else if (*line == SQUOTE)
		ns++;
#endif

	    *pargv = ++line;
	    while ((*line != 0L) && inquote)
	    {
		if (*line == BACKSLASH)
		    inesc = 1;
		else if (inesc > 0)
		    inesc++;

		if ((inesc == 0) && (*line == QUOTE))
		    nd++;
#if USE_SQUOTE
		else if ((inesc == 0) && (*line == SQUOTE))
		    ns++;
#endif

		if (EVEN (ns) && EVEN (nd))
		    inquote = FALSE;
		else
		    line++;

		if (inesc > 1)
		    inesc = 0;
	    }

	    if (*line == '\0')
		return (argc);
	    else
		*line++ = '\0';
	}
	else
	{
	    *pargv = line;
	    while ((*line != '\0') && (!isspace (*line)))
		line++;
	    if (*line == '\0')
		break;
	    else
		*line++ = '\0';
	}
    }

    argv[argc] = NULL;
    return (argc);
}
