/* LoadXRefLVO.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

LONG ASM LVOLoadXRef (REG (a6) struct AGLib *ag, REG (a0) BPTR lock, REG (a1) STRPTR name)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct List *list = &agt->agt_XRefList;
    struct XRefFile *xrf;
    LONG retval = 0L;
    BOOL load = TRUE;

    /* We need exclusive access */
    ObtainSemaphore (&agt->agt_Lock);

    /* Does this already exist? */
    if ((xrf = (struct XRefFile *) FindNameI (&agt->agt_XRefFileList, name)) &&
	(list->lh_TailPred != (struct Node *) list))
    {
	/* Is it the same directory and is it loaded already? */
	if ((SameLock (lock, xrf->xrf_Lock)) == LOCK_SAME)
	{
	    if (xrf->xrf_Flags & XRF_LOADED)
	    {
		load = FALSE;
		retval = 2;
	    }
	}
    }

    /* Should we load it? */
    if (load)
    {
	/* Make a new entry */
	if (xrf || (xrf = AllocXRefFile (ag, lock, name)))
	{
	    /* Load it */
	    if (retval = loadxref (ag, xrf->xrf_Lock, xrf->xrf_Node.ln_Name, 0))
	    {
		/* Mark it as being loaded */
		xrf->xrf_Flags |= XRF_LOADED;
	    }
	}
    }

    ReleaseSemaphore (&agt->agt_Lock);

    return (retval);
}
