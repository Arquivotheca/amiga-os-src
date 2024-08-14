/*
 * Expert Dictionary Information
 */
#ifndef        __EGCEXP__
#define        __EGCEXP__

#ifdef MAC
#include <Types.h>
#endif

/* Code conversion functions */
typedef struct _DICS {
    UBYTE           name[64];
    UBYTE           id;
}               DICS,*PDICS;

#endif    /* __EGCEXP__ */
