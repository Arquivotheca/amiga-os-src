/*   dll.c     doubly linked list
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

#include  "port.h"
#include  "cgif.h"
#include  "cache.h"

EXTERN DLL* MEMptr();


/* Returns TRUE if hp is empty list */
BOOLEAN
dll_empty (hp)
    MEM_HANDLE hp;
{
    DLL *p;

    p = MEMptr(hp);
    return hp == p->f;
}




/* Makes hp an empty list */
VOID
dll_null(hp)
    MEM_HANDLE hp;
{
    DLL *p;

    p = MEMptr(hp);
    p->b  = hp;
    p->f  = hp;
}




/* Removes p from a list */
VOID
dll_remove (hp)
    MEM_HANDLE hp;
{
    DLL *p, *prev, *next;
    
    p = MEMptr(hp);
    prev = MEMptr(p->b);
    next = MEMptr(p->f);

    prev->f = p->f;
    next->b = p->b;
    dll_null(hp);
}




/* Links q into p's list after p */
VOID
dll_after (hp, hq)
    MEM_HANDLE hp, hq;
{
    DLL *p,*q, *next;
    MEM_HANDLE hnext;

    p = MEMptr(hp);
    q = MEMptr(hq);

    q->b  = hp;
    hnext = p->f;
    q->f  = hnext;

    p->f = hq;
    next = MEMptr(hnext);
    next->b = hq;
}
