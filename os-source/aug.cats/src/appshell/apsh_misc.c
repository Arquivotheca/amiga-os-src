/* apsh_misc.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * misc modules for the AppShell
 *
 * $Id: apsh_misc.c,v 1.4 1992/09/07 17:56:59 johnw Exp johnw $
 *
 * $Log: apsh_misc.c,v $
 * Revision 1.4  1992/09/07  17:56:59  johnw
 * Many minor and not-so-minor changes.
 *
 * Revision 1.1  91/12/12  14:52:27  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:35:42  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"
#include <exec/libraries.h>

#define	DB(x)	;
#define	DN(x)	;
#define	DQ(x)	;
#define	DE(x)	;
#define DL(x)   ;

extern void kprintf( char *, ... );

STRPTR __regargs GT_Lin( struct AppInfo *, STRPTR *, ULONG, STRPTR );
extern STRPTR __regargs GT_Loc( struct AppInfo *, ULONG, LONG );

LONG WhichOption (struct AppInfo *ai, STRPTR word)
{
    LONG retval = (-1L);

    if (ai && word)
    {
	LONG cntr;

	    /* Step through the arguments */
	    for (cntr = 0; cntr < ai->ai_Tempargc; cntr++)
	    {
		/* Do they match? */
		if (QStrCmpI (ai->ai_Tempargv[cntr], word))
		{
		    /* Set the return value */
		    retval = cntr;

		    /* Stop the search process */
		    cntr = ai->ai_Tempargc;
		}
	    }
    }

    return (retval);
}

/****** appshell.library/GetText *******************************************
*
*   NAME
*	GetText - Obtain a pointer to a permanent read-only text message.
*
*   SYNOPSIS
*	text = GetText(ai, base, id, def)
*	D0             D0  D1    A0  A1
*
*	STRPTR text;
*	struct AppInfo *ai;
*	ULONG base, id;
*	STRPTR def;
*
*   FUNCTION
*	Used to obtain a pointer to a permanent text message.  The text
*	return value must not be modified.
*
*	The main use for this function is for setting up text labels.
*
*	NOTE: The text table can not have any NULL entries, except to
*	mark the end of the table.  Due to the text table being exposed
*	to the public (via the FAULT command), this function makes sure
*	the number is within the valid range of the table.
*
*   EXAMPLE
*
*	\* sample function to show how to use GetText *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    STRPTR label;
*
*	    \* get the current text to use for Okay *\
*	    label = GetText (ai, APSH_MAIN_ID, APSH_OKAY_TXT, NULL);
*	}
*
*   INPUTS
*	ai	- Pointer to the AppInfo structure for this application.
*	base    - Text table to use.
*
*		  APSH_USER_ID for the application text table.
*		  APSH_MAIN_ID for the AppShell text table.
*		  or may provide a custom message handler base ID.
*
*	id	- Text Table entry id.
*	def	- Must be NULL.
*
*   SEE ALSO
*	PrepText()
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

struct AppString
{
	LONG 	as_ID;
	STRPTR	as_Str;
};

STRPTR GetText(struct AppInfo *ai, ULONG baseid, ULONG tid, STRPTR def)
{
    STRPTR deftext = NULL;
    STRPTR *table = NULL;
    ULONG index;

    DL(kprintf("\n** Start GetText\n"))
    DL(kprintf("**   ai: 0x%lx, baseid: 0x%lx, tid: 0x%lx, def:0x%lx\n",ai,baseid,tid,def))

    index = tid;

   	switch(baseid)
   	{

   	case APSH_USER_ID:	deftext = GT_Loc(ai,baseid,(LONG)tid);
   						break;

   	case APSH_MAIN_ID:	deftext = GT_Loc(ai,baseid,(LONG)tid);
   						break;

   	default:			{
					    register struct List *list;
					    register struct Node *node;
					    register struct MsgHandler *mh;
					    register struct MHObject *mho;

					    list = &(ai->ai_MsgList);
					    if (list->lh_TailPred != (struct Node *) list)
					    {
							/* step through the list of message handlers */
							node = list->lh_Head;
							while (node->ln_Succ)
							{
							    mh = (struct MsgHandler *) node;
							    mho = &(mh->mh_Header);

							    if (node->ln_Type == APSH_MH_HANDLER_T)
							    {
									if (mho->mho_ID == baseid)
									{
									    table = mh->mh_DefText;
									    break;
									}
						    	}

							    /* next node */
							    node = node->ln_Succ;
							}
					    }
					    deftext = GT_Lin(ai,table,tid,def);
						}
	}

	DL(kprintf("**   S[0x%lx,%ld]: %s\n",deftext,tid,deftext))
    DL(kprintf("** End GetText\n"))

    return (deftext);
}

/****** appshell.library/PrepText ******************************************
*
*   NAME
*	PrepText - Obtain a pointer to a temporarily modifiable text message.
*
*   SYNOPSIS
*	text = PrepText(ai, base, id, args, ...)
*	D0		D0  D1    A0  A1
*
*	STRPTR text;
*	struct AppInfo *ai;
*	ULONG base, id;
*	int args;
*
*   FUNCTION
*	Build a temporary text message using a text table entry and the
*	passed arguments.  The text table entry must contain valid
*	RawDoFmt formatting commands.
*
*	The text pointer is only valid until the next call to PrepText.
*	There is one PrepText buffer per AppInfo, so each cloned AppShell
*	has its own work buffer.  The string must be copied to a more
*	permanent storage place if you wish to keep it for a longer period.
*
*	The main use for this function is for formatting of temporary
*	error messages.
*
*   EXAMPLE
*
*	\* set up error return values *\
*	ai->ai_Pri_Ret = RETURN_FAIL;
*	ai->ai_Sec_Ret = APSH_PORT_ACTIVE;
*
*	\* "%s port already active", pname *\
*	ai->ai_TextRtn = PrepText(ai, APSH_MAIN_ID, ai->ai_Sec_Ret, pname);
*
*
*   INPUTS
*	ai	- Pointer to the AppInfo structure for this application.
*	base    - Text table to use.
*
*		  APSH_USER_ID for the application text table.
*		  APSH_MAIN_ID for the AppShell text table.
*		  or may provide a custom message handler base ID.
*
*	id	- Text Table entry id.
*	args	- Variables to be sprintf'ed into the text entry.
*
*   SEE ALSO
*	GetText()
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

STRPTR PrepText (ai, baseid, tid, va_alist)
    struct AppInfo *ai;
    ULONG baseid, tid;
    va_dcl;
{

    va_list args;
    char *format;
    int dog;

    format = GetText (ai, baseid, tid, NULL);

    va_start (args);
    dog = va_arg (args, int);
    sprintf (ai->ai_WorkText, format, (char *) dog);
    va_end (args);

    return ((STRPTR) & ai->ai_WorkText);
}

/****** appshell.library/ParseLine *****************************************
*
*   NAME
*	ParseLine - Destructive string parser
*
*   SYNOPSIS
*	argc = ParseLine (line, argv)
*	D0		  D0    D1
*
*	ULONG argc;
*	STRPTR line;
*	STRPTR argv[MAXARGS];
*
*   FUNCTION
*	String parser.  Inserts '\0' after each word in the passed text
*	line.
*
*   INPUTS
*	line     - Pointer to the string to parse.
*	argv     - Pointer to an array to hold the arguments.  The array
*		   must contain MAXARGS entries.
*
*   RETURN
*	argc     - Number of arguments returned in argv
*
*   EXAMPLE
*
*	\* sample function showing how to use ParseLine *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    UBYTE text[] = "ACTIVATE ME";
*	    STRPTR argv[MAXARGS];
*	    ULONG argc;
*
*	    \* parse the command line *\
*	    argc = ParseLine (text, argv);
*
*	    \* simple display of some of the parse variables *\
*	    printf("%ld words, first is %s\n", argc, argv[0]);
*	}
*
*   SEE ALSO
*	BuildParseLine(), FreeParseLine()
*
**********************************************************************
*
* Created:  11-June-90, David N. Junod
*
*/

#define QUOTE  34
#define SQUOTE 39

/* remember ',' is the decimal point in some countries */
#define isspace(c)  ((c==' ')||(c=='\t')||(c=='\n'))

ULONG ParseLine (STRPTR line, STRPTR * argv)
{
    STRPTR *pargv;
    ULONG argc;
    WORD i;
    WORD l;

    /* How long is the line */
    l = strlen (line);

    /* Clear the work areas */
    argc = 0L;
    for (i = 0; i < MAXARG; i++)
	argv[i] = NULL;

    /* Parse the arguments */
    while (argc < MAXARG)
    {
	/* Find the next character */
	while (isspace (*line))
	{
	    line++;
	}

	/* End of the string? */
	if (*line == '\0')
	{
	    break;
	}

	/* Get a pointer to the current argument slot */
	pargv = &argv[(UWORD)(argc++)];

	/* Is this line quoted? */
	if ((*line == QUOTE) || (*line == SQUOTE))
	{
	    /* Point to the inside of the quoted string */
	    *pargv = ++line;

	    /* Skip to the end of the quote */
	    while ((*line != '\0') && (*line != QUOTE) && (*line != SQUOTE))
	    {
		line++;
	    }

	    /* Is this the end of the string */
	    if (*line == '\0')
	    {
		return (argc);
	    }
	    else
	    {
		/* Terminate the argument */
		*line++ = '\0';
	    }
	}
	/* Non-quoted argument */
	else
	{
	    /* Point to the beginning of the word */
	    *pargv = line;

	    /* Skip to the end of the word */
	    while ((*line != '\0') && !(isspace(*line)) && !((*line==',')))
	    {
		line++;
	    }

	    /* Is this the end of the line? */
	    if (*line == '\0')
	    {
		break;
	    }
	    else
	    {
		/* Terminate argument */
		*line++ = '\0';
	    }
	}			/* End of else quoted argument */
    }				/* End of while arguments to parse */

    /* Return the number of arguments */
    return (argc);
}

/****** appshell.library/BuildParseLine ************************************
*
*   NAME
*	BuildParseLine - Non-destructive string parser
*
*   SYNOPSIS
*	handle = BuildParseLine (line, argc, argv)
*	D0			 D0    D1    A0
*
*	STRPTR handle;
*	STRPTR line;
*	ULONG *argc;
*	STRPTR argv[MAXARGS];
*
*   FUNCTION
*	This function is used to parse a string that may end up being
*	passed to another function.  It does not modify the passed
*	string.  Requires a corresponding call to FreeParseLine, when the
*	application's function is done with the parsed line.
*
*   INPUTS
*	line     - Pointer to the string to parse.
*	argc     - Pointer to variable to hold the number of arguments.
*	argv     - Pointer to an array to hold the arguments.  The array
*		   must contain MAXARGS entries.
*
*   RETURN
*	handle   - Pointer to a temporary work area which must be passed
*		   back to FreeParseLine when done with the parsed array.
*
*   EXAMPLE
*
*	\* sample function showing how to use BuildParseLine & FreeParseLine *\
*	function()
*	{
*	    STRPTR argv[MAXARGS], handle = NULL;
*	    ULONG argc;
*
*	    \* Parse the command line *\
*	    handle = BuildParseLine ("ACTIVATE ME", &argc, argv);
*
*	    \* Do something with the parsed command line.  The strings
*	     * must be copied before calling FreeParseLine if they are
*	     * going to be used later.
*	     *\
*	    printf("%ld words, first is %s\n", argc, argv[0]);
*
*	    \* free the BuildParseLine resources *\
*	    FreeParseLine(handle);
*	}
*
*   SEE ALSO
*	FreeParseLine(), ParseLine()
*
**********************************************************************
*
* Created:  11-June-90, David N. Junod
*
*/

STRPTR BuildParseLine (STRPTR line, ULONG * argc, STRPTR * argv)
{
    STRPTR clone = NULL;
    ULONG msize = strlen (line) + 2L;

    /* allocate memory for the cloned string */
    if (clone = (STRPTR) AllocVec (msize, MEMF_CLEAR))
    {
	/* clone it! */
	strcpy (clone, line);

	/* parse the string into the passed array */
	*argc = ParseLine (clone, argv);
    }

    /* return the parsed line */
    return (clone);
}

/****** appshell.library/FreeParseLine *************************************
*
*   NAME
*	FreeParseLine - Free the BuildParseLine temporary work area
*
*   SYNOPSIS
*	FreeParseLine(handle)
*		      D0
*
*	STRPTR handle;
*
*   FUNCTION
*	Free the temporary work space used by BuildParseLine.  A NULL is a
*	valid argument.
*
*   INPUTS
*	handle   - Pointer to the return value from BuildParseLine.
*
*   SEE ALSO
*	BuildParseLine(), ParseLine()
*
**********************************************************************
*
* Created:  11-June-90, David N. Junod
*
*/

VOID FreeParseLine (STRPTR clone)
{

    if (clone)
	FreeVec ((APTR) clone);
}

/****** appshell.library/FindType ******************************************
*
*   NAME
*	FindType - Find the value of a variable.
*
*   SYNOPSIS
*	value = FindType(argv, name, defvalue)
*	D0		 D0    D1    A0
*
*	STRPTR value;
*	STRPTR argv[MAXARGS];
*	STRPTR name, defvalue;
*
*   FUNCTION
*	This function searches a parsed text array for a given entry and
*	returns a pointer to the value bound to that entry.  If the entry is
*	not found, then a pointer to defvalue is returned.
*
*   EXAMPLE
*
*	\* sample fragment showing how to use FindType *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    STRPTR name;
*	    STRPTR argv[MAXARG], clone=NULL;
*	    ULONG argc;
*
*	    \* make sure we have a command line *\
*	    if (cmd)
*	    {
*		\* parse the command line *\
*		clone = BuildParseLine (cmd, &argc, argv);
*
*		\* Get the file name.  If there isn't a FILE keyword, then
*		 * use "config".  If the return value is to be used outside
*		 * of this function, then it must be copied.
*		 *\
*		name = FindType (argv, "FILE", "config");
*	    }
*
*	    \* free the BuildParseLine *\
*	    FreeParseLine (clone);
*	}
*
*   INPUTS
*	argv	 - Pointer to the preparsed text array.
*	name	 - Entry to search for.
*	defvalue - Value to return if name isn't found.
*
*   RESULTS
*	value	 - A pointer to the string that is the value bound to name or
*		   defvalue if name isn't found.
*
*   SEE ALSO
*	MatchValue(), BuildParseLine(), FreeParseLine(), ParseLine(),
*	icon.library/FindToolType
*
**********************************************************************
*
* Created:  11-June-90, David N. Junod
*
*/

STRPTR FindType (STRPTR * array, STRPTR name, STRPTR defvalue)
{
    UWORD len;
    STRPTR string;

    len = strlen (name);

    /* search all the strings in the array */
    while (string = *array++)
    {
	if (!stricmpn (string, name, len))
	{
	    string += len;
	    if (!*string || *string++ == '=')
	    {
		return (string);
	    }
	}
    }
    return (defvalue);
}

/****** appshell.library/MatchValue ****************************************
*
*   NAME
*	MatchValue - Check a text argument for a particular flag.
*
*   SYNOPSIS
*	success = MatchValue(entry, value)
*	D0		     D0     D1
*
*	BOOL success;
*	STRPTR entry, value;
*
*   FUNCTION
*	This function searchs to see if a particular text flag is set in
*	a text entry.
*
*   EXAMPLE
*
*	\* sample fragment showing how to use MatchValue *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    STRPTR flagstr;
*	    ULONG flags = NULL;
*	    STRPTR argv[MAXARG], clone=NULL;
*	    ULONG argc;
*
*	    \* make sure we have a command line *\
*	    if (cmd)
*	    {
*		\* parse the command line *\
*		clone = BuildParseLine (cmd, &argc, argv);
*
*		\* get the flag entry from the argument list *\
*		if (flagstr = FindType (argv, "FLAGS", NULL))
*		{
*		    \* see if the CLOSE flag is present *\
*		    if (MatchValue (flagstr, "CLOSE"))
*			flags |= CLOSEWINDOW;
*
*		    \* see if the SIZE flag is present *\
*		    if (MatchValue (flagstr, "SIZE"))
*			flags |= NEWSIZE;
*		}
*	    }
*
*	    \* free the BuildParseLine *\
*	    FreeParseLine (clone);
*	}
*
*   INPUTS
*	entry	 - Entry to search in.
*	value	 - Value to search for.
*
*   RESULTS
*	value	 - TRUE if the value was in the entry, otherwise returns
*		   FALSE.
*
*   SEE ALSO
*	FindType(), BuildParseLine(), FreeParseLine(), ParseLine(),
*	icon.library/MatchToolValue
*
**********************************************************************
*
* Created:  11-June-90, David N. Junod
*
*/

BOOL MatchValue (STRPTR type, STRPTR value)
{
    WORD vallen, typelen;
    STRPTR bar;

    vallen = strlen (value);

    while (type)
    {
	bar = (char *) stpchr (type, '|');	/* was index */
	if (bar)
	    typelen = bar - type;
	else
	    typelen = strlen (type);

	if (typelen == vallen)
	{
	    if (!stricmpn (type, value, vallen))
	    {
		return (TRUE);
	    }
	}
	if (bar)
	    type = bar + 1;	/* advance past the bar */
	else
	    type = 0;
    }
    return (FALSE);
}

/****** appshell.library/QStrCmpI ****************************************
*
*   NAME
*	QStrCmpI - Quick case insensitive string comparision.
*
*   SYNOPSIS
*	success = QStrCmpI (str1, str2)
*	D0		    D0    D1
*
*	BOOL value;
*	STRPTR str1, str2;
*
*   FUNCTION
*	This function performs a quick, case insensitive, string comparision.
*	Stops as soon as it determines that the strings are not the same.
*
*   EXAMPLE
*
*	\* sample code fragment showing how to use QStrCmpI *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    STRPTR name;
*	    STRPTR argv[MAXARG], clone=NULL;
*	    ULONG argc;
*
*	    \* make sure we have a command line *\
*	    if (cmd)
*	    {
*		\* parse the command line *\
*		clone = BuildParseLine (cmd, &argc, argv);
*
*		\* make sure we have some arguments *\
*		if (argc >= 2L)
*		{
*		    \* check to see if the first argument is CLOSE.
*		     * Note that FindType(argv, "CLOSE", NULL) could also be
*		     * used in this example.
*		     *\
*		    if (QStrCmpI (argv[1], "CLOSE"))
*		    {
*			\* do something because of CLOSE *\
*		    }
*		}
*	    }
*
*	    \* free the BuildParseLine *\
*	    FreeParseLine (clone);
*	}
*
*   INPUTS
*	str1	 - Pointer to the first string.
*	str2	 - Pointer to the second string.
*
*   RESULTS
*	value	 - TRUE if the strings are the same, otherwise returns FALSE.
*
**********************************************************************
*
* Created:  13-June-90, David N. Junod
*
*/

BOOL QStrCmpI (STRPTR str1, STRPTR str2)
{
    int len1, len2;
    BOOL success = FALSE;

    len1 = strlen (str1);
    len2 = strlen (str2);

    if (len1 == len2)
    {
	if (!stricmpn (str1, str2, len1))
	    success = TRUE;
    }

    return (success);
}

/****** appshell.library/RemoveMsgPort *************************************
*
*   NAME
*	RemoveMsgPort - Safely remove a message port.
*
*   SYNOPSIS
*	RemoveMsgPort (mp)
*		       D0
*
*	struct MsgPort *mp;
*
*   FUNCTION
*	This function will remove and reply to all messages that are
*	outstanding on a message port before removing the port itself.
*
*	NULL is a valid argument.
*
*   INPUTS
*	mp	- A pointer to the message port to delete.
*
**********************************************************************
*
* Created:  17-June-90, David N. Junod
*
*/

VOID RemoveMsgPort (struct MsgPort * mp)
{
    struct Message *msg, *succ;

    /* make sure that there is a message port to delete! */
    if (mp)
    {
	/* disable multitasking so the port won't go away */
	Forbid ();

	/* get a pointer to the first message in the list */
	msg = (struct Message *) mp->mp_MsgList.lh_Head;

	/* while there are messages... */
	while (succ = (struct Message *) msg->mn_Node.ln_Succ)
	{
	    /* remove the message from the list */
	    Remove ((struct Node *) msg);

	    /* reply the message */
	    ReplyMsg (msg);

	    /* get a pointer to the next message in the list */
	    msg = succ;
	}

	/* delete the message port */
	DeletePort (mp);

	/* enable multitasking now that we're done */
	Permit ();
    }
}

/****** appshell.library/NotifyUser ****************************************
*
*   NAME
*	NotifyUser - Display a text message to the user.
*
*   SYNOPSIS
*	success = NotifyUser (ai, msg, attrs)
*	D0		      D0  D1   A0
*
*	BOOL success;
*	struct AppInfo * ai;
*	STRPTR msg;
*	struct TagItem * attrs;
*
*   FUNCTION
*	This function will display a text message to the user.  Currently
*	uses EasyRequest (AutoRequest if OS version less than 2.0) to display
*	messages.
*
*   INPUTS
*
*	ai	- Optional pointer to the AppInfo structure for this
*		  application.
*
*	msg	- Pointer to message to display.  If this field is NULL, then
*		  the text will come from the ai->ai_TextRtn field.
*
*	attrs	- Optional pointer to an array of TagItems.
*
***********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

/*	(tl == NULL) && */

BOOL NotifyUser (struct AppInfo * ai, STRPTR msg, struct TagItem * tl)
{

    BOOL retval = TRUE;

    /* This should go */
    if (ai &&
	(msg == NULL) &&
	(ai->ai_Pri_Ret <= ai->ai_FailAt))
    {
	return (TRUE);
    }

    /* display the message */
    if (IntuitionBase)
    {
	/* are we using 2.0 or greater? */
	if (((struct Library *) IntuitionBase)->lib_Version >= 36)
	{
	    struct EasyStruct myEs;
	    STRPTR title, button, args, message;
	    struct Window *win;

	    /* some defaults for the text */
	    title = "System Request";
	    message = msg;
	    button = "Continue";
	    args = NULL;
	    win = NULL;

	    /* make sure we have a valid AppInfo structure */
	    if (ai)
	    {
		title = ai->ai_AppMsgTitle;
		win = ai->ai_Window;

		/* get the default text */
		button = GetText (ai, APSH_MAIN_ID, APSH_CONTINUE_TXT, button);

		if (!message)
		{
		    message = ai->ai_TextRtn;
		}
	    }

	    /* only display if there's a message */
	    if (message)
	    {
		/* set up for the EasyRequest function */
		myEs.es_StructSize = sizeof (struct EasyStruct);
		myEs.es_Flags = NULL;
		myEs.es_Title = title;
		myEs.es_TextFormat = message;
		myEs.es_GadgetFormat = button;

		/* call EasyRequest to display the message */
		retval = (BOOL)
		  EasyRequestArgs (win, &myEs, NULL, args);
	    }
	}
	else if (msg)
	{
	    SHORT arwidth, arheight = 60, arwadj = 60;
	    struct TextAttr tpz8;
	    struct IntuiText etext;
	    struct IntuiText ctext;

	    /* initialize the font information */
	    tpz8.ta_Name = "topaz.font";
	    tpz8.ta_YSize = 8;
	    tpz8.ta_Style = NULL;
	    tpz8.ta_Flags = NULL;

	    /* initialize the Continue button */
	    ctext.FrontPen = AUTOFRONTPEN;
	    ctext.BackPen = AUTOBACKPEN;
	    ctext.DrawMode = AUTODRAWMODE;
	    ctext.LeftEdge = AUTOLEFTEDGE;
	    ctext.TopEdge = AUTOTOPEDGE;
	    ctext.ITextFont = &tpz8;
	    ctext.IText = " CONTINUE ";
	    ctext.NextText = NULL;

	    /* initialize the message text */
	    etext.FrontPen = 2;
	    etext.BackPen = 1;
	    etext.DrawMode = JAM1;
	    etext.LeftEdge = 20;
	    etext.TopEdge = 15;
	    etext.ITextFont = &tpz8;
	    etext.IText = msg;
	    etext.NextText = NULL;

	    /* figure out the width to use */
	    arwidth = IntuiTextLength (&etext) + arwadj;

	    /* call AutoRequest to display the message */
	    retval = (BOOL)
	      AutoRequest (NULL, &etext, NULL, &ctext, 0, 0, arwidth, arheight);
	}
    }

    return (retval);
}

VOID SetPPointer (struct Window *win, struct PointerPref *pp)
{
    /* make sure we have a window pointer */
    if (win)
    {
	struct ObjectInfo *oi = (struct ObjectInfo *) win->UserData;
	struct WinNode *wn = (struct WinNode *) oi->oi_SystemData;

	/* Remember our pointer type */
	wn->wn_PP = pp;

	/* Set the pointer */
	SetPointer (win, pp->pp_PData, pp->pp_Height, pp->pp_Width,
			 pp->pp_XOffset, pp->pp_YOffset);
    }
}

/****** appshell.library/APSHSetWaitPointer ********************************
*
*   NAME
*	APSHSetWaitPointer - Display a wait pointer.
*
*   SYNOPSIS
*	APSHSetWaitPointer (ai, attrs)
*			    D0  D1
*
*	struct AppInfo * ai;
*	struct TagItem * attrs;
*
*   FUNCTION
*	This function displays a wait pointer in the specified window.
*	Defaults to the current window (according to AppShell).  The image
*	comes from ENV:<basename>/busypointer.ilbm
*
*	Valid TagItems are:
*
*	    APSH_WinName, <name>
*		where <name> is a valid window name.
*
*	    APSH_WinPointer, <pointer>
*		where <pointer> points to a valid window.
*
*   EXAMPLE
*
*	struct TagItem attr[2];
*
*	\* Set the wait pointer *\
*	attr[0].ti_Tag = APSH_WinName;
*	attr[0].ti_Data = (ULONG) "Main";
*	attr[1].ti_Tag = TAG_DONE;
*	APSHSetWaitPointer (ai, attr);
*
*	\* Bring up the ASL file requester *\
*	if (AslRequest (ad->ad_FR, SaveTags))
*	{
*	    \* Build up the complete project file name *\
*	    strcpy (ad->ad_Name, ad->ad_FR->rf_Dir);
*	    AddPart (ad->ad_Name, ad->ad_FR->rf_File, 256);
*
*	    \* Save the file *\
*	}
*
*	\* Clear the wait pointer *\
*	APSHClearPointer (ai, attr);
*
*   INPUTS
*	ai	- Optional pointer to the AppInfo structure for this
*		  application.
*	attrs	- Optional pointer to an array of TagItems.
*
*   SEE ALSO
*	APSHClearPointer
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

VOID APSHSetWaitPointer (struct AppInfo * ai, struct TagItem * tl)
{
    struct Window *win = NULL;
    struct PointerPref *pp;
    STRPTR name;

    /* Get the busy pointer */
    pp = GetPrefRecord (&(ai->ai_PrefList), PREFS_BUSYPOINTER);

    if (ai && pp)
    {
	if (win = (struct Window *) GetTagData (APSH_WinPointer, NULL, tl))
	{
	}
	else if (name = (STRPTR) GetTagData (APSH_WinName, (ULONG)"Main", tl))
	{
	    APSHGetWindowInfo (ai, name, (ULONG *) & win);
	}

	/* supply a default window pointer */
	if (!win)
	{
	    win = ai->ai_Window;
	}

	SetPPointer (win, pp);
    }
}

/****** appshell.library/APSHClearPointer **********************************
*
*   NAME
*	APSHClearPointer - Restore the default pointer.
*
*   SYNOPSIS
*	APSHClearPointer (ai, attrs)
*			  D0  D1
*
*	struct AppInfo * ai;
*	struct TagItem * attrs;
*
*   FUNCTION
*	This function restores the default pointer for the specified window.
*	Defaults to the current window (according to AppShell).  The image
*	comes from ENV:<basename>/pointer.ilbm
*
*	Valid TagItems are:
*
*	    APSH_WinName, <name>
*		where <name> is a valid window name.
*
*	    APSH_WinPointer, <pointer>
*		where <pointer> points to a valid window.
*
*   INPUTS
*	ai	- Optional pointer to the AppInfo structure for this
*		  application.
*	attrs	- Optional pointer to an array of TagItems.
*
*   SEE ALSO
*	APSHSetWaitPointer
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

VOID APSHClearPointer (struct AppInfo * ai, struct TagItem * tl)
{
    struct Window *win = NULL;
    struct PointerPref *pp;
    STRPTR name;

    /* Get the busy pointer */
    pp = GetPrefRecord (&(ai->ai_PrefList), PREFS_POINTER);

    if (ai && pp)
    {
	if (win = (struct Window *) GetTagData (APSH_WinPointer, NULL, tl))
	{
	}
	else if (name = (STRPTR) GetTagData (APSH_WinName, (ULONG)"Main", tl))
	{
	    APSHGetWindowInfo (ai, name, (ULONG *) & win);
	}

	/* supply a default window pointer */
	if (!win)
	{
	    win = ai->ai_Window;
	}

	/* Set the pointer to the default */
	SetPPointer (win, pp);
    }
}

/*------------------------------------------------------------------------*/
/* string manipulation routines                                           */
/*------------------------------------------------------------------------*/

int atoi (char *ptr)
{
    char c;
    int result = 0, sign = 1;

    if (*ptr == '-')
    {
	sign = (-1);
	ptr++;
    }

    while (c = *ptr++)
    {
	if (c < '0' || c > '9')
	    continue;
	result = result * 10 + (int) (c - '0');
    }

    return (result * sign);
}

char *strupr (char *str)
{
    UWORD p, len;

    if ((len = strlen (str)) > 0)
	for (p = 0; p < len; p++)
	    str[p] = ToUpper (str[p]);
    return (str);
}


/****** appshell.library/LMatchFirst ***************************************
*
*   NAME
*	LMatchFirst - Finds node that matches pattern.
*
*   SYNOPSIS
*	error = LMatchFirst(list, pat, anchor);
*	D0		    D0    D1   D2
*
*   FUNCTION
*	Locates the first node that matches a given pattern.  This function
*	is the same as the corresponding DOS function MatchFirst except
*	that this function operates on an Exec list instead of the file
*	system.
*
*	This set of functions is case insensitive.  Set the
*	APSHF_ANCHOR_NOCASE flag in the al_Flags field to get a caseless
*	comparision.
*
*   INPUTS
*	list	- List to search through.
*	pat	- Pattern to search for.
*	anchor	- Bookmark for the search.
*
*   EXAMPLES
*
*	\* sample function showing how to step through a list of
*	 * of arguments and expand wildcards, if any *\
*	VOID StubFunc (struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    struct AppData *ad = ai->ai_UserData;
*	    struct List *list = &(ad->ad_MyList);
*	    struct AnchorList al = {NULL};
*	    LONG retval = RETURN_ERROR;
*	    STRPTR *sptr = NULL, name;
*	    struct Funcs *fe;
*
*	    \* make sure that we have a tag list *\
*	    if (tl)
*	    {
*		\* get a pointer to the function entry *\
*		if (fe = (struct Funcs *)GetTagData(APSH_FuncEntry, NULL, tl))
*		{
*		    \* our first argument is a MultiArg *\
*		    sptr = (STRPTR *) fe->fe_Options[0];
*		}
*	    }
*
*	    \* make sure we have an argument array to step thru *\
*	    if (sptr)
*	    {
*		\* step thru the argument array *\
*		while (*sptr)
*		{
*		    for (retval = LMatchFirst (list, *sptr, &al);
*			 retval == 0L;
*			 retval = LMatchNext (&al))
*		    {
*			\* get the current node name *\
*			name = al.al_CurNode.ln_Name;
*
*			\* do something with the name/node *\
*			printf ("%s\n", name);
*		    }
*
*		    \* end the pattern match *\
*		    LMatchEnd (&al);
*
*		    \* next name/pattern *\
*		    sptr++;
*
*		} \* end of while arguments *\
*	    } \* end of if arguments *\
*	}
*
*   RESULT
*	error	- 0 for success or error code from <dos/dos.h>
*
*   SEE ALSO
*	LMatchNext
*	LMatchEnd
*	dos/MatchFirst
*	dos/MatchNext
*	dos/MatchEnd
*
**********************************************************************
*
* Created:  09-Jul-90, David N. Junod
*
*/

#ifndef APSHF_ANCHOR_NOCASE
#define	APSHF_ANCHOR_NOCASE	(1L<<0)
#endif
#define	APSH_LMATCH_NOWILD	(1L<<1)

LONG LMatchFirst (struct List * list, STRPTR pat, struct AnchorList * al)
{
    LONG msize, test;
    LONG retval = RETURN_OK;

    /* clear any flags */
    al->al_Flags &= ~(APSH_LMATCH_NOWILD);;
    al->al_CurNode = NULL;

    if (list->lh_TailPred != (struct Node *) list)
    {
	/* compute space needed */
	msize = strlen (pat) + 20L;

	/* allocate room for the pattern token */
	if (al->al_Token = (STRPTR) AllocVec (msize, MEMF_CLEAR))
	{
	    /* tokenize the pattern */
	    if (al->al_Flags & APSHF_ANCHOR_NOCASE)
	    {
		test = ParsePatternNoCase (pat, al->al_Token, msize);
	    }
	    else
	    {
		test = ParsePattern (pat, al->al_Token, msize);
	    }

	    DN (kprintf ("LMF: PP(%s, %s, %ld)==%ld\n",
			 pat, al->al_Token, msize, test));

	    if (test == 0)
	    {
		/* don't match any more! */
		al->al_Flags |= APSH_LMATCH_NOWILD;

		/* it wasn't a wild card */
		if (!(al->al_CurNode = FindNameI (list, pat)))
		    retval = ERROR_NO_MORE_ENTRIES;
	    }
	    else if (test == 1)
	    {
		/* initialize the current node */
		al->al_NxtNode = al->al_CurNode = (struct Node *) list->lh_Head;

		/* okay, let's start */
		retval = LMatchNext (al);
	    }
	    else
	    {
		DN (kprintf ("LMF: couldn't parse\n"));

		retval = ERROR_BAD_TEMPLATE;
	    }
	}
	else
	{
	    /* not enough memory */
	    DN (kprintf ("LMF: not enough memory\n", (LONG) retval));

	    retval = ERROR_NO_FREE_STORE;
	}
    }
    else
    {
	/* empty list */
	DN (kprintf ("LMF: empty list\n", (LONG) retval));

	retval = ERROR_NO_MORE_ENTRIES;
    }

    DN (kprintf ("LMF: exit %ld, 0x%lx\n", (LONG) retval, al->al_CurNode));

    return (retval);
}

LONG LMatchNext (struct AnchorList * al)
{
    BOOL test;

    if (!(al->al_Flags & APSH_LMATCH_NOWILD))
    {
	/* step through the list of message handlers */
	while (al->al_NxtNode->ln_Succ)
	{
	    if (al->al_NxtNode->ln_Name)
	    {
		if (al->al_Flags & APSHF_ANCHOR_NOCASE)
		{
		    test = MatchPatternNoCase (al->al_Token, al->al_NxtNode->ln_Name);
		}
		else
		{
		    test = MatchPattern (al->al_Token, al->al_NxtNode->ln_Name);
		}

		DN (kprintf ("LMN: MP (%s, %s) == %ld\n",
			     al->al_Token, al->al_NxtNode->ln_Name, (LONG) test));

		/* remember where we are */
		al->al_CurNode = al->al_NxtNode;

		/* next node */
		al->al_NxtNode = al->al_NxtNode->ln_Succ;

		if (test)
		{
		    /* it matches */
		    return (RETURN_OK);
		}
	    }
	}

	/* no more */
	al->al_Flags |= APSH_LMATCH_NOWILD;
    }

    return (ERROR_NO_MORE_ENTRIES);
}

VOID LMatchEnd (struct AnchorList * al)
{

    DN (kprintf ("LME: al 0x%lx\n", al));

    /* make sure they passed something */
    if (al)
    {
	/* if there is a pattern string, then get rid of it */
	if (al->al_Token)
	    FreeVec (al->al_Token);
    }
}

/*
 * This routine will Enqueue an item to the list in alpha
 * order based on the ln_Name field...
 */
void AlphaEnqueue (struct List * list, struct Node * node)
{
    register struct Node *insert;

    /*
     * We first just add it to the end
     */
    DQ (kprintf ("L $%08lx, T%ld, H $%08lx, P $%08lx, N $%08lx Name %s\n", list, (LONG) list->lh_Type, list->lh_Head, list->lh_TailPred, node, node->ln_Name));

    AddTail (list, node);

    /*
     * Now, run through the list to find the correct place
     */
    insert = list->lh_Head;
    while (insert != node)
    {
	if (stricmp (insert->ln_Name, node->ln_Name) > 0)
	{
	    RemTail (list);
	    if (insert == list->lh_Head)
		AddHead (list, node);
	    else
		Insert (list, node, insert->ln_Pred);
	    insert = node;
	}
	else
	    insert = insert->ln_Succ;
    }
    DQ (kprintf ("L $%08lx, H $%08lx, P $%08lx, NS $%08lx NP $%08lx\n", list, list->lh_Head, list->lh_TailPred, node->ln_Succ, node->ln_Pred));
}

#if 0
VOID HandleList (
	struct List * list,
	VOID (*func)(struct Node *, ULONG, void *, ...),
	void *data,...);
#endif

/* Perform an operation on all members of a list */
VOID HandleList (
	struct List * list,
	VOID (*func)(struct Node *, ULONG, void *, ...),
	VOID *data, ...)
{
    /* Make sure we have a list */
    if (list)
    {
	/* Make sure there are entries in the list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;
	    ULONG cnt = 0L;

	    /* Let's start at the very beginning... */
	    node = list->lh_Head;

	    /* Continue while there are still nodes */
	    while (nxtnode = node->ln_Succ)
	    {
		(*(func)) (node, cnt, (VOID *)&data);

		/* Go on to the next node */
		node = nxtnode;
		cnt++;
	    }
	}
    }
}

ULONG __asm APSHSignal (register __a1 struct AppInfo *ai, register __d0 LONG bit)
{
    ULONG retval = 0L;

    if (ai)
    {
	retval = (LONG) ai->ai_Process;

	if (bit)
	{
	    /* Send a break signal */
	    Signal ((struct Task *) ai->ai_Process, bit);
	}
    }

    return (retval);
}

STRPTR __regargs GT_Lin
(
struct AppInfo *ai,
STRPTR *table,
ULONG tid,
STRPTR def
)
{
	register LONG	max = 0L;
	register STRPTR	txt = NULL;

	DL(kprintf("**   ** Start GT_Lin\n"))

    if (table)
    {

		while (table[max++]);

		if (tid < max)
		{
	    	txt = table[tid];
		    DL(kprintf("**   **   Found: %ld %ld %s\n", tid, max, txt))
	    }
		else
		{
		    DL(kprintf("**   **   Over: %ld %ld\n", tid, max))
		}
	}
    else
    {
		DL(kprintf("**   **   No text table!\n"));
    }

    if (!txt)
    {
    	DL(kprintf("**   **   Default used: %s\n",def))
    	txt = def;
    }

	DL(kprintf("**   ** End GT_Lin\n"))
	return(txt);
}

