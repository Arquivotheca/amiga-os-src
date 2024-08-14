#ifndef LOCALESTR_DEVS_H
#define LOCALESTR_DEVS_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/****************************************************************************/


#ifdef PRINTER_DEVICE
#define MSG_PRI_DEV_TROUBLE 0
#define MSG_PRI_DEV_TROUBLE_STR "Printer Trouble:\n%s"

#define MSG_PRI_DEV_DOQUERY 1
#define MSG_PRI_DEV_DOQUERY_STR "Resume|Cancel"

#define MSG_PRI_DEV_OFFLINE 2
#define MSG_PRI_DEV_OFFLINE_STR "Make printer on-line"

#define MSG_PRI_DEV_NOPAPER 3
#define MSG_PRI_DEV_NOPAPER_STR "Out of paper"

#define MSG_PRI_DEV_UNKNOWN 4
#define MSG_PRI_DEV_UNKNOWN_STR "Check printer and cabling"

#define MSG_PRI_DEV_MAYBEPAPER 5
#define MSG_PRI_DEV_MAYBEPAPER_STR "Check printer and paper"

#endif /* PRINTER_DEVICE */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
#ifdef PRINTER_DEVICE
    {MSG_PRI_DEV_TROUBLE,(STRPTR)MSG_PRI_DEV_TROUBLE_STR},
    {MSG_PRI_DEV_DOQUERY,(STRPTR)MSG_PRI_DEV_DOQUERY_STR},
    {MSG_PRI_DEV_OFFLINE,(STRPTR)MSG_PRI_DEV_OFFLINE_STR},
    {MSG_PRI_DEV_NOPAPER,(STRPTR)MSG_PRI_DEV_NOPAPER_STR},
    {MSG_PRI_DEV_UNKNOWN,(STRPTR)MSG_PRI_DEV_UNKNOWN_STR},
    {MSG_PRI_DEV_MAYBEPAPER,(STRPTR)MSG_PRI_DEV_MAYBEPAPER_STR},
#endif /* PRINTER_DEVICE */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* LOCALESTR_DEVS_H */
