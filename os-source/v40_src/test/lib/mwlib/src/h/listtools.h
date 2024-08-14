/******************************************************************************

    ListTools & LinkTools - structures

    Mitchell/Ware Systems, Inc.         Version 1.00            12 Nov 1990

******************************************************************************/

#ifndef _LIST_TOOLS_
#define _LIST_TOOLS_

typedef struct LTList   {
    struct LTList *next, *prev;
    } LTList;

#endif
