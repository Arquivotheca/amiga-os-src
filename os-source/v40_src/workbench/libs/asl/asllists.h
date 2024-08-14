#ifndef ASLLISTS_H
#define ASLLISTS_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#include "aslbase.h"


/*****************************************************************************/


VOID ASM NewList(REG(a0) struct List *list);
VOID ASM FreeList(REG(a0) struct List *list);
APTR ASM AllocNode(REG(d0) ULONG nodeSize);
APTR ASM AllocNamedNode(REG(d0) ULONG nodeSize, REG(a0) STRPTR name);

struct Node * ASM FindNum(REG(a0) struct List *list, REG(d0) UWORD number);
ULONG ASM FindNodeNum(REG(a0) struct List *list, REG(a1) struct Node *node);
struct Node * ASM FindNameNC(REG(a0) struct List *list, REG(a1) STRPTR name);

/* The following assume the lists are maintained in alphabetical order */
VOID ASM EnqueueAlpha(REG(a0) struct List *list, REG(a1) struct Node *new);
struct Node * ASM FindClosest(REG(a0) struct List *list, REG(a1) STRPTR name);


/*****************************************************************************/


APTR ASM AllocNamedNode2(REG(d0) ULONG nodeSize, REG(d1) UBYTE leading,
                         REG(a0) STRPTR name);

/* this one sticks the byte 'leading' in front of the name string. This is
 * used by the file requester as the main sorting key for the different types
 * of items displayed in the requester. The byte is stored in memory just
 * before the value pointed to by ln_Name. Decrement ln_Name by one before
 * doing a sorting operation.
 */

VOID ASM EnqueueAlpha2(REG(a0) struct List *list, REG(a1) struct Node *new);


/*****************************************************************************/


#endif /* ASLLISTS_H */
