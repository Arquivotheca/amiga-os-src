/******************************************************************************
    ListTools.c

    Fred Mitchell               Version 1.00                11/9/89

    Copyright  1989 Mitchell/Ware Systems, INC. All Rights Reserved.

_______________________________________________________________________________

    ListTools will allow you to manage a doubly-linked circular list with
        no perferred header. That is, any member of the list may act as
        a header for that list.

        A LTList is empty if ->next and ->prev is equal. The Header is not
        considered an actual element in the list per se. The Header pointers
        can never be NULL, but must point to itself, as is inited by InitLTHead.

        If the next and prev pointers of an LTList struct are actually
        NULL, then that LTList struct is considered a lone, isolated element
        not belonging to any list.


    InitLTHead (head)       initializes the head of the list.

    AddLTHead (head, list)  add single element to the head of list.

    AddLTTail (head, list)  add single element to the tail of list.

    DeleteLT (list)         delete this element from the list- notice that
                            the head is not needed!

    JoinLTLists(h1, h2)     join two lists toghether, making them one. h2 will
                            be joined to the tail-end of h1.

    FSplitLTLists(h1, h2)   foward-seperate one list into two

        struct LTList *head;
        struct LTList *list;
        struct LTList *h1, *h2;
******************************************************************************/

#include <exec/types.h>
#include "Tools.h"
#include "protos.h"

struct LTList *InitLTHead(struct LTList *h)
{
    return h->next = h->prev = h;
}

struct LTList *AddLTHead(struct LTList *h, struct LTList *l)
{
    l->next = h->next;
    l->prev = h;
    h->next->prev = l;
    h->next = l;

    return l;
}

struct LTList *AddLTTail(struct LTList *h, struct LTList *l)
{
    l->prev = h->prev;
    l->next = h;
    h->prev->next = l;
    h->prev = l;

    return l;
}

struct LTList *DeleteLT(struct LTList *l)
{
    l->next->prev = l->prev;
    l->prev->next = l->next;
    l->next = l->prev = NULL;

    return l;
}

void JoinLTLists(struct LTList *h1, struct LTList *h2)
{
    struct LTList *p1;

    p1 = h1->prev;

    h1->prev->next = h2;
    h1->prev = h2->prev;
    h2->prev->next = h1;
    h2->prev = p1;
}

void FSplitLTLists(struct LTList *h1, struct LTList *h2)
{
    struct LTList *p1;

    p1 = h1->prev;

    h1->prev = h2->prev;
    h2->prev->next = h1;
    h2->prev = p1;
    p1->next = h2;
}
