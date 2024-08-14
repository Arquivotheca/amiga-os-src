/****** mwhead/TokenTools *****************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

    TokenTools

******************************************************************************/

#ifndef _TOKEN_TOOLS_
#define _TOKEN_TOOLS_

typedef struct TTBreak {
    char *brk;          /* break sequence - one or more characters to break on */
    short pri;          /* priority for match   */
    void *(*func)();    /* function associated with this break  */
    ULONG anc, anc2, anc3; /* any ancillary data */

    /* Work area- do not touch
    */
    short wk_cnt;       /* length of matched area */
    } TTBreak;

#endif
