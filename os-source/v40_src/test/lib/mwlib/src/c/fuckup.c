/****** MWlib/fuckup **********************************************************
    fuckup.c

    Mitchell/Ware Systems           Version DEAD            9/15/88

    OBSOLETE. Use FatalRequest, Instead.
*****************************************************************************/

#include <exec/types.h>

BOOL fuckup(m)  /* Program bug detected - could be fatal! */
char *m;    /* message */
{
    return fatal2(m, "(fuckup() is OBSOLETE!)");
}

BOOL fuckup2(m, n)  /* Program bug detected - could be fatal! */
char *m, *n;    /* message */
{
    return fatal3(m, n, "(fuckup2() is OBSOLETE!)");
}

BOOL fuckup3(m, n, o)  /* Program bug detected - could be fatal! */
char *m, *n, *o;    /* message */
{
    return fatal3(m, n, o);
}

BOOL screwup(m) /* Far less serious - non-fatal error */
char *m;    /* message */
{
    return warning2(m, "(screwup() is OBSOLETE!)");
}

BOOL screwup2(m, n) /* Far less serious - non-fatal error */
char *m, *n;    /* messages */
{
    return warning3(m, n, "(screwup2() is OBSOLETE!)");
}

BOOL screwup3(m, n, o) /* Far less serious - non-fatal error */
char *m, *n, *o;    /* messages */
{
    return warning3(m,n,o);
}
