/******************************************************************************
    sfix_dec.c - Convert a long to a fixed decimal

    Mitchell/Ware Systems           Version 1.02            7/9/88

_______________________________________________________________________________

    sfix_dec (string, fix, long)

        returns length of string.
        string is null terminated.

    sdec_fix (string, fix)

        returns long int, properly aligned.
        string must be null-terminated


    fixof(string)
        returns the fix to use to represent the string.
_______________________________________________________________________________

    EDIT HISTORY:
        7/26/90 -   Added fixof(). FM


******************************************************************************/

#include <exec/types.h>
#include <math.h>

static int power[] = { 1L,
                       10L,
                       100L,
                       1000L,
                       10000L,
                       100000L,
                       1000000L,
                       10000000L,
                       100000000L,
                       1000000000L };


int fixof(s)
char *s;
{
    BOOL  point = FALSE;    /* decimal point has been reached */
    int fix = 0;

    s = stpblk(s);

    do
    {
        if (*s == '-')
            ++s;

        if (*s == '.')
            ++s, point = TRUE;

        if (!isdigit(*s))
            break;  /* break out if I encoounter non-digit garbage! */

        if (point)
            ++fix;
    } while (*s++);

    return fix;
}

int sfix_dec(s, fix, l)
char *s;
unsigned int fix;
long l;
{
    BOOL neg = FALSE;
    long w, f;
    int cnt = 0, i;

    if (l < 0)
    {
        neg = TRUE;
        l = -l;
        *s++ = '-';
        ++cnt;
    }

    w = l / power[fix];
    f = l % power[fix];
    cnt += i = stcl_d(s, w);
    s += i;
    *s++ = '.';
    for (i = 0, s += fix; i < fix; ++i, f /= 10)
        *--s = '0' + f % 10;
    /* now s is back to pointing to one after the decimal point. fix that. */
    s += fix;
    *s = '\0';
    cnt += fix;
    return cnt;
}

long sdec_fix(s, fix)
char *s;
unsigned int fix;
{
    long w, f, l;
    BOOL neg = FALSE;
    int cnt;

    s = stpblk(s);      /* skip whitespace */

    s += stcd_l(s, &w);
    if (w < 0)
        w = -w,
        neg = TRUE;
    
    s = stpblk(s);
    if (*s++ == '.')
    {
        s = stpblk(s);
        cnt = stcd_l(s, &f);
        
        if (cnt < fix)
            f *= power[fix - cnt];
        else
            f /= power[cnt - fix];
    }
    else
        f = 0;
    
    l = w * power[fix] + f;
    if (neg)
        l = -l;

    return l;
}