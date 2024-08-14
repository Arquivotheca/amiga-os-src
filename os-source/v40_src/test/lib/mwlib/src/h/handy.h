/*****************************************************************************
    Handy.h

    Mitchell/Ware Systems           Version 1.03            16-Feb-89

    Handy macros and such!
*****************************************************************************/

#ifndef _HANDY_
#define _HANDY_ 1

/*  iffree() - if a pointer is not NULL, frees it and sets it to NULL
*/
#define iffree(_thing) if (_thing) free(_thing), (_thing) = NULL;

/*  ScrewAlloc() - callocate the thing and if that fails, screwup until
                   either the thing gets allocated or a 'give up' is
                   requested. If so, return a NULL!
*/
#define ScrewAlloc(_thing, _count, _message) \
    while (!((_thing) = calloc((_count), sizeof(*(_thing))))) \
        if (!warning(_message)) return NULL;


/*  ifnot() - the reverse of if()
*/
#define ifnot(_thing) if (!(_thing))


/*  Rename SetDef... WindowTools calls to SetWin...
*/
#define SetDefLeftTop   SetWinLeftTop
#define SetDefIDCMP     SetWinIDCMP
#define SetDefFlags     SetWinFlags
#define SetDefWH        SetWinWH

#endif
