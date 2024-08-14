/****** MWLib/c/TokenTools ****************************************************

    NAME
        TokenTools -- tools for parsing and tokenization

    AUTHOR
        Copyright © 1990-91 Mitchell/Ware Systems. All rights reserved.

    VERSION
        1.02    14 Feb 90
        1.03    02 Jan 1991
        1.04    22 Feb 1991 - documentation cleanup
        1.05    22 Feb 1991 - NULL pointer on strings problem
        1.06    24 Feb 1991 - Double-quote token processing - esc. seq. fixed

    SYNOPSIS
        tt_GetToken -- parse a buffer into tokens

******* MWLib/c/TokenTools/tt_GetToken ****************************************

    NAME
        tt_GetToken -- parse a buffer into tokens

    SYNOPSIS
        char *tt_GetToken(  char    *buf,
                            char    *tok_buf,
                            TTBreak *break_array,
                            TTBreak **bentry,
                            TTBreak *keyword_array,
                            TTBreak **kentry,
                            char    *keyword_break)

    FUNCTION
        Parses and tokenizes the given buffer one token per call. Passes
        token and next position in buffer to parse back to caller.

        Parse information present in the TTBreak array must conform to the
        wildcard conventions of stcpm(). This may change in the future.

        Escape-sequences are processed within string-tokens, as follows:

            \a
            \b
            \f      form-feed
            \n      newline
            \r      carriage-return
            \t      tab
            \v      vertical tab
            \0nnn   octal character
            \xnnn   hex character
            \dnnn   decimal character

    INPUTS
        buf             - buffer to tokenize

        tok_buf         - where to place NULL-terminated token (size
                          of 256 is assumed- you were warned!)

        break_array     - NULL-ENTRY-terminated array of break sequence
                          or NULL

        pentry          - place to return either NULL or break entry.
                          If token was found in break array, returns
                          pointer to entry. Otherwise, returns NULL
                          here if token was not a break sequence.

        keyword_array   - NULL-ENTRY-terminated array of keywords
                          or NULL.

        kentry          - place to return either NULL or break entry.
                          If token was found in break array, returns
                          pointer to entry. Otherwise, returns NULL
                          here if token was not a break sequence. kentry
                          can also be NULL if you like.

        keyword_break   - characters that MUST surrond the keyword
                          for it to be considered a keyword! MUST be
                          present if keyword_array is given.

                          tt_GetToken() will process characters between
                          two '"'s as a single token UNLESS the '"' DOES NOT
                          appears as one of the keyword_break characters.
                          In that case, the '"' is treated no differently
                          from any other character.

    RESULTS
        Returns a pointer to the next token in the sequence, or NULL
        if end of null-terminated buffer is reached.

    NOTES
        Uses SAS's stcpm() pattern matching. TTBreak entries must conform
        to stcpm()'s wildcard spec. This is the most common 'gotcha', so
        be aware of this.

    SEE ALSO

******************************************************************************/

#include <exec/types.h>
#include "Tools.h"

char *tt_GetToken(b, tok_buf, brk, brk_ent, kwd, kwd_ent, kwd_brk)
char *b;
char tok_buf[];
struct TTBreak brk[];      /* can be NULL */
struct TTBreak **brk_ent;  /* can be NULL */
struct TTBreak kwd[];      /* can be NULL */
struct TTBreak **kwd_ent;  /* can be NULL */
char *kwd_brk;      /* keyword adjacent break characters
                       (must be present if kwd is non-null) */
{
    struct TTBreak *p, *pb, *pk;
    int c, i;

    /* skip over any leading whitespace
    */
    b = stpblk(b);

    /*------------------------------------------------------------------
        Now we'll check to see if the next character is a '"'. If so,
        we will check kwd_brk for the inclusion of the '"'. If it is
        present, we will ignore the '"' as a delimiter for a string
        of characters to be treated as a single token. If it is
        present, we will look for the matching '"', and process any
        '\' as escape sequences.
    -------------------------------------------------------------------*/
    if (*b == '\"' && kwd_brk && strchr(kwd_brk, '\"'))
    {
        for (i = 0, ++b; *b; ++i, ++b)
            if (*b == '\\')
            {
                switch(*++b)
                {
                case 'a':   tok_buf[i] = '\a'; break;
                case 'b':   tok_buf[i] = '\b'; break;
                case 'f':   tok_buf[i] = '\f'; break;
                case 'n':   tok_buf[i] = '\n'; break;
                case 'r':   tok_buf[i] = '\r'; break;
                case 't':   tok_buf[i] = '\t'; break;
                case 'v':   tok_buf[i] = '\v'; break;
                case 'x':
                case 'X':   tok_buf[i] = strtol(++b, &b, 16); --b; break;
                case 'D':
                case 'd':   tok_buf[i] = strtol(++b, &b, 10); --b; break;
                case '0':   tok_buf[i] = strtol(b, &b, 8);    --b; break;
                default:    tok_buf[i] = *b; break;
                }
            }
            else if (*b == '"')
            {
                ++b;
                break;
            }
            else
                tok_buf[i] = *b;

        tok_buf[i] = '\0';
        if (brk_ent) *brk_ent = NULL;
        if (kwd_ent) *kwd_ent = NULL;
        return b;
    }

    /* scan for break sequence
    */
    if (brk)
        for (pb = brk; pb->brk; ++pb)
            pb->wk_cnt = stcpma(b, pb->brk);

    /* scan for keyword
    */
    if (kwd)
        for (pk = kwd; pk->brk; ++pk)
            if (c = pk->wk_cnt = stcpma(b, pk->brk))
            {
                /*------------------------------------------------
                   check character after keyword
                   to see if it is one of the break characters
                --------------------------------------------------*/
                if (b[c] && !strchr(kwd_brk, b[c]))
                    pk->wk_cnt = NULL;
            }

    /*-----------------------------------------------------------------------
       So now we have the arrays (if they exist) brk[] and kwd[] marked for
       whether a match was found. Now we must sift through both arrays
       and find the highest priority match.
    ------------------------------------------------------------------------*/
    for (i = 0, pb = NULL; brk && brk[i].brk; ++i)
        if (brk[i].wk_cnt && (!pb || brk[i].pri > pb->pri))
            pb = &brk[i];

    for (i = 0, pk = NULL; kwd && kwd[i].brk; ++i)
        if (kwd[i].wk_cnt && (!pk || kwd[i].pri > pk->pri))
            pk = &kwd[i];

    /*-------------------------------------------------------------------
        Now we have pb and pk either NULL or pointing to the best match.
        Make a decision between them if both are non-NULL.
    ---------------------------------------------------------------------*/
    p = NULL;

    if (brk_ent)
    {
        *brk_ent = NULL;
        if (pb)
            if (!pk || pb->pri > pk->pri)
            {
                pk = NULL;
                p = *brk_ent = pb;
            }
    }

    if (kwd_ent)
    {
        *kwd_ent = NULL;
        if (pk)
            if (!pb || pk->pri > pb->pri)
            {
                pb = NULL;
                p = *kwd_ent = pk;
            }
    }

    /*----------------------------------------------------------------
        At this point, p points to the entry, or is NULL.
        Place token in buffer and return a pointer to the next area
        after token. If p is NULL, then a the token is NOT a keyword
        or a break sequence.
    -----------------------------------------------------------------*/

    if (p)  /* do we have a keyword/break token on our hands? */
    {
        if (tok_buf)
            memcpy(tok_buf, b, p->wk_cnt), tok_buf[p->wk_cnt] = NULL;

        return b + p->wk_cnt;
    }
    else // not a keyword/break,
    {
        b = stptok(b, tok_buf, 256, kwd_brk);
        return (*b) ? b : NULL;
    }
}
