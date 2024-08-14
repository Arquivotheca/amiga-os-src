
/* Original C version of matchix.asm */

#include "commoditiesbase.h"
#include "matchix.h"


/*****************************************************************************/


/* yields 0 (a match: nomatch is false)
 * if value V matches X for all bits set in mask M
 */
#define IX_NOMATCH(V, X, M) ( ((V) ^ (X)) & (M) )

/* if any bits of mask are set in qual, evaluates to qual | mask, else qual */
#define IXQ_SYNONYM(qual, mask) (((qual) & (mask))? ((qual) | (mask)): (qual))


/*****************************************************************************/


BOOL ASM MatchIXLVO(REG(a0) struct InputEvent *ie,
		    REG(a1) struct InputXpression *ix)
{
UWORD qual;
UWORD syns;

    if (NULL_IX(ix))
        return(TRUE);

    if (ix->ix_Class && (ie->ie_Class != ix->ix_Class))
        return(FALSE);

    if (IX_NOMATCH(ie->ie_Code,ix->ix_Code,ix->ix_CodeMask))
        return(FALSE);

    /* for qualifier test add synonym bits */
    qual = ie->ie_Qualifier;
    if (syns = ix->ix_QualSame)
    {
        if (syns & IXSYM_SHIFT) qual = IXQ_SYNONYM(qual, IXSYM_SHIFTMASK);
        if (syns & IXSYM_CAPS)  qual = IXQ_SYNONYM(qual, IXSYM_CAPSMASK);
        if (syns & IXSYM_ALT)   qual = IXQ_SYNONYM(qual, IXSYM_ALTMASK);
    }

    if (IX_NOMATCH(qual,ix->ix_Qualifier,ix->ix_QualMask))
        return(FALSE);

    return (TRUE);
}
