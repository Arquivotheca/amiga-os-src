/******* mwhead/LinkTools *****************************************************
    NAME
        LinkTools.h -- Structures and such for LinkTools

    VERSION
        1.01
******************************************************************************/

#ifndef _LINK_TOOLS_
#define _LINK_TOOLS_

#include "ListTools.h"

typedef struct Link    {
    struct LTList list; // orthogonal List
    void *myself;       // pointer to container struct, or NULL for header
    } Link, LinkT;

// Macros

#define InitLinkHead(h)     { InitLTHead(&(h)->list); (h)->myself = NULL; }
#define InitCLinkHead(h, c) { InitLTHead(&(h)->list); (h)->myself = (c); }

#define NextLink(l)         ((Link *) (l)->list.next)
#define PrevLink(l)         ((Link *) (l)->list.prev)
#define Container(l)        ((l)->myself)
#define DeleteLink(l)       DeleteLT(&(l)->list)
#define JoinLinks(h1, h2)   JoinLTLists(&(h1)->list, &(h2)->list)
#define FSplitLinks(h1, h2) FSplitLTLists(&(h1)->list, &(h2)->list)
#endif
