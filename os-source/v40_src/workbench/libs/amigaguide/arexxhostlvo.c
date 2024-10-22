/* arexxhostlvo.c
 *
 */

/* includes */
#include <exec/types.h>
#include <utility/tagitem.h>
#include <rexx/storage.h>
#include <rexx/errors.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)		;

/****** amigaguide.library/--rexxhost-- ****************************************
*
*   HOST INTERFACE
*	amigaguide.library provides an ARexx function host interface that
*	enables ARexx programs to take advantage of the features of
*	AmigaGuide.  The functions provided by the interface are directly
*	related to the functions described herein, with the differences
*	mostly being in the way they are called.
*
*	The function host library vector is located at offset -30 from
*	the library. This is the value you provide to ARexx in the
*	AddLib() function call.
*
*   FUNCTIONS
*	ShowNode (PUBSCREEN,DATABASE,NODE,LINE,XREF)
*
*	LoadXRef (NAME)
*
*	GetXRef (NODE)
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/*****************************************************************************/

#define REXX_NO_ERROR          0
#define REXX_PROGRAM_NOT_FOUND ERR10_001	/* program not found */
#define REXX_NO_MEMORY         ERR10_003	/* no memory available */
#define REXX_NO_LIBRARY        ERR10_014	/* required library not found */
#define REXX_WRONG_NUMBER_ARGS ERR10_017	/* wrong number of arguments */
#define REXX_BAD_ARGS	       ERR10_018	/* invalid argument to function */

#define ARG3(rmp) (rmp->rm_Args[3])		/* third argument */
#define ARG4(rmp) (rmp->rm_Args[4])		/* fourth argument */

/*****************************************************************************/

/* structure for holding return information */
struct RetBlock
{
    LONG	Type;
    union
    {
	LONG	IntVal;				/* Integers */
	UBYTE	StringVal[512];			/* Strings */
    } values;
};

/* variable types */
#define	RBINT		1
#define	RBSTRING	2

/*****************************************************************************/

struct RexxCmd
{
    STRPTR	  rc_Name;
    LONG	(*rc_Func)(struct AmigaGuideLib *,  struct RexxMsg *, struct RetBlock *);
    STRPTR	  rc_Template;
    UWORD	  rc_NumArgs;
};

static struct RexxCmd rexxCmds[] =
{
    /* Original commands */
    {"SHOWNODE",		rShowNode,	"PUBSCREEN/k,DATABASE/k,NODE/k,LINE/n,XREF/k", 5},
    {"LOADXREF",		rLoadXRef,	"NAME/k", 1},
    {"GETXREF",			rGetXRef,	"NODE/k", 1},
    {"ADDXREF",			rStub,		"WORD/k,FILE/k,LINE/n,TYPE/n", 4},
    {"EXPUNGEXREF",		rStub,},

    /* New non-conflicting commands */
    {"AMIGAGUIDE_SHOWNODE",	rShowNode,	"PUBSCREEN/k,DATABASE/k,NODE/k,LINE/n,XREF/k", 5},
    {"AMIGAGUIDE_LOADXREF",	rLoadXRef,	"NAME/k", 1},
    {"AMIGAGUIDE_GETXREF",	rGetXRef,	"NODE/k", 1},
    {"AMIGAGUIDE_ADDXREF",	rStub,		"WORD/k,FILE/k,LINE/n,TYPE/n", 4},
    {"AMIGAGUIDE_EXPUNGEXREF",	rStub,},
};

#define NUM_COMMANDS 5

/*****************************************************************************/

ULONG ASM ARexxHostLVO (REG (a0) struct RexxMsg *rm, REG (a1) STRPTR *result, REG (a6) struct AmigaGuideLib *ag)
{
    ULONG resCode = REXX_NO_LIBRARY;
    struct RetBlock *block;
    struct RexxCmd *rc;
    UBYTE buffer[30];
    WORD numArgs;
    UWORD cmd;

    *result = NULL;

	if (block = AllocPVec (ag, ag->ag_MemoryPool, sizeof (struct RetBlock)))
	{
	    resCode = REXX_PROGRAM_NOT_FOUND;
	    rc = NULL;
	    cmd = NUM_COMMANDS << 1;
	    while (cmd--)
	    {
		if (Stricmp (ARG0 (rm), rexxCmds[cmd].rc_Name) == 0)
		{
		    if (cmd >= NUM_COMMANDS)
			cmd -= NUM_COMMANDS;
		    rc = &rexxCmds[cmd];
		    break;
		}
	    }

	    if (rc)
	    {
		numArgs = (rm->rm_Action) & RXARGMASK;

		if (numArgs != rc->rc_NumArgs)
		{
		    resCode = REXX_WRONG_NUMBER_ARGS;
		    DB (kprintf ("error: wrong number of arguments for %s\n", ARG0(rm)));
		}
		/* perform the function */
		else if ((resCode = (*(rc->rc_Func)) (ag, rm, block)) == REXX_NO_ERROR)
		{
		    switch (block->Type)
		    {
			case RBINT:
			    DB (kprintf (" int : %ld\n", block->values.IntVal));
			    sprintf (buffer, "%ld", block->values.IntVal);
			    *result = CreateArgstring (buffer, strlen (buffer) );
			    break;

			case RBSTRING:
			    DB (kprintf (" string : %s\n", block->values.StringVal));
			    *result = CreateArgstring (block->values.StringVal, strlen (block->values.StringVal));
			    break;
		    }
		}
	    }

	    FreePVec (ag, ag->ag_MemoryPool, block);
	}

    DB (kprintf ("return %ld\n", resCode));
    return (resCode);
}

/*****************************************************************************/

LONG rShowNode (struct AmigaGuideLib *ag, struct RexxMsg *rm, struct RetBlock *block)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefList;
    struct NewAmigaGuide nag = {NULL};
    AMIGAGUIDECONTEXT handle;
    struct XRef *xr;

    DB (kprintf ("ShowNode (%s,%s,%s,%s)\n", ARG1(rm), ARG2(rm), ARG3(rm), ARG4(rm) ));

    /* return the value as an integer */
    block->Type = RBINT;
    block->values.IntVal = FALSE;

    /* Fill in the values */
    nag.nag_PubScreen = ARG1(rm);
    nag.nag_Name      = ARG2(rm);
    nag.nag_Node      = ARG3(rm);

    /* So that we share preferences */
    nag.nag_BaseName  = "AmigaGuide";

    /* Look for the node */
    ObtainSemaphore (&agt->agt_Lock);
    if (!(nag.nag_Name) && (nag.nag_Node) && (xr = (struct XRef *) FindNameI (list, nag.nag_Node)))
    {
	/* Fill in the NewAmigaGuide fields */
	nag.nag_Name = xr->xr_File;
	nag.nag_Node = xr->xr_Name;
	nag.nag_Line = xr->xr_Line;
    }
    ReleaseSemaphore (&agt->agt_Lock);

    /* Do we have a name? */
    if (nag.nag_Name)
    {
	/* Open the database */
	if (handle = OpenAmigaGuideALVO (ag, &nag, NULL))
	{
	    /* Show success */
	    block->values.IntVal = TRUE;

	    /* Close the database */
	    CloseAmigaGuideLVO (ag, handle);
	}
    }

    return (REXX_NO_ERROR);
}

/*****************************************************************************/

LONG rLoadXRef (struct AmigaGuideLib *ag, struct RexxMsg *rm, struct RetBlock *block)
{
    /* return the value as an integer */
    block->Type          = RBINT;
    block->values.IntVal = FALSE;

    /* Tell it to load the cross reference table */
    DB (kprintf ("LoadXRef (%s)\n", ARG1 (rm) ));
    if (LoadXRefLVO (ag, NULL, ARG1 (rm) ))
    {
	DB (kprintf (" loaded\n"));
	block->values.IntVal = TRUE;
    }
    else
    {
	DB (kprintf (" not loaded!\n"));
    }

    return (REXX_NO_ERROR);
}

/*****************************************************************************/

LONG rGetXRef (struct AmigaGuideLib *ag, struct RexxMsg *rm, struct RetBlock *block)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefList;
    struct XRef *xr;

    DB (kprintf ("getxref : FindNameI 0x%lx, %s\n", &agt->agt_XRefList, ARG1(rm) ));
    ObtainSemaphore (&agt->agt_Lock);
    if (xr = (struct XRef *) FindNameI (list, ARG1(rm) ))
    {
	/* return the value as an String */
	DB (kprintf (" found\n"));
	block->Type = RBSTRING;
	sprintf (block->values.StringVal, "\"%s\" \"%s\" %ld %ld", xr->xr_Name, xr->xr_File, (LONG)xr->xr_Node.ln_Type, xr->xr_Line);
    }
    else
    {
	/* return the value as an Integer */
	DB (kprintf (" couldn't find\n"));
	block->Type          = RBINT;
	block->values.IntVal = RETURN_ERROR;
    }
    ReleaseSemaphore (&agt->agt_Lock);
    return (REXX_NO_ERROR);
}

/*****************************************************************************/

LONG rStub (struct AmigaGuideLib *ag, struct RexxMsg *rm, struct RetBlock *block)
{
    /* return the value as an Integer */
    block->Type = RBINT;
    block->values.IntVal = FALSE;
    return (REXX_NO_ERROR);
}
