/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.
    
    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by 
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.    
*/
/*     Parse.c - parse an input stream
*
*      Mitchell/Ware           Version 2.00            10/18/87
*
*      char *parse(p)
*      PARSE *p
*
*       NOTE: PARSE is a extremely sophisticated configurable parser of
*               an ASCII input stream. Parse characters and Comment characters
*               can both be specified. All input less than a SPACE is
*               converted to a SPACE. Intrinsically special characters are
*               the SPACE and the DOUBLE-QUOTE. The DOUBLE-QUOTES causes
*               everything between them to be passed as one word.
*
*       BUGS: If DOUBLE-QUOTES are not properly matched up (i.e. odd) then
*               the system will go to hell in a hand-basket. 
*/

#include <parse.h>
#include <ascii.h>

#define comskips        (p->comskip || p->comskip2)
#define member(q, d)    (stpchr(q->plist, d) || (q->bcom && stpchr(q->bcom, d)) || (q->ecom && stpchr(q->ecom, d)))

char   *stpchr(), _getachar();

char   *parse(p)
PARSE  *p;
{
        char c, *w;

        while (c = _getachar(p))
        {
                if (p->bcom && !p->comskip2 && !p->skip)
                {
                        if (!p->pc) 
                                p->pc = p->bcom;
                                
                        if (!*p->pc) /* if perfect delim. match */
                        {
                                p->comskip2 = TRUE;
                                p->pc = p->ecom;
                        }
                        else if (*p->pc != c) /* if no char match */
                        {
                                if (p->pc != p->bcom) /* if matches were accum. */
                                {
                                        p->pushback = c;
                                        p->bc = p->bcom; /* simulated pusback   */
                                        p->comskip = FALSE;
                                        c = _getachar(p);
                                }
                        }
                        else /* another matching character found */
                        {
                                p->comskip = TRUE;
                                ++p->pc;
                        }
                }
                
                if (!comskips && !p->skip && (!c || (c == SP) || stpchr(p->plist, c))) /* if c is a parse character */
                {
                        if (p->w)       /* if characters are accumulated */
                        {
                                w = AllocRemember(&p->key, p->w+1, MEMF_CLEAR);
                                movmem(p->word, w, p->w);
                                p->w = 0;
                                p->pushback = (c == SP) ? NULL : c ;
                                return w;
                        }
                        else if (c != SP) /* if no characters are accumulated */
                        {
                                if (!c)
                                        return NULL;

                                w = AllocRemember(&p->key, 2, MEMF_CLEAR);
                                *w = c;
                                return w;
                        }
                }
                else if (!comskips) /* c is not a parse character */
                {
                        if (c == '"')
                                p->skip = !p->skip;
                        else
                                p->word[p->w++] = c;
                }
                
                if (p->ecom && p->comskip2)
                {
                        if (!*p->pc) /* if match */
                        {
                                p->pc = NULL;
                                p->comskip = p->comskip2 = FALSE;
                        }
                        else if (*p->pc++ != c)
                                p->pc = p->ecom;
                }
        }
}

char   _getachar(p)
PARSE  *p;
{
        char c;

        if (p->bc) /* simulated pushback activated */
        {
                c = *p->bc++;
                if (p->bc >= p->pc)
                        p->bc = p->pc = NULL;
        }
        else if (c = p->pushback)
                p->pushback = NULL;
        else
                c = (*p->getachar)(p);

        if (!p->skip && c && (c < SP) && !member(p, c)) 
                c = SP;
        return c;
}
