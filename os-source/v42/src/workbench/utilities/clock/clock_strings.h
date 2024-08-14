#ifndef LOCALESTR_UTILITIES_H
#define LOCALESTR_UTILITIES_H


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

#define MSG_NOTHING 0
#define MSG_PROGRAM_ERROR 1

#ifdef CLOCK
#define MSG_CLK_TITLE 6000
#define MSG_CLK_PROJECT_MENU 6001
#define MSG_CLK_PROJECT_ANALOG 6002
#define MSG_CLK_PROJECT_DIGITAL 6003
#define MSG_CLK_PROJECT_QUIT 6004
#define MSG_CLK_SETTINGS_MENU 6005
#define MSG_CLK_SETTINGS_DATE 6006
#define MSG_CLK_SETTINGS_SECONDS 6007
#define MSG_CLK_SETTINGS_DFORMAT 6008
#define MSG_CLK_SETTINGS_ALARM 6009
#define MSG_CLK_SETTINGS_SETALARM 6010
#define MSG_CLK_SETTINGS_SAVE 6011
#define MSG_CLK_ALARM_TITLE 6012
#define MSG_CLK_ALARM_USE_GAD 6013
#define MSG_CLK_ALARM_CANCEL_GAD 6014
#define MSG_CLK_ALARM_HOURS_GAD 6015
#define MSG_CLK_ALARM_MINUTES_GAD 6016
#endif /* CLOCK */

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


#include <dos.h>
STRPTR __asm GetString(register __a0 struct LocaleInfo *li,register __d0 ULONG id);



#endif /* LOCALESTR_UTILITIES_H */
