#ifndef INSTALLERSTR_H
#define INSTALLERSTR_H


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

#define TX_YES 3
#define TX_NO 4
#define TX_ABORT 5
#define TX_HELP 6
#define TX_PROCEED 7
#define TX_CANCEL 8
#define TX_PARENT 9
#define TX_DRIVES 10
#define TX_MAKEDIR 11
#define TX_SKIP 12
#define TX_EXITHELP 13
#define TX_MORE 14
#define TX_BACK 15
#define TX_EXITABOUT 16
#define TX_ABOUT 17
#define TX_COMPLETE 18
#define TX_OK 19
#define TX_COPYING_FILE 46
#define TX_TO_DRAWER 47
#define TX_MAKEDIR_INFO 48
#define TX_ENTER_PATH 49
#define TX_CHANGE_DEST 50
#define TX_COPY_DEST 51
#define TX_HELP_COPY 54
#define TX_HELP_ASKCHOICE 55
#define TX_HELP_ASKOPTIONS 56
#define TX_HELP_ASKSTRING 57
#define TX_HELP_ASKNUMBER 58
#define TX_HELP_ASKDISK 59
#define TX_PLEASE_INSERT 60
#define TX_HELP_ASKBOOL 61
#define TX_SELECTED_DRAWER 62
#define TX_HELP_SELECTDIR 63
#define TX_SELECTED_FILE 64
#define TX_CURRENT_DRAWER 65
#define TX_HELP_SELECTFILE 66
#define TX_INST_COMPLETE 67
#define TX_INST_COMP_WHERE 68
#define TX_INST_LOGFILE 69
#define TX_WORKING 70
#define TX_HELP_CONFIRM 71
#define TX_VERSION_SOURCE 72
#define TX_VERSION_DEST 73
#define TX_VERSION_NONE 74
#define TX_PROCEED_INST 75
#define TX_INST_MODE 76
#define TX_WELCOME 77
#define TX_HELP_INSTMODE 78
#define TX_LOG_TO 79
#define TX_INST_OPTIONS 80
#define TX_WELCOME_OPT 81
#define TX_HELP_SETTINGS 82
#define TX_MODIFY_SS 83
#define TX_ANOTHER_FILE 84
#define TX_PROCEED_CHANGES 85
#define TX_FILE_MODIFIED 86
#define TX_HELP_CHANGE_SS 87
#define TX_ERROR 88
#define TX_DOSERROR 89
#define TX_SORRY 90
#define TX_CONFIRM 91
#define TX_ESCAPE 92
#define TX_RANGE 93
#define TX_PROCEED_COPY 96
#define MSG_BOOTDISK 131
#define MSG_ASKDELETE 168
#define TX_SKIP_FILE 169
#define TX_DELETE 170
#define HELP_USER_LEVEL 138
#define HELP_INST_SETTINGS 139
#define HELP_CHANGE_STARTUP 140
#define STR_FILE_TO_MODIFY 141
#define HELP_SELECT_FILE 142
#define STR_NEW_DEST 143
#define HELP_SELECT_DRAWER 144
#define HELP_VERSION 145
#define HELP_COPYFILES 146
#define HELP_ASKDISK 147
#define HELP_ASKSTRING 148
#define HELP_ASKNUMBER 149
#define HELP_ASKCHOICE 150
#define HELP_ASKOPTIONS 151
#define HELP_MAKEDIR 152
#define STR_CREATE_ICON 153
#define STR_NOVICE_USER 154
#define STR_AVERAGE_USER 155
#define STR_EXPERT_USER 156
#define STR_FOR_REAL 157
#define STR_PRETEND 158
#define STR_PRINTER 159
#define STR_LOGFILE 160
#define STR_NOLOG 161
#define STR_SURE_ABORT 162
#define HELP_STARTUP 163
#define MSG_NO_PRINTER 26
#define MSG_ACCESS_TRANSCRIPT 27
#define MSG_ACCESS_PRINTER 28
#define MSG_PROB_SOURCE 52
#define MSG_PROB_FILE 53
#define MSG_NOEXAMINE_OBJECT 97
#define MSG_NOEXIST_OBJECT 98
#define MSG_CANNOT_ASSIGN 99
#define MSG_ERR_MEMORY 102
#define MSG_NO_DRAWER 117
#define MSG_NO_FILE 118
#define MSG_PROB_SS 119
#define MSG_ERR_WRITING 120
#define MSG_ERR_UNKNOWN 121
#define MSG_ERR_READING 122
#define MSG_CANNOT_CREATE 123
#define MSG_CANNOT_COPY 124
#define MSG_OVERWRITE 125
#define MSG_EXECUTE_PATH 126
#define MSG_NO_COMMAND 128
#define MSG_NO_REXX 129
#define MSG_NO_DUPLOCK 130
#define ERR_READING_US 136
#define ERR_WRITING_US 137
#define MSG_BAD_PARAMS_COPY 164
#define MSG_INVALID_DRAWER_NAME 167
#define MSG_NO_WINDOW 22
#define MSG_NO_SCRIPT_ICON 23
#define MSG_NO_PARTITION 24
#define MSG_NO_TRANSCRIPT 25
#define MSG_NO_SCRIPT 30
#define MSG_NO_INIT 31
#define MSG_NO_LIBRARIES 32
#define MSG_NO_WORKBENCH 33
#define MSG_TOO_LONG 95
#define MSG_DIVIDE_ZERO 100
#define MSG_BAD_CAT 101
#define TRANS_HEADER 0
#define TRANS_USER_DRAWER 34
#define TRANS_COPY_FILE 35
#define TRANS_COPY_DRAWER 36
#define TRANS_ASK_CHOICE 37
#define TRANS_NO_OPTIONS 38
#define TRANS_ASK_OPTIONS 39
#define TRANS_ASK_STRING 40
#define TRANS_ASK_NUMBER 41
#define TRANS_ASK_BOOL 42
#define TRANS_DEST_DRAWER 43
#define TRANS_ASK_DRAWER 44
#define TRANS_ASK_FILE 45
#define MSG_ONERROR 94
#define MSG_USER_ABORT 103
#define MSG_ABORT_LINE 104
#define MSG_PROBLEM_LINE 105
#define TRANS_CREATE_DRAWER 106
#define TRANS_INSERT_COMMANDS 107
#define TRANS_CREATE_TEXTFILE 108
#define TRANS_END_TEXTFILE 109
#define TRANS_EXECUTE 110
#define TRANS_RUN 111
#define TRANS_AREXX 112
#define TRANS_RENAME 113
#define TRANS_DELETE 114
#define TRANS_ABORTING 115
#define TRANS_EXITING 116
#define TRANS_START_US 134
#define TRANS_END_US 135
#define TX_ASSIGN_LABEL 132
#define TX_DRAWER_LABEL 133
#define TRANS_DEF_NAME 1
#define MSG_ABOUT_INSTALLER1 165
#define MSG_ABOUT_INSTALLER2 166
#define TX_USAGE 2
#define MSG_BAD_ARGS 20
#define MSG_NO_FONT 21
#define MSG_NO_COMPILE 29
#define MSG_NO_NIL 127

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define TX_YES_STR "Yes"
#define TX_NO_STR "No"
#define TX_ABORT_STR "Abort Install"
#define TX_HELP_STR "Help..."
#define TX_PROCEED_STR "Proceed"
#define TX_CANCEL_STR "Cancel"
#define TX_PARENT_STR "Parent Drawer"
#define TX_DRIVES_STR "Show Drives"
#define TX_MAKEDIR_STR "Make New Drawer..."
#define TX_SKIP_STR "Skip This Part"
#define TX_EXITHELP_STR "Exit from Help"
#define TX_MORE_STR "More"
#define TX_BACK_STR "Back"
#define TX_EXITABOUT_STR "Exit from About"
#define TX_ABOUT_STR "About..."
#define TX_COMPLETE_STR "done"
#define TX_OK_STR "OK"
#define TX_COPYING_FILE_STR "Copying file:"
#define TX_TO_DRAWER_STR "To drawer:"
#define TX_MAKEDIR_INFO_STR "\nEnter the complete pathname of the new drawer you wish to make. An icon for the new drawer will also be created unless you select otherwise."
#define TX_ENTER_PATH_STR "Enter the Complete Path of New Dirctory:"
#define TX_CHANGE_DEST_STR "Change Destination"
#define TX_COPY_DEST_STR "Destination Drawer to Copy to:"
#define TX_HELP_COPY_STR "Help with Copy Files"
#define TX_HELP_ASKCHOICE_STR "Help with Choice Selection"
#define TX_HELP_ASKOPTIONS_STR "Help with Option Selection"
#define TX_HELP_ASKSTRING_STR "Help with Ask String"
#define TX_HELP_ASKNUMBER_STR "Help with Ask Number"
#define TX_HELP_ASKDISK_STR "Help with Insert Disk"
#define TX_PLEASE_INSERT_STR "Please insert correct disk to continue!"
#define TX_HELP_ASKBOOL_STR "Help with Ask Question"
#define TX_SELECTED_DRAWER_STR "Selected Drawer"
#define TX_HELP_SELECTDIR_STR "Help with Select Drawer"
#define TX_SELECTED_FILE_STR "Selected File"
#define TX_CURRENT_DRAWER_STR "Current Drawer"
#define TX_HELP_SELECTFILE_STR "Help with Select File"
#define TX_INST_COMPLETE_STR "Installation of %s has been completed."
#define TX_INST_COMP_WHERE_STR "Installation complete!\n%s can be found in your \"%s\" drawer (or partition)."
#define TX_INST_LOGFILE_STR "(The installation log file is named \"%s\".)"
#define TX_WORKING_STR "Working on installation..."
#define TX_HELP_CONFIRM_STR "Help with Confirm"
#define TX_VERSION_SOURCE_STR "Version to install: %ld.%ld"
#define TX_VERSION_DEST_STR "Version currently installed: %ld.%ld"
#define TX_VERSION_NONE_STR "There is no currently installed version."
#define TX_PROCEED_INST_STR "Proceed With Install"
#define TX_INST_MODE_STR "Set Installation Mode"
#define TX_WELCOME_STR "Welcome to the %s installation utility. Please indicate how the installation should proceed (based upon your knowledge of the Amiga computer)."
#define TX_HELP_INSTMODE_STR "Help with Set Installation Mode"
#define TX_LOG_TO_STR "Log all actions to:"
#define TX_INST_OPTIONS_STR "Installation Options"
#define TX_WELCOME_OPT_STR "Welcome to the %s installation utility."
#define TX_HELP_SETTINGS_STR "Help with Additional Settings"
#define TX_MODIFY_SS_STR "Modify Startup-Sequence File"
#define TX_ANOTHER_FILE_STR "Select Another File"
#define TX_PROCEED_CHANGES_STR "Proceed with Changes"
#define TX_FILE_MODIFIED_STR "File being modified:"
#define TX_HELP_CHANGE_SS_STR "Help with Change Startup-Sequence"
#define TX_ERROR_STR "AN ERROR HAS OCCURED!"
#define TX_DOSERROR_STR "\nDOS Error Type: "
#define TX_SORRY_STR "Sorry... an Error Has Occured!"
#define TX_CONFIRM_STR "Please confirm..."
#define TX_ESCAPE_STR " - Press 'Esc' to Abort"
#define TX_RANGE_STR "Valid range is %ld to %ld"
#define TX_PROCEED_COPY_STR "Proceed with Copy"
#define MSG_BOOTDISK_STR "Please insert your boot disk into the drive you booted from and then click on \"Proceed\". If the disk is already in the drive, just click on \"Proceed\"."
#define MSG_ASKDELETE_STR "\nThe file \"%s\" is protected from deletion.\n\nCan the file be deleted and/or overwritten?"
#define TX_SKIP_FILE_STR "Skip File"
#define TX_DELETE_STR "Delete"
#define HELP_USER_LEVEL_STR "\x05\x74    You will need to select a \"User Level\" for the installer to continue. The different levels are as follows:\n    NOVICE USER: In this mode the installer proceeds entirely automatically, and will not ask you to make any decisions. However, this mode is only recommended for Amiga computers that are not extensively customized, as some of the assumptions made by the program are only valid on a \"stock\" Amiga. You may still be requested to insert disks or perform other actions.\n    INTERMEDIATE USER: The installer will allow you to make major choices, such as where to install the application, but will not bother you with the minor details of copying files, creating drawers, etc.\n    EXPERT USER: In this mode you will be asked for confirmation of each step of the installation process (although in some cases several steps may be combined into a single confirmation). In addition, you will have greater control over where installed items are placed.\n    In addition, there are several buttons at the bottom:\n    \"Proceed with Install\" indicates that you have chosen a User Level and that installation should proceed.\n    \"About...\" gives information about the installation program, such as copyright information, version number, author, etc.\n    \"Abort Install\" means that you have changed your mind and do not wish to install the application at this time.\n    \"Help...\" brings up this text."
#define HELP_INST_SETTINGS_STR "\x03\x99    Explanation of controls:\n\n    \"Install for Real\" -- This will cause the installer to actually carry out the installation.\n    \"Pretend to Install\" -- In this mode, the installer will go through all the steps of the installation, except that it will not make any permanent changes. You can use this option to get a \"preview\" of what the installer will do before it actually does it. You can also use this in combination with the \"log file\" options below to get a complete list of what happened during the rehearsal.\n\n    \"Printer\" -- This will cause a list of all installation actions to be printed out on the printer.\n    \"Log File\" -- This will cause a list of all installation actions to be written to a log file. You will be informed of the location of this file when the installation finishes. You can use a text editor to read this file.\n    \"None\" -- When this option is turned on, no log file will be produced."
#define HELP_CHANGE_STARTUP_STR "\x06\xB7    In order for this product to run correctly, a number of special commands need to be executed by the \"startup-sequence\", a file of commands which is executed each time your Amiga computer boots up. A line needs to be inserted into your startup-sequence to run these commands. The command to be inserted is \"Execute S:user-startup\". This command will execute a script file called \"S:user-startup\" (which will be created if it does not already exist). This file will contain all the special commands needed by this product. In addition, any other product that uses the Commodore-Amiga Installation Utility will also store its startup commands in the \"S:user-startup:\" file in the future.\n\nExplanation of controls:\n    The scrolling list box shows your startup sequence. This installer has already selected what it believes is the best place to insert the new command. If for some reason you feel that the installer has chosen incorrectly (which is certainly possible with a highly customized system), then you may select a new file and installation point.\n    The text box shows the name of the script file to be modified.\n    To select a different file: Select the \"Select another file\" button. The file you select should be one that is executed once each time you boot up and is not executed at any other time.\n    To select a new insertion point: Clicking on the scrolling list will cause the insertion point (labeled \"insert here\") to be moved to the new point.\n    \"Proceed with Changes\" indicates that you are satisfied with the point of insertion, and the command should be inserted into the file.\n    \"Abort Install\" indicates that you wish to abort the installation process.\n    \"Help...\" brings up this text."
#define STR_FILE_TO_MODIFY_STR "\x00\x15Select File To Modify"
#define HELP_SELECT_FILE_STR "\x05\x28Explanation of controls:\n    The large \"scrolling list box\" in the center displays the contents of the currently selected disk or directory. Drawers will be indicated by the word \"DRW\" in inverse block letters in front of the Drawer name. The name of the disk or directory being viewed is shown below it in a raised outlined box. Beneath that, in another raised outlined box, is the name of the file which is currently selected. To the right is a slider which can be used to scroll through the list, in case there are more names than will fit into the window.\n    You can change the selection either by typing a new drawer name into the \"Current Drawer\" text box, or by typing a new file name into the \"Selected File\" text box, or by clicking on the names of drawers and files in the scrolling list.\n    The button \"Parent Drawer\" will allow you to view the drawer that contains this one, in other words the \"Parent\" of the currently visible drawer.\n    The button \"Show Drives\" will show a list of all the disk drives in your system. You can then click on a drive name to view the contents of that drive.\n    The button \"Proceed\" indicates you are satisfied with the currently selected file.\n    The button \"Abort Install\" will abort the installation. No further changes will be made.\n    \"Help...\" brings up this text."
#define STR_NEW_DEST_STR "\x00\x1DSelect New Destination Drawer"
#define HELP_SELECT_DRAWER_STR "\x05\x18Explanation of controls:\n    The large \"scrolling list box\" in the center displays the contents of the currently selected disk or directory. Drawers will be indicated by the word \"DRW\" in inverse block letters in front of the Drawer name. The name of the disk or directory being viewed is shown below it in a raised outlined box. To the right is a slider which can be used to scroll through the list, in case there are more names than will fit into the window.\n    You can change the selection by typing a new drawer name into the \"Current Drawer\" text box, or by clicking on the names of drawers in the scrolling list.\n    The button \"Parent Drawer\" will allow you to view the drawer that contains this one, in other words the \"Parent\" of the currently visible drawer.\n    The button \"Show Drives\" will show a list of all the disk drives in your system. You can then click on a drive name to view the contents of that drive.\n    The button \"Make New Drawer\" will allow you to create a new drawer. The newly created drawer will appear inside the directory that is currently being viewed.\n    The button \"Proceed\" indicates you are satisfied with the currently selected drawer.\n    The button \"Abort Install\" will abort the installation. No further changes will be made.\n    \"Help...\" brings up this text."
#define HELP_VERSION_STR "\x02\x68Explanation of controls:\n    The file to be installed may or may not be replacing an older version. The display shows a comparison between the currently installed version and the new version that will replace it. The destination drawer that the file will be copied to is also shown.\n    The button \"Proceed with Copy\" will cause the file to be copied to the destination drawer.\n    The button \"Skip this part\" causes the installer to proceed to the next section. The file will not be copied.\n    The button \"Abort Install\" will cancel the installation with no further changes made.\n    \"Help...\" brings up this text."
#define HELP_COPYFILES_STR "\x03\x6EExplanation of controls:\n    The raised box shows a list of file or drawer names. Each name has a checkmark next to it. If you want the file to be copied, then leave the name checked. An unchecked name will not be copied.\n    The box labeled \"Destination Drawer to copy to:\", just beneath the listing of names shows the drawer to which the items will be copied.\n    The button \"Proceed with Copy\" will cause all the files or drawers which are checked to be copied to the destination drawer.\n    The button \"Skip this part\" causes the installer to proceed to the next section. None of the items listed will be copied.\n    The button \"Change Destination\" will allow you to select a new destination. You can use this to copy the items somewhere else.\n    The button \"Abort Install\" will cancel the installation with no further changes being made.\n    \"Help...\" brings up this text."
#define HELP_ASKDISK_STR "\x00\xAF    Once you have inserted the requested disk, you should select \"Proceed\". If you cannot find the disk, or wish to abort the installation for any reason, then select \"Abort\"."
#define HELP_ASKSTRING_STR "\x00\xAB    Once you have filled in the text box, you may select \"Proceed\" to continue with the installation. If you wish to abort the installation for any reason, select \"Abort\"."
#define HELP_ASKNUMBER_STR "\x00\xAD    Once you have filled in the number box, you may select \"Proceed\" to continue with the installation. If you wish to abort the installation for any reason, select \"Abort\"."
#define HELP_ASKCHOICE_STR "\x01\x92Explanation of controls:\n    Each of the raised buttons represents a choice which you may make. The button that is selected represents the currently choice. You can change the choice by selecting one of the choice buttons. Only one selection may be chosen. Once you are satisfied, select \"Proceed\" to continue with the installation. If you wish to abort the installation for any reason, select \"Abort\"."
#define HELP_ASKOPTIONS_STR "\x01\x7AExplanation of controls:\n    Each of the checkboxes represents a option which you may make. A checkmark in a box represents a currently selected option. You can change options by clicking on the checkboxes. Once you are satisfied with the selected options, select \"Proceed\" to continue with the installation. If you wish to abort the installation for any reason, select \"Abort\"."
#define HELP_MAKEDIR_STR "\x00\xC9    Select \"Proceed\" if you want the new drawer to be created. Select \"Skip This Part\" if you do not want the drawer to be created. If you wish to abort the installation for any reason, select \"Abort\"."
#define STR_CREATE_ICON_STR "\x00\x16Create icon for drawer"
#define STR_NOVICE_USER_STR "\x00\x27Novice User  - All Actions Automatic   "
#define STR_AVERAGE_USER_STR "\x00\x2CIntermediate User - Limited Manual Control  "
#define STR_EXPERT_USER_STR "\x00\x27Expert User  - Must Confirm all actions"
#define STR_FOR_REAL_STR "\x00\x10Install for Real"
#define STR_PRETEND_STR "\x00\x12Pretend to Install"
#define STR_PRINTER_STR "\x00\x07Printer"
#define STR_LOGFILE_STR "\x00\x08Log File"
#define STR_NOLOG_STR "\x00\x04None"
#define STR_SURE_ABORT_STR "\x00\x1FAre you sure you want to abort?"
#define HELP_STARTUP_STR "\x01\x5BThe \"user-startup\" file located in the \"S:\" drawer is used to store specific commands that some applications need executed at system boot. You are being asked to confirm that this file can be edited. Select the \"Proceed\" button to confirm, otherwise select the button labeled \"Skip This Part\". You can also abort installation by selecting \"Abort\"."
#define MSG_NO_PRINTER_STR "Unable to access the printer."
#define MSG_ACCESS_TRANSCRIPT_STR "Problem with writing to transcript file."
#define MSG_ACCESS_PRINTER_STR "Unable to access the printer."
#define MSG_PROB_SOURCE_STR "Problem with source file/drawer \"%s\""
#define MSG_PROB_FILE_STR "Problem with file \"%s\""
#define MSG_NOEXAMINE_OBJECT_STR "Can't examine file or drawer \"%s\""
#define MSG_NOEXIST_OBJECT_STR "File or drawer \"%s\" doesn't exist"
#define MSG_CANNOT_ASSIGN_STR "Can't make assign"
#define MSG_ERR_MEMORY_STR "Unable to perform operation -- not enough memory"
#define MSG_NO_DRAWER_STR "Can't open drawer \"%s\""
#define MSG_NO_FILE_STR "Can't open file \"%s\""
#define MSG_PROB_SS_STR "Problem with file \"startup-sequence\""
#define MSG_ERR_WRITING_STR "Error writing file \"%s\""
#define MSG_ERR_UNKNOWN_STR "Error was of an unknown type."
#define MSG_ERR_READING_STR "Error reading text include file!"
#define MSG_CANNOT_CREATE_STR "Couldn't create directory \"%s\""
#define MSG_CANNOT_COPY_STR "Couldn't copy file \"%s\""
#define MSG_OVERWRITE_STR "Tried to create drawer over file \"%s\""
#define MSG_EXECUTE_PATH_STR "Can't set execute path"
#define MSG_NO_COMMAND_STR "Couldn't find command to execute"
#define MSG_NO_REXX_STR "ARexx not available"
#define MSG_NO_DUPLOCK_STR "Can't duplicate lock"
#define ERR_READING_US_STR "Can't read \"%s\" file"
#define ERR_WRITING_US_STR "Error in writing \"%s\" file"
#define MSG_BAD_PARAMS_COPY_STR "Bad parameters for copying from drawer"
#define MSG_INVALID_DRAWER_NAME_STR "Invalid drawer name"
#define MSG_NO_WINDOW_STR "Can't open a window."
#define MSG_NO_SCRIPT_ICON_STR "Can't open install script's icon."
#define MSG_NO_PARTITION_STR "Can't find partition."
#define MSG_NO_TRANSCRIPT_STR "Unable to open transcript file."
#define MSG_NO_SCRIPT_STR "Unable to open script."
#define MSG_NO_INIT_STR "Unable to initialize system."
#define MSG_NO_LIBRARIES_STR "Can't open needed libraries."
#define MSG_NO_WORKBENCH_STR "Can't get Workbench information."
#define MSG_TOO_LONG_STR "String too long"
#define MSG_DIVIDE_ZERO_STR "Division by zero"
#define MSG_BAD_CAT_STR "%s: Result string too long"
#define TRANS_HEADER_STR "******* Installation Log *******\nUser Level: %s\nPretend: %s\n\n"
#define TRANS_USER_DRAWER_STR "User made a new drawer: \"%s\".\n"
#define TRANS_COPY_FILE_STR "Copy file \"%s\" to \"%s\".\n"
#define TRANS_COPY_DRAWER_STR "Copy contents of drawer \"%s\" to \"%s\".\n"
#define TRANS_ASK_CHOICE_STR "Ask Choice: User selected \"%s\".\n"
#define TRANS_NO_OPTIONS_STR "Ask Options: User selected no options.\n"
#define TRANS_ASK_OPTIONS_STR "Ask Options: User selected following options...\n"
#define TRANS_ASK_STRING_STR "Ask String: Result was \"%s\".\n"
#define TRANS_ASK_NUMBER_STR "Ask Number: Result was \"%ld\".\n"
#define TRANS_ASK_BOOL_STR "Ask Question: Result was \"%s\".\n"
#define TRANS_DEST_DRAWER_STR "User changed Copy Files destination drawer to \"%s\".\n"
#define TRANS_ASK_DRAWER_STR "Ask Drawer: Result was \"%s\".\n"
#define TRANS_ASK_FILE_STR "Ask File: Result was \"%s\".\n"
#define MSG_ONERROR_STR "*** Global error -- executing onerror statements ***\n"
#define MSG_USER_ABORT_STR "User aborted!\n\n"
#define MSG_ABORT_LINE_STR "Script aborted in line %ld.\n"
#define MSG_PROBLEM_LINE_STR "%s in line %ld.\n"
#define TRANS_CREATE_DRAWER_STR "Create New Directory: \"%s\"\n"
#define TRANS_INSERT_COMMANDS_STR "Insert commands to Execute \"%s\" in file \"%s\" at byte %ld.\n"
#define TRANS_CREATE_TEXTFILE_STR "Create Text File: \"%s\"\n================== Start of textfile ==================\n"
#define TRANS_END_TEXTFILE_STR "=================== End of textfile ===================\n"
#define TRANS_EXECUTE_STR "Execute DOS Script: \"%s\"\n"
#define TRANS_RUN_STR "Run Program: \"%s\"\n"
#define TRANS_AREXX_STR "Execute ARexx Script: \"%s\"\n"
#define TRANS_RENAME_STR "Renamed \"%s\" to \"%s\"\n"
#define TRANS_DELETE_STR "Delete file \"%s\"\n"
#define TRANS_ABORTING_STR "Aborting script.\n"
#define TRANS_EXITING_STR "Exiting script...\n"
#define TRANS_START_US_STR "Writing to %s\n==================== start of file ====================\n"
#define TRANS_END_US_STR "===================== end of file =====================\n"
#define TX_ASSIGN_LABEL_STR "<ASN>"
#define TX_DRAWER_LABEL_STR "<DRW>"
#define TRANS_DEF_NAME_STR "install_log_file"
#define MSG_ABOUT_INSTALLER1_STR "Amiga Application Installation Utility"
#define MSG_ABOUT_INSTALLER2_STR "\nCopyright ©1990-1993 Commodore-Amiga, Inc.\nAll Rights Reserved.\n\nDesigned and Developed by\nSylvan Technical Arts"
#define TX_USAGE_STR "USAGE: installer [SCRIPT] filename <[APPNAME] name> <[MINUSER] level>\n       <[DEFUSER] default> <[LOGFILE] logname> <NOPRETEND> <NOLOG>"
#define MSG_BAD_ARGS_STR "Bad Argument"
#define MSG_NO_FONT_STR "Can't find needed ROM font."
#define MSG_NO_COMPILE_STR "Unable to compile script.\nERROR: %s on line %ld."
#define MSG_NO_NIL_STR "Couldn't access NIL:"

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
    {TX_YES,(STRPTR)TX_YES_STR},
    {TX_NO,(STRPTR)TX_NO_STR},
    {TX_ABORT,(STRPTR)TX_ABORT_STR},
    {TX_HELP,(STRPTR)TX_HELP_STR},
    {TX_PROCEED,(STRPTR)TX_PROCEED_STR},
    {TX_CANCEL,(STRPTR)TX_CANCEL_STR},
    {TX_PARENT,(STRPTR)TX_PARENT_STR},
    {TX_DRIVES,(STRPTR)TX_DRIVES_STR},
    {TX_MAKEDIR,(STRPTR)TX_MAKEDIR_STR},
    {TX_SKIP,(STRPTR)TX_SKIP_STR},
    {TX_EXITHELP,(STRPTR)TX_EXITHELP_STR},
    {TX_MORE,(STRPTR)TX_MORE_STR},
    {TX_BACK,(STRPTR)TX_BACK_STR},
    {TX_EXITABOUT,(STRPTR)TX_EXITABOUT_STR},
    {TX_ABOUT,(STRPTR)TX_ABOUT_STR},
    {TX_COMPLETE,(STRPTR)TX_COMPLETE_STR},
    {TX_OK,(STRPTR)TX_OK_STR},
    {TX_COPYING_FILE,(STRPTR)TX_COPYING_FILE_STR},
    {TX_TO_DRAWER,(STRPTR)TX_TO_DRAWER_STR},
    {TX_MAKEDIR_INFO,(STRPTR)TX_MAKEDIR_INFO_STR},
    {TX_ENTER_PATH,(STRPTR)TX_ENTER_PATH_STR},
    {TX_CHANGE_DEST,(STRPTR)TX_CHANGE_DEST_STR},
    {TX_COPY_DEST,(STRPTR)TX_COPY_DEST_STR},
    {TX_HELP_COPY,(STRPTR)TX_HELP_COPY_STR},
    {TX_HELP_ASKCHOICE,(STRPTR)TX_HELP_ASKCHOICE_STR},
    {TX_HELP_ASKOPTIONS,(STRPTR)TX_HELP_ASKOPTIONS_STR},
    {TX_HELP_ASKSTRING,(STRPTR)TX_HELP_ASKSTRING_STR},
    {TX_HELP_ASKNUMBER,(STRPTR)TX_HELP_ASKNUMBER_STR},
    {TX_HELP_ASKDISK,(STRPTR)TX_HELP_ASKDISK_STR},
    {TX_PLEASE_INSERT,(STRPTR)TX_PLEASE_INSERT_STR},
    {TX_HELP_ASKBOOL,(STRPTR)TX_HELP_ASKBOOL_STR},
    {TX_SELECTED_DRAWER,(STRPTR)TX_SELECTED_DRAWER_STR},
    {TX_HELP_SELECTDIR,(STRPTR)TX_HELP_SELECTDIR_STR},
    {TX_SELECTED_FILE,(STRPTR)TX_SELECTED_FILE_STR},
    {TX_CURRENT_DRAWER,(STRPTR)TX_CURRENT_DRAWER_STR},
    {TX_HELP_SELECTFILE,(STRPTR)TX_HELP_SELECTFILE_STR},
    {TX_INST_COMPLETE,(STRPTR)TX_INST_COMPLETE_STR},
    {TX_INST_COMP_WHERE,(STRPTR)TX_INST_COMP_WHERE_STR},
    {TX_INST_LOGFILE,(STRPTR)TX_INST_LOGFILE_STR},
    {TX_WORKING,(STRPTR)TX_WORKING_STR},
    {TX_HELP_CONFIRM,(STRPTR)TX_HELP_CONFIRM_STR},
    {TX_VERSION_SOURCE,(STRPTR)TX_VERSION_SOURCE_STR},
    {TX_VERSION_DEST,(STRPTR)TX_VERSION_DEST_STR},
    {TX_VERSION_NONE,(STRPTR)TX_VERSION_NONE_STR},
    {TX_PROCEED_INST,(STRPTR)TX_PROCEED_INST_STR},
    {TX_INST_MODE,(STRPTR)TX_INST_MODE_STR},
    {TX_WELCOME,(STRPTR)TX_WELCOME_STR},
    {TX_HELP_INSTMODE,(STRPTR)TX_HELP_INSTMODE_STR},
    {TX_LOG_TO,(STRPTR)TX_LOG_TO_STR},
    {TX_INST_OPTIONS,(STRPTR)TX_INST_OPTIONS_STR},
    {TX_WELCOME_OPT,(STRPTR)TX_WELCOME_OPT_STR},
    {TX_HELP_SETTINGS,(STRPTR)TX_HELP_SETTINGS_STR},
    {TX_MODIFY_SS,(STRPTR)TX_MODIFY_SS_STR},
    {TX_ANOTHER_FILE,(STRPTR)TX_ANOTHER_FILE_STR},
    {TX_PROCEED_CHANGES,(STRPTR)TX_PROCEED_CHANGES_STR},
    {TX_FILE_MODIFIED,(STRPTR)TX_FILE_MODIFIED_STR},
    {TX_HELP_CHANGE_SS,(STRPTR)TX_HELP_CHANGE_SS_STR},
    {TX_ERROR,(STRPTR)TX_ERROR_STR},
    {TX_DOSERROR,(STRPTR)TX_DOSERROR_STR},
    {TX_SORRY,(STRPTR)TX_SORRY_STR},
    {TX_CONFIRM,(STRPTR)TX_CONFIRM_STR},
    {TX_ESCAPE,(STRPTR)TX_ESCAPE_STR},
    {TX_RANGE,(STRPTR)TX_RANGE_STR},
    {TX_PROCEED_COPY,(STRPTR)TX_PROCEED_COPY_STR},
    {MSG_BOOTDISK,(STRPTR)MSG_BOOTDISK_STR},
    {MSG_ASKDELETE,(STRPTR)MSG_ASKDELETE_STR},
    {TX_SKIP_FILE,(STRPTR)TX_SKIP_FILE_STR},
    {TX_DELETE,(STRPTR)TX_DELETE_STR},
    {HELP_USER_LEVEL,(STRPTR)HELP_USER_LEVEL_STR},
    {HELP_INST_SETTINGS,(STRPTR)HELP_INST_SETTINGS_STR},
    {HELP_CHANGE_STARTUP,(STRPTR)HELP_CHANGE_STARTUP_STR},
    {STR_FILE_TO_MODIFY,(STRPTR)STR_FILE_TO_MODIFY_STR},
    {HELP_SELECT_FILE,(STRPTR)HELP_SELECT_FILE_STR},
    {STR_NEW_DEST,(STRPTR)STR_NEW_DEST_STR},
    {HELP_SELECT_DRAWER,(STRPTR)HELP_SELECT_DRAWER_STR},
    {HELP_VERSION,(STRPTR)HELP_VERSION_STR},
    {HELP_COPYFILES,(STRPTR)HELP_COPYFILES_STR},
    {HELP_ASKDISK,(STRPTR)HELP_ASKDISK_STR},
    {HELP_ASKSTRING,(STRPTR)HELP_ASKSTRING_STR},
    {HELP_ASKNUMBER,(STRPTR)HELP_ASKNUMBER_STR},
    {HELP_ASKCHOICE,(STRPTR)HELP_ASKCHOICE_STR},
    {HELP_ASKOPTIONS,(STRPTR)HELP_ASKOPTIONS_STR},
    {HELP_MAKEDIR,(STRPTR)HELP_MAKEDIR_STR},
    {STR_CREATE_ICON,(STRPTR)STR_CREATE_ICON_STR},
    {STR_NOVICE_USER,(STRPTR)STR_NOVICE_USER_STR},
    {STR_AVERAGE_USER,(STRPTR)STR_AVERAGE_USER_STR},
    {STR_EXPERT_USER,(STRPTR)STR_EXPERT_USER_STR},
    {STR_FOR_REAL,(STRPTR)STR_FOR_REAL_STR},
    {STR_PRETEND,(STRPTR)STR_PRETEND_STR},
    {STR_PRINTER,(STRPTR)STR_PRINTER_STR},
    {STR_LOGFILE,(STRPTR)STR_LOGFILE_STR},
    {STR_NOLOG,(STRPTR)STR_NOLOG_STR},
    {STR_SURE_ABORT,(STRPTR)STR_SURE_ABORT_STR},
    {HELP_STARTUP,(STRPTR)HELP_STARTUP_STR},
    {MSG_NO_PRINTER,(STRPTR)MSG_NO_PRINTER_STR},
    {MSG_ACCESS_TRANSCRIPT,(STRPTR)MSG_ACCESS_TRANSCRIPT_STR},
    {MSG_ACCESS_PRINTER,(STRPTR)MSG_ACCESS_PRINTER_STR},
    {MSG_PROB_SOURCE,(STRPTR)MSG_PROB_SOURCE_STR},
    {MSG_PROB_FILE,(STRPTR)MSG_PROB_FILE_STR},
    {MSG_NOEXAMINE_OBJECT,(STRPTR)MSG_NOEXAMINE_OBJECT_STR},
    {MSG_NOEXIST_OBJECT,(STRPTR)MSG_NOEXIST_OBJECT_STR},
    {MSG_CANNOT_ASSIGN,(STRPTR)MSG_CANNOT_ASSIGN_STR},
    {MSG_ERR_MEMORY,(STRPTR)MSG_ERR_MEMORY_STR},
    {MSG_NO_DRAWER,(STRPTR)MSG_NO_DRAWER_STR},
    {MSG_NO_FILE,(STRPTR)MSG_NO_FILE_STR},
    {MSG_PROB_SS,(STRPTR)MSG_PROB_SS_STR},
    {MSG_ERR_WRITING,(STRPTR)MSG_ERR_WRITING_STR},
    {MSG_ERR_UNKNOWN,(STRPTR)MSG_ERR_UNKNOWN_STR},
    {MSG_ERR_READING,(STRPTR)MSG_ERR_READING_STR},
    {MSG_CANNOT_CREATE,(STRPTR)MSG_CANNOT_CREATE_STR},
    {MSG_CANNOT_COPY,(STRPTR)MSG_CANNOT_COPY_STR},
    {MSG_OVERWRITE,(STRPTR)MSG_OVERWRITE_STR},
    {MSG_EXECUTE_PATH,(STRPTR)MSG_EXECUTE_PATH_STR},
    {MSG_NO_COMMAND,(STRPTR)MSG_NO_COMMAND_STR},
    {MSG_NO_REXX,(STRPTR)MSG_NO_REXX_STR},
    {MSG_NO_DUPLOCK,(STRPTR)MSG_NO_DUPLOCK_STR},
    {ERR_READING_US,(STRPTR)ERR_READING_US_STR},
    {ERR_WRITING_US,(STRPTR)ERR_WRITING_US_STR},
    {MSG_BAD_PARAMS_COPY,(STRPTR)MSG_BAD_PARAMS_COPY_STR},
    {MSG_INVALID_DRAWER_NAME,(STRPTR)MSG_INVALID_DRAWER_NAME_STR},
    {MSG_NO_WINDOW,(STRPTR)MSG_NO_WINDOW_STR},
    {MSG_NO_SCRIPT_ICON,(STRPTR)MSG_NO_SCRIPT_ICON_STR},
    {MSG_NO_PARTITION,(STRPTR)MSG_NO_PARTITION_STR},
    {MSG_NO_TRANSCRIPT,(STRPTR)MSG_NO_TRANSCRIPT_STR},
    {MSG_NO_SCRIPT,(STRPTR)MSG_NO_SCRIPT_STR},
    {MSG_NO_INIT,(STRPTR)MSG_NO_INIT_STR},
    {MSG_NO_LIBRARIES,(STRPTR)MSG_NO_LIBRARIES_STR},
    {MSG_NO_WORKBENCH,(STRPTR)MSG_NO_WORKBENCH_STR},
    {MSG_TOO_LONG,(STRPTR)MSG_TOO_LONG_STR},
    {MSG_DIVIDE_ZERO,(STRPTR)MSG_DIVIDE_ZERO_STR},
    {MSG_BAD_CAT,(STRPTR)MSG_BAD_CAT_STR},
    {TRANS_HEADER,(STRPTR)TRANS_HEADER_STR},
    {TRANS_USER_DRAWER,(STRPTR)TRANS_USER_DRAWER_STR},
    {TRANS_COPY_FILE,(STRPTR)TRANS_COPY_FILE_STR},
    {TRANS_COPY_DRAWER,(STRPTR)TRANS_COPY_DRAWER_STR},
    {TRANS_ASK_CHOICE,(STRPTR)TRANS_ASK_CHOICE_STR},
    {TRANS_NO_OPTIONS,(STRPTR)TRANS_NO_OPTIONS_STR},
    {TRANS_ASK_OPTIONS,(STRPTR)TRANS_ASK_OPTIONS_STR},
    {TRANS_ASK_STRING,(STRPTR)TRANS_ASK_STRING_STR},
    {TRANS_ASK_NUMBER,(STRPTR)TRANS_ASK_NUMBER_STR},
    {TRANS_ASK_BOOL,(STRPTR)TRANS_ASK_BOOL_STR},
    {TRANS_DEST_DRAWER,(STRPTR)TRANS_DEST_DRAWER_STR},
    {TRANS_ASK_DRAWER,(STRPTR)TRANS_ASK_DRAWER_STR},
    {TRANS_ASK_FILE,(STRPTR)TRANS_ASK_FILE_STR},
    {MSG_ONERROR,(STRPTR)MSG_ONERROR_STR},
    {MSG_USER_ABORT,(STRPTR)MSG_USER_ABORT_STR},
    {MSG_ABORT_LINE,(STRPTR)MSG_ABORT_LINE_STR},
    {MSG_PROBLEM_LINE,(STRPTR)MSG_PROBLEM_LINE_STR},
    {TRANS_CREATE_DRAWER,(STRPTR)TRANS_CREATE_DRAWER_STR},
    {TRANS_INSERT_COMMANDS,(STRPTR)TRANS_INSERT_COMMANDS_STR},
    {TRANS_CREATE_TEXTFILE,(STRPTR)TRANS_CREATE_TEXTFILE_STR},
    {TRANS_END_TEXTFILE,(STRPTR)TRANS_END_TEXTFILE_STR},
    {TRANS_EXECUTE,(STRPTR)TRANS_EXECUTE_STR},
    {TRANS_RUN,(STRPTR)TRANS_RUN_STR},
    {TRANS_AREXX,(STRPTR)TRANS_AREXX_STR},
    {TRANS_RENAME,(STRPTR)TRANS_RENAME_STR},
    {TRANS_DELETE,(STRPTR)TRANS_DELETE_STR},
    {TRANS_ABORTING,(STRPTR)TRANS_ABORTING_STR},
    {TRANS_EXITING,(STRPTR)TRANS_EXITING_STR},
    {TRANS_START_US,(STRPTR)TRANS_START_US_STR},
    {TRANS_END_US,(STRPTR)TRANS_END_US_STR},
    {TX_ASSIGN_LABEL,(STRPTR)TX_ASSIGN_LABEL_STR},
    {TX_DRAWER_LABEL,(STRPTR)TX_DRAWER_LABEL_STR},
    {TRANS_DEF_NAME,(STRPTR)TRANS_DEF_NAME_STR},
    {MSG_ABOUT_INSTALLER1,(STRPTR)MSG_ABOUT_INSTALLER1_STR},
    {MSG_ABOUT_INSTALLER2,(STRPTR)MSG_ABOUT_INSTALLER2_STR},
    {TX_USAGE,(STRPTR)TX_USAGE_STR},
    {MSG_BAD_ARGS,(STRPTR)MSG_BAD_ARGS_STR},
    {MSG_NO_FONT,(STRPTR)MSG_NO_FONT_STR},
    {MSG_NO_COMPILE,(STRPTR)MSG_NO_COMPILE_STR},
    {MSG_NO_NIL,(STRPTR)MSG_NO_NIL_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x03\x00\x04"
    TX_YES_STR "\x00"
    "\x00\x00\x00\x04\x00\x04"
    TX_NO_STR "\x00\x00"
    "\x00\x00\x00\x05\x00\x0E"
    TX_ABORT_STR "\x00"
    "\x00\x00\x00\x06\x00\x08"
    TX_HELP_STR "\x00"
    "\x00\x00\x00\x07\x00\x08"
    TX_PROCEED_STR "\x00"
    "\x00\x00\x00\x08\x00\x08"
    TX_CANCEL_STR "\x00\x00"
    "\x00\x00\x00\x09\x00\x0E"
    TX_PARENT_STR "\x00"
    "\x00\x00\x00\x0A\x00\x0C"
    TX_DRIVES_STR "\x00"
    "\x00\x00\x00\x0B\x00\x14"
    TX_MAKEDIR_STR "\x00\x00"
    "\x00\x00\x00\x0C\x00\x10"
    TX_SKIP_STR "\x00\x00"
    "\x00\x00\x00\x0D\x00\x10"
    TX_EXITHELP_STR "\x00\x00"
    "\x00\x00\x00\x0E\x00\x06"
    TX_MORE_STR "\x00\x00"
    "\x00\x00\x00\x0F\x00\x06"
    TX_BACK_STR "\x00\x00"
    "\x00\x00\x00\x10\x00\x10"
    TX_EXITABOUT_STR "\x00"
    "\x00\x00\x00\x11\x00\x0A"
    TX_ABOUT_STR "\x00\x00"
    "\x00\x00\x00\x12\x00\x06"
    TX_COMPLETE_STR "\x00\x00"
    "\x00\x00\x00\x13\x00\x04"
    TX_OK_STR "\x00\x00"
    "\x00\x00\x00\x2E\x00\x0E"
    TX_COPYING_FILE_STR "\x00"
    "\x00\x00\x00\x2F\x00\x0C"
    TX_TO_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x30\x00\x8E"
    TX_MAKEDIR_INFO_STR "\x00"
    "\x00\x00\x00\x31\x00\x2A"
    TX_ENTER_PATH_STR "\x00\x00"
    "\x00\x00\x00\x32\x00\x14"
    TX_CHANGE_DEST_STR "\x00\x00"
    "\x00\x00\x00\x33\x00\x20"
    TX_COPY_DEST_STR "\x00\x00"
    "\x00\x00\x00\x36\x00\x16"
    TX_HELP_COPY_STR "\x00\x00"
    "\x00\x00\x00\x37\x00\x1C"
    TX_HELP_ASKCHOICE_STR "\x00\x00"
    "\x00\x00\x00\x38\x00\x1C"
    TX_HELP_ASKOPTIONS_STR "\x00\x00"
    "\x00\x00\x00\x39\x00\x16"
    TX_HELP_ASKSTRING_STR "\x00\x00"
    "\x00\x00\x00\x3A\x00\x16"
    TX_HELP_ASKNUMBER_STR "\x00\x00"
    "\x00\x00\x00\x3B\x00\x16"
    TX_HELP_ASKDISK_STR "\x00"
    "\x00\x00\x00\x3C\x00\x28"
    TX_PLEASE_INSERT_STR "\x00"
    "\x00\x00\x00\x3D\x00\x18"
    TX_HELP_ASKBOOL_STR "\x00\x00"
    "\x00\x00\x00\x3E\x00\x10"
    TX_SELECTED_DRAWER_STR "\x00"
    "\x00\x00\x00\x3F\x00\x18"
    TX_HELP_SELECTDIR_STR "\x00"
    "\x00\x00\x00\x40\x00\x0E"
    TX_SELECTED_FILE_STR "\x00"
    "\x00\x00\x00\x41\x00\x10"
    TX_CURRENT_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x42\x00\x16"
    TX_HELP_SELECTFILE_STR "\x00"
    "\x00\x00\x00\x43\x00\x28"
    TX_INST_COMPLETE_STR "\x00\x00"
    "\x00\x00\x00\x44\x00\x4C"
    TX_INST_COMP_WHERE_STR "\x00\x00"
    "\x00\x00\x00\x45\x00\x2C"
    TX_INST_LOGFILE_STR "\x00\x00"
    "\x00\x00\x00\x46\x00\x1C"
    TX_WORKING_STR "\x00\x00"
    "\x00\x00\x00\x47\x00\x12"
    TX_HELP_CONFIRM_STR "\x00"
    "\x00\x00\x00\x48\x00\x1C"
    TX_VERSION_SOURCE_STR "\x00"
    "\x00\x00\x00\x49\x00\x26"
    TX_VERSION_DEST_STR "\x00\x00"
    "\x00\x00\x00\x4A\x00\x2A"
    TX_VERSION_NONE_STR "\x00\x00"
    "\x00\x00\x00\x4B\x00\x16"
    TX_PROCEED_INST_STR "\x00\x00"
    "\x00\x00\x00\x4C\x00\x16"
    TX_INST_MODE_STR "\x00"
    "\x00\x00\x00\x4D\x00\x90"
    TX_WELCOME_STR "\x00\x00"
    "\x00\x00\x00\x4E\x00\x20"
    TX_HELP_INSTMODE_STR "\x00"
    "\x00\x00\x00\x4F\x00\x14"
    TX_LOG_TO_STR "\x00"
    "\x00\x00\x00\x50\x00\x16"
    TX_INST_OPTIONS_STR "\x00\x00"
    "\x00\x00\x00\x51\x00\x28"
    TX_WELCOME_OPT_STR "\x00"
    "\x00\x00\x00\x52\x00\x1E"
    TX_HELP_SETTINGS_STR "\x00"
    "\x00\x00\x00\x53\x00\x1E"
    TX_MODIFY_SS_STR "\x00\x00"
    "\x00\x00\x00\x54\x00\x14"
    TX_ANOTHER_FILE_STR "\x00"
    "\x00\x00\x00\x55\x00\x16"
    TX_PROCEED_CHANGES_STR "\x00\x00"
    "\x00\x00\x00\x56\x00\x16"
    TX_FILE_MODIFIED_STR "\x00\x00"
    "\x00\x00\x00\x57\x00\x22"
    TX_HELP_CHANGE_SS_STR "\x00"
    "\x00\x00\x00\x58\x00\x16"
    TX_ERROR_STR "\x00"
    "\x00\x00\x00\x59\x00\x12"
    TX_DOSERROR_STR "\x00"
    "\x00\x00\x00\x5A\x00\x20"
    TX_SORRY_STR "\x00\x00"
    "\x00\x00\x00\x5B\x00\x12"
    TX_CONFIRM_STR "\x00"
    "\x00\x00\x00\x5C\x00\x18"
    TX_ESCAPE_STR "\x00"
    "\x00\x00\x00\x5D\x00\x1A"
    TX_RANGE_STR "\x00"
    "\x00\x00\x00\x60\x00\x12"
    TX_PROCEED_COPY_STR "\x00"
    "\x00\x00\x00\x83\x00\x98"
    MSG_BOOTDISK_STR "\x00\x00"
    "\x00\x00\x00\xA8\x00\x58"
    MSG_ASKDELETE_STR "\x00"
    "\x00\x00\x00\xA9\x00\x0A"
    TX_SKIP_FILE_STR "\x00"
    "\x00\x00\x00\xAA\x00\x08"
    TX_DELETE_STR "\x00\x00"
    "\x00\x00\x00\x8A\x05\x78"
    HELP_USER_LEVEL_STR "\x00\x00"
    "\x00\x00\x00\x8B\x03\x9C"
    HELP_INST_SETTINGS_STR "\x00"
    "\x00\x00\x00\x8C\x06\xBA"
    HELP_CHANGE_STARTUP_STR "\x00"
    "\x00\x00\x00\x8D\x00\x18"
    STR_FILE_TO_MODIFY_STR "\x00"
    "\x00\x00\x00\x8E\x05\x2C"
    HELP_SELECT_FILE_STR "\x00\x00"
    "\x00\x00\x00\x8F\x00\x20"
    STR_NEW_DEST_STR "\x00"
    "\x00\x00\x00\x90\x05\x1C"
    HELP_SELECT_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x91\x02\x6C"
    HELP_VERSION_STR "\x00\x00"
    "\x00\x00\x00\x92\x03\x72"
    HELP_COPYFILES_STR "\x00\x00"
    "\x00\x00\x00\x93\x00\xB2"
    HELP_ASKDISK_STR "\x00"
    "\x00\x00\x00\x94\x00\xAE"
    HELP_ASKSTRING_STR "\x00"
    "\x00\x00\x00\x95\x00\xB0"
    HELP_ASKNUMBER_STR "\x00"
    "\x00\x00\x00\x96\x01\x96"
    HELP_ASKCHOICE_STR "\x00\x00"
    "\x00\x00\x00\x97\x01\x7E"
    HELP_ASKOPTIONS_STR "\x00\x00"
    "\x00\x00\x00\x98\x00\xCC"
    HELP_MAKEDIR_STR "\x00"
    "\x00\x00\x00\x99\x00\x1A"
    STR_CREATE_ICON_STR "\x00\x00"
    "\x00\x00\x00\x9A\x00\x2A"
    STR_NOVICE_USER_STR "\x00"
    "\x00\x00\x00\x9B\x00\x30"
    STR_AVERAGE_USER_STR "\x00\x00"
    "\x00\x00\x00\x9C\x00\x2A"
    STR_EXPERT_USER_STR "\x00"
    "\x00\x00\x00\x9D\x00\x14"
    STR_FOR_REAL_STR "\x00\x00"
    "\x00\x00\x00\x9E\x00\x16"
    STR_PRETEND_STR "\x00\x00"
    "\x00\x00\x00\x9F\x00\x0A"
    STR_PRINTER_STR "\x00"
    "\x00\x00\x00\xA0\x00\x0C"
    STR_LOGFILE_STR "\x00\x00"
    "\x00\x00\x00\xA1\x00\x08"
    STR_NOLOG_STR "\x00\x00"
    "\x00\x00\x00\xA2\x00\x22"
    STR_SURE_ABORT_STR "\x00"
    "\x00\x00\x00\xA3\x01\x5E"
    HELP_STARTUP_STR "\x00"
    "\x00\x00\x00\x1A\x00\x1E"
    MSG_NO_PRINTER_STR "\x00"
    "\x00\x00\x00\x1B\x00\x2A"
    MSG_ACCESS_TRANSCRIPT_STR "\x00\x00"
    "\x00\x00\x00\x1C\x00\x1E"
    MSG_ACCESS_PRINTER_STR "\x00"
    "\x00\x00\x00\x34\x00\x26"
    MSG_PROB_SOURCE_STR "\x00\x00"
    "\x00\x00\x00\x35\x00\x18"
    MSG_PROB_FILE_STR "\x00\x00"
    "\x00\x00\x00\x61\x00\x22"
    MSG_NOEXAMINE_OBJECT_STR "\x00"
    "\x00\x00\x00\x62\x00\x22"
    MSG_NOEXIST_OBJECT_STR "\x00"
    "\x00\x00\x00\x63\x00\x12"
    MSG_CANNOT_ASSIGN_STR "\x00"
    "\x00\x00\x00\x66\x00\x32"
    MSG_ERR_MEMORY_STR "\x00\x00"
    "\x00\x00\x00\x75\x00\x18"
    MSG_NO_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x76\x00\x16"
    MSG_NO_FILE_STR "\x00\x00"
    "\x00\x00\x00\x77\x00\x26"
    MSG_PROB_SS_STR "\x00\x00"
    "\x00\x00\x00\x78\x00\x18"
    MSG_ERR_WRITING_STR "\x00"
    "\x00\x00\x00\x79\x00\x1E"
    MSG_ERR_UNKNOWN_STR "\x00"
    "\x00\x00\x00\x7A\x00\x22"
    MSG_ERR_READING_STR "\x00\x00"
    "\x00\x00\x00\x7B\x00\x20"
    MSG_CANNOT_CREATE_STR "\x00\x00"
    "\x00\x00\x00\x7C\x00\x18"
    MSG_CANNOT_COPY_STR "\x00"
    "\x00\x00\x00\x7D\x00\x26"
    MSG_OVERWRITE_STR "\x00"
    "\x00\x00\x00\x7E\x00\x18"
    MSG_EXECUTE_PATH_STR "\x00\x00"
    "\x00\x00\x00\x80\x00\x22"
    MSG_NO_COMMAND_STR "\x00\x00"
    "\x00\x00\x00\x81\x00\x14"
    MSG_NO_REXX_STR "\x00"
    "\x00\x00\x00\x82\x00\x16"
    MSG_NO_DUPLOCK_STR "\x00\x00"
    "\x00\x00\x00\x88\x00\x16"
    ERR_READING_US_STR "\x00\x00"
    "\x00\x00\x00\x89\x00\x1C"
    ERR_WRITING_US_STR "\x00\x00"
    "\x00\x00\x00\xA4\x00\x28"
    MSG_BAD_PARAMS_COPY_STR "\x00\x00"
    "\x00\x00\x00\xA7\x00\x14"
    MSG_INVALID_DRAWER_NAME_STR "\x00"
    "\x00\x00\x00\x16\x00\x16"
    MSG_NO_WINDOW_STR "\x00\x00"
    "\x00\x00\x00\x17\x00\x22"
    MSG_NO_SCRIPT_ICON_STR "\x00"
    "\x00\x00\x00\x18\x00\x16"
    MSG_NO_PARTITION_STR "\x00"
    "\x00\x00\x00\x19\x00\x20"
    MSG_NO_TRANSCRIPT_STR "\x00"
    "\x00\x00\x00\x1E\x00\x18"
    MSG_NO_SCRIPT_STR "\x00\x00"
    "\x00\x00\x00\x1F\x00\x1E"
    MSG_NO_INIT_STR "\x00\x00"
    "\x00\x00\x00\x20\x00\x1E"
    MSG_NO_LIBRARIES_STR "\x00\x00"
    "\x00\x00\x00\x21\x00\x22"
    MSG_NO_WORKBENCH_STR "\x00\x00"
    "\x00\x00\x00\x5F\x00\x10"
    MSG_TOO_LONG_STR "\x00"
    "\x00\x00\x00\x64\x00\x12"
    MSG_DIVIDE_ZERO_STR "\x00\x00"
    "\x00\x00\x00\x65\x00\x1C"
    MSG_BAD_CAT_STR "\x00\x00"
    "\x00\x00\x00\x00\x00\x3E"
    TRANS_HEADER_STR "\x00"
    "\x00\x00\x00\x22\x00\x20"
    TRANS_USER_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x23\x00\x1A"
    TRANS_COPY_FILE_STR "\x00\x00"
    "\x00\x00\x00\x24\x00\x28"
    TRANS_COPY_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x25\x00\x22"
    TRANS_ASK_CHOICE_STR "\x00\x00"
    "\x00\x00\x00\x26\x00\x28"
    TRANS_NO_OPTIONS_STR "\x00"
    "\x00\x00\x00\x27\x00\x32"
    TRANS_ASK_OPTIONS_STR "\x00\x00"
    "\x00\x00\x00\x28\x00\x1E"
    TRANS_ASK_STRING_STR "\x00"
    "\x00\x00\x00\x29\x00\x20"
    TRANS_ASK_NUMBER_STR "\x00\x00"
    "\x00\x00\x00\x2A\x00\x20"
    TRANS_ASK_BOOL_STR "\x00"
    "\x00\x00\x00\x2B\x00\x36"
    TRANS_DEST_DRAWER_STR "\x00\x00"
    "\x00\x00\x00\x2C\x00\x1E"
    TRANS_ASK_DRAWER_STR "\x00"
    "\x00\x00\x00\x2D\x00\x1C"
    TRANS_ASK_FILE_STR "\x00"
    "\x00\x00\x00\x5E\x00\x36"
    MSG_ONERROR_STR "\x00"
    "\x00\x00\x00\x67\x00\x10"
    MSG_USER_ABORT_STR "\x00"
    "\x00\x00\x00\x68\x00\x1E"
    MSG_ABORT_LINE_STR "\x00\x00"
    "\x00\x00\x00\x69\x00\x12"
    MSG_PROBLEM_LINE_STR "\x00\x00"
    "\x00\x00\x00\x6A\x00\x1C"
    TRANS_CREATE_DRAWER_STR "\x00"
    "\x00\x00\x00\x6B\x00\x3C"
    TRANS_INSERT_COMMANDS_STR "\x00\x00"
    "\x00\x00\x00\x6C\x00\x50"
    TRANS_CREATE_TEXTFILE_STR "\x00"
    "\x00\x00\x00\x6D\x00\x3A"
    TRANS_END_TEXTFILE_STR "\x00\x00"
    "\x00\x00\x00\x6E\x00\x1A"
    TRANS_EXECUTE_STR "\x00"
    "\x00\x00\x00\x6F\x00\x14"
    TRANS_RUN_STR "\x00\x00"
    "\x00\x00\x00\x70\x00\x1C"
    TRANS_AREXX_STR "\x00"
    "\x00\x00\x00\x71\x00\x16"
    TRANS_RENAME_STR "\x00"
    "\x00\x00\x00\x72\x00\x12"
    TRANS_DELETE_STR "\x00"
    "\x00\x00\x00\x73\x00\x12"
    TRANS_ABORTING_STR "\x00"
    "\x00\x00\x00\x74\x00\x14"
    TRANS_EXITING_STR "\x00\x00"
    "\x00\x00\x00\x86\x00\x48"
    TRANS_START_US_STR "\x00\x00"
    "\x00\x00\x00\x87\x00\x3A"
    TRANS_END_US_STR "\x00\x00"
    "\x00\x00\x00\x84\x00\x06"
    TX_ASSIGN_LABEL_STR "\x00"
    "\x00\x00\x00\x85\x00\x06"
    TX_DRAWER_LABEL_STR "\x00"
    "\x00\x00\x00\x01\x00\x12"
    TRANS_DEF_NAME_STR "\x00\x00"
    "\x00\x00\x00\xA5\x00\x28"
    MSG_ABOUT_INSTALLER1_STR "\x00\x00"
    "\x00\x00\x00\xA6\x00\x72"
    MSG_ABOUT_INSTALLER2_STR "\x00"
    "\x00\x00\x00\x02\x00\x8A"
    TX_USAGE_STR "\x00\x00"
    "\x00\x00\x00\x14\x00\x0E"
    MSG_BAD_ARGS_STR "\x00\x00"
    "\x00\x00\x00\x15\x00\x1C"
    MSG_NO_FONT_STR "\x00"
    "\x00\x00\x00\x1D\x00\x32"
    MSG_NO_COMPILE_STR "\x00\x00"
    "\x00\x00\x00\x7F\x00\x16"
    MSG_NO_NIL_STR "\x00\x00"
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


#endif /* INSTALLERSTR_H */
