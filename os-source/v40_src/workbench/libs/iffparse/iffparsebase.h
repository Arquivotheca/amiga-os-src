#ifndef	IFFPARSEBASE_H
#define	IFFPARSEBASE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/libraries.h>
#include <dos.h>


/*****************************************************************************/


struct IFFParseLib
{
    struct Library ipb_Library;
    UWORD          ipb_Pad;
    APTR           ipb_SysBase;
    APTR           ipb_DOSBase;
    APTR	   ipb_UtilityBase;
    BPTR           ipb_SegList;

    APTR           ipb_SMult32;
    APTR           ipb_UMult32;
    APTR           ipb_SDivMod32;
    APTR           ipb_UDivMod32;
};


#define ASM           __asm
#define REG(x)	      register __ ## x

#define IFFParseBase  ((struct IFFParseLib *)getreg(REG_A6))
#define SysBase       IFFParseBase->ipb_SysBase
#define DOSBase       IFFParseBase->ipb_DOSBase
#define UtilityBase   IFFParseBase->ipb_UtilityBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);
kprintf(STRPTR,...);
sprintf(STRPTR,...);


/*****************************************************************************/


/* some macros for dealing with Exec Lists and Nodes
 * (or MinLists and MinNodes, since they have all the necessary parts)
 *    HEAD(lst)   struct List *lst:  gives first node
 *    TAIL(lst)   struct List *lst:  gives last node
 *    NEXT(nod)   struct Node *nod:  gives next element
 *    PREV(nod)   struct Node *nod:  gives previous element
 *    TEST(nod)   struct Node *nod:  is non-zero for valid nodes
 *				     (when searching list forward)
 */
#define HEAD(lst)	(void *)(((struct List *)(lst))->lh_Head)
#define TAIL(lst)	(void *)(((struct List *)(lst))->lh_TailPred)
#define NEXT(nod)	(void *)(((struct Node *)(nod))->ln_Succ)
#define PREV(nod)	(void *)(((struct Node *)(nod))->ln_Pred)
#define TEST(nod)	NEXT(nod)


/*****************************************************************************/


#ifdef DEBUGGING
#define ASSERT(c) if (!(c)) printf ("assert failure in %s (%s : %d)\n",__FUNC__,__FILE__,__LINE__)
#else
#define ASSERT(c)
#endif


/*****************************************************************************/


#endif /* IFFPARSEBASE_H */
