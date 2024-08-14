/* info.c
 *
 */

#include "memcheck.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

static char *hex = "0123456789ABCDEF";

#define	TEXT_OFFSET	56
/* 61 */

/*****************************************************************************/

void ASM MemoryDump (REG (a0) struct GlobalData * gd, REG (d0) STRPTR prefix, REG (a1) char *ptr, REG (d1) ULONG size)
{
    ULONG i, j, k, l, m, n, o;
    UBYTE buffer[110];
    char ch;

    /* Step through the memory chunk */
    for (i = j = l = m = n = 0, k = TEXT_OFFSET; i < size; i++)
    {
	/* New line, then show the memory address for this line */
	if ((l == 0) && (j == 0))
	{
	    n = i;
	    memset (buffer, 32, sizeof (buffer));
	    sprintf (buffer, "%s %08lx:  ", prefix, &ptr[i]);
	    m = strlen (buffer) - 1;
	}

	/* Get the byte from memory */
	ch = ptr[i];

	/* Display the byte */
	buffer[m++] = hex[(0xF0 & ch) >> 4];
	buffer[m++] = hex[(0x0F & ch)];

	/* Prepare the text line */
	buffer[k++] = ((ch >= 32) && (ch < 127)) ? ch : '.';

	j++;
	if (j > 3)
	{
	    buffer[m++] = ' ';
	    j = 0;
	    l++;
	}

	if (l >= 4)
	{
	    ULONG cnt, db;

	    buffer[k] = 0;

	    for (o = n, cnt = db = 0; o < i; o += 4, cnt++)
		if (MAKE_LONG (ptr[o + 0], ptr[o + 1], ptr[o + 2], ptr[o + 3]) == 0xDEADBEEF)
		    db++;

	    if (cnt != db)
		kprintf ("%s\n", buffer);

	    j = l = 0;
	    k = TEXT_OFFSET;
	}
    }

    /* Do we have an incomplete line? */
    if (l)
    {
	/* Finish out the text line */
	buffer[k] = 0;
	kprintf ("%s\n", buffer);
    }
}

/*****************************************************************************/

BOOL ASM GetHunkInfo (
    REG (a0) struct GlobalData *gd,
    REG (a1) struct Task *tc,
    REG (a2) ULONG *stackptr,
    REG (a3) ULONG *h,
    REG (a4) ULONG *o)
{
    BOOL retval = FALSE;

    /* Is it a process? */
    if (tc->tc_Node.ln_Type == NT_PROCESS)
    {
	struct Process *pr = (struct Process *) tc;
	struct CommandLineInterface *cli;
	BPTR seg = NULL;
	ULONG hunk;
	ULONG *ptr;
	ULONG len;

	/* Try getting the CLI seglist */
	if (pr->pr_CLI && (cli = BADDR (pr->pr_CLI)))
	    seg = cli->cli_Module;

	/* No CLI seglist, then get it from the process structure */
	if ((seg == NULL) && pr->pr_SegList)
	    seg = ((LONG *) BADDR (pr->pr_SegList))[3];

	/* Find the hunk and offset that the PC resides in. */
	if (seg)
	{
	    hunk = 0;
	    DB (kprintf ("\nseg=%08lx\n", seg));
	    while (ptr = (ULONG *) (seg << 2))
	    {
		len = (*(((ULONG *) ptr) - 1)) - 8;
		seg = NULL;

		/* Make sure we have a valid length */
		if (!(len & 0xFF700000))
		{
		    DB (kprintf ("  Hunk %ld: %08lx, %ld\n", hunk, ptr, len));

		    /* Does the PC reside within this segment? */
		    if (((ULONG) stackptr[0] >= (ULONG) ptr) && ((ULONG) stackptr[0] <= (ULONG) ptr + len))
		    {
			*h = hunk;
			*o = ((ULONG) stackptr[0] - (ULONG) ptr - 8);
			retval = TRUE;
		    }
		    else
		    {
			/* Get the next segment */
			seg = *ptr;
			hunk++;
		    }
		}
	    }
	}
    }
    return retval;
}

/*****************************************************************************/

void ASM ShowProcessInfo (
    REG (a0) struct GlobalData * gd,
    REG (a1) ULONG * stackptr,
    REG (a2) struct MemCheck *mc)
{
    struct MemCheckInfo *mci = &gd->gd_MemCheckInfo;
    struct Task *tc = SysBase->ThisTask;
    UBYTE date[LEN_DATSTRING];
    UBYTE time[LEN_DATSTRING];
    register ULONG i, j, k;
    struct DateTime dat;
    UBYTE status[10];

    /**********************************/
    /* Display allocation information */
    /**********************************/
    if (mc)
    {
#if 0
----- Allocation Information ------------------------------------------------
Info: Address: 00000000  Size: 25  Date: 28-Aug-92  Time: 10:45 (old)
Stck: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
Stck: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
TCB : 00000000  Name "testing"  Hunk 0000  Offset 00000D0A
#endif

	/* Print a header to separate this chunk from the current chunk */
	kprintf ("----- Allocation Information ------------------------------------------------\n");

	/* Convert the cookie date stamp to strings */
	dat.dat_Stamp   = *(&mc->mc_DateStamp);
	dat.dat_Format  = FORMAT_DOS;
	dat.dat_Flags   = NULL;
	dat.dat_StrDay  = NULL;
	dat.dat_StrDate = date;
	dat.dat_StrTime = time;
	DateToStr (&dat);

	/* Compare the age of the cookie date to the time MemCheck was enabled */
	status[0] = 0;
	if (CompareDates (&mci->mci_DateStamp, &mc->mc_DateStamp) < 0)
	    strcpy (status, "(old)");

	/* Display the information line */
	kprintf ("Info: Address %08lx  Size %ld  Date %s  Time %s %s\n",
		 (ULONG)mc + INFOSIZE + mci->mci_PreSize, mc->mc_Size, date, time, status);

	/* Display stack lines */
	for (i = k = 0; i < 2; i++)
	{
	    kprintf ("Stck: ");
	    for (j = 0; j < 8; j++, k++)
		kprintf ("%08lx ", mc->mc_StackPtr[k]);
	    kprintf ("\n");
	}

	/* Display the task & process information */
	kprintf ("TCB : %08lx  Name \"%s\"", mc->mc_Task, mc->mc_Name);
	if (mc->mc_Hunk && mc->mc_Offset)
	    kprintf ("  Hunk %04lx  Offset %08lx", mc->mc_Hunk, mc->mc_Offset);
	kprintf ("\n");

	/* Separate this block from the current block */
	kprintf ("----- Current Information ---------------------------------------------------\n");
    }

    /*******************************/
    /* Display current information */
    /*******************************/

    /* Display stack lines */
    for (i = k = 0; i < mci->mci_Stack; i++)
    {
	kprintf ("Stck: ");
	for (j = 0; j < 8; j++, k++)
	    kprintf ("%08lx ", stackptr[k]);
	kprintf ("\n");
    }

    /* Display task information */
    kprintf ("TCB : %08lx  PC: %08lx\n", tc, stackptr[0]);
    kprintf ("Name: \"%s\"", tc->tc_Node.ln_Name);

    /* Is it a process? */
    if (tc->tc_Node.ln_Type == NT_PROCESS)
    {
	struct Process *pr = (struct Process *) tc;
	struct CommandLineInterface *cli;
	STRPTR tmp, name = NULL;
	ULONG h, o;

	/* Prepare the CLI name for printing */
	if (pr->pr_CLI)
	{
	    if ((cli = BADDR (pr->pr_CLI)) && cli->cli_CommandName)
	    {
		/* Try getting command name */
		tmp = BADDR (cli->cli_CommandName);
		if (tmp[0] > 0)
		    name = ++tmp;
	    }
	    if (name)
		kprintf ("  CLI: \"%s\"", name);
	    else
		kprintf ("  Invalid CLI name");
	}

	/* Try getting the hunk information */
	if (GetHunkInfo (gd, tc, stackptr, &h, &o))
	{
	    /* Show the hunk and the offset */
	    kprintf ("  Hunk %04lx  Offset %08lx", h, o);
	}
    }

    /* Give them a break */
    kprintf ("\n\n");
}
