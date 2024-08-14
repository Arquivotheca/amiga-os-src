#ifndef LOCALESTR_C_H
#define LOCALESTR_C_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/****************************************************************************/


#define MSG_NOTHING 0
#define MSG_NOTHING_STR ""

#ifdef CONCLIP
#define MSG_CC_OK_GAD 7000
#define MSG_CC_OK_GAD_STR "OK"

#define MSG_CC_NOIFFPARSE 7001
#define MSG_CC_NOIFFPARSE_STR "Requires iffparse.library V39"

#define MSG_CC_NOCLIPBOARD 7002
#define MSG_CC_NOCLIPBOARD_STR "Requires clipboard.device"

#define MSG_CC_NOMEMORY 7003
#define MSG_CC_NOMEMORY_STR "Not enough memory"

#endif /* CONCLIP */

#ifdef IPREFS
#define MSG_IP_ERROR 23000
#define MSG_IP_ERROR_STR "Error"

#define MSG_IP_REQTITLE 23001
#define MSG_IP_REQTITLE_STR "Preferences"

#define MSG_IP_CLOSEWINDOW_PROMPT 23002
#define MSG_IP_CLOSEWINDOW_PROMPT_STR "Intuition is attempting to reset the\nWorkbench screen. Please close all windows,\nexcept drawers."

#define MSG_IP_CLOSEWINDOW_GADGETS 23003
#define MSG_IP_CLOSEWINDOW_GADGETS_STR "Retry|Cancel"

#define MSG_IP_CONTINUE_GADGET 23005
#define MSG_IP_CONTINUE_GADGET_STR "Continue"

#define MSG_IP_ERROR_NO_IFFPARSE_LIB 23008
#define MSG_IP_ERROR_NO_IFFPARSE_LIB_STR "ERROR: couldn't open iffparse.library V39"

#define MSG_IP_ERROR_NO_LOCALE_LIB 23009
#define MSG_IP_ERROR_NO_LOCALE_LIB_STR "ERROR: couldn't open locale.library V38"

#define MSG_IP_ERROR_NO_DISKFONT_LIB 23010
#define MSG_IP_ERROR_NO_DISKFONT_LIB_STR "ERROR: couldn't open diskfont.library V38"

#define MSG_IP_ERROR_CANT_OPEN 23013
#define MSG_IP_ERROR_CANT_OPEN_STR "ERROR: can't open file\n%s"

#define MSG_IP_ERROR_BAD_PREFS_FILE 23014
#define MSG_IP_ERROR_BAD_PREFS_FILE_STR "ERROR: can't read %s"

#define MSG_IP_ERROR_NO_KEYMAP 23015
#define MSG_IP_ERROR_NO_KEYMAP_STR "ERROR: can't load keymap\n%s"

#define MSG_IP_ERROR_NO_DATATYPES_LIB 23016
#define MSG_IP_ERROR_NO_DATATYPES_LIB_STR "ERROR: couldn't open datatypes.library V39"

#define MSG_IP_ERROR_NO_FONT 23017
#define MSG_IP_ERROR_NO_FONT_STR "ERROR: can't load font\n%s"

#define MSG_IP_ERROR_NO_SOUND 23018
#define MSG_IP_ERROR_NO_SOUND_STR "ERROR: can't load sound\n%s"

#define MSG_IP_ERROR_NO_PICTURE 23019
#define MSG_IP_ERROR_NO_PICTURE_STR "ERROR: can't load picture\n%s"

#endif /* IPREFS */

#ifdef MOUNT
#define MSG_MT_REQ_TITLE 31000
#define MSG_MT_REQ_TITLE_STR "Mount Failure"

#define MSG_MT_OK_GAD 31001
#define MSG_MT_OK_GAD_STR "OK"

#define MSG_MT_ERROR 31002
#define MSG_MT_ERROR_STR "ERROR: \0"

#define MSG_MT_NOMEM 31003
#define MSG_MT_NOMEM_STR "Not enough memory\n"

#define MSG_MT_OPEN 31004
#define MSG_MT_OPEN_STR "Can't open file '%s'\n"

#define MSG_MT_ALREADY_MOUNTED 31005
#define MSG_MT_ALREADY_MOUNTED_STR "Device '%s' is already mounted\n"

#define MSG_MT_DEVICE_NOT_FOUND 31006
#define MSG_MT_DEVICE_NOT_FOUND_STR "Device '%s' not found in file '%s'\n"

#define MSG_MT_UNKNOWN_KEYWORD 31007
#define MSG_MT_UNKNOWN_KEYWORD_STR "'%s' is not a valid keyword\n       (file '%s', line %ld, column %ld)\n"

#define MSG_MT_STRING_EXPECTED 31008
#define MSG_MT_STRING_EXPECTED_STR "Keyword '%s' requires a string\n       (file '%s', line %ld, column %ld)\n"

#define MSG_MT_NUMBER_EXPECTED 31009
#define MSG_MT_NUMBER_EXPECTED_STR "Keyword '%s' requires a number\n       (file '%s', line %ld, column %ld)\n"

#define MSG_MT_EQUAL_EXPECTED 31010
#define MSG_MT_EQUAL_EXPECTED_STR "Expecting '='\n       (file '%s', line %ld, column %ld)\n"

#define MSG_MT_ARGUMENT_EXPECTED 31011
#define MSG_MT_ARGUMENT_EXPECTED_STR "Expecting argument for keyword '%s'\n       (file '%s', line %ld, column %ld)\n"

#define MSG_MT_MISSING_KEYWORDS 31012
#define MSG_MT_MISSING_KEYWORDS_STR "Could not find some of the following keywords:\n       Surfaces, BlocksPerTrack, LowCyl, HighCyl, Device.\n"

#endif /* MOUNT */

#ifdef LOADRESOURCE
#define MSG_LR_RESTYPE_1 50000
#define MSG_LR_RESTYPE_1_STR "Library"

#define MSG_LR_RESTYPE_2 50001
#define MSG_LR_RESTYPE_2_STR "Device"

#define MSG_LR_RESTYPE_3 50002
#define MSG_LR_RESTYPE_3_STR "Font"

#define MSG_LR_RESTYPE_4 50003
#define MSG_LR_RESTYPE_4_STR "Catalog"

#define MSG_LR_RESTYPE_5 50004
#define MSG_LR_RESTYPE_5_STR "TYPE"

#define MSG_LR_FORMAT 50005
#define MSG_LR_FORMAT_STR "%-9s%s\n"

#define MSG_LR_NAME 50006
#define MSG_LR_NAME_STR "NAME\n"

#define MSG_LR_NO_RESOURCES 50007
#define MSG_LR_NO_RESOURCES_STR "No resources currently locked\n"

#define MSG_LR_ALREADY_LOCKED 50008
#define MSG_LR_ALREADY_LOCKED_STR "'%s' is already a locked resource\n"

#define MSG_LR_LOAD_ERROR 50009
#define MSG_LR_LOAD_ERROR_STR "Error while loading '%s' - \0"

#define MSG_LR_CANT_OPEN_DISKFONT 50010
#define MSG_LR_CANT_OPEN_DISKFONT_STR "Requires diskfont.library V37 - \0"

#define MSG_LR_COULD_NOT_LOAD 50011
#define MSG_LR_COULD_NOT_LOAD_STR "'%s' couldn't be loaded as a resource - \0"

#define MSG_LR_NOT_FOUND 50012
#define MSG_LR_NOT_FOUND_STR "'%s' is not a locked resource\n"

#define MSG_LR_ERROR 50013
#define MSG_LR_ERROR_STR "'%s' - \0"

#endif /* LOADRESOURCE */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {MSG_NOTHING,(STRPTR)MSG_NOTHING_STR},
#ifdef CONCLIP
    {MSG_CC_OK_GAD,(STRPTR)MSG_CC_OK_GAD_STR},
    {MSG_CC_NOIFFPARSE,(STRPTR)MSG_CC_NOIFFPARSE_STR},
    {MSG_CC_NOCLIPBOARD,(STRPTR)MSG_CC_NOCLIPBOARD_STR},
    {MSG_CC_NOMEMORY,(STRPTR)MSG_CC_NOMEMORY_STR},
#endif /* CONCLIP */

#ifdef IPREFS
    {MSG_IP_ERROR,(STRPTR)MSG_IP_ERROR_STR},
    {MSG_IP_REQTITLE,(STRPTR)MSG_IP_REQTITLE_STR},
    {MSG_IP_CLOSEWINDOW_PROMPT,(STRPTR)MSG_IP_CLOSEWINDOW_PROMPT_STR},
    {MSG_IP_CLOSEWINDOW_GADGETS,(STRPTR)MSG_IP_CLOSEWINDOW_GADGETS_STR},
    {MSG_IP_CONTINUE_GADGET,(STRPTR)MSG_IP_CONTINUE_GADGET_STR},
    {MSG_IP_ERROR_NO_IFFPARSE_LIB,(STRPTR)MSG_IP_ERROR_NO_IFFPARSE_LIB_STR},
    {MSG_IP_ERROR_NO_LOCALE_LIB,(STRPTR)MSG_IP_ERROR_NO_LOCALE_LIB_STR},
    {MSG_IP_ERROR_NO_DISKFONT_LIB,(STRPTR)MSG_IP_ERROR_NO_DISKFONT_LIB_STR},
    {MSG_IP_ERROR_CANT_OPEN,(STRPTR)MSG_IP_ERROR_CANT_OPEN_STR},
    {MSG_IP_ERROR_BAD_PREFS_FILE,(STRPTR)MSG_IP_ERROR_BAD_PREFS_FILE_STR},
    {MSG_IP_ERROR_NO_KEYMAP,(STRPTR)MSG_IP_ERROR_NO_KEYMAP_STR},
    {MSG_IP_ERROR_NO_DATATYPES_LIB,(STRPTR)MSG_IP_ERROR_NO_DATATYPES_LIB_STR},
    {MSG_IP_ERROR_NO_FONT,(STRPTR)MSG_IP_ERROR_NO_FONT_STR},
    {MSG_IP_ERROR_NO_SOUND,(STRPTR)MSG_IP_ERROR_NO_SOUND_STR},
    {MSG_IP_ERROR_NO_PICTURE,(STRPTR)MSG_IP_ERROR_NO_PICTURE_STR},
#endif /* IPREFS */

#ifdef MOUNT
    {MSG_MT_REQ_TITLE,(STRPTR)MSG_MT_REQ_TITLE_STR},
    {MSG_MT_OK_GAD,(STRPTR)MSG_MT_OK_GAD_STR},
    {MSG_MT_ERROR,(STRPTR)MSG_MT_ERROR_STR},
    {MSG_MT_NOMEM,(STRPTR)MSG_MT_NOMEM_STR},
    {MSG_MT_OPEN,(STRPTR)MSG_MT_OPEN_STR},
    {MSG_MT_ALREADY_MOUNTED,(STRPTR)MSG_MT_ALREADY_MOUNTED_STR},
    {MSG_MT_DEVICE_NOT_FOUND,(STRPTR)MSG_MT_DEVICE_NOT_FOUND_STR},
    {MSG_MT_UNKNOWN_KEYWORD,(STRPTR)MSG_MT_UNKNOWN_KEYWORD_STR},
    {MSG_MT_STRING_EXPECTED,(STRPTR)MSG_MT_STRING_EXPECTED_STR},
    {MSG_MT_NUMBER_EXPECTED,(STRPTR)MSG_MT_NUMBER_EXPECTED_STR},
    {MSG_MT_EQUAL_EXPECTED,(STRPTR)MSG_MT_EQUAL_EXPECTED_STR},
    {MSG_MT_ARGUMENT_EXPECTED,(STRPTR)MSG_MT_ARGUMENT_EXPECTED_STR},
    {MSG_MT_MISSING_KEYWORDS,(STRPTR)MSG_MT_MISSING_KEYWORDS_STR},
#endif /* MOUNT */

#ifdef LOADRESOURCE
    {MSG_LR_RESTYPE_1,(STRPTR)MSG_LR_RESTYPE_1_STR},
    {MSG_LR_RESTYPE_2,(STRPTR)MSG_LR_RESTYPE_2_STR},
    {MSG_LR_RESTYPE_3,(STRPTR)MSG_LR_RESTYPE_3_STR},
    {MSG_LR_RESTYPE_4,(STRPTR)MSG_LR_RESTYPE_4_STR},
    {MSG_LR_RESTYPE_5,(STRPTR)MSG_LR_RESTYPE_5_STR},
    {MSG_LR_FORMAT,(STRPTR)MSG_LR_FORMAT_STR},
    {MSG_LR_NAME,(STRPTR)MSG_LR_NAME_STR},
    {MSG_LR_NO_RESOURCES,(STRPTR)MSG_LR_NO_RESOURCES_STR},
    {MSG_LR_ALREADY_LOCKED,(STRPTR)MSG_LR_ALREADY_LOCKED_STR},
    {MSG_LR_LOAD_ERROR,(STRPTR)MSG_LR_LOAD_ERROR_STR},
    {MSG_LR_CANT_OPEN_DISKFONT,(STRPTR)MSG_LR_CANT_OPEN_DISKFONT_STR},
    {MSG_LR_COULD_NOT_LOAD,(STRPTR)MSG_LR_COULD_NOT_LOAD_STR},
    {MSG_LR_NOT_FOUND,(STRPTR)MSG_LR_NOT_FOUND_STR},
    {MSG_LR_ERROR,(STRPTR)MSG_LR_ERROR_STR},
#endif /* LOADRESOURCE */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* LOCALESTR_C_H */
