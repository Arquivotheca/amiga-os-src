#ifndef SETALERT_STRINGS_H
#define SETALERT_STRINGS_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif


/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_NOALERTS 0
#define MSG_FOREVER 1
#define MSG_SHOWTIME 2
#define MSG_CONTINUE 3

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_NOALERTS_STR "System alerts have been disabled."
#define MSG_FOREVER_STR "System alerts will not timeout."
#define MSG_SHOWTIME_STR "System alerts timeout in %ld seconds."
#define MSG_CONTINUE_STR "Continue"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_NOALERTS,(STRPTR)MSG_NOALERTS_STR},
    {MSG_FOREVER,(STRPTR)MSG_FOREVER_STR},
    {MSG_SHOWTIME,(STRPTR)MSG_SHOWTIME_STR},
    {MSG_CONTINUE,(STRPTR)MSG_CONTINUE_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x00\x00\x22"
    MSG_NOALERTS_STR "\x00"
    "\x00\x00\x00\x01\x00\x20"
    MSG_FOREVER_STR "\x00"
    "\x00\x00\x00\x02\x00\x26"
    MSG_SHOWTIME_STR "\x00"
    "\x00\x00\x00\x03\x00\x0A"
    MSG_CONTINUE_STR "\x00\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


#ifdef CATCOMP_CODE

STRPTR GetString(struct LocaleInfo *li, LONG stringNum)
{
LONG   *l;
UWORD  *w;
STRPTR  builtIn;

    l = (LONG *)CatCompBlock;

    while (*l != stringNum)
    {
        w = (UWORD *)((ULONG)l + 4);
        l = (LONG *)((ULONG)l + (ULONG)*w + 6);
    }
    builtIn = (STRPTR)((ULONG)l + 6);

#define XLocaleBase LocaleBase
#define LocaleBase li->li_LocaleBase
    
    if (LocaleBase)
        return(GetCatalogStr(li->li_Catalog,stringNum,builtIn));
#define LocaleBase XLocaleBase
#undef XLocaleBase

    return(builtIn);
}


#endif /* CATCOMP_CODE */


/****************************************************************************/


#endif /* SETALERT_STRINGS_H */
