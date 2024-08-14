/* cmdprocessor.c
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <utility/hooks.h>
#include <string.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*****************************************************************************/

#include "cmdprocessor.h"

/*****************************************************************************/

void kprintf (void *,...);

/*****************************************************************************/

#define ASM				__asm
#define REG(x)				register __ ## x

/*****************************************************************************/

#define	RDBUF_SIZE	128

/*****************************************************************************/

struct CmdHeader
{
    struct Library	*ch_SysBase;
    struct Library	*ch_DOSBase;
    struct Library	*ch_UtilityBase;

    ULONG		*ch_CmdArray;			/* Pointer to the command array */
    ULONG		 ch_NumCmds;			/* Number of commands */
    UBYTE		 ch_Work[RDBUF_SIZE+2];		/* Temporary work buffer */
    struct Hook		 ch_Hook;
    struct RDArgs	 ch_RDArgs;			/* Since we can only have one at a time, here it is */
};

/*****************************************************************************/

#define	DOSBase		 ch->ch_DOSBase
#define	UtilityBase	 ch->ch_UtilityBase

/*****************************************************************************/

struct CmdHeader *AllocCmdProcessor (struct Cmd * cmdArray, APTR data)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Cmd *tcmd, *dcmd;
    struct CmdHeader *ch;
    register ULONG i;
    ULONG msize;
    char *ptr;

    /* Count the members and accumulate the size */
    msize = sizeof (struct CmdHeader);
    for (tcmd = cmdArray, i = 0; tcmd->c_Cmd; i++, tcmd++)
	msize += (sizeof (struct Cmd) + (sizeof (ULONG *) * tcmd->c_NumOpts));
    msize += (sizeof (ULONG *) * i);

    /* Allocate the command block */
    if (ch = (struct CmdHeader *) AllocVec (msize, MEMF_CLEAR))
    {
	/* We need DOS functions, so open the library */
	ch->ch_SysBase = SysBase;
#define	SysBase		ch->ch_SysBase
	ch->ch_DOSBase = OpenLibrary ("dos.library", 37);
	ch->ch_UtilityBase = OpenLibrary ("utility.library", 37);

	/* Remember the count */
	ch->ch_NumCmds = i;

	/* Remember the user data */
	ch->ch_Hook.h_Data = data;

	/* Adjust the pointer */
	ptr = (char *) ((ULONG) ch + sizeof (struct CmdHeader));

	/* Point to the beginning of the command array */
	ch->ch_CmdArray = (ULONG *) ptr;
	ptr = (char *) ((ULONG) ptr + (sizeof (ULONG *) * i));

	/* Copy the array */
	for (tcmd = cmdArray, i = 0; i < ch->ch_NumCmds; i++, tcmd++)
	{
	    ch->ch_CmdArray[i] = dcmd = (struct Cmd *) ptr;
	    CopyMem (tcmd, dcmd, sizeof (struct Cmd));
	    ptr = (char *) ((ULONG) ptr + sizeof (struct Cmd));

	    /* Setup the option table */
	    dcmd->c_Options = (ULONG *) ptr;
	    ptr = (char *) ((ULONG) ptr + (sizeof (ULONG *) * tcmd->c_NumOpts));
	}
    }

    /* Return a pointer to the beginning */
    return ch;
}

/*****************************************************************************/

void FreeCmdProcessor (struct CmdHeader * ch)
{

    if (ch)
    {
	/* Close the ROM libraries now */
	CloseLibrary (ch->ch_UtilityBase);
	CloseLibrary (ch->ch_DOSBase);

	/* Free the block */
	FreeVec (ch);
    }
}

/*****************************************************************************/

#define isspace(c)  ((c==' ')||(c=='\t')||(c=='\n'))

/*****************************************************************************/

static ULONG BreakCommand (STRPTR line, STRPTR * argv)
{
    STRPTR *pargv;
    ULONG argc;

    /* clear the work areas */
    argc = 0L;
    argv[0] = NULL;
    argv[1] = NULL;

    /* parse the arguments */
    while (1)
    {
	/* skip past all the spaces at beginning of word. */
	while (isspace (*line))
	    line++;

	/* did we reach the end of the line? */
	if (*line == '\0')
	    break;

	/* remember the beginning of the word */
	pargv = &argv[(UWORD) (argc++)];
	*pargv = line;

	/* found the second portion of the string */
	if (argc == 2)
	    break;

	/* find the end of the current word */
	while ((*line != '\0') && (!isspace (*line)))
	    line++;

	/* did we reach the end of the string? */
	if (*line == '\0')
	    break;
	/* no, then terminate the word */
	else
	    *line++ = '\0';
    }

    return (argc);
}

/*****************************************************************************/

ULONG DispatchCmd (struct CmdHeader * ch, ULONG cmdID, STRPTR cmdStr)
{
    struct RDArgs *ra = &ch->ch_RDArgs;
    struct RDArgs *rdargs;
    ULONG retval = 0;
    register ULONG i;
    struct Cmd *cmd;
    STRPTR argv[2];
    ULONG msg = 0L;
    ULONG argc;

    STRPTR buffer;
    ULONG len;

    if (cmdID)
    {
	/* Search for the command */
	for (i = 0; i < ch->ch_NumCmds; i++)
	{
	    cmd = (struct Cmd *) ch->ch_CmdArray[i];
	    if (cmdID == cmd->c_CmdID)
	    {
		/* Clear the option array */
		memset (&cmd->c_Options[0], 0, (sizeof (ULONG *) * cmd->c_NumOpts));

		/* Set up the hook entry */
		ch->ch_Hook.h_Entry = cmd->c_Func;

		/* Call the hook */
		retval = ((*cmd->c_Func) (&ch->ch_Hook, cmd, &msg));
		break;
	    }
	}
    }
    else if (cmdStr)
    {
	/* Allocate a temporary buffer */
	len = strlen (cmdStr);
	if (buffer = AllocVec (len + 1, NULL))
	{
	    strcpy (buffer, cmdStr);

	/* Split the string into two parts.  First being command, second being arguments */
	if (argc = BreakCommand (buffer, argv))
	{
	    /* Search for the command */
	    for (i = 0; i < ch->ch_NumCmds; i++)
	    {
		cmd = (struct Cmd *) ch->ch_CmdArray[i];
		if (Stricmp (argv[0], cmd->c_Cmd) == 0)
		{
		    /* Prepare the ReadArgs array */
		    ch->ch_Work[0] = 0;
		    if (argc > 1)
			strcpy (ch->ch_Work, argv[1]);
		    strcat (ch->ch_Work, "\n");
		    memset (ra, 0, sizeof (struct RDArgs));
		    ra->RDA_Source.CS_Buffer = ch->ch_Work;
		    ra->RDA_Source.CS_Length = strlen (ch->ch_Work);
		    ra->RDA_Flags = RDAF_NOPROMPT;

		    /* Clear the option array */
		    memset (&cmd->c_Options[0], 0, (sizeof (ULONG *) * cmd->c_NumOpts));

		    /* Parse the arguments */
		    if (rdargs = ReadArgs (cmd->c_Template, cmd->c_Options, ra))
		    {
			/* Set up the hook entry */
			ch->ch_Hook.h_Entry = cmd->c_Func;

			/* Call the hook */
			retval = ((*cmd->c_Func) (&ch->ch_Hook, cmd, &msg));

			/* Free the extra buffers */
			FreeArgs (rdargs);
		    }
		    break;
		}
	    }
	}
	    /* Free the temporary buffer */
	    FreeVec (buffer);
	}
    }

    return (retval);
}
