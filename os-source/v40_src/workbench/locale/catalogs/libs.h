#ifndef LOCALESTR_LIBS_H
#define LOCALESTR_LIBS_H


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

#ifdef ASL
#define MSG_ASL_OK_GAD 1
#define MSG_ASL_OK_GAD_STR "OK"

#define MSG_ASL_CANCEL_GAD 2
#define MSG_ASL_CANCEL_GAD_STR "Cancel"

#define MSG_ASL_SM_TITLE 1000
#define MSG_ASL_SM_TITLE_STR "Select Screen Mode"

#define MSG_ASL_SM_OVERSCAN_GAD 1001
#define MSG_ASL_SM_OVERSCAN_GAD_STR "Overscan:"

#define MSG_ASL_SM_WIDTH_GAD 1002
#define MSG_ASL_SM_WIDTH_GAD_STR "Width:"

#define MSG_ASL_SM_HEIGHT_GAD 1003
#define MSG_ASL_SM_HEIGHT_GAD_STR "Height:"

#define MSG_ASL_SM_COLORS_GAD 1004
#define MSG_ASL_SM_COLORS_GAD_STR "Colors:"

#define MSG_ASL_SM_DEFAULT_GAD 1005
#define MSG_ASL_SM_DEFAULT_GAD_STR "Default"

#define MSG_ASL_SM_AUTOSCROLL_GAD 1006
#define MSG_ASL_SM_AUTOSCROLL_GAD_STR "AutoScroll:"

#define MSG_ASL_SMINFO_TITLE 1007
#define MSG_ASL_SMINFO_TITLE_STR "Mode Properties"

#define MSG_ASL_SM_VISIBLESIZE 1008
#define MSG_ASL_SM_VISIBLESIZE_STR "Visible Size  : %5lu � %5lu"

#define MSG_ASL_SM_MAXSIZE 1009
#define MSG_ASL_SM_MAXSIZE_STR "Maximum Size  : %5lu � %5lu"

#define MSG_ASL_SM_MINSIZE 1010
#define MSG_ASL_SM_MINSIZE_STR "Minimum Size  : %5lu � %5lu"

#define MSG_ASL_SM_MAXCOLORS 1011
#define MSG_ASL_SM_MAXCOLORS_STR "Maximum Colors: %lu"

#define MSG_ASL_SM_INTERLACED 1012
#define MSG_ASL_SM_INTERLACED_STR "Interlaced"

#define MSG_ASL_SM_HOLD_AND_MODIFY 1013
#define MSG_ASL_SM_HOLD_AND_MODIFY_STR "Hold & Modify"

#define MSG_ASL_SM_EXTRAHALFBRITE 1014
#define MSG_ASL_SM_EXTRAHALFBRITE_STR "Extra-HalfBright"

#define MSG_ASL_SM_REQUIRES_ECS 1015
#define MSG_ASL_SM_REQUIRES_ECS_STR "Requires ECS"

#define MSG_ASL_SM_SUPPORTS_GLOCK 1016
#define MSG_ASL_SM_SUPPORTS_GLOCK_STR "Supports genlock"

#define MSG_ASL_SM_NO_GLOCK_SUPPORT 1017
#define MSG_ASL_SM_NO_GLOCK_SUPPORT_STR "Does not support genlock"

#define MSG_ASL_SM_SUPPORTS_WB 1018
#define MSG_ASL_SM_SUPPORTS_WB_STR "Supports Workbench"

#define MSG_ASL_SM_NO_WB_SUPPORT 1019
#define MSG_ASL_SM_NO_WB_SUPPORT_STR "Does not support Workbench"

#define MSG_ASL_SM_DRAGGABLE 1020
#define MSG_ASL_SM_DRAGGABLE_STR "Draggable"

#define MSG_ASL_SM_NOT_DRAGGABLE 1021
#define MSG_ASL_SM_NOT_DRAGGABLE_STR "Not draggable"

#define MSG_ASL_SM_DUALPLAYFIELD 1022
#define MSG_ASL_SM_DUALPLAYFIELD_STR "DualPlayfield"

#define MSG_ASL_SM_PF_2_PRIORITY 1023
#define MSG_ASL_SM_PF_2_PRIORITY_STR "DualPlayfield Priority 2"

#define MSG_ASL_SM_TEXT 1025
#define MSG_ASL_SM_TEXT_STR "Text Size"

#define MSG_ASL_SM_VIDEO 1026
#define MSG_ASL_SM_VIDEO_STR "Graphics Size"

#define MSG_ASL_SM_MAXIMUM 1027
#define MSG_ASL_SM_MAXIMUM_STR "Extreme Size"

#define MSG_ASL_FO_TITLE 1028
#define MSG_ASL_FO_TITLE_STR "Select Font"

#define MSG_ASL_FO_PLAIN_GAD 1029
#define MSG_ASL_FO_PLAIN_GAD_STR "Plain"

#define MSG_ASL_FO_BOLD_GAD 1030
#define MSG_ASL_FO_BOLD_GAD_STR "Bold"

#define MSG_ASL_FO_ITALIC_GAD 1031
#define MSG_ASL_FO_ITALIC_GAD_STR "Italic"

#define MSG_ASL_FO_UNDERLINED_GAD 1032
#define MSG_ASL_FO_UNDERLINED_GAD_STR "Underlined"

#define MSG_ASL_FO_TEXT_GAD 1033
#define MSG_ASL_FO_TEXT_GAD_STR "Text:"

#define MSG_ASL_FO_FIELD_GAD 1034
#define MSG_ASL_FO_FIELD_GAD_STR "Field:"

#define MSG_ASL_FO_MODE_GAD 1035
#define MSG_ASL_FO_MODE_GAD_STR "Mode:"

#define MSG_ASL_FO_TEXT 1036
#define MSG_ASL_FO_TEXT_STR "Text"

#define MSG_ASL_FO_TEXTFIELD 1037
#define MSG_ASL_FO_TEXTFIELD_STR "Text+Field"

#define MSG_ASL_FO_COMPLEMENT 1038
#define MSG_ASL_FO_COMPLEMENT_STR "Complement"

#define MSG_ASL_FR_TITLE 1039
#define MSG_ASL_FR_TITLE_STR "Select File"

#define MSG_ASL_FR_VOLUMES_GAD 1040
#define MSG_ASL_FR_VOLUMES_GAD_STR "Volumes"

#define MSG_ASL_FR_PARENT_GAD 1041
#define MSG_ASL_FR_PARENT_GAD_STR "Parent"

#define MSG_ASL_FR_DRAWER_GAD 1042
#define MSG_ASL_FR_DRAWER_GAD_STR "Drawer"

#define MSG_ASL_FR_FILE_GAD 1043
#define MSG_ASL_FR_FILE_GAD_STR "File"

#define MSG_ASL_FR_PATTERN_GAD 1044
#define MSG_ASL_FR_PATTERN_GAD_STR "Pattern"

#define MSG_ASL_FR_VOLUME_INFO 1045
#define MSG_ASL_FR_VOLUME_INFO_STR "%3lu%% full, %4lu%lc free, %4lu%lc in use"

#define MSG_ASL_FR_DRAWER 1046
#define MSG_ASL_FR_DRAWER_STR "Drawer"

#define MSG_ASL_FR_ASSIGN 1047
#define MSG_ASL_FR_ASSIGN_STR "Assign"

#define MSG_ASL_FR_VOLUME 1048
#define MSG_ASL_FR_VOLUME_STR "Volume"

#define MSG_ASL_FR_DELETE_TITLE 1049
#define MSG_ASL_FR_DELETE_TITLE_STR "Delete File"

#define MSG_ASL_FR_DELETE_PROMPT 1050
#define MSG_ASL_FR_DELETE_PROMPT_STR "Warning: you cannot get back\nwhat you delete! Ok to delete\n%s?"

#define MSG_ASL_FR_DELETE_GAD 1051
#define MSG_ASL_FR_DELETE_GAD_STR "Delete|Cancel"

#define MSG_ASL_FR_NEWDIR_TITLE 1052
#define MSG_ASL_FR_NEWDIR_TITLE_STR "Create Drawer"

#define MSG_ASL_FR_NEWDIR_PROMPT 1053
#define MSG_ASL_FR_NEWDIR_PROMPT_STR "Drawer '%s' does\nnot exist. Create it?"

#define MSG_ASL_FR_NEWDIR_GAD 1054
#define MSG_ASL_FR_NEWDIR_GAD_STR "Create|Cancel"

#define MSG_ASL_CONTROL_MENU 1055
#define MSG_ASL_CONTROL_MENU_STR "Control"

#define MSG_ASL_CONTROL_RESTORE 1056
#define MSG_ASL_CONTROL_RESTORE_STR "R\000Restore"

#define MSG_ASL_CONTROL_OK 1057
#define MSG_ASL_CONTROL_OK_STR "O\000OK"

#define MSG_ASL_CONTROL_CANCEL 1058
#define MSG_ASL_CONTROL_CANCEL_STR "C\000Cancel"

#define MSG_ASL_CONTROL_LASTMODE 1059
#define MSG_ASL_CONTROL_LASTMODE_STR "L\000Last Mode"

#define MSG_ASL_CONTROL_NEXTMODE 1060
#define MSG_ASL_CONTROL_NEXTMODE_STR "N\000Next Mode"

#define MSG_ASL_CONTROL_PROPS 1061
#define MSG_ASL_CONTROL_PROPS_STR "?\000Property List..."

#define MSG_ASL_CONTROL_LASTFONT 1062
#define MSG_ASL_CONTROL_LASTFONT_STR "L\000Last Font"

#define MSG_ASL_CONTROL_NEXTFONT 1063
#define MSG_ASL_CONTROL_NEXTFONT_STR "N\000Next Font"

#define MSG_ASL_CONTROL_RESCAN 1064
#define MSG_ASL_CONTROL_RESCAN_STR "D\000Rescan Disk"

#define MSG_ASL_CONTROL_LASTNAME 1065
#define MSG_ASL_CONTROL_LASTNAME_STR "L\000Last Name"

#define MSG_ASL_CONTROL_NEXTNAME 1066
#define MSG_ASL_CONTROL_NEXTNAME_STR "N\000Next Name"

#define MSG_ASL_CONTROL_PARENT 1067
#define MSG_ASL_CONTROL_PARENT_STR "P\000Parent"

#define MSG_ASL_CONTROL_VOLUMES 1068
#define MSG_ASL_CONTROL_VOLUMES_STR "V\000Volumes"

#define MSG_ASL_CONTROL_DELETE 1069
#define MSG_ASL_CONTROL_DELETE_STR "D\000Delete"

#define MSG_ASL_SM_REALVIDEO 1070
#define MSG_ASL_SM_REALVIDEO_STR "Maximum Size"

#endif /* ASL */

#ifdef DATATYPES
#define DTERROR_UNKNOWN_DATATYPE 2000
#define DTERROR_UNKNOWN_DATATYPE_STR "Unknown data type for %s"

#define DTERROR_COULDNT_SAVE 2001
#define DTERROR_COULDNT_SAVE_STR "Couldn't save %s"

#define DTERROR_COULDNT_OPEN 2002
#define DTERROR_COULDNT_OPEN_STR "Couldn't open %s"

#define DTERROR_COULDNT_SEND_MESSAGE 2003
#define DTERROR_COULDNT_SEND_MESSAGE_STR "Couldn't send message"

#define DTERROR_COULDNT_OPEN_CLIPBOARD 2004
#define DTERROR_COULDNT_OPEN_CLIPBOARD_STR "Couldn't open clipboard"

#define DTERROR_UNKNOWN_DATA_TYPE 2005
#define DTERROR_UNKNOWN_DATA_TYPE_STR "Unknown data type"

#define DTERROR_UNKNOWN_COMPRESSION 2006
#define DTERROR_UNKNOWN_COMPRESSION_STR "Unknown compression type"

#define DTERROR_NOT_ENOUGH_DATA 2007
#define DTERROR_NOT_ENOUGH_DATA_STR "Not enough data"

#define DTERROR_INVALID_DATA 2008
#define DTERROR_INVALID_DATA_STR "Invalid data"

#define DTMSG_TYPE_BINARY 2100
#define DTMSG_TYPE_BINARY_STR "Binary"

#define DTMSG_TYPE_ASCII 2101
#define DTMSG_TYPE_ASCII_STR "ASCII"

#define DTMSG_TYPE_IFF 2102
#define DTMSG_TYPE_IFF_STR "IFF"

#define DTMSG_TYPE_MISC 2103
#define DTMSG_TYPE_MISC_STR "Miscellaneous"

#define MSG_GID_SYSTEM 1937339252
#define MSG_GID_SYSTEM_STR "System"

#define MSG_GID_TEXT 1952807028
#define MSG_GID_TEXT_STR "Text"

#define MSG_GID_DOCUMENT 1685021557
#define MSG_GID_DOCUMENT_STR "Document"

#define MSG_GID_SOUND 1936684398
#define MSG_GID_SOUND_STR "Sound"

#define MSG_GID_INSTRUMENT 1768846196
#define MSG_GID_INSTRUMENT_STR "Instrument"

#define MSG_GID_MUSIC 1836413801
#define MSG_GID_MUSIC_STR "Music"

#define MSG_GID_PICTURE 1885954932
#define MSG_GID_PICTURE_STR "Picture"

#define MSG_GID_ANIMATION 1634625901
#define MSG_GID_ANIMATION_STR "Animation"

#define MSG_GID_MOVIE 1836021353
#define MSG_GID_MOVIE_STR "Movie"

#endif /* DATATYPES */


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
#ifdef ASL
    {MSG_ASL_OK_GAD,(STRPTR)MSG_ASL_OK_GAD_STR},
    {MSG_ASL_CANCEL_GAD,(STRPTR)MSG_ASL_CANCEL_GAD_STR},
    {MSG_ASL_SM_TITLE,(STRPTR)MSG_ASL_SM_TITLE_STR},
    {MSG_ASL_SM_OVERSCAN_GAD,(STRPTR)MSG_ASL_SM_OVERSCAN_GAD_STR},
    {MSG_ASL_SM_WIDTH_GAD,(STRPTR)MSG_ASL_SM_WIDTH_GAD_STR},
    {MSG_ASL_SM_HEIGHT_GAD,(STRPTR)MSG_ASL_SM_HEIGHT_GAD_STR},
    {MSG_ASL_SM_COLORS_GAD,(STRPTR)MSG_ASL_SM_COLORS_GAD_STR},
    {MSG_ASL_SM_DEFAULT_GAD,(STRPTR)MSG_ASL_SM_DEFAULT_GAD_STR},
    {MSG_ASL_SM_AUTOSCROLL_GAD,(STRPTR)MSG_ASL_SM_AUTOSCROLL_GAD_STR},
    {MSG_ASL_SMINFO_TITLE,(STRPTR)MSG_ASL_SMINFO_TITLE_STR},
    {MSG_ASL_SM_VISIBLESIZE,(STRPTR)MSG_ASL_SM_VISIBLESIZE_STR},
    {MSG_ASL_SM_MAXSIZE,(STRPTR)MSG_ASL_SM_MAXSIZE_STR},
    {MSG_ASL_SM_MINSIZE,(STRPTR)MSG_ASL_SM_MINSIZE_STR},
    {MSG_ASL_SM_MAXCOLORS,(STRPTR)MSG_ASL_SM_MAXCOLORS_STR},
    {MSG_ASL_SM_INTERLACED,(STRPTR)MSG_ASL_SM_INTERLACED_STR},
    {MSG_ASL_SM_HOLD_AND_MODIFY,(STRPTR)MSG_ASL_SM_HOLD_AND_MODIFY_STR},
    {MSG_ASL_SM_EXTRAHALFBRITE,(STRPTR)MSG_ASL_SM_EXTRAHALFBRITE_STR},
    {MSG_ASL_SM_REQUIRES_ECS,(STRPTR)MSG_ASL_SM_REQUIRES_ECS_STR},
    {MSG_ASL_SM_SUPPORTS_GLOCK,(STRPTR)MSG_ASL_SM_SUPPORTS_GLOCK_STR},
    {MSG_ASL_SM_NO_GLOCK_SUPPORT,(STRPTR)MSG_ASL_SM_NO_GLOCK_SUPPORT_STR},
    {MSG_ASL_SM_SUPPORTS_WB,(STRPTR)MSG_ASL_SM_SUPPORTS_WB_STR},
    {MSG_ASL_SM_NO_WB_SUPPORT,(STRPTR)MSG_ASL_SM_NO_WB_SUPPORT_STR},
    {MSG_ASL_SM_DRAGGABLE,(STRPTR)MSG_ASL_SM_DRAGGABLE_STR},
    {MSG_ASL_SM_NOT_DRAGGABLE,(STRPTR)MSG_ASL_SM_NOT_DRAGGABLE_STR},
    {MSG_ASL_SM_DUALPLAYFIELD,(STRPTR)MSG_ASL_SM_DUALPLAYFIELD_STR},
    {MSG_ASL_SM_PF_2_PRIORITY,(STRPTR)MSG_ASL_SM_PF_2_PRIORITY_STR},
    {MSG_ASL_SM_TEXT,(STRPTR)MSG_ASL_SM_TEXT_STR},
    {MSG_ASL_SM_VIDEO,(STRPTR)MSG_ASL_SM_VIDEO_STR},
    {MSG_ASL_SM_MAXIMUM,(STRPTR)MSG_ASL_SM_MAXIMUM_STR},
    {MSG_ASL_FO_TITLE,(STRPTR)MSG_ASL_FO_TITLE_STR},
    {MSG_ASL_FO_PLAIN_GAD,(STRPTR)MSG_ASL_FO_PLAIN_GAD_STR},
    {MSG_ASL_FO_BOLD_GAD,(STRPTR)MSG_ASL_FO_BOLD_GAD_STR},
    {MSG_ASL_FO_ITALIC_GAD,(STRPTR)MSG_ASL_FO_ITALIC_GAD_STR},
    {MSG_ASL_FO_UNDERLINED_GAD,(STRPTR)MSG_ASL_FO_UNDERLINED_GAD_STR},
    {MSG_ASL_FO_TEXT_GAD,(STRPTR)MSG_ASL_FO_TEXT_GAD_STR},
    {MSG_ASL_FO_FIELD_GAD,(STRPTR)MSG_ASL_FO_FIELD_GAD_STR},
    {MSG_ASL_FO_MODE_GAD,(STRPTR)MSG_ASL_FO_MODE_GAD_STR},
    {MSG_ASL_FO_TEXT,(STRPTR)MSG_ASL_FO_TEXT_STR},
    {MSG_ASL_FO_TEXTFIELD,(STRPTR)MSG_ASL_FO_TEXTFIELD_STR},
    {MSG_ASL_FO_COMPLEMENT,(STRPTR)MSG_ASL_FO_COMPLEMENT_STR},
    {MSG_ASL_FR_TITLE,(STRPTR)MSG_ASL_FR_TITLE_STR},
    {MSG_ASL_FR_VOLUMES_GAD,(STRPTR)MSG_ASL_FR_VOLUMES_GAD_STR},
    {MSG_ASL_FR_PARENT_GAD,(STRPTR)MSG_ASL_FR_PARENT_GAD_STR},
    {MSG_ASL_FR_DRAWER_GAD,(STRPTR)MSG_ASL_FR_DRAWER_GAD_STR},
    {MSG_ASL_FR_FILE_GAD,(STRPTR)MSG_ASL_FR_FILE_GAD_STR},
    {MSG_ASL_FR_PATTERN_GAD,(STRPTR)MSG_ASL_FR_PATTERN_GAD_STR},
    {MSG_ASL_FR_VOLUME_INFO,(STRPTR)MSG_ASL_FR_VOLUME_INFO_STR},
    {MSG_ASL_FR_DRAWER,(STRPTR)MSG_ASL_FR_DRAWER_STR},
    {MSG_ASL_FR_ASSIGN,(STRPTR)MSG_ASL_FR_ASSIGN_STR},
    {MSG_ASL_FR_VOLUME,(STRPTR)MSG_ASL_FR_VOLUME_STR},
    {MSG_ASL_FR_DELETE_TITLE,(STRPTR)MSG_ASL_FR_DELETE_TITLE_STR},
    {MSG_ASL_FR_DELETE_PROMPT,(STRPTR)MSG_ASL_FR_DELETE_PROMPT_STR},
    {MSG_ASL_FR_DELETE_GAD,(STRPTR)MSG_ASL_FR_DELETE_GAD_STR},
    {MSG_ASL_FR_NEWDIR_TITLE,(STRPTR)MSG_ASL_FR_NEWDIR_TITLE_STR},
    {MSG_ASL_FR_NEWDIR_PROMPT,(STRPTR)MSG_ASL_FR_NEWDIR_PROMPT_STR},
    {MSG_ASL_FR_NEWDIR_GAD,(STRPTR)MSG_ASL_FR_NEWDIR_GAD_STR},
    {MSG_ASL_CONTROL_MENU,(STRPTR)MSG_ASL_CONTROL_MENU_STR},
    {MSG_ASL_CONTROL_RESTORE,(STRPTR)MSG_ASL_CONTROL_RESTORE_STR},
    {MSG_ASL_CONTROL_OK,(STRPTR)MSG_ASL_CONTROL_OK_STR},
    {MSG_ASL_CONTROL_CANCEL,(STRPTR)MSG_ASL_CONTROL_CANCEL_STR},
    {MSG_ASL_CONTROL_LASTMODE,(STRPTR)MSG_ASL_CONTROL_LASTMODE_STR},
    {MSG_ASL_CONTROL_NEXTMODE,(STRPTR)MSG_ASL_CONTROL_NEXTMODE_STR},
    {MSG_ASL_CONTROL_PROPS,(STRPTR)MSG_ASL_CONTROL_PROPS_STR},
    {MSG_ASL_CONTROL_LASTFONT,(STRPTR)MSG_ASL_CONTROL_LASTFONT_STR},
    {MSG_ASL_CONTROL_NEXTFONT,(STRPTR)MSG_ASL_CONTROL_NEXTFONT_STR},
    {MSG_ASL_CONTROL_RESCAN,(STRPTR)MSG_ASL_CONTROL_RESCAN_STR},
    {MSG_ASL_CONTROL_LASTNAME,(STRPTR)MSG_ASL_CONTROL_LASTNAME_STR},
    {MSG_ASL_CONTROL_NEXTNAME,(STRPTR)MSG_ASL_CONTROL_NEXTNAME_STR},
    {MSG_ASL_CONTROL_PARENT,(STRPTR)MSG_ASL_CONTROL_PARENT_STR},
    {MSG_ASL_CONTROL_VOLUMES,(STRPTR)MSG_ASL_CONTROL_VOLUMES_STR},
    {MSG_ASL_CONTROL_DELETE,(STRPTR)MSG_ASL_CONTROL_DELETE_STR},
    {MSG_ASL_SM_REALVIDEO,(STRPTR)MSG_ASL_SM_REALVIDEO_STR},
#endif /* ASL */

#ifdef DATATYPES
    {DTERROR_UNKNOWN_DATATYPE,(STRPTR)DTERROR_UNKNOWN_DATATYPE_STR},
    {DTERROR_COULDNT_SAVE,(STRPTR)DTERROR_COULDNT_SAVE_STR},
    {DTERROR_COULDNT_OPEN,(STRPTR)DTERROR_COULDNT_OPEN_STR},
    {DTERROR_COULDNT_SEND_MESSAGE,(STRPTR)DTERROR_COULDNT_SEND_MESSAGE_STR},
    {DTERROR_COULDNT_OPEN_CLIPBOARD,(STRPTR)DTERROR_COULDNT_OPEN_CLIPBOARD_STR},
    {DTERROR_UNKNOWN_DATA_TYPE,(STRPTR)DTERROR_UNKNOWN_DATA_TYPE_STR},
    {DTERROR_UNKNOWN_COMPRESSION,(STRPTR)DTERROR_UNKNOWN_COMPRESSION_STR},
    {DTERROR_NOT_ENOUGH_DATA,(STRPTR)DTERROR_NOT_ENOUGH_DATA_STR},
    {DTERROR_INVALID_DATA,(STRPTR)DTERROR_INVALID_DATA_STR},
    {DTMSG_TYPE_BINARY,(STRPTR)DTMSG_TYPE_BINARY_STR},
    {DTMSG_TYPE_ASCII,(STRPTR)DTMSG_TYPE_ASCII_STR},
    {DTMSG_TYPE_IFF,(STRPTR)DTMSG_TYPE_IFF_STR},
    {DTMSG_TYPE_MISC,(STRPTR)DTMSG_TYPE_MISC_STR},
    {MSG_GID_SYSTEM,(STRPTR)MSG_GID_SYSTEM_STR},
    {MSG_GID_TEXT,(STRPTR)MSG_GID_TEXT_STR},
    {MSG_GID_DOCUMENT,(STRPTR)MSG_GID_DOCUMENT_STR},
    {MSG_GID_SOUND,(STRPTR)MSG_GID_SOUND_STR},
    {MSG_GID_INSTRUMENT,(STRPTR)MSG_GID_INSTRUMENT_STR},
    {MSG_GID_MUSIC,(STRPTR)MSG_GID_MUSIC_STR},
    {MSG_GID_PICTURE,(STRPTR)MSG_GID_PICTURE_STR},
    {MSG_GID_ANIMATION,(STRPTR)MSG_GID_ANIMATION_STR},
    {MSG_GID_MOVIE,(STRPTR)MSG_GID_MOVIE_STR},
#endif /* DATATYPES */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* LOCALESTR_LIBS_H */