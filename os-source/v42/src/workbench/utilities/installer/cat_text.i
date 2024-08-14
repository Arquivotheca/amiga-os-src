	IFND INSTALLERSTR_I
INSTALLERSTR_I	SET	1


;-----------------------------------------------------------------------------


* This file was created automatically by CatComp.
* Do NOT edit by hand!
*


	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC

	IFD CATCOMP_ARRAY
CATCOMP_NUMBERS SET 1
CATCOMP_STRINGS SET 1
	ENDC

	IFD CATCOMP_CODE
CATCOMP_BLOCK SET 1
	ENDC


;-----------------------------------------------------------------------------


	IFD CATCOMP_NUMBERS

TX_YES EQU 3
TX_NO EQU 4
TX_ABORT EQU 5
TX_HELP EQU 6
TX_PROCEED EQU 7
TX_CANCEL EQU 8
TX_PARENT EQU 9
TX_DRIVES EQU 10
TX_MAKEDIR EQU 11
TX_SKIP EQU 12
TX_EXITHELP EQU 13
TX_MORE EQU 14
TX_BACK EQU 15
TX_EXITABOUT EQU 16
TX_ABOUT EQU 17
TX_COMPLETE EQU 18
TX_OK EQU 19
TX_COPYING_FILE EQU 46
TX_TO_DRAWER EQU 47
TX_MAKEDIR_INFO EQU 48
TX_ENTER_PATH EQU 49
TX_CHANGE_DEST EQU 50
TX_COPY_DEST EQU 51
TX_HELP_COPY EQU 54
TX_HELP_ASKCHOICE EQU 55
TX_HELP_ASKOPTIONS EQU 56
TX_HELP_ASKSTRING EQU 57
TX_HELP_ASKNUMBER EQU 58
TX_HELP_ASKDISK EQU 59
TX_PLEASE_INSERT EQU 60
TX_HELP_ASKBOOL EQU 61
TX_SELECTED_DRAWER EQU 62
TX_HELP_SELECTDIR EQU 63
TX_SELECTED_FILE EQU 64
TX_CURRENT_DRAWER EQU 65
TX_HELP_SELECTFILE EQU 66
TX_INST_COMPLETE EQU 67
TX_INST_COMP_WHERE EQU 68
TX_INST_LOGFILE EQU 69
TX_WORKING EQU 70
TX_HELP_CONFIRM EQU 71
TX_VERSION_SOURCE EQU 72
TX_VERSION_DEST EQU 73
TX_VERSION_NONE EQU 74
TX_PROCEED_INST EQU 75
TX_INST_MODE EQU 76
TX_WELCOME EQU 77
TX_HELP_INSTMODE EQU 78
TX_LOG_TO EQU 79
TX_INST_OPTIONS EQU 80
TX_WELCOME_OPT EQU 81
TX_HELP_SETTINGS EQU 82
TX_MODIFY_SS EQU 83
TX_ANOTHER_FILE EQU 84
TX_PROCEED_CHANGES EQU 85
TX_FILE_MODIFIED EQU 86
TX_HELP_CHANGE_SS EQU 87
TX_ERROR EQU 88
TX_DOSERROR EQU 89
TX_SORRY EQU 90
TX_CONFIRM EQU 91
TX_ESCAPE EQU 92
TX_RANGE EQU 93
TX_PROCEED_COPY EQU 96
MSG_BOOTDISK EQU 131
MSG_ASKDELETE EQU 168
TX_SKIP_FILE EQU 169
TX_DELETE EQU 170
HELP_USER_LEVEL EQU 138
HELP_INST_SETTINGS EQU 139
HELP_CHANGE_STARTUP EQU 140
STR_FILE_TO_MODIFY EQU 141
HELP_SELECT_FILE EQU 142
STR_NEW_DEST EQU 143
HELP_SELECT_DRAWER EQU 144
HELP_VERSION EQU 145
HELP_COPYFILES EQU 146
HELP_ASKDISK EQU 147
HELP_ASKSTRING EQU 148
HELP_ASKNUMBER EQU 149
HELP_ASKCHOICE EQU 150
HELP_ASKOPTIONS EQU 151
HELP_MAKEDIR EQU 152
STR_CREATE_ICON EQU 153
STR_NOVICE_USER EQU 154
STR_AVERAGE_USER EQU 155
STR_EXPERT_USER EQU 156
STR_FOR_REAL EQU 157
STR_PRETEND EQU 158
STR_PRINTER EQU 159
STR_LOGFILE EQU 160
STR_NOLOG EQU 161
STR_SURE_ABORT EQU 162
HELP_STARTUP EQU 163
MSG_NO_PRINTER EQU 26
MSG_ACCESS_TRANSCRIPT EQU 27
MSG_ACCESS_PRINTER EQU 28
MSG_PROB_SOURCE EQU 52
MSG_PROB_FILE EQU 53
MSG_NOEXAMINE_OBJECT EQU 97
MSG_NOEXIST_OBJECT EQU 98
MSG_CANNOT_ASSIGN EQU 99
MSG_ERR_MEMORY EQU 102
MSG_NO_DRAWER EQU 117
MSG_NO_FILE EQU 118
MSG_PROB_SS EQU 119
MSG_ERR_WRITING EQU 120
MSG_ERR_UNKNOWN EQU 121
MSG_ERR_READING EQU 122
MSG_CANNOT_CREATE EQU 123
MSG_CANNOT_COPY EQU 124
MSG_OVERWRITE EQU 125
MSG_EXECUTE_PATH EQU 126
MSG_NO_COMMAND EQU 128
MSG_NO_REXX EQU 129
MSG_NO_DUPLOCK EQU 130
ERR_READING_US EQU 136
ERR_WRITING_US EQU 137
MSG_BAD_PARAMS_COPY EQU 164
MSG_INVALID_DRAWER_NAME EQU 167
MSG_NO_WINDOW EQU 22
MSG_NO_SCRIPT_ICON EQU 23
MSG_NO_PARTITION EQU 24
MSG_NO_TRANSCRIPT EQU 25
MSG_NO_SCRIPT EQU 30
MSG_NO_INIT EQU 31
MSG_NO_LIBRARIES EQU 32
MSG_NO_WORKBENCH EQU 33
MSG_TOO_LONG EQU 95
MSG_DIVIDE_ZERO EQU 100
MSG_BAD_CAT EQU 101
TRANS_HEADER EQU 0
TRANS_USER_DRAWER EQU 34
TRANS_COPY_FILE EQU 35
TRANS_COPY_DRAWER EQU 36
TRANS_ASK_CHOICE EQU 37
TRANS_NO_OPTIONS EQU 38
TRANS_ASK_OPTIONS EQU 39
TRANS_ASK_STRING EQU 40
TRANS_ASK_NUMBER EQU 41
TRANS_ASK_BOOL EQU 42
TRANS_DEST_DRAWER EQU 43
TRANS_ASK_DRAWER EQU 44
TRANS_ASK_FILE EQU 45
MSG_ONERROR EQU 94
MSG_USER_ABORT EQU 103
MSG_ABORT_LINE EQU 104
MSG_PROBLEM_LINE EQU 105
TRANS_CREATE_DRAWER EQU 106
TRANS_INSERT_COMMANDS EQU 107
TRANS_CREATE_TEXTFILE EQU 108
TRANS_END_TEXTFILE EQU 109
TRANS_EXECUTE EQU 110
TRANS_RUN EQU 111
TRANS_AREXX EQU 112
TRANS_RENAME EQU 113
TRANS_DELETE EQU 114
TRANS_ABORTING EQU 115
TRANS_EXITING EQU 116
TRANS_START_US EQU 134
TRANS_END_US EQU 135
TX_ASSIGN_LABEL EQU 132
TX_DRAWER_LABEL EQU 133
TRANS_DEF_NAME EQU 1
MSG_ABOUT_INSTALLER1 EQU 165
MSG_ABOUT_INSTALLER2 EQU 166
TX_USAGE EQU 2
MSG_BAD_ARGS EQU 20
MSG_NO_FONT EQU 21
MSG_NO_COMPILE EQU 29
MSG_NO_NIL EQU 127

	ENDC ; CATCOMP_NUMBERS


;-----------------------------------------------------------------------------


	IFD CATCOMP_STRINGS

TX_YES_STR: DC.B 'Yes',$00
TX_NO_STR: DC.B 'No',$00
TX_ABORT_STR: DC.B 'Abort Install',$00
TX_HELP_STR: DC.B 'Help...',$00
TX_PROCEED_STR: DC.B 'Proceed',$00
TX_CANCEL_STR: DC.B 'Cancel',$00
TX_PARENT_STR: DC.B 'Parent Drawer',$00
TX_DRIVES_STR: DC.B 'Show Drives',$00
TX_MAKEDIR_STR: DC.B 'Make New Drawer...',$00
TX_SKIP_STR: DC.B 'Skip This Part',$00
TX_EXITHELP_STR: DC.B 'Exit from Help',$00
TX_MORE_STR: DC.B 'More',$00
TX_BACK_STR: DC.B 'Back',$00
TX_EXITABOUT_STR: DC.B 'Exit from About',$00
TX_ABOUT_STR: DC.B 'About...',$00
TX_COMPLETE_STR: DC.B 'done',$00
TX_OK_STR: DC.B 'OK',$00
TX_COPYING_FILE_STR: DC.B 'Copying file:',$00
TX_TO_DRAWER_STR: DC.B 'To drawer:',$00
TX_MAKEDIR_INFO_STR: DC.B '',10,'Enter the complete pathname of the new drawer you wish to make. An ic'
	DC.B 'on for the new drawer will also be created unless you select otherwise'
	DC.B '.',$00
TX_ENTER_PATH_STR: DC.B 'Enter the Complete Path of New Dirctory:',$00
TX_CHANGE_DEST_STR: DC.B 'Change Destination',$00
TX_COPY_DEST_STR: DC.B 'Destination Drawer to Copy to:',$00
TX_HELP_COPY_STR: DC.B 'Help with Copy Files',$00
TX_HELP_ASKCHOICE_STR: DC.B 'Help with Choice Selection',$00
TX_HELP_ASKOPTIONS_STR: DC.B 'Help with Option Selection',$00
TX_HELP_ASKSTRING_STR: DC.B 'Help with Ask String',$00
TX_HELP_ASKNUMBER_STR: DC.B 'Help with Ask Number',$00
TX_HELP_ASKDISK_STR: DC.B 'Help with Insert Disk',$00
TX_PLEASE_INSERT_STR: DC.B 'Please insert correct disk to continue!',$00
TX_HELP_ASKBOOL_STR: DC.B 'Help with Ask Question',$00
TX_SELECTED_DRAWER_STR: DC.B 'Selected Drawer',$00
TX_HELP_SELECTDIR_STR: DC.B 'Help with Select Drawer',$00
TX_SELECTED_FILE_STR: DC.B 'Selected File',$00
TX_CURRENT_DRAWER_STR: DC.B 'Current Drawer',$00
TX_HELP_SELECTFILE_STR: DC.B 'Help with Select File',$00
TX_INST_COMPLETE_STR: DC.B 'Installation of %s has been completed.',$00
TX_INST_COMP_WHERE_STR: DC.B 'Installation complete!',10,'%s can be found in your "%s" drawer (or partiti'
	DC.B 'on).',$00
TX_INST_LOGFILE_STR: DC.B '(The installation log file is named "%s".)',$00
TX_WORKING_STR: DC.B 'Working on installation...',$00
TX_HELP_CONFIRM_STR: DC.B 'Help with Confirm',$00
TX_VERSION_SOURCE_STR: DC.B 'Version to install: %ld.%ld',$00
TX_VERSION_DEST_STR: DC.B 'Version currently installed: %ld.%ld',$00
TX_VERSION_NONE_STR: DC.B 'There is no currently installed version.',$00
TX_PROCEED_INST_STR: DC.B 'Proceed With Install',$00
TX_INST_MODE_STR: DC.B 'Set Installation Mode',$00
TX_WELCOME_STR: DC.B 'Welcome to the %s installation utility. Please indicate how the instal'
	DC.B 'lation should proceed (based upon your knowledge of the Amiga computer'
	DC.B ').',$00
TX_HELP_INSTMODE_STR: DC.B 'Help with Set Installation Mode',$00
TX_LOG_TO_STR: DC.B 'Log all actions to:',$00
TX_INST_OPTIONS_STR: DC.B 'Installation Options',$00
TX_WELCOME_OPT_STR: DC.B 'Welcome to the %s installation utility.',$00
TX_HELP_SETTINGS_STR: DC.B 'Help with Additional Settings',$00
TX_MODIFY_SS_STR: DC.B 'Modify Startup-Sequence File',$00
TX_ANOTHER_FILE_STR: DC.B 'Select Another File',$00
TX_PROCEED_CHANGES_STR: DC.B 'Proceed with Changes',$00
TX_FILE_MODIFIED_STR: DC.B 'File being modified:',$00
TX_HELP_CHANGE_SS_STR: DC.B 'Help with Change Startup-Sequence',$00
TX_ERROR_STR: DC.B 'AN ERROR HAS OCCURED!',$00
TX_DOSERROR_STR: DC.B '',10,'DOS Error Type: ',$00
TX_SORRY_STR: DC.B 'Sorry... an Error Has Occured!',$00
TX_CONFIRM_STR: DC.B 'Please confirm...',$00
TX_ESCAPE_STR: DC.B ' - Press ',39,'Esc',39,' to Abort',$00
TX_RANGE_STR: DC.B 'Valid range is %ld to %ld',$00
TX_PROCEED_COPY_STR: DC.B 'Proceed with Copy',$00
MSG_BOOTDISK_STR: DC.B 'Please insert your boot disk into the drive you booted from and then c'
	DC.B 'lick on "Proceed". If the disk is already in the drive, just click on '
	DC.B '"Proceed".',$00
MSG_ASKDELETE_STR: DC.B '',10,'The file "%s" is protected from deletion.',10,10,'Can the file be deleted an'
	DC.B 'd/or overwritten?',$00
TX_SKIP_FILE_STR: DC.B 'Skip File',$00
TX_DELETE_STR: DC.B 'Delete',$00
HELP_USER_LEVEL_STR: DC.B 5,119,'    You will need to select a "User Level" for the installer to contin'
	DC.B 'ue. The different levels are as follows:',10,'    NOVICE USER: In this mode'
	DC.B ' the installer proceeds entirely automatically, and will not ask you t'
	DC.B 'o make any decisions. However, this mode is only recommended for Amiga'
	DC.B ' computers that are not extensively customized, as some of the assumpt'
	DC.B 'ions made by the program are only valid on a "stock" Amiga. You may st'
	DC.B 'ill be requested to insert disks or perform other actions.',10,'    INTERME'
	DC.B 'DIATE USER: The installer will allow you to make major choices, such a'
	DC.B 's where to install the application, but will not bother you with the m'
	DC.B 'inor details of copying files, creating drawers, etc.',10,'    EXPERT USER:'
	DC.B ' In this mode you will be asked for confirmation of each step of the i'
	DC.B 'nstallation process (although in some cases several steps may be combi'
	DC.B 'ned into a single confirmation). In addition, you will have greater co'
	DC.B 'ntrol over where installed items are placed.',10,'    In addition, there ar'
	DC.B 'e several buttons at the bottom:',10,'    "Proceed with Install" indicates '
	DC.B 'that you have chosen a User Level and that installation should proceed'
	DC.B '.',10,'    "About..." gives information about the installation program, suc'
	DC.B 'h as copyright information, version number, author, etc.',10,'    "Abort In'
	DC.B 'stall" means that you have changed your mind and do not wish to instal'
	DC.B 'l the application at this time.',10,'    "Help..." brings up this text.',$00
HELP_INST_SETTINGS_STR: DC.B 3,156,'    Explanation of controls:',10,10,'    "Install for Real" -- This will caus'
	DC.B 'e the installer to actually carry out the installation.',10,'    "Pretend t'
	DC.B 'o Install" -- In this mode, the installer will go through all the step'
	DC.B 's of the installation, except that it will not make any permanent chan'
	DC.B 'ges. You can use this option to get a "preview" of what the installer '
	DC.B 'will do before it actually does it. You can also use this in combinati'
	DC.B 'on with the "log file" options below to get a complete list of what ha'
	DC.B 'ppened during the rehearsal.',10,10,'    "Printer" -- This will cause a list '
	DC.B 'of all installation actions to be printed out on the printer.',10,'    "Log'
	DC.B ' File" -- This will cause a list of all installation actions to be wri'
	DC.B 'tten to a log file. You will be informed of the location of this file '
	DC.B 'when the installation finishes. You can use a text editor to read this'
	DC.B ' file.',10,'    "None" -- When this option is turned on, no log file will b'
	DC.B 'e produced.',$00
HELP_CHANGE_STARTUP_STR: DC.B 6,186,'    In order for this product to run correctly, a number of special co'
	DC.B 'mmands need to be executed by the "startup-sequence", a file of comman'
	DC.B 'ds which is executed each time your Amiga computer boots up. A line ne'
	DC.B 'eds to be inserted into your startup-sequence to run these commands. T'
	DC.B 'he command to be inserted is "Execute S:user-startup". This command wi'
	DC.B 'll execute a script file called "S:user-startup" (which will be create'
	DC.B 'd if it does not already exist). This file will contain all the specia'
	DC.B 'l commands needed by this product. In addition, any other product that'
	DC.B ' uses the Commodore-Amiga Installation Utility will also store its sta'
	DC.B 'rtup commands in the "S:user-startup:" file in the future.',10,10,'Explanatio'
	DC.B 'n of controls:',10,'    The scrolling list box shows your startup sequence.'
	DC.B ' This installer has already selected what it believes is the best plac'
	DC.B 'e to insert the new command. If for some reason you feel that the inst'
	DC.B 'aller has chosen incorrectly (which is certainly possible with a highl'
	DC.B 'y customized system), then you may select a new file and installation '
	DC.B 'point.',10,'    The text box shows the name of the script file to be modifi'
	DC.B 'ed.',10,'    To select a different file: Select the "Select another file" b'
	DC.B 'utton. The file you select should be one that is executed once each ti'
	DC.B 'me you boot up and is not executed at any other time.',10,'    To select a '
	DC.B 'new insertion point: Clicking on the scrolling list will cause the ins'
	DC.B 'ertion point (labeled "insert here") to be moved to the new point.',10,'   '
	DC.B ' "Proceed with Changes" indicates that you are satisfied with the poin'
	DC.B 't of insertion, and the command should be inserted into the file.',10,'    '
	DC.B '"Abort Install" indicates that you wish to abort the installation proc'
	DC.B 'ess.',10,'    "Help..." brings up this text.',$00
STR_FILE_TO_MODIFY_STR: DC.B 0,24,'Select File To Modify',$00
HELP_SELECT_FILE_STR: DC.B 5,43,'Explanation of controls:',10,'    The large "scrolling list box" in the cen'
	DC.B 'ter displays the contents of the currently selected disk or directory.'
	DC.B ' Drawers will be indicated by the word "DRW" in inverse block letters '
	DC.B 'in front of the Drawer name. The name of the disk or directory being v'
	DC.B 'iewed is shown below it in a raised outlined box. Beneath that, in ano'
	DC.B 'ther raised outlined box, is the name of the file which is currently s'
	DC.B 'elected. To the right is a slider which can be used to scroll through '
	DC.B 'the list, in case there are more names than will fit into the window.',10
	DC.B '    You can change the selection either by typing a new drawer name in'
	DC.B 'to the "Current Drawer" text box, or by typing a new file name into th'
	DC.B 'e "Selected File" text box, or by clicking on the names of drawers and'
	DC.B ' files in the scrolling list.',10,'    The button "Parent Drawer" will allo'
	DC.B 'w you to view the drawer that contains this one, in other words the "P'
	DC.B 'arent" of the currently visible drawer.',10,'    The button "Show Drives" w'
	DC.B 'ill show a list of all the disk drives in your system. You can then cl'
	DC.B 'ick on a drive name to view the contents of that drive.',10,'    The button'
	DC.B ' "Proceed" indicates you are satisfied with the currently selected fil'
	DC.B 'e.',10,'    The button "Abort Install" will abort the installation. No furt'
	DC.B 'her changes will be made.',10,'    "Help..." brings up this text.',$00
STR_NEW_DEST_STR: DC.B 0,32,'Select New Destination Drawer',$00
HELP_SELECT_DRAWER_STR: DC.B 5,27,'Explanation of controls:',10,'    The large "scrolling list box" in the cen'
	DC.B 'ter displays the contents of the currently selected disk or directory.'
	DC.B ' Drawers will be indicated by the word "DRW" in inverse block letters '
	DC.B 'in front of the Drawer name. The name of the disk or directory being v'
	DC.B 'iewed is shown below it in a raised outlined box. To the right is a sl'
	DC.B 'ider which can be used to scroll through the list, in case there are m'
	DC.B 'ore names than will fit into the window.',10,'    You can change the select'
	DC.B 'ion by typing a new drawer name into the "Current Drawer" text box, or'
	DC.B ' by clicking on the names of drawers in the scrolling list.',10,'    The bu'
	DC.B 'tton "Parent Drawer" will allow you to view the drawer that contains t'
	DC.B 'his one, in other words the "Parent" of the currently visible drawer.',10
	DC.B '    The button "Show Drives" will show a list of all the disk drives i'
	DC.B 'n your system. You can then click on a drive name to view the contents'
	DC.B ' of that drive.',10,'    The button "Make New Drawer" will allow you to cre'
	DC.B 'ate a new drawer. The newly created drawer will appear inside the dire'
	DC.B 'ctory that is currently being viewed.',10,'    The button "Proceed" indicat'
	DC.B 'es you are satisfied with the currently selected drawer.',10,'    The butto'
	DC.B 'n "Abort Install" will abort the installation. No further changes will'
	DC.B ' be made.',10,'    "Help..." brings up this text.',$00
HELP_VERSION_STR: DC.B 2,107,'Explanation of controls:',10,'    The file to be installed may or may not b'
	DC.B 'e replacing an older version. The display shows a comparison between t'
	DC.B 'he currently installed version and the new version that will replace i'
	DC.B 't. The destination drawer that the file will be copied to is also show'
	DC.B 'n.',10,'    The button "Proceed with Copy" will cause the file to be copied'
	DC.B ' to the destination drawer.',10,'    The button "Skip this part" causes the'
	DC.B ' installer to proceed to the next section. The file will not be copied'
	DC.B '.',10,'    The button "Abort Install" will cancel the installation with no '
	DC.B 'further changes made.',10,'    "Help..." brings up this text.',$00
HELP_COPYFILES_STR: DC.B 3,113,'Explanation of controls:',10,'    The raised box shows a list of file or dr'
	DC.B 'awer names. Each name has a checkmark next to it. If you want the file'
	DC.B ' to be copied, then leave the name checked. An unchecked name will not'
	DC.B ' be copied.',10,'    The box labeled "Destination Drawer to copy to:", just'
	DC.B ' beneath the listing of names shows the drawer to which the items will'
	DC.B ' be copied.',10,'    The button "Proceed with Copy" will cause all the file'
	DC.B 's or drawers which are checked to be copied to the destination drawer.'
	DC.B '',10,'    The button "Skip this part" causes the installer to proceed to th'
	DC.B 'e next section. None of the items listed will be copied.',10,'    The butto'
	DC.B 'n "Change Destination" will allow you to select a new destination. You'
	DC.B ' can use this to copy the items somewhere else.',10,'    The button "Abort '
	DC.B 'Install" will cancel the installation with no further changes being ma'
	DC.B 'de.',10,'    "Help..." brings up this text.',$00
HELP_ASKDISK_STR: DC.B 0,178,'    Once you have inserted the requested disk, you should select "Proc'
	DC.B 'eed". If you cannot find the disk, or wish to abort the installation f'
	DC.B 'or any reason, then select "Abort".',$00
HELP_ASKSTRING_STR: DC.B 0,174,'    Once you have filled in the text box, you may select "Proceed" to '
	DC.B 'continue with the installation. If you wish to abort the installation '
	DC.B 'for any reason, select "Abort".',$00
HELP_ASKNUMBER_STR: DC.B 0,176,'    Once you have filled in the number box, you may select "Proceed" t'
	DC.B 'o continue with the installation. If you wish to abort the installatio'
	DC.B 'n for any reason, select "Abort".',$00
HELP_ASKCHOICE_STR: DC.B 1,149,'Explanation of controls:',10,'    Each of the raised buttons represents a c'
	DC.B 'hoice which you may make. The button that is selected represents the c'
	DC.B 'urrently choice. You can change the choice by selecting one of the cho'
	DC.B 'ice buttons. Only one selection may be chosen. Once you are satisfied,'
	DC.B ' select "Proceed" to continue with the installation. If you wish to ab'
	DC.B 'ort the installation for any reason, select "Abort".',$00
HELP_ASKOPTIONS_STR: DC.B 1,125,'Explanation of controls:',10,'    Each of the checkboxes represents a optio'
	DC.B 'n which you may make. A checkmark in a box represents a currently sele'
	DC.B 'cted option. You can change options by clicking on the checkboxes. Onc'
	DC.B 'e you are satisfied with the selected options, select "Proceed" to con'
	DC.B 'tinue with the installation. If you wish to abort the installation for'
	DC.B ' any reason, select "Abort".',$00
HELP_MAKEDIR_STR: DC.B 0,204,'    Select "Proceed" if you want the new drawer to be created. Select '
	DC.B '"Skip This Part" if you do not want the drawer to be created. If you w'
	DC.B 'ish to abort the installation for any reason, select "Abort".',$00
STR_CREATE_ICON_STR: DC.B 0,25,'Create icon for drawer',$00
STR_NOVICE_USER_STR: DC.B 0,42,'Novice User  - All Actions Automatic   ',$00
STR_AVERAGE_USER_STR: DC.B 0,47,'Intermediate User - Limited Manual Control  ',$00
STR_EXPERT_USER_STR: DC.B 0,42,'Expert User  - Must Confirm all actions',$00
STR_FOR_REAL_STR: DC.B 0,19,'Install for Real',$00
STR_PRETEND_STR: DC.B 0,21,'Pretend to Install',$00
STR_PRINTER_STR: DC.B 0,10,'Printer',$00
STR_LOGFILE_STR: DC.B 0,11,'Log File',$00
STR_NOLOG_STR: DC.B 0,7,'None',$00
STR_SURE_ABORT_STR: DC.B 0,34,'Are you sure you want to abort?',$00
HELP_STARTUP_STR: DC.B 1,94,'The "user-startup" file located in the "S:" drawer is used to store sp'
	DC.B 'ecific commands that some applications need executed at system boot. Y'
	DC.B 'ou are being asked to confirm that this file can be edited. Select the'
	DC.B ' "Proceed" button to confirm, otherwise select the button labeled "Ski'
	DC.B 'p This Part". You can also abort installation by selecting "Abort".',$00
MSG_NO_PRINTER_STR: DC.B 'Unable to access the printer.',$00
MSG_ACCESS_TRANSCRIPT_STR: DC.B 'Problem with writing to transcript file.',$00
MSG_ACCESS_PRINTER_STR: DC.B 'Unable to access the printer.',$00
MSG_PROB_SOURCE_STR: DC.B 'Problem with source file/drawer "%s"',$00
MSG_PROB_FILE_STR: DC.B 'Problem with file "%s"',$00
MSG_NOEXAMINE_OBJECT_STR: DC.B 'Can',39,'t examine file or drawer "%s"',$00
MSG_NOEXIST_OBJECT_STR: DC.B 'File or drawer "%s" doesn',39,'t exist',$00
MSG_CANNOT_ASSIGN_STR: DC.B 'Can',39,'t make assign',$00
MSG_ERR_MEMORY_STR: DC.B 'Unable to perform operation -- not enough memory',$00
MSG_NO_DRAWER_STR: DC.B 'Can',39,'t open drawer "%s"',$00
MSG_NO_FILE_STR: DC.B 'Can',39,'t open file "%s"',$00
MSG_PROB_SS_STR: DC.B 'Problem with file "startup-sequence"',$00
MSG_ERR_WRITING_STR: DC.B 'Error writing file "%s"',$00
MSG_ERR_UNKNOWN_STR: DC.B 'Error was of an unknown type.',$00
MSG_ERR_READING_STR: DC.B 'Error reading text include file!',$00
MSG_CANNOT_CREATE_STR: DC.B 'Couldn',39,'t create directory "%s"',$00
MSG_CANNOT_COPY_STR: DC.B 'Couldn',39,'t copy file "%s"',$00
MSG_OVERWRITE_STR: DC.B 'Tried to create drawer over file "%s"',$00
MSG_EXECUTE_PATH_STR: DC.B 'Can',39,'t set execute path',$00
MSG_NO_COMMAND_STR: DC.B 'Couldn',39,'t find command to execute',$00
MSG_NO_REXX_STR: DC.B 'ARexx not available',$00
MSG_NO_DUPLOCK_STR: DC.B 'Can',39,'t duplicate lock',$00
ERR_READING_US_STR: DC.B 'Can',39,'t read "%s" file',$00
ERR_WRITING_US_STR: DC.B 'Error in writing "%s" file',$00
MSG_BAD_PARAMS_COPY_STR: DC.B 'Bad parameters for copying from drawer',$00
MSG_INVALID_DRAWER_NAME_STR: DC.B 'Invalid drawer name',$00
MSG_NO_WINDOW_STR: DC.B 'Can',39,'t open a window.',$00
MSG_NO_SCRIPT_ICON_STR: DC.B 'Can',39,'t open install script',39,'s icon.',$00
MSG_NO_PARTITION_STR: DC.B 'Can',39,'t find partition.',$00
MSG_NO_TRANSCRIPT_STR: DC.B 'Unable to open transcript file.',$00
MSG_NO_SCRIPT_STR: DC.B 'Unable to open script.',$00
MSG_NO_INIT_STR: DC.B 'Unable to initialize system.',$00
MSG_NO_LIBRARIES_STR: DC.B 'Can',39,'t open needed libraries.',$00
MSG_NO_WORKBENCH_STR: DC.B 'Can',39,'t get Workbench information.',$00
MSG_TOO_LONG_STR: DC.B 'String too long',$00
MSG_DIVIDE_ZERO_STR: DC.B 'Division by zero',$00
MSG_BAD_CAT_STR: DC.B '%s: Result string too long',$00
TRANS_HEADER_STR: DC.B '******* Installation Log *******',10,'User Level: %s',10,'Pretend: %s',10,10,$00
TRANS_USER_DRAWER_STR: DC.B 'User made a new drawer: "%s".',10,$00
TRANS_COPY_FILE_STR: DC.B 'Copy file "%s" to "%s".',10,$00
TRANS_COPY_DRAWER_STR: DC.B 'Copy contents of drawer "%s" to "%s".',10,$00
TRANS_ASK_CHOICE_STR: DC.B 'Ask Choice: User selected "%s".',10,$00
TRANS_NO_OPTIONS_STR: DC.B 'Ask Options: User selected no options.',10,$00
TRANS_ASK_OPTIONS_STR: DC.B 'Ask Options: User selected following options...',10,$00
TRANS_ASK_STRING_STR: DC.B 'Ask String: Result was "%s".',10,$00
TRANS_ASK_NUMBER_STR: DC.B 'Ask Number: Result was "%ld".',10,$00
TRANS_ASK_BOOL_STR: DC.B 'Ask Question: Result was "%s".',10,$00
TRANS_DEST_DRAWER_STR: DC.B 'User changed Copy Files destination drawer to "%s".',10,$00
TRANS_ASK_DRAWER_STR: DC.B 'Ask Drawer: Result was "%s".',10,$00
TRANS_ASK_FILE_STR: DC.B 'Ask File: Result was "%s".',10,$00
MSG_ONERROR_STR: DC.B '*** Global error -- executing onerror statements ***',10,$00
MSG_USER_ABORT_STR: DC.B 'User aborted!',10,10,$00
MSG_ABORT_LINE_STR: DC.B 'Script aborted in line %ld.',10,$00
MSG_PROBLEM_LINE_STR: DC.B '%s in line %ld.',10,$00
TRANS_CREATE_DRAWER_STR: DC.B 'Create New Directory: "%s"',10,$00
TRANS_INSERT_COMMANDS_STR: DC.B 'Insert commands to Execute "%s" in file "%s" at byte %ld.',10,$00
TRANS_CREATE_TEXTFILE_STR: DC.B 'Create Text File: "%s"',10,'================== Start of textfile =========='
	DC.B '========',10,$00
TRANS_END_TEXTFILE_STR: DC.B '=================== End of textfile ===================',10,$00
TRANS_EXECUTE_STR: DC.B 'Execute DOS Script: "%s"',10,$00
TRANS_RUN_STR: DC.B 'Run Program: "%s"',10,$00
TRANS_AREXX_STR: DC.B 'Execute ARexx Script: "%s"',10,$00
TRANS_RENAME_STR: DC.B 'Renamed "%s" to "%s"',10,$00
TRANS_DELETE_STR: DC.B 'Delete file "%s"',10,$00
TRANS_ABORTING_STR: DC.B 'Aborting script.',10,$00
TRANS_EXITING_STR: DC.B 'Exiting script...',10,$00
TRANS_START_US_STR: DC.B 'Writing to %s',10,'==================== start of file ====================',10,$00
TRANS_END_US_STR: DC.B '===================== end of file =====================',10,$00
TX_ASSIGN_LABEL_STR: DC.B '<ASN>',$00
TX_DRAWER_LABEL_STR: DC.B '<DRW>',$00
TRANS_DEF_NAME_STR: DC.B 'install_log_file',$00
MSG_ABOUT_INSTALLER1_STR: DC.B 'Amiga Application Installation Utility',$00
MSG_ABOUT_INSTALLER2_STR: DC.B '',10,'Copyright ©1990-1993 Commodore-Amiga, Inc.',10,'All Rights Reserved.',10,10,'Desi'
	DC.B 'gned and Developed by',10,'Sylvan Technical Arts',$00
TX_USAGE_STR: DC.B 'USAGE: installer [SCRIPT] filename <[APPNAME] name> <[MINUSER] level>',10
	DC.B '       <[DEFUSER] default> <[LOGFILE] logname> <NOPRETEND> <NOLOG>',$00
MSG_BAD_ARGS_STR: DC.B 'Bad Argument',$00
MSG_NO_FONT_STR: DC.B 'Can',39,'t find needed ROM font.',$00
MSG_NO_COMPILE_STR: DC.B 'Unable to compile script.',10,'ERROR: %s on line %ld.',$00
MSG_NO_NIL_STR: DC.B 'Couldn',39,'t access NIL:',$00

	ENDC ; CATCOMP_STRINGS


;-----------------------------------------------------------------------------


	IFD CATCOMP_ARRAY

   STRUCTURE CatCompArrayType,0
	LONG cca_ID
	APTR cca_Str
   LABEL CatCompArrayType_SIZEOF

	CNOP 0,4

CatCompArray:
AS0:	DC.L TX_YES,TX_YES_STR
AS1:	DC.L TX_NO,TX_NO_STR
AS2:	DC.L TX_ABORT,TX_ABORT_STR
AS3:	DC.L TX_HELP,TX_HELP_STR
AS4:	DC.L TX_PROCEED,TX_PROCEED_STR
AS5:	DC.L TX_CANCEL,TX_CANCEL_STR
AS6:	DC.L TX_PARENT,TX_PARENT_STR
AS7:	DC.L TX_DRIVES,TX_DRIVES_STR
AS8:	DC.L TX_MAKEDIR,TX_MAKEDIR_STR
AS9:	DC.L TX_SKIP,TX_SKIP_STR
AS10:	DC.L TX_EXITHELP,TX_EXITHELP_STR
AS11:	DC.L TX_MORE,TX_MORE_STR
AS12:	DC.L TX_BACK,TX_BACK_STR
AS13:	DC.L TX_EXITABOUT,TX_EXITABOUT_STR
AS14:	DC.L TX_ABOUT,TX_ABOUT_STR
AS15:	DC.L TX_COMPLETE,TX_COMPLETE_STR
AS16:	DC.L TX_OK,TX_OK_STR
AS17:	DC.L TX_COPYING_FILE,TX_COPYING_FILE_STR
AS18:	DC.L TX_TO_DRAWER,TX_TO_DRAWER_STR
AS19:	DC.L TX_MAKEDIR_INFO,TX_MAKEDIR_INFO_STR
AS20:	DC.L TX_ENTER_PATH,TX_ENTER_PATH_STR
AS21:	DC.L TX_CHANGE_DEST,TX_CHANGE_DEST_STR
AS22:	DC.L TX_COPY_DEST,TX_COPY_DEST_STR
AS23:	DC.L TX_HELP_COPY,TX_HELP_COPY_STR
AS24:	DC.L TX_HELP_ASKCHOICE,TX_HELP_ASKCHOICE_STR
AS25:	DC.L TX_HELP_ASKOPTIONS,TX_HELP_ASKOPTIONS_STR
AS26:	DC.L TX_HELP_ASKSTRING,TX_HELP_ASKSTRING_STR
AS27:	DC.L TX_HELP_ASKNUMBER,TX_HELP_ASKNUMBER_STR
AS28:	DC.L TX_HELP_ASKDISK,TX_HELP_ASKDISK_STR
AS29:	DC.L TX_PLEASE_INSERT,TX_PLEASE_INSERT_STR
AS30:	DC.L TX_HELP_ASKBOOL,TX_HELP_ASKBOOL_STR
AS31:	DC.L TX_SELECTED_DRAWER,TX_SELECTED_DRAWER_STR
AS32:	DC.L TX_HELP_SELECTDIR,TX_HELP_SELECTDIR_STR
AS33:	DC.L TX_SELECTED_FILE,TX_SELECTED_FILE_STR
AS34:	DC.L TX_CURRENT_DRAWER,TX_CURRENT_DRAWER_STR
AS35:	DC.L TX_HELP_SELECTFILE,TX_HELP_SELECTFILE_STR
AS36:	DC.L TX_INST_COMPLETE,TX_INST_COMPLETE_STR
AS37:	DC.L TX_INST_COMP_WHERE,TX_INST_COMP_WHERE_STR
AS38:	DC.L TX_INST_LOGFILE,TX_INST_LOGFILE_STR
AS39:	DC.L TX_WORKING,TX_WORKING_STR
AS40:	DC.L TX_HELP_CONFIRM,TX_HELP_CONFIRM_STR
AS41:	DC.L TX_VERSION_SOURCE,TX_VERSION_SOURCE_STR
AS42:	DC.L TX_VERSION_DEST,TX_VERSION_DEST_STR
AS43:	DC.L TX_VERSION_NONE,TX_VERSION_NONE_STR
AS44:	DC.L TX_PROCEED_INST,TX_PROCEED_INST_STR
AS45:	DC.L TX_INST_MODE,TX_INST_MODE_STR
AS46:	DC.L TX_WELCOME,TX_WELCOME_STR
AS47:	DC.L TX_HELP_INSTMODE,TX_HELP_INSTMODE_STR
AS48:	DC.L TX_LOG_TO,TX_LOG_TO_STR
AS49:	DC.L TX_INST_OPTIONS,TX_INST_OPTIONS_STR
AS50:	DC.L TX_WELCOME_OPT,TX_WELCOME_OPT_STR
AS51:	DC.L TX_HELP_SETTINGS,TX_HELP_SETTINGS_STR
AS52:	DC.L TX_MODIFY_SS,TX_MODIFY_SS_STR
AS53:	DC.L TX_ANOTHER_FILE,TX_ANOTHER_FILE_STR
AS54:	DC.L TX_PROCEED_CHANGES,TX_PROCEED_CHANGES_STR
AS55:	DC.L TX_FILE_MODIFIED,TX_FILE_MODIFIED_STR
AS56:	DC.L TX_HELP_CHANGE_SS,TX_HELP_CHANGE_SS_STR
AS57:	DC.L TX_ERROR,TX_ERROR_STR
AS58:	DC.L TX_DOSERROR,TX_DOSERROR_STR
AS59:	DC.L TX_SORRY,TX_SORRY_STR
AS60:	DC.L TX_CONFIRM,TX_CONFIRM_STR
AS61:	DC.L TX_ESCAPE,TX_ESCAPE_STR
AS62:	DC.L TX_RANGE,TX_RANGE_STR
AS63:	DC.L TX_PROCEED_COPY,TX_PROCEED_COPY_STR
AS64:	DC.L MSG_BOOTDISK,MSG_BOOTDISK_STR
AS65:	DC.L MSG_ASKDELETE,MSG_ASKDELETE_STR
AS66:	DC.L TX_SKIP_FILE,TX_SKIP_FILE_STR
AS67:	DC.L TX_DELETE,TX_DELETE_STR
AS68:	DC.L HELP_USER_LEVEL,HELP_USER_LEVEL_STR
AS69:	DC.L HELP_INST_SETTINGS,HELP_INST_SETTINGS_STR
AS70:	DC.L HELP_CHANGE_STARTUP,HELP_CHANGE_STARTUP_STR
AS71:	DC.L STR_FILE_TO_MODIFY,STR_FILE_TO_MODIFY_STR
AS72:	DC.L HELP_SELECT_FILE,HELP_SELECT_FILE_STR
AS73:	DC.L STR_NEW_DEST,STR_NEW_DEST_STR
AS74:	DC.L HELP_SELECT_DRAWER,HELP_SELECT_DRAWER_STR
AS75:	DC.L HELP_VERSION,HELP_VERSION_STR
AS76:	DC.L HELP_COPYFILES,HELP_COPYFILES_STR
AS77:	DC.L HELP_ASKDISK,HELP_ASKDISK_STR
AS78:	DC.L HELP_ASKSTRING,HELP_ASKSTRING_STR
AS79:	DC.L HELP_ASKNUMBER,HELP_ASKNUMBER_STR
AS80:	DC.L HELP_ASKCHOICE,HELP_ASKCHOICE_STR
AS81:	DC.L HELP_ASKOPTIONS,HELP_ASKOPTIONS_STR
AS82:	DC.L HELP_MAKEDIR,HELP_MAKEDIR_STR
AS83:	DC.L STR_CREATE_ICON,STR_CREATE_ICON_STR
AS84:	DC.L STR_NOVICE_USER,STR_NOVICE_USER_STR
AS85:	DC.L STR_AVERAGE_USER,STR_AVERAGE_USER_STR
AS86:	DC.L STR_EXPERT_USER,STR_EXPERT_USER_STR
AS87:	DC.L STR_FOR_REAL,STR_FOR_REAL_STR
AS88:	DC.L STR_PRETEND,STR_PRETEND_STR
AS89:	DC.L STR_PRINTER,STR_PRINTER_STR
AS90:	DC.L STR_LOGFILE,STR_LOGFILE_STR
AS91:	DC.L STR_NOLOG,STR_NOLOG_STR
AS92:	DC.L STR_SURE_ABORT,STR_SURE_ABORT_STR
AS93:	DC.L HELP_STARTUP,HELP_STARTUP_STR
AS94:	DC.L MSG_NO_PRINTER,MSG_NO_PRINTER_STR
AS95:	DC.L MSG_ACCESS_TRANSCRIPT,MSG_ACCESS_TRANSCRIPT_STR
AS96:	DC.L MSG_ACCESS_PRINTER,MSG_ACCESS_PRINTER_STR
AS97:	DC.L MSG_PROB_SOURCE,MSG_PROB_SOURCE_STR
AS98:	DC.L MSG_PROB_FILE,MSG_PROB_FILE_STR
AS99:	DC.L MSG_NOEXAMINE_OBJECT,MSG_NOEXAMINE_OBJECT_STR
AS100:	DC.L MSG_NOEXIST_OBJECT,MSG_NOEXIST_OBJECT_STR
AS101:	DC.L MSG_CANNOT_ASSIGN,MSG_CANNOT_ASSIGN_STR
AS102:	DC.L MSG_ERR_MEMORY,MSG_ERR_MEMORY_STR
AS103:	DC.L MSG_NO_DRAWER,MSG_NO_DRAWER_STR
AS104:	DC.L MSG_NO_FILE,MSG_NO_FILE_STR
AS105:	DC.L MSG_PROB_SS,MSG_PROB_SS_STR
AS106:	DC.L MSG_ERR_WRITING,MSG_ERR_WRITING_STR
AS107:	DC.L MSG_ERR_UNKNOWN,MSG_ERR_UNKNOWN_STR
AS108:	DC.L MSG_ERR_READING,MSG_ERR_READING_STR
AS109:	DC.L MSG_CANNOT_CREATE,MSG_CANNOT_CREATE_STR
AS110:	DC.L MSG_CANNOT_COPY,MSG_CANNOT_COPY_STR
AS111:	DC.L MSG_OVERWRITE,MSG_OVERWRITE_STR
AS112:	DC.L MSG_EXECUTE_PATH,MSG_EXECUTE_PATH_STR
AS113:	DC.L MSG_NO_COMMAND,MSG_NO_COMMAND_STR
AS114:	DC.L MSG_NO_REXX,MSG_NO_REXX_STR
AS115:	DC.L MSG_NO_DUPLOCK,MSG_NO_DUPLOCK_STR
AS116:	DC.L ERR_READING_US,ERR_READING_US_STR
AS117:	DC.L ERR_WRITING_US,ERR_WRITING_US_STR
AS118:	DC.L MSG_BAD_PARAMS_COPY,MSG_BAD_PARAMS_COPY_STR
AS119:	DC.L MSG_INVALID_DRAWER_NAME,MSG_INVALID_DRAWER_NAME_STR
AS120:	DC.L MSG_NO_WINDOW,MSG_NO_WINDOW_STR
AS121:	DC.L MSG_NO_SCRIPT_ICON,MSG_NO_SCRIPT_ICON_STR
AS122:	DC.L MSG_NO_PARTITION,MSG_NO_PARTITION_STR
AS123:	DC.L MSG_NO_TRANSCRIPT,MSG_NO_TRANSCRIPT_STR
AS124:	DC.L MSG_NO_SCRIPT,MSG_NO_SCRIPT_STR
AS125:	DC.L MSG_NO_INIT,MSG_NO_INIT_STR
AS126:	DC.L MSG_NO_LIBRARIES,MSG_NO_LIBRARIES_STR
AS127:	DC.L MSG_NO_WORKBENCH,MSG_NO_WORKBENCH_STR
AS128:	DC.L MSG_TOO_LONG,MSG_TOO_LONG_STR
AS129:	DC.L MSG_DIVIDE_ZERO,MSG_DIVIDE_ZERO_STR
AS130:	DC.L MSG_BAD_CAT,MSG_BAD_CAT_STR
AS131:	DC.L TRANS_HEADER,TRANS_HEADER_STR
AS132:	DC.L TRANS_USER_DRAWER,TRANS_USER_DRAWER_STR
AS133:	DC.L TRANS_COPY_FILE,TRANS_COPY_FILE_STR
AS134:	DC.L TRANS_COPY_DRAWER,TRANS_COPY_DRAWER_STR
AS135:	DC.L TRANS_ASK_CHOICE,TRANS_ASK_CHOICE_STR
AS136:	DC.L TRANS_NO_OPTIONS,TRANS_NO_OPTIONS_STR
AS137:	DC.L TRANS_ASK_OPTIONS,TRANS_ASK_OPTIONS_STR
AS138:	DC.L TRANS_ASK_STRING,TRANS_ASK_STRING_STR
AS139:	DC.L TRANS_ASK_NUMBER,TRANS_ASK_NUMBER_STR
AS140:	DC.L TRANS_ASK_BOOL,TRANS_ASK_BOOL_STR
AS141:	DC.L TRANS_DEST_DRAWER,TRANS_DEST_DRAWER_STR
AS142:	DC.L TRANS_ASK_DRAWER,TRANS_ASK_DRAWER_STR
AS143:	DC.L TRANS_ASK_FILE,TRANS_ASK_FILE_STR
AS144:	DC.L MSG_ONERROR,MSG_ONERROR_STR
AS145:	DC.L MSG_USER_ABORT,MSG_USER_ABORT_STR
AS146:	DC.L MSG_ABORT_LINE,MSG_ABORT_LINE_STR
AS147:	DC.L MSG_PROBLEM_LINE,MSG_PROBLEM_LINE_STR
AS148:	DC.L TRANS_CREATE_DRAWER,TRANS_CREATE_DRAWER_STR
AS149:	DC.L TRANS_INSERT_COMMANDS,TRANS_INSERT_COMMANDS_STR
AS150:	DC.L TRANS_CREATE_TEXTFILE,TRANS_CREATE_TEXTFILE_STR
AS151:	DC.L TRANS_END_TEXTFILE,TRANS_END_TEXTFILE_STR
AS152:	DC.L TRANS_EXECUTE,TRANS_EXECUTE_STR
AS153:	DC.L TRANS_RUN,TRANS_RUN_STR
AS154:	DC.L TRANS_AREXX,TRANS_AREXX_STR
AS155:	DC.L TRANS_RENAME,TRANS_RENAME_STR
AS156:	DC.L TRANS_DELETE,TRANS_DELETE_STR
AS157:	DC.L TRANS_ABORTING,TRANS_ABORTING_STR
AS158:	DC.L TRANS_EXITING,TRANS_EXITING_STR
AS159:	DC.L TRANS_START_US,TRANS_START_US_STR
AS160:	DC.L TRANS_END_US,TRANS_END_US_STR
AS161:	DC.L TX_ASSIGN_LABEL,TX_ASSIGN_LABEL_STR
AS162:	DC.L TX_DRAWER_LABEL,TX_DRAWER_LABEL_STR
AS163:	DC.L TRANS_DEF_NAME,TRANS_DEF_NAME_STR
AS164:	DC.L MSG_ABOUT_INSTALLER1,MSG_ABOUT_INSTALLER1_STR
AS165:	DC.L MSG_ABOUT_INSTALLER2,MSG_ABOUT_INSTALLER2_STR
AS166:	DC.L TX_USAGE,TX_USAGE_STR
AS167:	DC.L MSG_BAD_ARGS,MSG_BAD_ARGS_STR
AS168:	DC.L MSG_NO_FONT,MSG_NO_FONT_STR
AS169:	DC.L MSG_NO_COMPILE,MSG_NO_COMPILE_STR
AS170:	DC.L MSG_NO_NIL,MSG_NO_NIL_STR

	ENDC ; CATCOMP_ARRAY


;-----------------------------------------------------------------------------


	IFD CATCOMP_BLOCK

CatCompBlock:
	DC.L $3
	DC.W $4
	DC.B 'Yes',$00
	DC.L $4
	DC.W $4
	DC.B 'No',$00,$00
	DC.L $5
	DC.W $E
	DC.B 'Abort Install',$00
	DC.L $6
	DC.W $8
	DC.B 'Help...',$00
	DC.L $7
	DC.W $8
	DC.B 'Proceed',$00
	DC.L $8
	DC.W $8
	DC.B 'Cancel',$00,$00
	DC.L $9
	DC.W $E
	DC.B 'Parent Drawer',$00
	DC.L $A
	DC.W $C
	DC.B 'Show Drives',$00
	DC.L $B
	DC.W $14
	DC.B 'Make New Drawer...',$00,$00
	DC.L $C
	DC.W $10
	DC.B 'Skip This Part',$00,$00
	DC.L $D
	DC.W $10
	DC.B 'Exit from Help',$00,$00
	DC.L $E
	DC.W $6
	DC.B 'More',$00,$00
	DC.L $F
	DC.W $6
	DC.B 'Back',$00,$00
	DC.L $10
	DC.W $10
	DC.B 'Exit from About',$00
	DC.L $11
	DC.W $A
	DC.B 'About...',$00,$00
	DC.L $12
	DC.W $6
	DC.B 'done',$00,$00
	DC.L $13
	DC.W $4
	DC.B 'OK',$00,$00
	DC.L $2E
	DC.W $E
	DC.B 'Copying file:',$00
	DC.L $2F
	DC.W $C
	DC.B 'To drawer:',$00,$00
	DC.L $30
	DC.W $8E
	DC.B '',10,'Enter the complete pathname of the new drawer you wish to make. An ic'
	DC.B 'on for the new drawer will also be created unless you select otherwise'
	DC.B '.',$00
	DC.L $31
	DC.W $2A
	DC.B 'Enter the Complete Path of New Dirctory:',$00,$00
	DC.L $32
	DC.W $14
	DC.B 'Change Destination',$00,$00
	DC.L $33
	DC.W $20
	DC.B 'Destination Drawer to Copy to:',$00,$00
	DC.L $36
	DC.W $16
	DC.B 'Help with Copy Files',$00,$00
	DC.L $37
	DC.W $1C
	DC.B 'Help with Choice Selection',$00,$00
	DC.L $38
	DC.W $1C
	DC.B 'Help with Option Selection',$00,$00
	DC.L $39
	DC.W $16
	DC.B 'Help with Ask String',$00,$00
	DC.L $3A
	DC.W $16
	DC.B 'Help with Ask Number',$00,$00
	DC.L $3B
	DC.W $16
	DC.B 'Help with Insert Disk',$00
	DC.L $3C
	DC.W $28
	DC.B 'Please insert correct disk to continue!',$00
	DC.L $3D
	DC.W $18
	DC.B 'Help with Ask Question',$00,$00
	DC.L $3E
	DC.W $10
	DC.B 'Selected Drawer',$00
	DC.L $3F
	DC.W $18
	DC.B 'Help with Select Drawer',$00
	DC.L $40
	DC.W $E
	DC.B 'Selected File',$00
	DC.L $41
	DC.W $10
	DC.B 'Current Drawer',$00,$00
	DC.L $42
	DC.W $16
	DC.B 'Help with Select File',$00
	DC.L $43
	DC.W $28
	DC.B 'Installation of %s has been completed.',$00,$00
	DC.L $44
	DC.W $4C
	DC.B 'Installation complete!',10,'%s can be found in your "%s" drawer (or partiti'
	DC.B 'on).',$00,$00
	DC.L $45
	DC.W $2C
	DC.B '(The installation log file is named "%s".)',$00,$00
	DC.L $46
	DC.W $1C
	DC.B 'Working on installation...',$00,$00
	DC.L $47
	DC.W $12
	DC.B 'Help with Confirm',$00
	DC.L $48
	DC.W $1C
	DC.B 'Version to install: %ld.%ld',$00
	DC.L $49
	DC.W $26
	DC.B 'Version currently installed: %ld.%ld',$00,$00
	DC.L $4A
	DC.W $2A
	DC.B 'There is no currently installed version.',$00,$00
	DC.L $4B
	DC.W $16
	DC.B 'Proceed With Install',$00,$00
	DC.L $4C
	DC.W $16
	DC.B 'Set Installation Mode',$00
	DC.L $4D
	DC.W $90
	DC.B 'Welcome to the %s installation utility. Please indicate how the instal'
	DC.B 'lation should proceed (based upon your knowledge of the Amiga computer'
	DC.B ').',$00,$00
	DC.L $4E
	DC.W $20
	DC.B 'Help with Set Installation Mode',$00
	DC.L $4F
	DC.W $14
	DC.B 'Log all actions to:',$00
	DC.L $50
	DC.W $16
	DC.B 'Installation Options',$00,$00
	DC.L $51
	DC.W $28
	DC.B 'Welcome to the %s installation utility.',$00
	DC.L $52
	DC.W $1E
	DC.B 'Help with Additional Settings',$00
	DC.L $53
	DC.W $1E
	DC.B 'Modify Startup-Sequence File',$00,$00
	DC.L $54
	DC.W $14
	DC.B 'Select Another File',$00
	DC.L $55
	DC.W $16
	DC.B 'Proceed with Changes',$00,$00
	DC.L $56
	DC.W $16
	DC.B 'File being modified:',$00,$00
	DC.L $57
	DC.W $22
	DC.B 'Help with Change Startup-Sequence',$00
	DC.L $58
	DC.W $16
	DC.B 'AN ERROR HAS OCCURED!',$00
	DC.L $59
	DC.W $12
	DC.B '',10,'DOS Error Type: ',$00
	DC.L $5A
	DC.W $20
	DC.B 'Sorry... an Error Has Occured!',$00,$00
	DC.L $5B
	DC.W $12
	DC.B 'Please confirm...',$00
	DC.L $5C
	DC.W $18
	DC.B ' - Press ',39,'Esc',39,' to Abort',$00
	DC.L $5D
	DC.W $1A
	DC.B 'Valid range is %ld to %ld',$00
	DC.L $60
	DC.W $12
	DC.B 'Proceed with Copy',$00
	DC.L $83
	DC.W $98
	DC.B 'Please insert your boot disk into the drive you booted from and then c'
	DC.B 'lick on "Proceed". If the disk is already in the drive, just click on '
	DC.B '"Proceed".',$00,$00
	DC.L $A8
	DC.W $58
	DC.B '',10,'The file "%s" is protected from deletion.',10,10,'Can the file be deleted an'
	DC.B 'd/or overwritten?',$00
	DC.L $A9
	DC.W $A
	DC.B 'Skip File',$00
	DC.L $AA
	DC.W $8
	DC.B 'Delete',$00,$00
	DC.L $8A
	DC.W $578
	DC.B 5,120,'    You will need to select a "User Level" for the installer to contin'
	DC.B 'ue. The different levels are as follows:',10,'    NOVICE USER: In this mode'
	DC.B ' the installer proceeds entirely automatically, and will not ask you t'
	DC.B 'o make any decisions. However, this mode is only recommended for Amiga'
	DC.B ' computers that are not extensively customized, as some of the assumpt'
	DC.B 'ions made by the program are only valid on a "stock" Amiga. You may st'
	DC.B 'ill be requested to insert disks or perform other actions.',10,'    INTERME'
	DC.B 'DIATE USER: The installer will allow you to make major choices, such a'
	DC.B 's where to install the application, but will not bother you with the m'
	DC.B 'inor details of copying files, creating drawers, etc.',10,'    EXPERT USER:'
	DC.B ' In this mode you will be asked for confirmation of each step of the i'
	DC.B 'nstallation process (although in some cases several steps may be combi'
	DC.B 'ned into a single confirmation). In addition, you will have greater co'
	DC.B 'ntrol over where installed items are placed.',10,'    In addition, there ar'
	DC.B 'e several buttons at the bottom:',10,'    "Proceed with Install" indicates '
	DC.B 'that you have chosen a User Level and that installation should proceed'
	DC.B '.',10,'    "About..." gives information about the installation program, suc'
	DC.B 'h as copyright information, version number, author, etc.',10,'    "Abort In'
	DC.B 'stall" means that you have changed your mind and do not wish to instal'
	DC.B 'l the application at this time.',10,'    "Help..." brings up this text.',$00,$00
	DC.L $8B
	DC.W $39C
	DC.B 3,156,'    Explanation of controls:',10,10,'    "Install for Real" -- This will caus'
	DC.B 'e the installer to actually carry out the installation.',10,'    "Pretend t'
	DC.B 'o Install" -- In this mode, the installer will go through all the step'
	DC.B 's of the installation, except that it will not make any permanent chan'
	DC.B 'ges. You can use this option to get a "preview" of what the installer '
	DC.B 'will do before it actually does it. You can also use this in combinati'
	DC.B 'on with the "log file" options below to get a complete list of what ha'
	DC.B 'ppened during the rehearsal.',10,10,'    "Printer" -- This will cause a list '
	DC.B 'of all installation actions to be printed out on the printer.',10,'    "Log'
	DC.B ' File" -- This will cause a list of all installation actions to be wri'
	DC.B 'tten to a log file. You will be informed of the location of this file '
	DC.B 'when the installation finishes. You can use a text editor to read this'
	DC.B ' file.',10,'    "None" -- When this option is turned on, no log file will b'
	DC.B 'e produced.',$00
	DC.L $8C
	DC.W $6BA
	DC.B 6,186,'    In order for this product to run correctly, a number of special co'
	DC.B 'mmands need to be executed by the "startup-sequence", a file of comman'
	DC.B 'ds which is executed each time your Amiga computer boots up. A line ne'
	DC.B 'eds to be inserted into your startup-sequence to run these commands. T'
	DC.B 'he command to be inserted is "Execute S:user-startup". This command wi'
	DC.B 'll execute a script file called "S:user-startup" (which will be create'
	DC.B 'd if it does not already exist). This file will contain all the specia'
	DC.B 'l commands needed by this product. In addition, any other product that'
	DC.B ' uses the Commodore-Amiga Installation Utility will also store its sta'
	DC.B 'rtup commands in the "S:user-startup:" file in the future.',10,10,'Explanatio'
	DC.B 'n of controls:',10,'    The scrolling list box shows your startup sequence.'
	DC.B ' This installer has already selected what it believes is the best plac'
	DC.B 'e to insert the new command. If for some reason you feel that the inst'
	DC.B 'aller has chosen incorrectly (which is certainly possible with a highl'
	DC.B 'y customized system), then you may select a new file and installation '
	DC.B 'point.',10,'    The text box shows the name of the script file to be modifi'
	DC.B 'ed.',10,'    To select a different file: Select the "Select another file" b'
	DC.B 'utton. The file you select should be one that is executed once each ti'
	DC.B 'me you boot up and is not executed at any other time.',10,'    To select a '
	DC.B 'new insertion point: Clicking on the scrolling list will cause the ins'
	DC.B 'ertion point (labeled "insert here") to be moved to the new point.',10,'   '
	DC.B ' "Proceed with Changes" indicates that you are satisfied with the poin'
	DC.B 't of insertion, and the command should be inserted into the file.',10,'    '
	DC.B '"Abort Install" indicates that you wish to abort the installation proc'
	DC.B 'ess.',10,'    "Help..." brings up this text.',$00
	DC.L $8D
	DC.W $18
	DC.B 0,24,'Select File To Modify',$00
	DC.L $8E
	DC.W $52C
	DC.B 5,44,'Explanation of controls:',10,'    The large "scrolling list box" in the cen'
	DC.B 'ter displays the contents of the currently selected disk or directory.'
	DC.B ' Drawers will be indicated by the word "DRW" in inverse block letters '
	DC.B 'in front of the Drawer name. The name of the disk or directory being v'
	DC.B 'iewed is shown below it in a raised outlined box. Beneath that, in ano'
	DC.B 'ther raised outlined box, is the name of the file which is currently s'
	DC.B 'elected. To the right is a slider which can be used to scroll through '
	DC.B 'the list, in case there are more names than will fit into the window.',10
	DC.B '    You can change the selection either by typing a new drawer name in'
	DC.B 'to the "Current Drawer" text box, or by typing a new file name into th'
	DC.B 'e "Selected File" text box, or by clicking on the names of drawers and'
	DC.B ' files in the scrolling list.',10,'    The button "Parent Drawer" will allo'
	DC.B 'w you to view the drawer that contains this one, in other words the "P'
	DC.B 'arent" of the currently visible drawer.',10,'    The button "Show Drives" w'
	DC.B 'ill show a list of all the disk drives in your system. You can then cl'
	DC.B 'ick on a drive name to view the contents of that drive.',10,'    The button'
	DC.B ' "Proceed" indicates you are satisfied with the currently selected fil'
	DC.B 'e.',10,'    The button "Abort Install" will abort the installation. No furt'
	DC.B 'her changes will be made.',10,'    "Help..." brings up this text.',$00,$00
	DC.L $8F
	DC.W $20
	DC.B 0,32,'Select New Destination Drawer',$00
	DC.L $90
	DC.W $51C
	DC.B 5,28,'Explanation of controls:',10,'    The large "scrolling list box" in the cen'
	DC.B 'ter displays the contents of the currently selected disk or directory.'
	DC.B ' Drawers will be indicated by the word "DRW" in inverse block letters '
	DC.B 'in front of the Drawer name. The name of the disk or directory being v'
	DC.B 'iewed is shown below it in a raised outlined box. To the right is a sl'
	DC.B 'ider which can be used to scroll through the list, in case there are m'
	DC.B 'ore names than will fit into the window.',10,'    You can change the select'
	DC.B 'ion by typing a new drawer name into the "Current Drawer" text box, or'
	DC.B ' by clicking on the names of drawers in the scrolling list.',10,'    The bu'
	DC.B 'tton "Parent Drawer" will allow you to view the drawer that contains t'
	DC.B 'his one, in other words the "Parent" of the currently visible drawer.',10
	DC.B '    The button "Show Drives" will show a list of all the disk drives i'
	DC.B 'n your system. You can then click on a drive name to view the contents'
	DC.B ' of that drive.',10,'    The button "Make New Drawer" will allow you to cre'
	DC.B 'ate a new drawer. The newly created drawer will appear inside the dire'
	DC.B 'ctory that is currently being viewed.',10,'    The button "Proceed" indicat'
	DC.B 'es you are satisfied with the currently selected drawer.',10,'    The butto'
	DC.B 'n "Abort Install" will abort the installation. No further changes will'
	DC.B ' be made.',10,'    "Help..." brings up this text.',$00,$00
	DC.L $91
	DC.W $26C
	DC.B 2,108,'Explanation of controls:',10,'    The file to be installed may or may not b'
	DC.B 'e replacing an older version. The display shows a comparison between t'
	DC.B 'he currently installed version and the new version that will replace i'
	DC.B 't. The destination drawer that the file will be copied to is also show'
	DC.B 'n.',10,'    The button "Proceed with Copy" will cause the file to be copied'
	DC.B ' to the destination drawer.',10,'    The button "Skip this part" causes the'
	DC.B ' installer to proceed to the next section. The file will not be copied'
	DC.B '.',10,'    The button "Abort Install" will cancel the installation with no '
	DC.B 'further changes made.',10,'    "Help..." brings up this text.',$00,$00
	DC.L $92
	DC.W $372
	DC.B 3,114,'Explanation of controls:',10,'    The raised box shows a list of file or dr'
	DC.B 'awer names. Each name has a checkmark next to it. If you want the file'
	DC.B ' to be copied, then leave the name checked. An unchecked name will not'
	DC.B ' be copied.',10,'    The box labeled "Destination Drawer to copy to:", just'
	DC.B ' beneath the listing of names shows the drawer to which the items will'
	DC.B ' be copied.',10,'    The button "Proceed with Copy" will cause all the file'
	DC.B 's or drawers which are checked to be copied to the destination drawer.'
	DC.B '',10,'    The button "Skip this part" causes the installer to proceed to th'
	DC.B 'e next section. None of the items listed will be copied.',10,'    The butto'
	DC.B 'n "Change Destination" will allow you to select a new destination. You'
	DC.B ' can use this to copy the items somewhere else.',10,'    The button "Abort '
	DC.B 'Install" will cancel the installation with no further changes being ma'
	DC.B 'de.',10,'    "Help..." brings up this text.',$00,$00
	DC.L $93
	DC.W $B2
	DC.B 0,178,'    Once you have inserted the requested disk, you should select "Proc'
	DC.B 'eed". If you cannot find the disk, or wish to abort the installation f'
	DC.B 'or any reason, then select "Abort".',$00
	DC.L $94
	DC.W $AE
	DC.B 0,174,'    Once you have filled in the text box, you may select "Proceed" to '
	DC.B 'continue with the installation. If you wish to abort the installation '
	DC.B 'for any reason, select "Abort".',$00
	DC.L $95
	DC.W $B0
	DC.B 0,176,'    Once you have filled in the number box, you may select "Proceed" t'
	DC.B 'o continue with the installation. If you wish to abort the installatio'
	DC.B 'n for any reason, select "Abort".',$00
	DC.L $96
	DC.W $196
	DC.B 1,150,'Explanation of controls:',10,'    Each of the raised buttons represents a c'
	DC.B 'hoice which you may make. The button that is selected represents the c'
	DC.B 'urrently choice. You can change the choice by selecting one of the cho'
	DC.B 'ice buttons. Only one selection may be chosen. Once you are satisfied,'
	DC.B ' select "Proceed" to continue with the installation. If you wish to ab'
	DC.B 'ort the installation for any reason, select "Abort".',$00,$00
	DC.L $97
	DC.W $17E
	DC.B 1,126,'Explanation of controls:',10,'    Each of the checkboxes represents a optio'
	DC.B 'n which you may make. A checkmark in a box represents a currently sele'
	DC.B 'cted option. You can change options by clicking on the checkboxes. Onc'
	DC.B 'e you are satisfied with the selected options, select "Proceed" to con'
	DC.B 'tinue with the installation. If you wish to abort the installation for'
	DC.B ' any reason, select "Abort".',$00,$00
	DC.L $98
	DC.W $CC
	DC.B 0,204,'    Select "Proceed" if you want the new drawer to be created. Select '
	DC.B '"Skip This Part" if you do not want the drawer to be created. If you w'
	DC.B 'ish to abort the installation for any reason, select "Abort".',$00
	DC.L $99
	DC.W $1A
	DC.B 0,26,'Create icon for drawer',$00,$00
	DC.L $9A
	DC.W $2A
	DC.B 0,42,'Novice User  - All Actions Automatic   ',$00
	DC.L $9B
	DC.W $30
	DC.B 0,48,'Intermediate User - Limited Manual Control  ',$00,$00
	DC.L $9C
	DC.W $2A
	DC.B 0,42,'Expert User  - Must Confirm all actions',$00
	DC.L $9D
	DC.W $14
	DC.B 0,20,'Install for Real',$00,$00
	DC.L $9E
	DC.W $16
	DC.B 0,22,'Pretend to Install',$00,$00
	DC.L $9F
	DC.W $A
	DC.B 0,10,'Printer',$00
	DC.L $A0
	DC.W $C
	DC.B 0,12,'Log File',$00,$00
	DC.L $A1
	DC.W $8
	DC.B 0,8,'None',$00,$00
	DC.L $A2
	DC.W $22
	DC.B 0,34,'Are you sure you want to abort?',$00
	DC.L $A3
	DC.W $15E
	DC.B 1,94,'The "user-startup" file located in the "S:" drawer is used to store sp'
	DC.B 'ecific commands that some applications need executed at system boot. Y'
	DC.B 'ou are being asked to confirm that this file can be edited. Select the'
	DC.B ' "Proceed" button to confirm, otherwise select the button labeled "Ski'
	DC.B 'p This Part". You can also abort installation by selecting "Abort".',$00
	DC.L $1A
	DC.W $1E
	DC.B 'Unable to access the printer.',$00
	DC.L $1B
	DC.W $2A
	DC.B 'Problem with writing to transcript file.',$00,$00
	DC.L $1C
	DC.W $1E
	DC.B 'Unable to access the printer.',$00
	DC.L $34
	DC.W $26
	DC.B 'Problem with source file/drawer "%s"',$00,$00
	DC.L $35
	DC.W $18
	DC.B 'Problem with file "%s"',$00,$00
	DC.L $61
	DC.W $22
	DC.B 'Can',39,'t examine file or drawer "%s"',$00
	DC.L $62
	DC.W $22
	DC.B 'File or drawer "%s" doesn',39,'t exist',$00
	DC.L $63
	DC.W $12
	DC.B 'Can',39,'t make assign',$00
	DC.L $66
	DC.W $32
	DC.B 'Unable to perform operation -- not enough memory',$00,$00
	DC.L $75
	DC.W $18
	DC.B 'Can',39,'t open drawer "%s"',$00,$00
	DC.L $76
	DC.W $16
	DC.B 'Can',39,'t open file "%s"',$00,$00
	DC.L $77
	DC.W $26
	DC.B 'Problem with file "startup-sequence"',$00,$00
	DC.L $78
	DC.W $18
	DC.B 'Error writing file "%s"',$00
	DC.L $79
	DC.W $1E
	DC.B 'Error was of an unknown type.',$00
	DC.L $7A
	DC.W $22
	DC.B 'Error reading text include file!',$00,$00
	DC.L $7B
	DC.W $20
	DC.B 'Couldn',39,'t create directory "%s"',$00,$00
	DC.L $7C
	DC.W $18
	DC.B 'Couldn',39,'t copy file "%s"',$00
	DC.L $7D
	DC.W $26
	DC.B 'Tried to create drawer over file "%s"',$00
	DC.L $7E
	DC.W $18
	DC.B 'Can',39,'t set execute path',$00,$00
	DC.L $80
	DC.W $22
	DC.B 'Couldn',39,'t find command to execute',$00,$00
	DC.L $81
	DC.W $14
	DC.B 'ARexx not available',$00
	DC.L $82
	DC.W $16
	DC.B 'Can',39,'t duplicate lock',$00,$00
	DC.L $88
	DC.W $16
	DC.B 'Can',39,'t read "%s" file',$00,$00
	DC.L $89
	DC.W $1C
	DC.B 'Error in writing "%s" file',$00,$00
	DC.L $A4
	DC.W $28
	DC.B 'Bad parameters for copying from drawer',$00,$00
	DC.L $A7
	DC.W $14
	DC.B 'Invalid drawer name',$00
	DC.L $16
	DC.W $16
	DC.B 'Can',39,'t open a window.',$00,$00
	DC.L $17
	DC.W $22
	DC.B 'Can',39,'t open install script',39,'s icon.',$00
	DC.L $18
	DC.W $16
	DC.B 'Can',39,'t find partition.',$00
	DC.L $19
	DC.W $20
	DC.B 'Unable to open transcript file.',$00
	DC.L $1E
	DC.W $18
	DC.B 'Unable to open script.',$00,$00
	DC.L $1F
	DC.W $1E
	DC.B 'Unable to initialize system.',$00,$00
	DC.L $20
	DC.W $1E
	DC.B 'Can',39,'t open needed libraries.',$00,$00
	DC.L $21
	DC.W $22
	DC.B 'Can',39,'t get Workbench information.',$00,$00
	DC.L $5F
	DC.W $10
	DC.B 'String too long',$00
	DC.L $64
	DC.W $12
	DC.B 'Division by zero',$00,$00
	DC.L $65
	DC.W $1C
	DC.B '%s: Result string too long',$00,$00
	DC.L $0
	DC.W $3E
	DC.B '******* Installation Log *******',10,'User Level: %s',10,'Pretend: %s',10,10,$00
	DC.L $22
	DC.W $20
	DC.B 'User made a new drawer: "%s".',10,$00,$00
	DC.L $23
	DC.W $1A
	DC.B 'Copy file "%s" to "%s".',10,$00,$00
	DC.L $24
	DC.W $28
	DC.B 'Copy contents of drawer "%s" to "%s".',10,$00,$00
	DC.L $25
	DC.W $22
	DC.B 'Ask Choice: User selected "%s".',10,$00,$00
	DC.L $26
	DC.W $28
	DC.B 'Ask Options: User selected no options.',10,$00
	DC.L $27
	DC.W $32
	DC.B 'Ask Options: User selected following options...',10,$00,$00
	DC.L $28
	DC.W $1E
	DC.B 'Ask String: Result was "%s".',10,$00
	DC.L $29
	DC.W $20
	DC.B 'Ask Number: Result was "%ld".',10,$00,$00
	DC.L $2A
	DC.W $20
	DC.B 'Ask Question: Result was "%s".',10,$00
	DC.L $2B
	DC.W $36
	DC.B 'User changed Copy Files destination drawer to "%s".',10,$00,$00
	DC.L $2C
	DC.W $1E
	DC.B 'Ask Drawer: Result was "%s".',10,$00
	DC.L $2D
	DC.W $1C
	DC.B 'Ask File: Result was "%s".',10,$00
	DC.L $5E
	DC.W $36
	DC.B '*** Global error -- executing onerror statements ***',10,$00
	DC.L $67
	DC.W $10
	DC.B 'User aborted!',10,10,$00
	DC.L $68
	DC.W $1E
	DC.B 'Script aborted in line %ld.',10,$00,$00
	DC.L $69
	DC.W $12
	DC.B '%s in line %ld.',10,$00,$00
	DC.L $6A
	DC.W $1C
	DC.B 'Create New Directory: "%s"',10,$00
	DC.L $6B
	DC.W $3C
	DC.B 'Insert commands to Execute "%s" in file "%s" at byte %ld.',10,$00,$00
	DC.L $6C
	DC.W $50
	DC.B 'Create Text File: "%s"',10,'================== Start of textfile =========='
	DC.B '========',10,$00
	DC.L $6D
	DC.W $3A
	DC.B '=================== End of textfile ===================',10,$00,$00
	DC.L $6E
	DC.W $1A
	DC.B 'Execute DOS Script: "%s"',10,$00
	DC.L $6F
	DC.W $14
	DC.B 'Run Program: "%s"',10,$00,$00
	DC.L $70
	DC.W $1C
	DC.B 'Execute ARexx Script: "%s"',10,$00
	DC.L $71
	DC.W $16
	DC.B 'Renamed "%s" to "%s"',10,$00
	DC.L $72
	DC.W $12
	DC.B 'Delete file "%s"',10,$00
	DC.L $73
	DC.W $12
	DC.B 'Aborting script.',10,$00
	DC.L $74
	DC.W $14
	DC.B 'Exiting script...',10,$00,$00
	DC.L $86
	DC.W $48
	DC.B 'Writing to %s',10,'==================== start of file ====================',10,$00,$00
	DC.L $87
	DC.W $3A
	DC.B '===================== end of file =====================',10,$00,$00
	DC.L $84
	DC.W $6
	DC.B '<ASN>',$00
	DC.L $85
	DC.W $6
	DC.B '<DRW>',$00
	DC.L $1
	DC.W $12
	DC.B 'install_log_file',$00,$00
	DC.L $A5
	DC.W $28
	DC.B 'Amiga Application Installation Utility',$00,$00
	DC.L $A6
	DC.W $72
	DC.B '',10,'Copyright ©1990-1993 Commodore-Amiga, Inc.',10,'All Rights Reserved.',10,10,'Desi'
	DC.B 'gned and Developed by',10,'Sylvan Technical Arts',$00
	DC.L $2
	DC.W $8A
	DC.B 'USAGE: installer [SCRIPT] filename <[APPNAME] name> <[MINUSER] level>',10
	DC.B '       <[DEFUSER] default> <[LOGFILE] logname> <NOPRETEND> <NOLOG>',$00,$00
	DC.L $14
	DC.W $E
	DC.B 'Bad Argument',$00,$00
	DC.L $15
	DC.W $1C
	DC.B 'Can',39,'t find needed ROM font.',$00
	DC.L $1D
	DC.W $32
	DC.B 'Unable to compile script.',10,'ERROR: %s on line %ld.',$00,$00
	DC.L $7F
	DC.W $16
	DC.B 'Couldn',39,'t access NIL:',$00,$00

	ENDC ; CATCOMP_BLOCK


;-----------------------------------------------------------------------------


   STRUCTURE LocaleInfo,0
	APTR li_LocaleBase
	APTR li_Catalog
   LABEL LocaleInfo_SIZEOF

	IFD CATCOMP_CODE

	XREF	_LVOGetCatalogStr
	XDEF	_GetString
	XDEF	GetString
GetString:
_GetString:
	lea	CatCompBlock(pc),a1
	bra.s	2$
1$:	move.w	(a1)+,d1
	add.w	d1,a1
2$:	cmp.l	(a1)+,d0
	bne.s	1$
	addq.l	#2,a1
	move.l	(a0)+,d1
	bne.s	3$
	move.l	a1,d0
	rts
3$:	move.l	a6,-(sp)
	move.l	d1,a6
	move.l	(a0),a0
	jsr	_LVOGetCatalogStr(a6)
	move.l	(sp)+,a6
	rts
	END

	ENDC ; CATCOMP_CODE


;-----------------------------------------------------------------------------


	ENDC ; INSTALLERSTR_I
