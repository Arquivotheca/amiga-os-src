/* amigaguidebase.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)		;
#define	MAXPATHS	64

/****** amigaguide/amigaguide.datatype ****************************************
*
*    NAME
*	amigaguide.datatype -- data type for AmigaGuide databases.
*
*    FUNCTION
*	The amigaguide.datatype is the DataTypes class for AmigaGuide
*	databases.
*
*    METHODS
*	OM_NEW -- Create a new AmigaGuide object.
*
*	OM_GET -- Obtain the value of an attribute.
*
*	OM_SET -- Set the values of multiple attributes.
*
*	OM_UPDATE -- Update the values of multiple attributes.
*
*	OM_DISPOSE -- Dispose of an AmigaGuide object.
*
*	GM_LAYOUT -- Layout the object and notify the application of the
*	    title and size.
*
*	GM_HITTEST -- Determine if the object has been hit with the
*	    mouse.
*
*	GM_GOACTIVE -- Tell the object to go active.
*
*	GM_HANDLEINPUT -- Handle input.
*
*	GM_RENDER -- Cause the AmigaGuide database to render.
*
*	DTM_GOTO -- Cause the AmigaGuide database to load and display
*	    document.
*
*	DTM_TRIGGER -- Trigger an event.
*
*	    STM_COMMAND - Invoke an AmigaGuide command.
*
*	    STM_CONTENTS - Display the table of contents.
*
*	    STM_INDEX - Display the index.
*
*	    STM_HELP - Display the help file.
*
*	    STM_RETRACE - Retrace.
*
*	    STM_BROWSE_PREV - Go to the previous sequential document.
*
*	    STM_BROWSE_NEXT - Go to the next sequential document.
*
*	DTM_PROCLAYOUT -- Layout (remap) the AmigaGuide database on the
*	    application's process.
*
*	DTM_FRAMEBOX -- Obtain the display environment that the
*	    AmigaGuide database requires.
*
*	DTM_SELECT -- Select an area in the AmigaGuide database.
*
*	DTM_CLEARSELECTED -- Deselect the selected area of the
*	    AmigaGuide database.
*
*	DTM_COPY -- Copy the selected area of the text to the clipboard
*	    as FTXT.  If no area is selected, then the entire text
*	    is copied.
*
*	DTM_PRINT -- Print the selected area of the text.  If no area
*	    is selected, then the entire text is printed.
*
*	DTM_WRITE -- Write the selected area of the text to a file.
*	    If no area is selected, then the entire text is saved.
*
*    TAGS
*	DTA_TriggerMethods -- List of the trigger methods supported.
*
*	    Applicability is (G).
*
*	DTA_Methods -- List of the methods supported.
*
*	    Applicability is (G).
*
*	DTA_TextAttr (struct TextAttr *) -- Text attribute to use for
*	    the AmigaGuide database.
*
*	    Applicability is (ISG).
*
*	DTA_Name (STRPTR) -- Name of the AmigaGuide database.
*
*	    Applicability is (I).
*
*	DTA_Handle (BPTR) -- File handle for the AmigaGuide database.
*
*	    Applicability is (I).
*
*	DTA_NodeName (STRPTR) -- Document to display.
*
*	    Applicability is (I).
*
*	TDTA_WordDelim (STRPTR) -- Characters used to deliminate words.
*
*	    Applicability is (IS).
*
*	AGA_HelpGroup (ULONG) -- Help group that the AmigaGuide object
*	    belongs in.
*
*	    Applicability is (I).
*
*    COMMANDS
*	Commands must start in the very first column of a line, and
*	can be the only thing on the line.  If a line begins with an
*	@ sign, then it is interpreted as a command.
*
*	The following commands can be used in the global section
*	of a database.
*
*	  @$VER: <AmigaDOS version string>
*	  Specify the version of the database.  This command
*	  must always be in upper-case.
*
*	  @(C) <copyright>
*	  Specify the copyright notice for the database.
*
*	  @AUTHOR <name>
*	  Specify the author of the database.
*
*	  @DATABASE <name>
*	  Must be the very first line of an AmigaGuide document.
*
*	  @DNODE <name>
*	  Indicates the start of a dynamic node.  The AmigaGuide system
*	  uses the callback hooks to obtain the document from a document
*	  provider.  This is obsolete, do not use.
*
*	  @FONT <name> <size>
*	  Specify the font to use for the database.
*
*	  @HEIGHT <chars>
*	  How high, in characters, the largest document is.
*
*	  @HELP <name/node>
*	  Specify the name of the help node, which will be accessed
*	  by the Help button.  Can be a node in an external database.
*
*	  @INDEX <name/node>
*	  Specify the name of the index node, which will be accessed
*	  by the Index button.  Can be a node in an external database.
*
*	  @MACRO <name> <commands>
*	  This is used to construct a macro.  A macro can be used the
*	  same way as an attribute command, and can only consist of
*	  other attribute commands or macros.  The macro list is
*	  searched before the attribute command list is searched.  This
*	  is new for V40.
*
*	      @macro icom "@{i}$1 @{ui}"
*
*	      ...
*
*	      This is an @{icom "example"} of macro.
*
*	  @MASTER <path>
*	  Complete path of the source document used to define this
*	  AmigaGuide database.
*
*	  @NODE <name> <title>
*	  Indicate the start of a node (page/article/section). The first
*	  node, or main node, must be named MAIN.  MAIN must be the master
*	  table of contents for the database.
*
*	  @ONCLOSE <rxcommand>
*	  This is used to specify an ARexx macro file to execute when
*	  the database is closed.  The return value of the script has no
*	  effect on the database.  New for V40.
*
*	  @ONOPEN <rxcommand>
*	  This is used to specify an ARexx macro file to execute when
*	  the database is opened.  If the script returns an error, then
*	  the database will not be opened.  New for V40.
*
*	  @REM <remark>
*	  @REMARK <remark>
*	  This is used to place remarks in the database.  These remarks
*	  are not displayed to the user.
*
*	  @SMARTWRAP
*	  This is used to indicate that the text of the database is to
*	  wordwrapped using a more intelligent algorithm than @wordwrap.
*	  A paragraph doesn't need to be restricted to one physical
*	  line, but paragraphs must be separated by two line feeds.
*	  New for V40.
*
*	  @TAB <num>
*	  Number of spaces in a tab.  Defaults to 8.  New for V40.
*
*	  @WIDTH <chars>
*	  How wide, in characters, the largest document is.
*
*	  @WORDWRAP
*	  Turn on wordwrapping for the whole database.  A paragraph must
*	  be restrained to one physical line.
*
*	The following commands can be used within nodes of a database.
*
*	  \
*	  A backslash is the escape character.  A backslash in front of
*	  the @ sign is used to escape it.
*
*	  @ENDNODE <name>
*	  Indicate the end of a node.  Must start at the beginning of a
*	  line.
*
*	  @FONT <name> <size>
*	  Specify the font to use for the node.
*
*	  @HELP <name/node>
*	  Specify the name of the help node, which will be accessed
*	  by the Help button.  Can be a node in an external database.
*
*	  @INDEX <name/node>
*	  Specify the name of the index node, which will be accessed
*	  by the Index button.  Can be a node in an external database.
*
*	  @KEYWORDS <keywords>
*	  Keywords of the node.  Someday when searching is
*	  reimplemented, there will be a keyword search.
*
*	  @NEXT <node name>
*	  Node to display when the user selects "Browse >"
*
*	  @ONCLOSE <rxcommand>
*	  This is used to specify an ARexx macro file to execute when
*	  the node is closed.  The return value of the script has no
*	  effect on the node.  New for V40.
*
*	  @ONOPEN <rxcommand>
*	  This is used to specify an ARexx macro file to execute when
*	  the node is opened.  If the script returns an error, then
*	  the node will not be opened.  New for V40.
*
*	  @PREV <node name>
*	  Node to display when the user selects "< Browse"
*
*	  @SMARTWRAP
*	  This is used to indicate that the text of the node is to
*	  wordwrapped using a more intelligent algorithm than @wordwrap.
*	  A paragraph doesn't need to be restricted to one physical
*	  line, but paragraphs must be separated by two line feeds.
*	  New for V40.
*
*	  @TAB <num>
*	  Number of spaces in a tab.  Defaults to 8.  New for V40.
*
*	  @TITLE <title>
*	  Title to display in the title bar of the window during the
*	  display of this node.  Must start at the beginning of a line.
*
*	  @TOC <node name>
*	  Name of the node that contains the table of contents for this
*	  node.  Defaults to MAIN.  This is the node that is displayed
*	  when the user presses the "Contents" button.
*
*	  @WORDWRAP
*	  Turn on wordwrapping for the node.  A paragraph must
*	  be restrained to one physical line.
*
*	  @{<label> <command>}
*	  Indicate a textual link point.  Can be anywhere in a line.
*	  Starting with 3.0, AmigaGuide can can link to graphics,
*	  sounds, animations and other DataTypes.
*
*    ATTRIBUTES
*	Following is a list of attributes that can be applied to the
*	text of a node.
*
*	  @{AMIGAGUIDE}
*	  Displays the word AmigaGuide in bold followed by the � symbol.
*	  New for V40.
*
*	  @{APEN}
*	  Use to change the foreground color to a specific pen number.
*	  New for V40.
*
*	  @{B}
*	  Turn bold on.
*
*	  @{BG <color>}
*	  Used to change the background text color.  Color can be:
*
*	    Text
*	    Shine
*	    Shadow
*	    Fill
*	    FillText
*	    Background
*	    Highlight
*
*	  @{BODY}
*	  Indicate that the following text is the body of the document.
*	  Word wrap will be turned back on if it is the default.  New
*	  for V40.
*
*	  @{BPEN}
*	  Use to change the background color to a specific pen number.
*	  New for V40.
*
*	  @{CLEARTABS}
*	  Restore the default tab stops.  New for V40.
*
*	  @{CODE}
*	  Indicate that the following text is not to be word-wrapped.
*	  New for V40.
*
*	  @{FG <color>}
*	  Used to change the foreground color.  The same colors can be
*	  used as in the FG command.
*
*	  @{I}
*	  Turn italic on.
*
*	  @{JCENTER}
*	  Turn on centering.  New for V40.
*
*	  @{JLEFT}
*	  Turn on left justification.  New for V40.
*
*	  @{JRIGHT}
*	  Turn on right justification.  New for V40.
*
*	  @{LINDENT}
*	  Set the number of spaces to indent the body of a paragraph.
*	  New for V40.
*
*	  @{LINE}
*	  Force a line feed without starting a new paragraph.  New for V40.
*
*	  @{PAR}
*	  Used to indicate the end of a paragraph.  This is the same as
*	  two sequential LF's in the source file.  New for V40.
*
*	  @{PARD}
*	  Restore the default settings for a paragraph.  Text pen to 1,
*	  background to 0, normal font, and no indentation.  New for V40.
*
*	  @{PARI}
*	  Set the number of spaces to indent the first line of a
*	  paragraph relative to the normal paragraph indentation.  The
*	  value may be a negative number.  New for V40.
*
*	  @{PLAIN}
*	  Used to turn off all @{B}, @{I}, and @{U} commands.  New for
*	  V40.
*
*	  @{SETTABS <n> ... <n>}
*	  This is used to establish tab stops.  New for V40.
*
*	  @{TAB}
*	  The same as character 9 in the source file.  New for V40.
*
*	  @{U}
*	  Turn underline on.
*
*	  @{UB}
*	  Turn bold off.
*
*	  @{UI}
*	  Turn italic off.
*
*	  @{UU}
*	  Turn underline off.
*
*    AREXX COMMANDS
*	AmigaGuide supports the following ARexx commands.
*
*	  BEEP
*	  DisplayBeep().
*
*	  CLOSE
*	  Close the current database.
*
*	  GETNODECOUNT
*	  Returns the number of nodes in the database using the RESULT
*	  variable.  New for V40.
*
*	  LINK
*	  Go to the named node.
*
*	  NEXT
*	  Go to the next physical node in the database.  Same as
*	  pressing the "Browse >" button.  New for V40.
*
*	  PREVIOUS
*	  Go to the previous physical node in the database.  Same as
*	  pressing the "Browse <" button.  New for V40.
*
*	  PRINT
*	  Print the current node.  Doesn't return until complete.  New for
*	  V40.
*
*	  QUIT
*	  Close the current database.
*
*	  RETRACE
*	  Go to the previous node in the database.  Same as pressing the
*	  "Retrace" button.  New for V40.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

Class *ASM ObtainAGEngine (REG (a6) struct AGLib * ag)
{

    return (ag->ag_Class);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct AGLib * ag, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{

    /* Initialize the library base */
    ag->ag_SegList = seglist;
    ag->ag_SysBase = sysbase;

    InitSemaphore (&ag->ag_Lock);

    NewList ((struct List *) &ag->ag_DatabaseList);
    NewList ((struct List *) &ag->ag_HostList);

    /* Make sure we're running on the correct version of the OS */
    if (sysbase->lib_Version >= 39)
    {
	/* Open the ROM libraries */
	DOSBase = OpenLibrary ("dos.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);
	GfxBase = OpenLibrary ("graphics.library", 39);
	UtilityBase = OpenLibrary ("utility.library", 39);
	LayersBase = OpenLibrary ("layers.library", 39);
	return (ag);
    }

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct AGLib * ag)
{
    struct ExecBase *eb = (struct ExecBase *) ag->ag_SysBase;
    LONG retval = (LONG) ag;
    BOOL success = FALSE;
    BYTE nest;

    /************/
    /* FORBID() */
    /************/
    ObtainSemaphore (&(ag->ag_Lock));
    nest = eb->TDNestCnt;
    Permit ();

    /* Use an internal use counter */
    ag->ag_UsageCnt++;
    ag->ag_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (ag->ag_UsageCnt == 1)
    {
	ag->ag_Token = ObtainXRefToken (ag);

	if (ag->ag_Class == NULL)
	{
	    /* Open disk libraries here */
	    ag->ag_RexxSysBase = OpenLibrary ("rexxsyslib.library", 0);
	    if (ag->ag_LocaleBase = OpenLibrary ("locale.library", 38))
#if 1
		ag->ag_Catalog = OpenCatalogA (NULL, "Sys/amigaguide.catalog", NULL);
#else
		ag->ag_Catalog = OpenCatalogA (NULL, "FUCK:amigaguide.catalog", NULL);
#endif
	    if (ag->ag_DiskfontBase = OpenLibrary ("diskfont.library", 0))
		if (ag->ag_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		    if (ag->ag_DataTypesBase = OpenLibrary ("datatypes.library", 0))
			if (ag->ag_Class = initClass (ag))
			    if (ag->ag_ModelClass = initModelClass (ag))
				if (ag->ag_DatabaseClass = initDBClass (ag))
				    if (ag->ag_NodeClass = initHNClass (ag))
					success = TRUE;
	}
    }
    else
    {
	success = TRUE;
    }

    if (success)
    {
	STRPTR names[MAXPATHS + 1];
	STRPTR buff = NULL;
	LONG i, num, len;
	ULONG msize;
	BPTR fh;

	ag->ag_Token = ObtainXRefToken (ag);

	LVOFreePathList (ag, ag->ag_GlobalPath);
	ag->ag_GlobalPath = NULL;

	for (i = 0; i < MAXPATHS; i++)
	    names[i] = 0;

	/* Attempt to open the environment file */
	if (fh = Open ("ENV:AmigaGuide/Path", MODE_OLDFILE))
	{
	    /* Seek to the end */
	    if ((num = Seek (fh, 0L, OFFSET_END)) >= 0L)
	    {
		/* Get the length, and seek back to the beginning */
		len = Seek (fh, 0L, OFFSET_BEGINNING);

		/* Compute buffer size */
		msize = len + 2;

		/* Allocate a temporary buffer */
		if (buff = (STRPTR) AllocMem (msize, MEMF_CLEAR))
		{
		    /* Read in the value of the variable */
		    if ((num = Read (fh, buff, len)) == len)
		    {
			/* Break the string into paths */
			LVOParsePathString (ag, buff, names, MAXPATHS);
		    }
		}
	    }

	    /* close the file */
	    Close (fh);
	}

	/* Did we get an environment variable that contains the path? */
	if (buff)
	{
	    /* Add the entries from the environment variable */
	    ag->ag_GlobalPath = LVOAddPathEntries (ag, ag->ag_GlobalPath, names);

	    /* Free the buffer */
	    FreeMem (buff, msize);
	}

	/* Add one default entry to the list */
	names[0] = "S:";
	names[1] = NULL;
	ag->ag_GlobalPath = LVOAddPathEntries (ag, ag->ag_GlobalPath, names);
    }
    else
    {
	ag->ag_UsageCnt--;
	retval = NULL;
    }

    /************/
    /* PERMIT() */
    /************/
    eb->TDNestCnt = nest;
    ReleaseSemaphore (&(ag->ag_Lock));

    return (retval);
}

/*****************************************************************************/

BOOL closestuff (struct AGLib * ag)
{
    if (FreeClass (ag->ag_DatabaseClass))
    {
	if (FreeClass (ag->ag_NodeClass))
	{
	    if (FreeClass (ag->ag_ModelClass))
	    {
		if (FreeClass (ag->ag_Class))
		{
		    if (ag->ag_LocaleBase)
		    {
			CloseCatalog (ag->ag_Catalog);
			CloseLibrary (ag->ag_LocaleBase);
		    }

		    CloseLibrary (ag->ag_DataTypesBase);
		    CloseLibrary (ag->ag_RexxSysBase);
		    CloseLibrary (ag->ag_IFFParseBase);
		    CloseLibrary (ag->ag_DiskfontBase);
		    ag->ag_DatabaseClass = ag->ag_NodeClass = ag->ag_ModelClass = ag->ag_Class = NULL;
		    ag->ag_LocaleBase = ag->ag_DataTypesBase = ag->ag_RexxSysBase = ag->ag_DiskfontBase = NULL;
		    ag->ag_IFFParseBase = NULL;
		    ag->ag_Catalog = NULL;

		    LVOFreePathList (ag, ag->ag_GlobalPath);
		    ag->ag_GlobalPath = NULL;

		    return TRUE;
		}
		ag->ag_ModelClass = initModelClass (ag);
	    }
	    ag->ag_NodeClass = initHNClass (ag);
	}
	ag->ag_DatabaseClass = initDBClass (ag);
    }

    return FALSE;
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct AGLib * ag)
{

    if (ag->ag_UsageCnt)
	ag->ag_UsageCnt--;

    /* This is probably in reverse order.  Should be client, node and then database */
    if ((ag->ag_UsageCnt == 0) && ag->ag_Class)
    {
	if (!closestuff (ag))
	{
	    ag->ag_Lib.lib_Flags |= LIBF_DELEXP;
	}
    }

    if (ag->ag_Lib.lib_Flags & LIBF_DELEXP)
	return (LibExpunge (ag));

    return (NULL);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct AGLib * ag)
{
    BPTR seg = ag->ag_SegList;

    if (ag->ag_UsageCnt)
    {
	ag->ag_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    if (ag->ag_Class)
    {
	if (!closestuff (ag))
	{
	    ag->ag_Lib.lib_Flags |= LIBF_DELEXP;
	    return (NULL);
	}
    }

    Remove ((struct Node *) ag);

    CloseLibrary (LayersBase);
    CloseLibrary (UtilityBase);
    CloseLibrary (GfxBase);
    CloseLibrary (IntuitionBase);
    CloseLibrary (DOSBase);

    FreeMem ((APTR)((ULONG)(ag) - (ULONG)(ag->ag_Lib.lib_NegSize)), ag->ag_Lib.lib_NegSize + ag->ag_Lib.lib_PosSize);

    return ((LONG) seg);
}
