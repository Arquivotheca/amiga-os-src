/* things which both displaypacket and testdevice can use */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "packmonCommon_protos.h"

/* strCpy =========================================
 */
VOID strCpy(STRPTR dest, STRPTR src) {
    STRPTR a = dest, b = src;

    if ((!a) || (!b)) {
        return;
    }
    while (*b) {
        *a++ = *b++;
    }
    return;
}

/* strLen =========================================
 */
LONG strLen(STRPTR str) {
    STRPTR s;
    LONG res = 0L;

    if (str) {
        for (s = str ; *s != NULL; s++) {
            res++;
        }
    }
    
    return(res);
}

/* createPort =============================================================
 * to avoid the amiga.lib thing, which seems to have problems with "parms=r"
 * Since we use AllocVec, not AllocMem, you must use deletePort() to dispose
 * of it afterwards.
 */
struct MsgPort *createPort(STRPTR mpName, LONG mpPri) {
    struct MsgPort *mp;
    struct Node *n;

    if (mp = CreateMsgPort()) {
        n = &(mp->mp_Node);
        if (mpName) {
            n->ln_Name = AllocVec(strLen(mpName)+1UL, MEMF_CLEAR);
            if (n->ln_Name) {
                strCpy(n->ln_Name, mpName);
                n->ln_Pri = mpPri;
                AddPort(mp);
                /* fall thru to return(mp) */
            }
            else {
                DeleteMsgPort(mp);
                return(NULL);
            }
        }
    }

    return(mp);
}

/* deletePort =============================================================
 * Again, avoid amiga.lib.  Will not RemPort() an unnamed port.
 * Uses FreeVec rather than FreeMem, so must be used with createPort() (above).
 */
VOID deletePort(struct MsgPort *mp) {
    if (mp) {
        if (mp->mp_Node.ln_Name && TypeOfMem(mp->mp_Node.ln_Name)) {
            Forbid();
            if (FindPort(mp->mp_Node.ln_Name)) {
                RemPort(mp);
            }
            Permit();
            FreeVec(mp->mp_Node.ln_Name);
        }
        DeleteMsgPort(mp);
    }

    return;
}

/* newList ==================================================================
 * yet again, avoid amiga.lib.  Modelled on the asm macro in exec/lists.i.
 * not sure about NewList, but you can call this one with NULL (does nothing).
 */
VOID newList(struct List *lyst) {

/*
    NEWLIST     MACRO   ; list
                MOVE.L  \1,LH_TAILPRED(\1)
                ADDQ.L  #4,\1       ;Get address of LH_TAIL
                CLR.L   (\1)        ;Clear LH_TAIL
                MOVE.L  \1,-(\1)    ;Address of LH_TAIL to LH_HEAD
                ENDM
*/

    if (lyst) {
        lyst->lh_TailPred = (struct Node *)lyst;
        lyst->lh_Tail = NULL;
        lyst->lh_Head = (struct Node *)(&(lyst->lh_Tail));  /* which is lyst+4... */
    }

    return;
}

