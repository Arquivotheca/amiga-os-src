/* xrefmgr.c
 *
 */

#include "amigaguidebase.h"
#include "asyncio.h"

//#define PROF
#include <internal/prof.h>

/*****************************************************************************/

#define	DB(x)	;
#define	DL(x)	;
#define	DN(x)	;

/*****************************************************************************/

#define TOK_NUMBER 0
#define TOK_STRING 1

/*****************************************************************************/

struct XRefWorkData
{
    STRPTR		 gd_FileName;
    struct AsyncFile	*gd_InputFile;
    UWORD		 gd_InputLine;
    UWORD		 gd_InputColumn;
    UWORD		 gd_TokenLine;
    UWORD		 gd_TokenColumn;
    UBYTE		 gd_TokenType;
    UBYTE		 gd_CurrentToken[256];
    UBYTE		 gd_LastChar;
    BOOL		 gd_TokenQuoted;	/* last token had quotes around it */
    BOOL		 gd_LineFeed;

    /* Current line */
    UBYTE		 gd_Name[80];
    UBYTE		 gd_File[256];
    LONG		 gd_Line;
    LONG		 gd_Type;
};

/*****************************************************************************/

/* different possible parsing states */
#define SCAN_SECTION		0
#define SCAN_KEYWORD		1
#define SCAN_DONE		2

#define CODE_SUCCESS		0
#define CODE_NOMEMORY		1
#define CODE_STRINGEXPECTED	3
#define CODE_NUMBEREXPECTED	4
#define CODE_ARGUMENTEXPECTED	6
#define CODE_MISSINGKEYWORDS	7
#define CODE_UNKNOWNKEYWORD	8
#define CODE_SECTIONNOTFOUND	9

/*****************************************************************************/

LONG loadxref (struct AGLib *ag, BPTR lock, STRPTR fileName, LONG mode)
{
    struct XRefWorkData *gd;
    char colonName[40];
    BOOL foundSection;
    LONG retval = 0L;		/* Return value of function */
    UWORD wordCount;
    UBYTE state;
    UWORD code;
    BPTR old;

    ONTIMER(0);

    /* Allocate the work data */
    if (gd = AllocVec (sizeof (struct XRefWorkData), MEMF_CLEAR))
    {
	if (lock)
	    old = CurrentDir (lock);

	strcpy (colonName, "XREF:");
	if (gd->gd_InputFile = OpenAsync (ag, fileName, MODE_READ, 16384))
	{
	    gd->gd_FileName    = fileName;
	    gd->gd_InputLine   = 1;
	    gd->gd_InputColumn = 0;
	    gd->gd_LastChar    = ' ';
	    wordCount          = 0;
	    code               = CODE_SUCCESS;
	    state              = SCAN_SECTION;
	    foundSection       = FALSE;

	    ONTIMER(1);
	    while ((code == CODE_SUCCESS) && (state != SCAN_DONE) && GetToken (ag, gd))
	    {
		switch (state)
		{
		    case SCAN_SECTION:
			ONTIMER(2);
			if (Stricmp (gd->gd_CurrentToken, colonName) == 0)
			    foundSection = TRUE;
			state = SCAN_KEYWORD;
			gd->gd_LineFeed = FALSE;
			wordCount = 0;
			OFFTIMER(2);
			break;

		    case SCAN_KEYWORD:
			ONTIMER(3);
			if (Stricmp (gd->gd_CurrentToken, "#") == 0)
			{
			    if (foundSection)
				state = SCAN_DONE;
			    else
				state = SCAN_SECTION;
			    break;
			}

			if (foundSection)
			{
			    if (gd->gd_LineFeed)
			    {
				ONTIMER(4);
				if (wordCount == 4)
				    AddXRef (ag, gd->gd_Name, gd->gd_File, gd->gd_Line, (UBYTE)gd->gd_Type);
				gd->gd_LineFeed = FALSE;
				wordCount = 0;
				OFFTIMER(4);
			    }

			    if (code == CODE_SUCCESS)
			    {
				switch (wordCount++)
				{
				    case 0:
					strcpy (gd->gd_Name, gd->gd_CurrentToken);
					break;

				    case 1:
					strcpy (gd->gd_File, gd->gd_CurrentToken);
					break;

				    case 2:
					StrToLong (gd->gd_CurrentToken, &gd->gd_Line);
					break;

				    case 3:
					StrToLong (gd->gd_CurrentToken, &gd->gd_Type);
					break;
				}
			    }
			}
			OFFTIMER(3);
			break;
		}
	    }
	    OFFTIMER(1);

	    CloseAsync (ag, gd->gd_InputFile);

	    if ((code == CODE_SUCCESS) && foundSection)
		retval = 1L;
	}

	if (lock)
	    CurrentDir (old);

	FreeVec (gd);
    }

    OFFTIMER(0);

    return (retval);
}

/*****************************************************************************/

char GetCh (struct AGLib * ag, struct XRefWorkData * gd)
{
    LONG num;

    num = ReadCharAsync (ag, gd->gd_InputFile);
    if (num < 0)
	num = 0;

    if (gd->gd_LastChar == '\n')
    {
	gd->gd_InputLine++;
	gd->gd_InputColumn = 0;
	gd->gd_LineFeed = TRUE;
    }
    gd->gd_InputColumn++;

    return (gd->gd_LastChar = (char) num);
}

/*****************************************************************************/

BOOL GetToken (struct AGLib * ag, struct XRefWorkData * gd)
{
    UWORD nest, tokenLen;
    char ch, oldCh;

    tokenLen = 0;
    ch = gd->gd_LastChar;
    gd->gd_TokenQuoted = FALSE;

    while ((ch == '\t') || (ch == ' ') || (ch == '\n') || (ch == ';'))
    {
	do
	{
	    ch = GetCh (ag, gd);
	}
	while ((ch == '\t') || (ch == ' ') || (ch == '\n') || (ch == ';'));

	if (ch == '/')
	{
	    gd->gd_TokenLine = gd->gd_InputLine;
	    gd->gd_TokenColumn = gd->gd_InputColumn;

	    ch = GetCh (ag, gd);
	    if (ch == '*')
	    {
		oldCh = ' ';
		nest = 1;
		do
		{
		    ch = GetCh (ag, gd);

		    if ((ch == '/') && (oldCh == '*'))
		    {
			nest--;
			oldCh = ' ';
		    }
		    else if ((ch == '*') && (oldCh == '/'))
		    {
			nest++;
			oldCh = ' ';
		    }
		    else
		    {
			oldCh = ch;
		    }
		}
		while (nest && ch);
		ch = GetCh (ag, gd);
	    }
	    else
	    {
		gd->gd_CurrentToken[0] = '/';
		tokenLen++;
	    }
	}
    }

    if (tokenLen == 0)
    {
	gd->gd_TokenLine = gd->gd_InputLine;
	gd->gd_TokenColumn = gd->gd_InputColumn;
    }

    gd->gd_TokenType = TOK_STRING;
    if (ch == '\"')
    {
	ch = GetCh (ag, gd);
	while ((ch != '\"') && (ch != '\n') && (ch != 0))
	{
	    gd->gd_CurrentToken[tokenLen++] = ch;
	    ch = GetCh (ag, gd);
	}

	if (ch == '\"')
	{
	    GetCh (ag, gd);
	    gd->gd_TokenQuoted = TRUE;
	}
    }
    else if (ch == '=')
    {
	gd->gd_CurrentToken[0] = '=';
	tokenLen = 1;
	GetCh (ag, gd);		/* keep mr.lookahead happy! */
    }
    else
    {
	if (((ch >= '0') && (ch <= '9')) || (ch == '-'))
	    gd->gd_TokenType = TOK_NUMBER;

	while ((ch != '\t') && (ch != ' ') && (ch != '\n') && (ch != ';') && (ch != '=') && (ch != 0))
	{
	    gd->gd_CurrentToken[tokenLen++] = ch;
	    ch = GetCh (ag, gd);
	}
    }
    gd->gd_CurrentToken[tokenLen] = 0;

    return ((BOOL)(tokenLen > 0));
}

/*****************************************************************************/

VOID LoadXRefFiles (struct AGLib * ag)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefFileList;
    struct Node *node, *next;
    struct XRefFile *xrf;

    /* Clear out the cross reference table */
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	/* start at the beginning */
	node = list->lh_Head;
	while (next = node->ln_Succ)
	{
	    /* Cast the node */
	    xrf = (struct XRefFile *) node;

	    /* Has the file already been loaded? */
	    if (!(xrf->xrf_Flags & XRF_LOADED))
	    {
		/* Load the file */
		loadxref (ag, xrf->xrf_Lock, xrf->xrf_Node.ln_Name, 0);

		/* Mark it as being loaded */
		xrf->xrf_Flags |= XRF_LOADED;
	    }

	    /* get the next node */
	    node = next;
	}
    }
}

/*****************************************************************************/

struct XRefFile *AllocXRefFile (struct AGLib * ag, BPTR lock, STRPTR name)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct XRefFile *xrf = NULL;
    BPTR old = NULL;
    ULONG msize;
    BPTR lck;

    /* Did they provide a lock? */
    if (lock)
	old = CurrentDir (lock);

    /* Lock the file */
    if (lck = LVOLockE (ag, NULL, name, ACCESS_READ))
    {
	/* How much memory do we need */
	msize = sizeof (struct XRefFile) + sizeof (struct FileInfoBlock) + strlen (name) + 1;

	/* Allocate the cross reference file information record */
	if (xrf = AllocVec (msize, MEMF_CLEAR))
	{
	    /* Set up the file info block */
	    xrf->xrf_FIB = MEMORY_FOLLOWING (xrf);

	    /* Set up the name */
	    xrf->xrf_Node.ln_Name = MEMORY_FOLLOWING (xrf->xrf_FIB);
	    strcpy (xrf->xrf_Node.ln_Name, name);

	    /* Did they pass a lock? */
	    if (lock)
	    {
		/* Duplicate the lock */
		xrf->xrf_Lock = DupLock (lock);
	    }

	    /* Examine the file, and make sure it is a file */
	    if ((Examine (lck, xrf->xrf_FIB)) && (xrf->xrf_FIB->fib_DirEntryType < 0))
	    {
		/* Add it to the list */
		AddTail (&agt->agt_XRefFileList, &(xrf->xrf_Node));
	    }
	    else
	    {
		/* Clean failure path */
		FreeVec (xrf);
		xrf = NULL;
	    }
	}

	/* Unlock the temporary lock */
	UnLock (lck);
    }

    if (lock)
	CurrentDir (old);

    return (xrf);
}

/*****************************************************************************/

VOID FreeXRefFile (struct AGLib * ag, struct XRefFile * xrf)
{
    if (xrf)
    {
	/* Remove the node from the list */
	Remove (&(xrf->xrf_Node));

	/* Is there a lock? */
	if (xrf->xrf_Lock)
	{
	    /* Unlock it */
	    UnLock (xrf->xrf_Lock);
	}

	/* Deallocate it */
	FreeVec (xrf);
    }
}

/*****************************************************************************/

VOID UnLoadXRefFiles (struct AGLib * ag)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefFileList;
    struct Node *node, *next;

    /* Clear out the cross reference table */
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	/* start at the beginning */
	node = list->lh_Head;
	while (next = node->ln_Succ)
	{
	    /* Remove the record */
	    FreeXRefFile (ag, (struct XRefFile *) node);

	    /* get the next node */
	    node = next;
	}
    }
}

/*****************************************************************************/

struct XRef *AddXRef (struct AGLib * ag, STRPTR name, STRPTR file, LONG line, UBYTE type)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefList;
    ULONG msize, nsize;
    struct XRef *xr;
    BOOL add = TRUE;

    /* See if it already exists */
    if (xr = (struct XRef *) FindNameI (list, name))
    {
	/* See if it is the same file. */
	if (strcmp (xr->xr_File, file) == 0)
	{
	    /* Set the type */
	    xr->xr_Node.ln_Type = type;

	    /* Make the line number the same */
	    xr->xr_Line = line;

	    /* Don't create a new node */
	    add = FALSE;
	}
	else
	{
	    /* Remove it from the list */
	    Remove ((struct Node *) xr);

	    /* Free the memory used */
	    FreeVec (xr);

	    /* Clear it */
	    xr = NULL;
	}
    }

    /* Are we supposed to add it? */
    if (add)
    {
	/* Compute the amount of memory needed */
	nsize = XRSIZE + strlen (file) + 2;
	msize = nsize + strlen (name) + 2;

	/* Allocate a new node */
	if (xr = (struct XRef *) AllocVec (msize, MEMF_CLEAR))
	{
	    /* Set the type */
	    xr->xr_Node.ln_Type = type;

	    /* Set the line */
	    xr->xr_Line = line;

	    /* Prepare the file name */
	    xr->xr_File = MEMORY_N_FOLLOWING (xr, XRSIZE);
	    strcpy (xr->xr_File, file);

	    /* Copy the name into the node */
	    xr->xr_Node.ln_Name = xr->xr_Name = MEMORY_N_FOLLOWING (xr, nsize);
	    strcpy (xr->xr_Name, name);

	    /* Add the node to the list */
	    AddTail (list, (struct Node *) xr);
	}
    }

    /* Return the pointer */
    return (xr);
}
