/* open.c
 *
 */

#include "dtdesc.h"

/* A table of valid ASCII characters (7 bits). */
BYTE ASCIITable[256] =
{
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* A table of clearly invalid ASCII characters (8 bits). */
BYTE BinaryTable[256] =
{
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

VOID AddWBArgs (struct AppInfo * ai, struct WBArg * wbarglist, LONG numargs)
{
    struct FileNode *fn = NULL;
    WORD onf = ai->ai_NumFiles;
    struct WBArg *wbarg;
    BPTR old, lock;
    BOOL success;
    ULONG head;
    ULONG next;
    WORD ascii;
    BOOL iffc;
    WORD i;
    WORD j;

    for (i = 0, wbarg = wbarglist; i < (WORD) numargs; i++, wbarg++)
    {
	success = FALSE;

	old = CurrentDir (wbarg->wa_Lock);

	if (lock = Lock (wbarg->wa_Name, ACCESS_READ))
	{
	    if (!FindFileNodeLock (ai, lock))
	    {
		if (Examine (lock, &ai->ai_FIB))
		{
		    if (ai->ai_FIB.fib_DirEntryType < 0)
		    {
			if (fn = AllocVec (FNSIZE, MEMF_CLEAR))
			{
			    fn->fn_Lock = DupLock (lock);

			    strcpy (fn->fn_Name, wbarg->wa_Name);
			    fn->fn_Node.ln_Name = fn->fn_Name;
			    fn->fn_Extens = strrchr (fn->fn_Name, '.');

			    if (fn->fn_FH = Open (wbarg->wa_Name, MODE_OLDFILE))
			    {
				fn->fn_Len = Read (fn->fn_FH, fn->fn_Buffer, FNBUF_SIZE);

				if (fn->fn_Len >= 0)
				{
				    success = TRUE;

				    head = MAKE_ID (fn->fn_Buffer[0], fn->fn_Buffer[1], fn->fn_Buffer[2], fn->fn_Buffer[3]);
				    next = MAKE_ID (fn->fn_Buffer[4], fn->fn_Buffer[5], fn->fn_Buffer[6], fn->fn_Buffer[7]) + 8;
				    iffc = FALSE;
				    if ((next >= (3 * (ai->ai_FIB.fib_Size / 4))) &&
					(next <= (4 * (ai->ai_FIB.fib_Size / 3))))
				    {
					iffc = TRUE;
				    }

				    /* IFF file */
				    if (iffc && (head == MAKE_ID ('F', 'O', 'R', 'M')))
				    {
					fn->fn_Flags |= DTF_IFF;
				    }
				    /* IFF CAT file */
				    else if (iffc && (head == MAKE_ID ('C', 'A', 'T', ' ')))
				    {
					fn->fn_Flags |= DTF_IFF;
				    }
				    /* IFF LIST file */
				    else if (iffc && (head == MAKE_ID ('L', 'I', 'S', 'T')))
				    {
					fn->fn_Flags |= DTF_IFF;
				    }
				    else
				    {
					for (j = ascii = 0; j < fn->fn_Len; j++)
					{
					    if (ASCIITable[fn->fn_Buffer[j]])
					    {
						ascii++;
					    }
					    else
					    {
						if (BinaryTable[fn->fn_Buffer[j]])
						{
						    ascii = 0;
						    break;
						}
					    }
					}

					/*
					 * If more than 75% of the characters in the first bytes are legal ASCII characters this
					 * file is probably a text file.
					 */
					if (ascii > (3 * (fn->fn_Len / 4)))
					{
					    fn->fn_Flags |= DTF_ASCII;
					}
				    }

				    AddTail (&ai->ai_Files, (struct Node *) fn);
				    ai->ai_NumFiles++;
				}
			    }
			}
		    }
		}
	    }

	    UnLock (lock);
	}

	CurrentDir (old);

	if (!success)
	{
	    if (fn)
	    {
		if (fn->fn_FH)
		{
		    Close (fn->fn_FH);
		}

		FreeVec (fn);
		fn = NULL;
	    }

	    DisplayBeep (ai->ai_Window->WScreen);
	}
    }

    if (onf != ai->ai_NumFiles)
    {
	ScanFiles (ai);
    }
}
