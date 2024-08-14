#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#define MSG_HELLO 0
#define MSG_HELLO_STR "Hello World!\n"

#define MSG_BYE 1
#define MSG_BYE_STR "Goodbye World!\n"


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {MSG_HELLO,MSG_HELLO_STR},
    {MSG_BYE,MSG_BYE_STR},
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
