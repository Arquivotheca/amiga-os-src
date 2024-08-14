/* env.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/dosasl.h"
#include <string.h>

#include "libhdr.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"

#define SAME 0

LONG ASM FunkyMatchFirst (REG(d1) UBYTE *pat,
			  REG(d2) struct AnchorPath *anchor);

/* strange returns - must be the same as IoErr() */

LONG ASM
FunkyMatchFirst (REG(d1) UBYTE *pat, REG(d2) struct AnchorPath *anchor)
{
	/* if it's * or it has a : and is not a filesystem */
	setresult2(0);
	if (mystricmp(pat,"*") == SAME ||
	    mystricmp(pat,"CONSOLE:") == SAME ||
	    mystricmp(pat,"NIL:") == SAME ||
	    (strchr(pat,':') && !isfilesystem(pat)))
	{
		LONG res;

		/* either res = 0 ("*", new handler) or it equals	    */
		/* unknown action (old handler), or there was a real error. */
		/* the first two are ok, the third should be returned as an */
		/* error.						    */
		res = getresult2();
		if (res && res != ERROR_ACTION_NOT_KNOWN)
			return res;

		/* need to allocate an AChain, fill in some stuff
		   and make it so MatchNext will return no_more_entries */

		anchor->ap_Base    = 
		anchor->ap_Current = AllocVecPubClear(sizeof(struct AChain));
		if (!(anchor->ap_Current))
			return ERROR_NO_FREE_STORE;  /* ioerr will be set */

		/* an_child is null, which is needed for MatchEnd */
		anchor->ap_Current->an_Lock = copydir(currentdir(FALSE,NULL));

		/* says that MatchNext should return NO_MORE_ENTRIES */
		/* also that this is the top-level node.	     */
		anchor->ap_Current->an_Flags = DDF_Completed | DDF_Single;

		strcpy(anchor->ap_Info.fib_FileName,pat);
		anchor->ap_Info.fib_DirEntryType =
		anchor->ap_Info.fib_EntryType	 = -1;	/* file */

		setresult2(FALSE);
		return FALSE;
	}

	return FindFirst(pat,anchor);
}
