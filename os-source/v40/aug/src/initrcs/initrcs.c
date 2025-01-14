/* initrcs.c
 *
 */

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

extern struct Library *SysBase, *DOSBase;

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define	RCS_LINK	"rcs_link"
#define	RCS_INPUT	"T:RCS_INPUT"

int main (int argc, char **argv)
{
    struct AnchorPath __aligned anchor;
    UBYTE copy_pattern[256];
    UBYTE ci_pattern[256];
    UBYTE command[512];
    UBYTE buffer[256];
    UBYTE path[256];
    LONG result;
    UWORD i, j;
    BPTR olock;
    BPTR rlock;
    BPTR lock;
    BPTR fh;

    struct List DeleteList;
    struct Node *ln;
    ULONG msize;

    NewList (&DeleteList);

    if (fh = Open (RCS_LINK, MODE_OLDFILE))
    {
	printf ("%s already exists\n", RCS_LINK);
	Close (fh);
    }
    else if (fh = Open (RCS_LINK, MODE_NEWFILE))
    {
	if (NameFromFH (fh, buffer, 256))
	{
	    j = strlen (buffer);
	    for (i = 0; i < j; i++)
	    {
		if (buffer[i] == ':')
		{
		    buffer[i] = 0;
		    strcpy (path, &buffer[(i + 1)]);

		    j = strlen (path);
		    for (i = 0; i < j; i++)
		    {
			if (path[i] == '/')
			{
			    path[i] = ':';
			    i = j = 0;
			}
		    }
		}
	    }

	    /* Remove the rcs_link name from the buffer */
	    stcgfp (buffer, path);
	    strcpy (path, buffer);

	    printf ("Initialize RCS for \"%s\"\n\n", buffer);

	    /* Create the directory on the VAX */
	    if (lock = CreateDir (buffer))
	    {
		printf (" created \"%s\"...\n", buffer);
		UnLock (lock);
	    }

	    if (rlock = Lock (buffer, ACCESS_READ))
	    {
		/* Create the RCS directory on the VAX */
		strcat (buffer, "/RCS");
		if (lock = CreateDir (buffer))
		{
		    printf (" created \"%s\"...\n", buffer);
		    UnLock (lock);
		}

		strcat (buffer, "/\n");
		Write (fh, buffer, strlen (buffer));

		Close (fh);

		ParsePatternNoCase ("(\#?.(doc|ld|strip))|(#?_rev.#?)", copy_pattern, sizeof (copy_pattern));
		ParsePatternNoCase ("((\#?.(c|h|asm|i|sfd|cd))|lmkfile|make#?)", ci_pattern, sizeof (ci_pattern));

		memset (&anchor, 0, sizeof (struct AnchorPath));
		anchor.ap_Flags = APF_DOWILD;
		anchor.ap_BreakBits = SIGBREAKF_CTRL_C;

		if (fh = Open (RCS_INPUT, MODE_NEWFILE))
		{
		    Write (fh, "initial RCS\n.\n", 14);
		    Close (fh);
		}

		if (!(result = MatchFirst ("#?", &anchor)))
		{
		    while (TRUE)
		    {
			if (CheckSignal (SIGBREAKF_CTRL_C))
			{
			    break;
			}

			if (anchor.ap_Info.fib_DirEntryType < 0)
			{
			    lock = CurrentDir (anchor.ap_Current->an_Lock);
			    if (MatchPatternNoCase (ci_pattern, anchor.ap_Info.fib_FileName))
			    {
				printf (" ci %s ... ", anchor.ap_Info.fib_FileName);

				sprintf (command, "rcs -q -i \"%s\" <T:RCS_INPUT", anchor.ap_Info.fib_FileName);
				if (SystemTagList (command, NULL) == -1)
				{
				    PrintFault (IoErr (), NULL);
				    break;
				}
				else
				{
				    sprintf (command, "ci -q -u%ld.0 \"%s\" <T:RCS_INPUT", (LONG) SysBase->lib_Version, anchor.ap_Info.fib_FileName);
				    if (SystemTagList (command, NULL) == -1)
				    {
					PrintFault (IoErr (), NULL);
					break;
				    }
				    else
				    {
					olock = CurrentDir (rlock);
					sprintf (command, "co -q -u \"%s\"", anchor.ap_Info.fib_FileName);
					if (SystemTagList (command, NULL) == -1)
					{
					    PrintFault (IoErr (), NULL);
					    break;
					}
					CurrentDir (olock);
				    }

				}

				msize = sizeof (struct Node) + strlen (anchor.ap_Info.fib_FileName) + 1;
				if (ln = (struct Node *) AllocVec (msize, MEMF_CLEAR))
				{
				    ln->ln_Name = MEMORY_FOLLOWING (ln);
				    strcpy (ln->ln_Name, anchor.ap_Info.fib_FileName);
				    AddTail (&DeleteList, ln);
				}
				printf ("done\n");
			    }
			    else if (MatchPatternNoCase (copy_pattern, anchor.ap_Info.fib_FileName))
			    {
				printf (" copy %s ... ", anchor.ap_Info.fib_FileName);
				sprintf (command, "copy %s %s", anchor.ap_Info.fib_FileName, path);
				if (SystemTagList (command, NULL) == -1)
				{
				    PrintFault (IoErr (), NULL);
				    break;
				}
				DeleteFile (anchor.ap_Info.fib_FileName);
				printf ("done\n");
			    }

			    CurrentDir (lock);
			}

			if (result = MatchNext (&anchor))
			{
			    if (result != ERROR_NO_MORE_ENTRIES)
			    {
				PrintFault (result, NULL);
			    }
			    break;
			}
		    }
		    MatchEnd (&anchor);

		    SystemTagList ("protect #? +r+w+d quiet", NULL);

		    SystemTagList ("delete #?.(strip|ld|map|o|obj|country|language) #?_rev.#? Errs quiet", NULL);
		}
		else
		{
		    printf ("couldn't MatchFirst\n");
		}
		UnLock (rlock);
	    }
	}
	else
	{
	    Close (fh);
	}
    }
    else
    {
	printf ("couldn't create %s\n", RCS_LINK);
    }

    while (ln = RemHead (&DeleteList))
    {
	DeleteFile (ln->ln_Name);
	FreeVec (ln);
    }
}
