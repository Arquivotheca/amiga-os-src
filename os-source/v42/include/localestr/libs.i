	IFND LOCALESTR_LIBS_I
LOCALESTR_LIBS_I	SET	1


;-----------------------------------------------------------------------------


* This file was created automatically by CatComp.
* Do NOT edit by hand!
*


	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC


;-----------------------------------------------------------------------------


MSG_NOTHING EQU 0

	IFD ASL
MSG_ASL_OK_GAD EQU 1
MSG_ASL_CANCEL_GAD EQU 2
MSG_ASL_SM_TITLE EQU 1000
MSG_ASL_SM_OVERSCAN_GAD EQU 1001
MSG_ASL_SM_WIDTH_GAD EQU 1002
MSG_ASL_SM_HEIGHT_GAD EQU 1003
MSG_ASL_SM_COLORS_GAD EQU 1004
MSG_ASL_SM_DEFAULT_GAD EQU 1005
MSG_ASL_SM_AUTOSCROLL_GAD EQU 1006
MSG_ASL_SMINFO_TITLE EQU 1007
MSG_ASL_SM_VISIBLESIZE EQU 1008
MSG_ASL_SM_MAXSIZE EQU 1009
MSG_ASL_SM_MINSIZE EQU 1010
MSG_ASL_SM_MAXCOLORS EQU 1011
MSG_ASL_SM_INTERLACED EQU 1012
MSG_ASL_SM_HOLD_AND_MODIFY EQU 1013
MSG_ASL_SM_EXTRAHALFBRITE EQU 1014
MSG_ASL_SM_REQUIRES_ECS EQU 1015
MSG_ASL_SM_SUPPORTS_GLOCK EQU 1016
MSG_ASL_SM_NO_GLOCK_SUPPORT EQU 1017
MSG_ASL_SM_SUPPORTS_WB EQU 1018
MSG_ASL_SM_NO_WB_SUPPORT EQU 1019
MSG_ASL_SM_DRAGGABLE EQU 1020
MSG_ASL_SM_NOT_DRAGGABLE EQU 1021
MSG_ASL_SM_DUALPLAYFIELD EQU 1022
MSG_ASL_SM_PF_2_PRIORITY EQU 1023
MSG_ASL_SM_TEXT EQU 1025
MSG_ASL_SM_VIDEO EQU 1026
MSG_ASL_SM_MAXIMUM EQU 1027
MSG_ASL_FO_TITLE EQU 1028
MSG_ASL_FO_PLAIN_GAD EQU 1029
MSG_ASL_FO_BOLD_GAD EQU 1030
MSG_ASL_FO_ITALIC_GAD EQU 1031
MSG_ASL_FO_UNDERLINED_GAD EQU 1032
MSG_ASL_FO_TEXT_GAD EQU 1033
MSG_ASL_FO_FIELD_GAD EQU 1034
MSG_ASL_FO_MODE_GAD EQU 1035
MSG_ASL_FO_TEXT EQU 1036
MSG_ASL_FO_TEXTFIELD EQU 1037
MSG_ASL_FO_COMPLEMENT EQU 1038
MSG_ASL_FR_TITLE EQU 1039
MSG_ASL_FR_VOLUMES_GAD EQU 1040
MSG_ASL_FR_PARENT_GAD EQU 1041
MSG_ASL_FR_DRAWER_GAD EQU 1042
MSG_ASL_FR_FILE_GAD EQU 1043
MSG_ASL_FR_PATTERN_GAD EQU 1044
MSG_ASL_FR_VOLUME_INFO EQU 1045
MSG_ASL_FR_DRAWER EQU 1046
MSG_ASL_FR_ASSIGN EQU 1047
MSG_ASL_FR_VOLUME EQU 1048
MSG_ASL_FR_DELETE_TITLE EQU 1049
MSG_ASL_FR_DELETE_PROMPT EQU 1050
MSG_ASL_FR_DELETE_GAD EQU 1051
MSG_ASL_FR_NEWDIR_TITLE EQU 1052
MSG_ASL_FR_NEWDIR_PROMPT EQU 1053
MSG_ASL_FR_NEWDIR_GAD EQU 1054
MSG_ASL_CONTROL_MENU EQU 1055
MSG_ASL_CONTROL_RESTORE EQU 1056
MSG_ASL_CONTROL_OK EQU 1057
MSG_ASL_CONTROL_CANCEL EQU 1058
MSG_ASL_CONTROL_LASTMODE EQU 1059
MSG_ASL_CONTROL_NEXTMODE EQU 1060
MSG_ASL_CONTROL_PROPS EQU 1061
MSG_ASL_CONTROL_LASTFONT EQU 1062
MSG_ASL_CONTROL_NEXTFONT EQU 1063
MSG_ASL_CONTROL_RESCAN EQU 1064
MSG_ASL_CONTROL_LASTNAME EQU 1065
MSG_ASL_CONTROL_NEXTNAME EQU 1066
MSG_ASL_CONTROL_PARENT EQU 1067
MSG_ASL_CONTROL_VOLUMES EQU 1068
MSG_ASL_CONTROL_DELETE EQU 1069
MSG_ASL_SM_REALVIDEO EQU 1070
	ENDC ; ASL

	IFD DATATYPES
DTERROR_UNKNOWN_DATATYPE EQU 2000
DTERROR_COULDNT_SAVE EQU 2001
DTERROR_COULDNT_OPEN EQU 2002
DTERROR_COULDNT_SEND_MESSAGE EQU 2003
DTMSG_TYPE_BINARY EQU 2100
DTMSG_TYPE_ASCII EQU 2101
DTMSG_TYPE_IFF EQU 2102
DTMSG_TYPE_MISC EQU 2103
MSG_GID_SYSTEM EQU 1937339252
MSG_GID_TEXT EQU 1952807028
MSG_GID_DOCUMENT EQU 1685021557
MSG_GID_SOUND EQU 1936684398
MSG_GID_INSTRUMENT EQU 1768846196
MSG_GID_MUSIC EQU 1836413801
MSG_GID_PICTURE EQU 1885954932
MSG_GID_ANIMATION EQU 1634625901
MSG_GID_MOVIE EQU 1836021353
	ENDC ; DATATYPES


;-----------------------------------------------------------------------------


	IFD STRINGARRAY

   STRUCTURE AppString,0
	LONG   as_ID
	APTR as_Str
   LABEL AppString_SIZEOF

MSG_NOTHING_STR: DC.B ,0

	IFD ASL
MSG_ASL_OK_GAD_STR: DC.B 'OK',0
MSG_ASL_CANCEL_GAD_STR: DC.B 'Cancel',0
MSG_ASL_SM_TITLE_STR: DC.B 'Select Screen Mode',0
MSG_ASL_SM_OVERSCAN_GAD_STR: DC.B 'Overscan:',0
MSG_ASL_SM_WIDTH_GAD_STR: DC.B 'Width:',0
MSG_ASL_SM_HEIGHT_GAD_STR: DC.B 'Height:',0
MSG_ASL_SM_COLORS_GAD_STR: DC.B 'Colors:',0
MSG_ASL_SM_DEFAULT_GAD_STR: DC.B 'Default',0
MSG_ASL_SM_AUTOSCROLL_GAD_STR: DC.B 'AutoScroll:',0
MSG_ASL_SMINFO_TITLE_STR: DC.B 'Mode Properties',0
MSG_ASL_SM_VISIBLESIZE_STR: DC.B 'Visible Size  : %5lu � %5lu',0
MSG_ASL_SM_MAXSIZE_STR: DC.B 'Maximum Size  : %5lu � %5lu',0
MSG_ASL_SM_MINSIZE_STR: DC.B 'Minimum Size  : %5lu � %5lu',0
MSG_ASL_SM_MAXCOLORS_STR: DC.B 'Maximum Colors: %lu',0
MSG_ASL_SM_INTERLACED_STR: DC.B 'Interlaced',0
MSG_ASL_SM_HOLD_AND_MODIFY_STR: DC.B 'Hold & Modify',0
MSG_ASL_SM_EXTRAHALFBRITE_STR: DC.B 'Extra-HalfBright',0
MSG_ASL_SM_REQUIRES_ECS_STR: DC.B 'Requires ECS',0
MSG_ASL_SM_SUPPORTS_GLOCK_STR: DC.B 'Supports genlock',0
MSG_ASL_SM_NO_GLOCK_SUPPORT_STR: DC.B 'Does not support genlock',0
MSG_ASL_SM_SUPPORTS_WB_STR: DC.B 'Supports Workbench',0
MSG_ASL_SM_NO_WB_SUPPORT_STR: DC.B 'Does not support Workbench',0
MSG_ASL_SM_DRAGGABLE_STR: DC.B 'Draggable',0
MSG_ASL_SM_NOT_DRAGGABLE_STR: DC.B 'Not draggable',0
MSG_ASL_SM_DUALPLAYFIELD_STR: DC.B 'DualPlayfield',0
MSG_ASL_SM_PF_2_PRIORITY_STR: DC.B 'DualPlayfield Priority 2',0
MSG_ASL_SM_TEXT_STR: DC.B 'Text Size',0
MSG_ASL_SM_VIDEO_STR: DC.B 'Graphics Size',0
MSG_ASL_SM_MAXIMUM_STR: DC.B 'Extreme Size',0
MSG_ASL_FO_TITLE_STR: DC.B 'Select Font',0
MSG_ASL_FO_PLAIN_GAD_STR: DC.B 'Plain',0
MSG_ASL_FO_BOLD_GAD_STR: DC.B 'Bold',0
MSG_ASL_FO_ITALIC_GAD_STR: DC.B 'Italic',0
MSG_ASL_FO_UNDERLINED_GAD_STR: DC.B 'Underlined',0
MSG_ASL_FO_TEXT_GAD_STR: DC.B 'Text:',0
MSG_ASL_FO_FIELD_GAD_STR: DC.B 'Field:',0
MSG_ASL_FO_MODE_GAD_STR: DC.B 'Mode:',0
MSG_ASL_FO_TEXT_STR: DC.B 'Text',0
MSG_ASL_FO_TEXTFIELD_STR: DC.B 'Text+Field',0
MSG_ASL_FO_COMPLEMENT_STR: DC.B 'Complement',0
MSG_ASL_FR_TITLE_STR: DC.B 'Select File',0
MSG_ASL_FR_VOLUMES_GAD_STR: DC.B 'Volumes',0
MSG_ASL_FR_PARENT_GAD_STR: DC.B 'Parent',0
MSG_ASL_FR_DRAWER_GAD_STR: DC.B 'Drawer',0
MSG_ASL_FR_FILE_GAD_STR: DC.B 'File',0
MSG_ASL_FR_PATTERN_GAD_STR: DC.B 'Pattern',0
MSG_ASL_FR_VOLUME_INFO_STR: DC.B '%3lu%% full, %4lu%lc free, %4lu%lc in use',0
MSG_ASL_FR_DRAWER_STR: DC.B 'Drawer',0
MSG_ASL_FR_ASSIGN_STR: DC.B 'Assign',0
MSG_ASL_FR_VOLUME_STR: DC.B 'Volume',0
MSG_ASL_FR_DELETE_TITLE_STR: DC.B 'Delete File',0
MSG_ASL_FR_DELETE_PROMPT_STR: DC.B 'Warning: you cannot get back',10,'what you delete! Ok to delete',10,'%s?',0
MSG_ASL_FR_DELETE_GAD_STR: DC.B 'Delete|Cancel',0
MSG_ASL_FR_NEWDIR_TITLE_STR: DC.B 'Create Drawer',0
MSG_ASL_FR_NEWDIR_PROMPT_STR: DC.B 'Drawer ',39,'%s',39,' does',10,'not exist. Create it?',0
MSG_ASL_FR_NEWDIR_GAD_STR: DC.B 'Create|Cancel',0
MSG_ASL_CONTROL_MENU_STR: DC.B 'Control',0
MSG_ASL_CONTROL_RESTORE_STR: DC.B 'R',0,'Restore',0
MSG_ASL_CONTROL_OK_STR: DC.B 'O',0,'OK',0
MSG_ASL_CONTROL_CANCEL_STR: DC.B 'C',0,'Cancel',0
MSG_ASL_CONTROL_LASTMODE_STR: DC.B 'L',0,'Last Mode',0
MSG_ASL_CONTROL_NEXTMODE_STR: DC.B 'N',0,'Next Mode',0
MSG_ASL_CONTROL_PROPS_STR: DC.B '?',0,'Property List...',0
MSG_ASL_CONTROL_LASTFONT_STR: DC.B 'L',0,'Last Font',0
MSG_ASL_CONTROL_NEXTFONT_STR: DC.B 'N',0,'Next Font',0
MSG_ASL_CONTROL_RESCAN_STR: DC.B 'D',0,'Rescan Disk',0
MSG_ASL_CONTROL_LASTNAME_STR: DC.B 'L',0,'Last Name',0
MSG_ASL_CONTROL_NEXTNAME_STR: DC.B 'N',0,'Next Name',0
MSG_ASL_CONTROL_PARENT_STR: DC.B 'P',0,'Parent',0
MSG_ASL_CONTROL_VOLUMES_STR: DC.B 'V',0,'Volumes',0
MSG_ASL_CONTROL_DELETE_STR: DC.B 'D',0,'Delete',0
MSG_ASL_SM_REALVIDEO_STR: DC.B 'Maximum Size',0
	ENDC ; ASL

	IFD DATATYPES
DTERROR_UNKNOWN_DATATYPE_STR: DC.B 'Unknown data type for %s',0
DTERROR_COULDNT_SAVE_STR: DC.B 'Couldn',39,'t save %s',0
DTERROR_COULDNT_OPEN_STR: DC.B 'Couldn',39,'t open %s',0
DTERROR_COULDNT_SEND_MESSAGE_STR: DC.B 'Couldn',39,'t send message',0
DTMSG_TYPE_BINARY_STR: DC.B 'Binary',0
DTMSG_TYPE_ASCII_STR: DC.B 'ASCII',0
DTMSG_TYPE_IFF_STR: DC.B 'IFF',0
DTMSG_TYPE_MISC_STR: DC.B 'Miscellaneous',0
MSG_GID_SYSTEM_STR: DC.B 'System',0
MSG_GID_TEXT_STR: DC.B 'Text',0
MSG_GID_DOCUMENT_STR: DC.B 'Document',0
MSG_GID_SOUND_STR: DC.B 'Sound',0
MSG_GID_INSTRUMENT_STR: DC.B 'Instrument',0
MSG_GID_MUSIC_STR: DC.B 'Music',0
MSG_GID_PICTURE_STR: DC.B 'Picture',0
MSG_GID_ANIMATION_STR: DC.B 'Animation',0
MSG_GID_MOVIE_STR: DC.B 'Movie',0
	ENDC ; DATATYPES

	CNOP 0,4


AppString:
AS0:	DC.L MSG_NOTHING,MSG_NOTHING_STR

	IFD ASL
AS1:	DC.L MSG_ASL_OK_GAD,MSG_ASL_OK_GAD_STR
AS2:	DC.L MSG_ASL_CANCEL_GAD,MSG_ASL_CANCEL_GAD_STR
AS3:	DC.L MSG_ASL_SM_TITLE,MSG_ASL_SM_TITLE_STR
AS4:	DC.L MSG_ASL_SM_OVERSCAN_GAD,MSG_ASL_SM_OVERSCAN_GAD_STR
AS5:	DC.L MSG_ASL_SM_WIDTH_GAD,MSG_ASL_SM_WIDTH_GAD_STR
AS6:	DC.L MSG_ASL_SM_HEIGHT_GAD,MSG_ASL_SM_HEIGHT_GAD_STR
AS7:	DC.L MSG_ASL_SM_COLORS_GAD,MSG_ASL_SM_COLORS_GAD_STR
AS8:	DC.L MSG_ASL_SM_DEFAULT_GAD,MSG_ASL_SM_DEFAULT_GAD_STR
AS9:	DC.L MSG_ASL_SM_AUTOSCROLL_GAD,MSG_ASL_SM_AUTOSCROLL_GAD_STR
AS10:	DC.L MSG_ASL_SMINFO_TITLE,MSG_ASL_SMINFO_TITLE_STR
AS11:	DC.L MSG_ASL_SM_VISIBLESIZE,MSG_ASL_SM_VISIBLESIZE_STR
AS12:	DC.L MSG_ASL_SM_MAXSIZE,MSG_ASL_SM_MAXSIZE_STR
AS13:	DC.L MSG_ASL_SM_MINSIZE,MSG_ASL_SM_MINSIZE_STR
AS14:	DC.L MSG_ASL_SM_MAXCOLORS,MSG_ASL_SM_MAXCOLORS_STR
AS15:	DC.L MSG_ASL_SM_INTERLACED,MSG_ASL_SM_INTERLACED_STR
AS16:	DC.L MSG_ASL_SM_HOLD_AND_MODIFY,MSG_ASL_SM_HOLD_AND_MODIFY_STR
AS17:	DC.L MSG_ASL_SM_EXTRAHALFBRITE,MSG_ASL_SM_EXTRAHALFBRITE_STR
AS18:	DC.L MSG_ASL_SM_REQUIRES_ECS,MSG_ASL_SM_REQUIRES_ECS_STR
AS19:	DC.L MSG_ASL_SM_SUPPORTS_GLOCK,MSG_ASL_SM_SUPPORTS_GLOCK_STR
AS20:	DC.L MSG_ASL_SM_NO_GLOCK_SUPPORT,MSG_ASL_SM_NO_GLOCK_SUPPORT_STR
AS21:	DC.L MSG_ASL_SM_SUPPORTS_WB,MSG_ASL_SM_SUPPORTS_WB_STR
AS22:	DC.L MSG_ASL_SM_NO_WB_SUPPORT,MSG_ASL_SM_NO_WB_SUPPORT_STR
AS23:	DC.L MSG_ASL_SM_DRAGGABLE,MSG_ASL_SM_DRAGGABLE_STR
AS24:	DC.L MSG_ASL_SM_NOT_DRAGGABLE,MSG_ASL_SM_NOT_DRAGGABLE_STR
AS25:	DC.L MSG_ASL_SM_DUALPLAYFIELD,MSG_ASL_SM_DUALPLAYFIELD_STR
AS26:	DC.L MSG_ASL_SM_PF_2_PRIORITY,MSG_ASL_SM_PF_2_PRIORITY_STR
AS27:	DC.L MSG_ASL_SM_TEXT,MSG_ASL_SM_TEXT_STR
AS28:	DC.L MSG_ASL_SM_VIDEO,MSG_ASL_SM_VIDEO_STR
AS29:	DC.L MSG_ASL_SM_MAXIMUM,MSG_ASL_SM_MAXIMUM_STR
AS30:	DC.L MSG_ASL_FO_TITLE,MSG_ASL_FO_TITLE_STR
AS31:	DC.L MSG_ASL_FO_PLAIN_GAD,MSG_ASL_FO_PLAIN_GAD_STR
AS32:	DC.L MSG_ASL_FO_BOLD_GAD,MSG_ASL_FO_BOLD_GAD_STR
AS33:	DC.L MSG_ASL_FO_ITALIC_GAD,MSG_ASL_FO_ITALIC_GAD_STR
AS34:	DC.L MSG_ASL_FO_UNDERLINED_GAD,MSG_ASL_FO_UNDERLINED_GAD_STR
AS35:	DC.L MSG_ASL_FO_TEXT_GAD,MSG_ASL_FO_TEXT_GAD_STR
AS36:	DC.L MSG_ASL_FO_FIELD_GAD,MSG_ASL_FO_FIELD_GAD_STR
AS37:	DC.L MSG_ASL_FO_MODE_GAD,MSG_ASL_FO_MODE_GAD_STR
AS38:	DC.L MSG_ASL_FO_TEXT,MSG_ASL_FO_TEXT_STR
AS39:	DC.L MSG_ASL_FO_TEXTFIELD,MSG_ASL_FO_TEXTFIELD_STR
AS40:	DC.L MSG_ASL_FO_COMPLEMENT,MSG_ASL_FO_COMPLEMENT_STR
AS41:	DC.L MSG_ASL_FR_TITLE,MSG_ASL_FR_TITLE_STR
AS42:	DC.L MSG_ASL_FR_VOLUMES_GAD,MSG_ASL_FR_VOLUMES_GAD_STR
AS43:	DC.L MSG_ASL_FR_PARENT_GAD,MSG_ASL_FR_PARENT_GAD_STR
AS44:	DC.L MSG_ASL_FR_DRAWER_GAD,MSG_ASL_FR_DRAWER_GAD_STR
AS45:	DC.L MSG_ASL_FR_FILE_GAD,MSG_ASL_FR_FILE_GAD_STR
AS46:	DC.L MSG_ASL_FR_PATTERN_GAD,MSG_ASL_FR_PATTERN_GAD_STR
AS47:	DC.L MSG_ASL_FR_VOLUME_INFO,MSG_ASL_FR_VOLUME_INFO_STR
AS48:	DC.L MSG_ASL_FR_DRAWER,MSG_ASL_FR_DRAWER_STR
AS49:	DC.L MSG_ASL_FR_ASSIGN,MSG_ASL_FR_ASSIGN_STR
AS50:	DC.L MSG_ASL_FR_VOLUME,MSG_ASL_FR_VOLUME_STR
AS51:	DC.L MSG_ASL_FR_DELETE_TITLE,MSG_ASL_FR_DELETE_TITLE_STR
AS52:	DC.L MSG_ASL_FR_DELETE_PROMPT,MSG_ASL_FR_DELETE_PROMPT_STR
AS53:	DC.L MSG_ASL_FR_DELETE_GAD,MSG_ASL_FR_DELETE_GAD_STR
AS54:	DC.L MSG_ASL_FR_NEWDIR_TITLE,MSG_ASL_FR_NEWDIR_TITLE_STR
AS55:	DC.L MSG_ASL_FR_NEWDIR_PROMPT,MSG_ASL_FR_NEWDIR_PROMPT_STR
AS56:	DC.L MSG_ASL_FR_NEWDIR_GAD,MSG_ASL_FR_NEWDIR_GAD_STR
AS57:	DC.L MSG_ASL_CONTROL_MENU,MSG_ASL_CONTROL_MENU_STR
AS58:	DC.L MSG_ASL_CONTROL_RESTORE,MSG_ASL_CONTROL_RESTORE_STR
AS59:	DC.L MSG_ASL_CONTROL_OK,MSG_ASL_CONTROL_OK_STR
AS60:	DC.L MSG_ASL_CONTROL_CANCEL,MSG_ASL_CONTROL_CANCEL_STR
AS61:	DC.L MSG_ASL_CONTROL_LASTMODE,MSG_ASL_CONTROL_LASTMODE_STR
AS62:	DC.L MSG_ASL_CONTROL_NEXTMODE,MSG_ASL_CONTROL_NEXTMODE_STR
AS63:	DC.L MSG_ASL_CONTROL_PROPS,MSG_ASL_CONTROL_PROPS_STR
AS64:	DC.L MSG_ASL_CONTROL_LASTFONT,MSG_ASL_CONTROL_LASTFONT_STR
AS65:	DC.L MSG_ASL_CONTROL_NEXTFONT,MSG_ASL_CONTROL_NEXTFONT_STR
AS66:	DC.L MSG_ASL_CONTROL_RESCAN,MSG_ASL_CONTROL_RESCAN_STR
AS67:	DC.L MSG_ASL_CONTROL_LASTNAME,MSG_ASL_CONTROL_LASTNAME_STR
AS68:	DC.L MSG_ASL_CONTROL_NEXTNAME,MSG_ASL_CONTROL_NEXTNAME_STR
AS69:	DC.L MSG_ASL_CONTROL_PARENT,MSG_ASL_CONTROL_PARENT_STR
AS70:	DC.L MSG_ASL_CONTROL_VOLUMES,MSG_ASL_CONTROL_VOLUMES_STR
AS71:	DC.L MSG_ASL_CONTROL_DELETE,MSG_ASL_CONTROL_DELETE_STR
AS72:	DC.L MSG_ASL_SM_REALVIDEO,MSG_ASL_SM_REALVIDEO_STR
	ENDC ; ASL

	IFD DATATYPES
AS73:	DC.L DTERROR_UNKNOWN_DATATYPE,DTERROR_UNKNOWN_DATATYPE_STR
AS74:	DC.L DTERROR_COULDNT_SAVE,DTERROR_COULDNT_SAVE_STR
AS75:	DC.L DTERROR_COULDNT_OPEN,DTERROR_COULDNT_OPEN_STR
AS76:	DC.L DTERROR_COULDNT_SEND_MESSAGE,DTERROR_COULDNT_SEND_MESSAGE_STR
AS77:	DC.L DTMSG_TYPE_BINARY,DTMSG_TYPE_BINARY_STR
AS78:	DC.L DTMSG_TYPE_ASCII,DTMSG_TYPE_ASCII_STR
AS79:	DC.L DTMSG_TYPE_IFF,DTMSG_TYPE_IFF_STR
AS80:	DC.L DTMSG_TYPE_MISC,DTMSG_TYPE_MISC_STR
AS81:	DC.L MSG_GID_SYSTEM,MSG_GID_SYSTEM_STR
AS82:	DC.L MSG_GID_TEXT,MSG_GID_TEXT_STR
AS83:	DC.L MSG_GID_DOCUMENT,MSG_GID_DOCUMENT_STR
AS84:	DC.L MSG_GID_SOUND,MSG_GID_SOUND_STR
AS85:	DC.L MSG_GID_INSTRUMENT,MSG_GID_INSTRUMENT_STR
AS86:	DC.L MSG_GID_MUSIC,MSG_GID_MUSIC_STR
AS87:	DC.L MSG_GID_PICTURE,MSG_GID_PICTURE_STR
AS88:	DC.L MSG_GID_ANIMATION,MSG_GID_ANIMATION_STR
AS89:	DC.L MSG_GID_MOVIE,MSG_GID_MOVIE_STR
	ENDC ; DATATYPES


	ENDC ; STRINGARRAY


;-----------------------------------------------------------------------------


	ENDC ; LOCALESTR_LIBS_I