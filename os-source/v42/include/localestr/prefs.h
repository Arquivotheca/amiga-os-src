#ifndef LOCALESTR_PREFS_H
#define LOCALESTR_PREFS_H


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

#define MSG_PROGRAM_ERROR 1
#define MSG_PROGRAM_ERROR_STR "Program Error"

#define MSG_NO_ASL_LIBRARY 2
#define MSG_NO_ASL_LIBRARY_STR "Requires asl.library V38"

#define MSG_NO_IFFPARSE_LIBRARY 3
#define MSG_NO_IFFPARSE_LIBRARY_STR "Requires iffparse.library V39"

#define MSG_PROJECT_MENU 4
#define MSG_PROJECT_MENU_STR "Project"

#define MSG_PROJECT_OPEN 5
#define MSG_PROJECT_OPEN_STR "O\000Open..."

#define MSG_PROJECT_SAVE_AS 6
#define MSG_PROJECT_SAVE_AS_STR "A\000Save As..."

#define MSG_PROJECT_QUIT 7
#define MSG_PROJECT_QUIT_STR "Q\000Quit"

#define MSG_EDIT_MENU 8
#define MSG_EDIT_MENU_STR "Edit"

#define MSG_EDIT_CUT 9
#define MSG_EDIT_CUT_STR "X\000Cut"

#define MSG_EDIT_COPY 10
#define MSG_EDIT_COPY_STR "C\000Copy"

#define MSG_EDIT_PASTE 11
#define MSG_EDIT_PASTE_STR "V\000Paste"

#define MSG_EDIT_ERASE 12
#define MSG_EDIT_ERASE_STR " \000Erase"

#define MSG_EDIT_UNDO 13
#define MSG_EDIT_UNDO_STR "Z\000Undo"

#define MSG_EDIT_RESET_TO_DEFAULTS 14
#define MSG_EDIT_RESET_TO_DEFAULTS_STR "D\000Reset To Defaults"

#define MSG_EDIT_LAST_SAVED 15
#define MSG_EDIT_LAST_SAVED_STR "L\000Last Saved"

#define MSG_EDIT_RESTORE 16
#define MSG_EDIT_RESTORE_STR "R\000Restore"

#define MSG_OPTIONS_MENU 17
#define MSG_OPTIONS_MENU_STR "Settings"

#define MSG_OPTIONS_SAVE_ICONS 18
#define MSG_OPTIONS_SAVE_ICONS_STR "I\000Create Icons?"

#define MSG_SAVE_GAD 19
#define MSG_SAVE_GAD_STR "Save"

#define MSG_USE_GAD 20
#define MSG_USE_GAD_STR "Use"

#define MSG_CANCEL_GAD 21
#define MSG_CANCEL_GAD_STR "Cancel"

#define MSG_OK_GAD 22
#define MSG_OK_GAD_STR "OK"

#define MSG_NORMAL_STAT 23
#define MSG_NORMAL_STAT_STR "Normal"

#define MSG_NO_MEMORY_STAT 24
#define MSG_NO_MEMORY_STAT_STR "Not enough memory"

#define MSG_IFF_UNKNOWNCHUNK_STAT 25
#define MSG_IFF_UNKNOWNCHUNK_STAT_STR "Unknown IFF chunk"

#define MSG_IFF_UNKNOWNVERSION_STAT 26
#define MSG_IFF_UNKNOWNVERSION_STAT_STR "Unknown prefs file version"

#define MSG_IFFERROR_STAT 27
#define MSG_IFFERROR_STAT_STR "Error processing IFF file\n%s"

#define MSG_DOSERROR_STAT 28
#define MSG_DOSERROR_STAT_STR "Error accessing file\n%s,\n%s"

#ifdef FONT_PREFS
#define MSG_FNT_NAME 1000
#define MSG_FNT_NAME_STR "Font Preferences"

#define MSG_FNT_REQ_LOAD 1001
#define MSG_FNT_REQ_LOAD_STR "Load Font Preferences"

#define MSG_FNT_REQ_SAVE 1002
#define MSG_FNT_REQ_SAVE_STR "Save Font Preferences"

#define MSG_FNT_FONTS_HDR 1003
#define MSG_FNT_FONTS_HDR_STR "Selected Fonts"

#define MSG_FNT_WBFONT 1004
#define MSG_FNT_WBFONT_STR "Workbench Icon Text: %s %lu"

#define MSG_FNT_SYSFONT 1005
#define MSG_FNT_SYSFONT_STR "System Default Text: %s %lu"

#define MSG_FNT_SCREENFONT 1006
#define MSG_FNT_SCREENFONT_STR "        Screen Text: %s %lu"

#define MSG_FNT_WBFONT_GAD 1007
#define MSG_FNT_WBFONT_GAD_STR "Select Workbench Icon Text..."

#define MSG_FNT_SYSFONT_GAD 1008
#define MSG_FNT_SYSFONT_GAD_STR "Select System Default Text..."

#define MSG_FNT_SCREENFONT_GAD 1009
#define MSG_FNT_SCREENFONT_GAD_STR "Select Screen Text..."

#define MSG_FNT_WBFONT_TITLE 1010
#define MSG_FNT_WBFONT_TITLE_STR "Select Workbench Icon Text"

#define MSG_FNT_SYSFONT_TITLE 1011
#define MSG_FNT_SYSFONT_TITLE_STR "Select System Default Text"

#define MSG_FNT_SCREENFONT_TITLE 1012
#define MSG_FNT_SCREENFONT_TITLE_STR "Select Screen Text"

#define MSG_FNT_MODE_GAD 1013
#define MSG_FNT_MODE_GAD_STR "Mode:"

#define MSG_FNT_TEXT 1014
#define MSG_FNT_TEXT_STR "Text"

#define MSG_FNT_TEXTFIELD 1015
#define MSG_FNT_TEXTFIELD_STR "Text+Field"

#endif /* FONT_PREFS */

#ifdef ICONTROL_PREFS
#define MSG_ICTL_NAME 2000
#define MSG_ICTL_NAME_STR "IControl Preferences"

#define MSG_ICTL_REQ_LOAD 2001
#define MSG_ICTL_REQ_LOAD_STR "Load IControl Preferences"

#define MSG_ICTL_REQ_SAVE 2002
#define MSG_ICTL_REQ_SAVE_STR "Save IControl Preferences"

#define MSG_ICTL_SHIFT_GAD 2003
#define MSG_ICTL_SHIFT_GAD_STR "(SHIFT)"

#define MSG_ICTL_CTRL_GAD 2004
#define MSG_ICTL_CTRL_GAD_STR "CTRL"

#define MSG_ICTL_ALT_GAD 2005
#define MSG_ICTL_ALT_GAD_STR "ALT"

#define MSG_ICTL_LAMIGA_GAD 2006
#define MSG_ICTL_LAMIGA_GAD_STR "(LEFT AMIGA)"

#define MSG_ICTL_AVOIDFLICKER_GAD 2007
#define MSG_ICTL_AVOIDFLICKER_GAD_STR "Avoid Flicker:"

#define MSG_ICTL_MENUSNAP_GAD 2009
#define MSG_ICTL_MENUSNAP_GAD_STR "Screen Menu Snap:"

#define MSG_ICTL_GADGETFILTER_GAD 2010
#define MSG_ICTL_GADGETFILTER_GAD_STR "Text Gadget Filter:"

#define MSG_ICTL_MODEPROMOTE_GAD 2011
#define MSG_ICTL_MODEPROMOTE_GAD_STR "Mode Promotion:"

#define MSG_ICTL_SCRDRAG_HDR 2012
#define MSG_ICTL_SCRDRAG_HDR_STR "Screen Drag"

#define MSG_ICTL_MISC_HDR 2014
#define MSG_ICTL_MISC_HDR_STR "Miscellaneous"

#endif /* ICONTROL_PREFS */

#ifdef INPUT_PREFS
#define MSG_INP_NAME 3000
#define MSG_INP_NAME_STR "Input Preferences"

#define MSG_INP_ERROR_NO_KEYMAP 3001
#define MSG_INP_ERROR_NO_KEYMAP_STR "Could not load keymap file"

#define MSG_INP_REQ_LOAD 3002
#define MSG_INP_REQ_LOAD_STR "Load Input Preferences"

#define MSG_INP_REQ_SAVE 3003
#define MSG_INP_REQ_SAVE_STR "Save Input Preferences"

#define MSG_INP_MOUSE_HDR 3004
#define MSG_INP_MOUSE_HDR_STR "Mouse"

#define MSG_INP_MOUSESPEED_GAD 3005
#define MSG_INP_MOUSESPEED_GAD_STR "Mouse Speed:    \0"

#define MSG_INP_ACCELERATION_GAD 3006
#define MSG_INP_ACCELERATION_GAD_STR "Acceleration:    \0"

#define MSG_INP_DOUBLECLICK_GAD 3007
#define MSG_INP_DOUBLECLICK_GAD_STR "Double-Click Delay:    \0"

#define MSG_INP_SHOW_GAD 3008
#define MSG_INP_SHOW_GAD_STR "Show Double-Click  >"

#define MSG_INP_TEST_GAD 3009
#define MSG_INP_TEST_GAD_STR "Test Double-Click  >"

#define MSG_INP_KEYBOARD_HDR 3010
#define MSG_INP_KEYBOARD_HDR_STR "Keyboard"

#define MSG_INP_KEYDELAY_GAD 3011
#define MSG_INP_KEYDELAY_GAD_STR "Key Repeat Delay:    \0"

#define MSG_INP_KEYRATE_GAD 3012
#define MSG_INP_KEYRATE_GAD_STR "Key Repeat Rate:    \0"

#define MSG_INP_KEYTEST_GAD 3013
#define MSG_INP_KEYTEST_GAD_STR "Keyboard Test:"

#define MSG_INP_KEYBOARDTYPE_GAD 3014
#define MSG_INP_KEYBOARDTYPE_GAD_STR "Keyboard Type"

#define MSG_INP_CLICKAGAIN 3015
#define MSG_INP_CLICKAGAIN_STR " Click again"

#define MSG_INP_DCYES 3016
#define MSG_INP_DCYES_STR " Double-clicked"

#define MSG_INP_DCNO 3017
#define MSG_INP_DCNO_STR " Too slow"

#endif /* INPUT_PREFS */

#ifdef LOCALE_PREFS
#define MSG_LOC_NAME 4000
#define MSG_LOC_NAME_STR "Locale Preferences"

#define MSG_LOC_REQ_LOAD 4001
#define MSG_LOC_REQ_LOAD_STR "Load Locale Preferences"

#define MSG_LOC_REQ_SAVE 4002
#define MSG_LOC_REQ_SAVE_STR "Save Locale Preferences"

#define MSG_LOC_AVAILLANG_GAD 4003
#define MSG_LOC_AVAILLANG_GAD_STR "Available Languages"

#define MSG_LOC_PREFLANG_GAD 4004
#define MSG_LOC_PREFLANG_GAD_STR "Preferred Languages"

#define MSG_LOC_CLEARLANG_GAD 4005
#define MSG_LOC_CLEARLANG_GAD_STR "Clear Languages"

#define MSG_LOC_COUNTRY_GAD 4006
#define MSG_LOC_COUNTRY_GAD_STR "Country"

#define MSG_LOC_TIMEZONE_BOX 4007
#define MSG_LOC_TIMEZONE_BOX_STR "Time Zone: \0"

#define MSG_LOC_HR 4008
#define MSG_LOC_HR_STR "%ld Hours from GMT"

#define MSG_LOC_HR_MIN 4009
#define MSG_LOC_HR_MIN_STR "%ld Hours, %ld Minutes from GMT"

#define MSG_LOC_GMT 4010
#define MSG_LOC_GMT_STR "GMT"

#endif /* LOCALE_PREFS */

#ifdef OVERSCAN_PREFS
#define MSG_OSCN_NAME 5000
#define MSG_OSCN_NAME_STR "Overscan Preferences"

#define MSG_OSCN_REQ_LOAD 5001
#define MSG_OSCN_REQ_LOAD_STR "Load Overscan Preferences"

#define MSG_OSCN_REQ_SAVE 5002
#define MSG_OSCN_REQ_SAVE_STR "Save Overscan Preferences"

#define MSG_OSCN_MONITORS_GAD 5003
#define MSG_OSCN_MONITORS_GAD_STR "Monitor Types"

#define MSG_OSCN_EDITTEXT_GAD 5004
#define MSG_OSCN_EDITTEXT_GAD_STR "Edit Text Size..."

#define MSG_OSCN_EDITGFX_GAD 5005
#define MSG_OSCN_EDITGFX_GAD_STR "Edit Graphics Size..."

#define MSG_OSCN_MINSIZE 5006
#define MSG_OSCN_MINSIZE_STR " Minimum Size: %4lu × %4lu"

#define MSG_OSCN_TEXTSIZE 5007
#define MSG_OSCN_TEXTSIZE_STR "    Text Size: %4lu × %4lu"

#define MSG_OSCN_GFXSIZE 5008
#define MSG_OSCN_GFXSIZE_STR "Graphics Size: %4lu × %4lu"

#define MSG_OSCN_MAXSIZE 5009
#define MSG_OSCN_MAXSIZE_STR " Maximum Size: %4lu × %4lu"

#define MSG_OSCN_MONITOR 5010
#define MSG_OSCN_MONITOR_STR "Monitor %lu"

#define MSG_OSCN_TEXTEDIT 5011
#define MSG_OSCN_TEXTEDIT_STR "Use the handles to size\nand place your Text overscan\narea. It may be as big as\nyou like, but the whole area\nshould remain visible."

#define MSG_OSCN_GFXEDIT 5012
#define MSG_OSCN_GFXEDIT_STR "Use the handles to size and\nplace your Graphics overscan\narea. It should fill your\nwhole screen."

#define MSG_OSCN_EDITSIZE 5013
#define MSG_OSCN_EDITSIZE_STR "    Current Size: %4lu × %4lu"

#define MSG_OSCN_EDITPOS 5014
#define MSG_OSCN_EDITPOS_STR "Current Position: (%4lu,%4lu)"

#define MSG_OSCN_OVERSCAN_DIM_HDR 5015
#define MSG_OSCN_OVERSCAN_DIM_HDR_STR "Dimensions"

#endif /* OVERSCAN_PREFS */

#ifdef PRINTER_PREFS
#define MSG_PRT_NAME 8000
#define MSG_PRT_NAME_STR "Printer Preferences"

#define MSG_PRT_REQ_LOAD 8001
#define MSG_PRT_REQ_LOAD_STR "Load Printer Preferences"

#define MSG_PRT_REQ_SAVE 8002
#define MSG_PRT_REQ_SAVE_STR "Save Printer Preferences"

#define MSG_PRT_PRINTERTYPE_GAD 8003
#define MSG_PRT_PRINTERTYPE_GAD_STR "Printer Type"

#define MSG_PRT_PRINTERPORT_GAD 8004
#define MSG_PRT_PRINTERPORT_GAD_STR "Printer Port:"

#define MSG_PRT_PAPERTYPE_GAD 8005
#define MSG_PRT_PAPERTYPE_GAD_STR "Paper Type:"

#define MSG_PRT_PAPERSIZE_GAD 8006
#define MSG_PRT_PAPERSIZE_GAD_STR "Paper Format:"

#define MSG_PRT_PRINTPITCH_GAD 8007
#define MSG_PRT_PRINTPITCH_GAD_STR "Print Pitch:"

#define MSG_PRT_PRINTSPACING_GAD 8008
#define MSG_PRT_PRINTSPACING_GAD_STR "Print Spacing:"

#define MSG_PRT_PRINTQUALITY_GAD 8009
#define MSG_PRT_PRINTQUALITY_GAD_STR "Print Quality:"

#define MSG_PRT_PAPERLENGTH_GAD 8010
#define MSG_PRT_PAPERLENGTH_GAD_STR "Paper Length (lines)     :"

#define MSG_PRT_LEFTMARGIN_GAD 8011
#define MSG_PRT_LEFTMARGIN_GAD_STR "Left Margin (characters):"

#define MSG_PRT_RIGHTMARGIN_GAD 8012
#define MSG_PRT_RIGHTMARGIN_GAD_STR "Right Margin (characters):"

#define MSG_PRT_PARALLEL 8013
#define MSG_PRT_PARALLEL_STR "Parallel"

#define MSG_PRT_SERIAL 8014
#define MSG_PRT_SERIAL_STR "Serial"

#define MSG_PRT_SINGLE 8015
#define MSG_PRT_SINGLE_STR "Single"

#define MSG_PRT_FANFOLD 8016
#define MSG_PRT_FANFOLD_STR "Continuous"

#define MSG_PRT_USLETTER 8017
#define MSG_PRT_USLETTER_STR "U.S. Letter"

#define MSG_PRT_USLEGAL 8018
#define MSG_PRT_USLEGAL_STR "U.S. Legal"

#define MSG_PRT_NARROWTRACTOR 8019
#define MSG_PRT_NARROWTRACTOR_STR "Narrow Tractor"

#define MSG_PRT_WIDETRACTOR 8020
#define MSG_PRT_WIDETRACTOR_STR "Wide Tractor"

#define MSG_PRT_CUSTOM 8021
#define MSG_PRT_CUSTOM_STR "Custom"

#define MSG_PRT_DINA4 8022
#define MSG_PRT_DINA4_STR "DIN A4"

#define MSG_PRT_PICA10 8023
#define MSG_PRT_PICA10_STR "Pica (10 cpi)"

#define MSG_PRT_ELITE12 8024
#define MSG_PRT_ELITE12_STR "Elite (12 cpi)"

#define MSG_PRT_FINE15 8025
#define MSG_PRT_FINE15_STR "Fine (15-17 cpi)"

#define MSG_PRT_LPI6 8026
#define MSG_PRT_LPI6_STR "6 Lines Per Inch"

#define MSG_PRT_LPI8 8027
#define MSG_PRT_LPI8_STR "8 Lines Per Inch"

#define MSG_PRT_DRAFT 8028
#define MSG_PRT_DRAFT_STR "Draft"

#define MSG_PRT_LETTER 8029
#define MSG_PRT_LETTER_STR "Letter"

#define MSG_PRT_DEVICEUNIT_GAD 8030
#define MSG_PRT_DEVICEUNIT_GAD_STR "Device Unit:"

#endif /* PRINTER_PREFS */

#ifdef PRINTERGFX_PREFS
#define MSG_PGFX_NAME 9000
#define MSG_PGFX_NAME_STR "Graphics Printer Preferences"

#define MSG_PGFX_REQ_LOAD 9001
#define MSG_PGFX_REQ_LOAD_STR "Load Graphics Printer Preferences"

#define MSG_PGFX_REQ_SAVE 9002
#define MSG_PGFX_REQ_SAVE_STR "Save Graphics Printer Preferences"

#define MSG_PGFX_RED_GAD 9003
#define MSG_PGFX_RED_GAD_STR "Red:"

#define MSG_PGFX_GREEN_GAD 9004
#define MSG_PGFX_GREEN_GAD_STR "Green:"

#define MSG_PGFX_BLUE_GAD 9005
#define MSG_PGFX_BLUE_GAD_STR "Blue:"

#define MSG_PGFX_COLORS_GAD 9006
#define MSG_PGFX_COLORS_GAD_STR "Colors:"

#define MSG_PGFX_SMOOTHING_GAD 9007
#define MSG_PGFX_SMOOTHING_GAD_STR "Smoothing"

#define MSG_PGFX_DITHERING_GAD 9008
#define MSG_PGFX_DITHERING_GAD_STR "Dithering:"

#define MSG_PGFX_SCALING_GAD 9009
#define MSG_PGFX_SCALING_GAD_STR "Scaling:"

#define MSG_PGFX_IMAGE_GAD 9010
#define MSG_PGFX_IMAGE_GAD_STR "Image:"

#define MSG_PGFX_ASPECT_GAD 9011
#define MSG_PGFX_ASPECT_GAD_STR "Aspect:"

#define MSG_PGFX_SHADE_GAD 9012
#define MSG_PGFX_SHADE_GAD_STR "Shade:"

#define MSG_PGFX_THRESHOLD_GAD 9013
#define MSG_PGFX_THRESHOLD_GAD_STR "Threshold:   \0"

#define MSG_PGFX_LEFTOFFSET0_GAD 9014
#define MSG_PGFX_LEFTOFFSET0_GAD_STR "Left Edge (mm)       :"

#define MSG_PGFX_LEFTOFFSET1_GAD 9015
#define MSG_PGFX_LEFTOFFSET1_GAD_STR "Left Edge (inches/10):"

#define MSG_PGFX_CENTERPICTURE_GAD 9016
#define MSG_PGFX_CENTERPICTURE_GAD_STR "Center Picture"

#define MSG_PGFX_TYPE_GAD 9017
#define MSG_PGFX_TYPE_GAD_STR "Type:"

#define MSG_PGFX_WIDTH0_GAD 9018
#define MSG_PGFX_WIDTH0_GAD_STR "Width             :"

#define MSG_PGFX_HEIGHT0_GAD 9019
#define MSG_PGFX_HEIGHT0_GAD_STR "Height            :"

#define MSG_PGFX_WIDTH1_GAD 9020
#define MSG_PGFX_WIDTH1_GAD_STR "Width  (mm)       :"

#define MSG_PGFX_HEIGHT1_GAD 9021
#define MSG_PGFX_HEIGHT1_GAD_STR "Height (mm)       :"

#define MSG_PGFX_WIDTH2_GAD 9022
#define MSG_PGFX_WIDTH2_GAD_STR "Width  (inches/10):"

#define MSG_PGFX_HEIGHT2_GAD 9023
#define MSG_PGFX_HEIGHT2_GAD_STR "Height (inches/10):"

#define MSG_PGFX_WIDTH3_GAD 9024
#define MSG_PGFX_WIDTH3_GAD_STR "Width  (pixels)   :"

#define MSG_PGFX_HEIGHT3_GAD 9025
#define MSG_PGFX_HEIGHT3_GAD_STR "Height (pixels)   :"

#define MSG_PGFX_WIDTH4_GAD 9026
#define MSG_PGFX_WIDTH4_GAD_STR "Width  (times)    :"

#define MSG_PGFX_HEIGHT4_GAD 9027
#define MSG_PGFX_HEIGHT4_GAD_STR "Height (times)    :"

#define MSG_PGFX_DENSITY_GAD 9028
#define MSG_PGFX_DENSITY_GAD_STR "Density:   \0"

#define MSG_PGFX_COLORCORRECT_HDR 9029
#define MSG_PGFX_COLORCORRECT_HDR_STR "Color Correction"

#define MSG_PGFX_LIMITS_HDR 9030
#define MSG_PGFX_LIMITS_HDR_STR "Limits"

#define MSG_PGFX_ORDERED 9031
#define MSG_PGFX_ORDERED_STR "Ordered"

#define MSG_PGFX_HALFTONE 9032
#define MSG_PGFX_HALFTONE_STR "Halftone"

#define MSG_PGFX_FLOYDSTEINBERG 9033
#define MSG_PGFX_FLOYDSTEINBERG_STR "Floyd-Steinberg"

#define MSG_PGFX_FRACTION 9034
#define MSG_PGFX_FRACTION_STR "Fraction"

#define MSG_PGFX_INTEGER 9035
#define MSG_PGFX_INTEGER_STR "Integer"

#define MSG_PGFX_POSITIVE 9036
#define MSG_PGFX_POSITIVE_STR "Positive"

#define MSG_PGFX_NEGATIVE 9037
#define MSG_PGFX_NEGATIVE_STR "Negative"

#define MSG_PGFX_HORIZONTAL 9038
#define MSG_PGFX_HORIZONTAL_STR "Horizontal"

#define MSG_PGFX_VERTICAL 9039
#define MSG_PGFX_VERTICAL_STR "Vertical"

#define MSG_PGFX_BLACK_AND_WHITE 9040
#define MSG_PGFX_BLACK_AND_WHITE_STR "Black & White"

#define MSG_PGFX_GREY_SCALE_1 9041
#define MSG_PGFX_GREY_SCALE_1_STR "Grey Scale 1"

#define MSG_PGFX_COLOR 9042
#define MSG_PGFX_COLOR_STR "Color"

#define MSG_PGFX_GREY_SCALE_2 9043
#define MSG_PGFX_GREY_SCALE_2_STR "Grey Scale 2"

#define MSG_PGFX_IGNORE 9044
#define MSG_PGFX_IGNORE_STR "Ignore"

#define MSG_PGFX_BOUNDED 9045
#define MSG_PGFX_BOUNDED_STR "Bounded"

#define MSG_PGFX_ABSOLUTE 9046
#define MSG_PGFX_ABSOLUTE_STR "Absolute"

#define MSG_PGFX_PIXELS 9047
#define MSG_PGFX_PIXELS_STR "Pixels"

#define MSG_PGFX_MULTIPLY 9048
#define MSG_PGFX_MULTIPLY_STR "Multiply"

#define MSG_PGFX_OPTIONS_METRIC 9049
#define MSG_PGFX_OPTIONS_METRIC_STR "M\000Use Metric System?"

#endif /* PRINTERGFX_PREFS */

#ifdef SCREENMODE_PREFS
#define MSG_SM_NAME 10000
#define MSG_SM_NAME_STR "ScreenMode Preferences"

#define MSG_SM_REQ_LOAD 10001
#define MSG_SM_REQ_LOAD_STR "Load ScreenMode Preferences"

#define MSG_SM_REQ_SAVE 10002
#define MSG_SM_REQ_SAVE_STR "Save ScreenMode Preferences"

#define MSG_SM_DISPLAY_MODE_GAD 10003
#define MSG_SM_DISPLAY_MODE_GAD_STR "Display Mode"

#define MSG_SM_PROPS_GAD 10004
#define MSG_SM_PROPS_GAD_STR "Mode Properties"

#define MSG_SM_WIDTH_GAD 10005
#define MSG_SM_WIDTH_GAD_STR "Width:"

#define MSG_SM_HEIGHT_GAD 10006
#define MSG_SM_HEIGHT_GAD_STR "Height:"

#define MSG_SM_DEFAULT_GAD 10007
#define MSG_SM_DEFAULT_GAD_STR "Default"

#define MSG_SM_COLORS_GAD 10008
#define MSG_SM_COLORS_GAD_STR "Colors:    \0"

#define MSG_SM_AUTOSCROLL_GAD 10009
#define MSG_SM_AUTOSCROLL_GAD_STR "AutoScroll:"

#define MSG_SM_VISIBLESIZE_PROP 10010
#define MSG_SM_VISIBLESIZE_PROP_STR "Visible Size  : %5lu × %5lu"

#define MSG_SM_MAXSIZE_PROP 10011
#define MSG_SM_MAXSIZE_PROP_STR "Maximum Size  : %5lu × %5lu"

#define MSG_SM_MINSIZE_PROP 10012
#define MSG_SM_MINSIZE_PROP_STR "Minimum Size  : %5lu × %5lu"

#define MSG_SM_MAXCOLORS_PROP 10013
#define MSG_SM_MAXCOLORS_PROP_STR "Maximum Colors: %lu"

#define MSG_SM_INTERLACE_PROP 10014
#define MSG_SM_INTERLACE_PROP_STR "Interlaced"

#define MSG_SM_ECS_PROP 10015
#define MSG_SM_ECS_PROP_STR "Requires ECS"

#define MSG_SM_GLOCK_PROP 10016
#define MSG_SM_GLOCK_PROP_STR "Supports genlock"

#define MSG_SM_NOGLOCK_PROP 10017
#define MSG_SM_NOGLOCK_PROP_STR "Does not support genlock"

#define MSG_SM_DRAG_PROP 10018
#define MSG_SM_DRAG_PROP_STR "Draggable"

#define MSG_SM_NODRAG_PROP 10019
#define MSG_SM_NODRAG_PROP_STR "Not draggable"

#endif /* SCREENMODE_PREFS */

#ifdef SERIAL_PREFS
#define MSG_SER_NAME 11000
#define MSG_SER_NAME_STR "Serial Preferences"

#define MSG_SER_REQ_LOAD 11001
#define MSG_SER_REQ_LOAD_STR "Load Serial Preferences"

#define MSG_SER_REQ_SAVE 11002
#define MSG_SER_REQ_SAVE_STR "Save Serial Preferences"

#define MSG_SER_BAUDRATE_GAD 11003
#define MSG_SER_BAUDRATE_GAD_STR "Baud Rate:        \0"

#define MSG_SER_INPUTBUF_GAD 11004
#define MSG_SER_INPUTBUF_GAD_STR "Input Buffer Size:        \0"

#define MSG_SER_HANDSHAKING_GAD 11005
#define MSG_SER_HANDSHAKING_GAD_STR "Handshaking"

#define MSG_SER_PARITY_GAD 11006
#define MSG_SER_PARITY_GAD_STR "Parity"

#define MSG_SER_DATABITS_GAD 11007
#define MSG_SER_DATABITS_GAD_STR "Bits/Char"

#define MSG_SER_STOPBITS_GAD 11008
#define MSG_SER_STOPBITS_GAD_STR "Stop Bits"

#define MSG_SER_DEFAULTUNIT_GAD 11009
#define MSG_SER_DEFAULTUNIT_GAD_STR "Default Unit:"

#define MSG_SER_XON_XOFF 11010
#define MSG_SER_XON_XOFF_STR "XON/XOFF"

#define MSG_SER_RTS_CTS 11011
#define MSG_SER_RTS_CTS_STR "RTS/CTS"

#define MSG_SER_NONE 11012
#define MSG_SER_NONE_STR "None"

#define MSG_SER_PNONE 11013
#define MSG_SER_PNONE_STR "None"

#define MSG_SER_EVEN 11014
#define MSG_SER_EVEN_STR "Even"

#define MSG_SER_ODD 11015
#define MSG_SER_ODD_STR "Odd"

#define MSG_SER_MARK 11016
#define MSG_SER_MARK_STR "Mark"

#define MSG_SER_SPACE 11017
#define MSG_SER_SPACE_STR "Space"

#endif /* SERIAL_PREFS */

#ifdef TIME_PREFS
#define MSG_TIME_NAME 13000
#define MSG_TIME_NAME_STR "Time Preferences"

#define MSG_TIME_HOURS_GAD 13001
#define MSG_TIME_HOURS_GAD_STR "Hours"

#define MSG_TIME_MINUTES_GAD 13002
#define MSG_TIME_MINUTES_GAD_STR "Minutes"

#endif /* TIME_PREFS */

#ifdef PRINTERPS_PREFS
#define MSG_PS_NAME 16000
#define MSG_PS_NAME_STR "PostScript Printer Preferences"

#define MSG_PS_REQ_LOAD 16001
#define MSG_PS_REQ_LOAD_STR "Load PostScript Printer Preferences"

#define MSG_PS_REQ_SAVE 16002
#define MSG_PS_REQ_SAVE_STR "Save PostScript Printer Preferences"

#define MSG_PS_DRIVERMODE_GAD 16003
#define MSG_PS_DRIVERMODE_GAD_STR "Driver Mode:"

#define MSG_PS_COPIES_GAD 16004
#define MSG_PS_COPIES_GAD_STR "Copies:   \0"

#define MSG_PS_PAPERFORMAT_GAD 16006
#define MSG_PS_PAPERFORMAT_GAD_STR "Paper Format:"

#define MSG_PS_PAPERWIDTH_GAD 16007
#define MSG_PS_PAPERWIDTH_GAD_STR "Paper Width:"

#define MSG_PS_PAPERHEIGHT_GAD 16008
#define MSG_PS_PAPERHEIGHT_GAD_STR "Paper Height:"

#define MSG_PS_HORIZDPI_GAD 16009
#define MSG_PS_HORIZDPI_GAD_STR "Horizontal DPI:"

#define MSG_PS_VERTDPI_GAD 16010
#define MSG_PS_VERTDPI_GAD_STR "Vertical DPI:"

#define MSG_PS_FONT_GAD 16011
#define MSG_PS_FONT_GAD_STR "Font:"

#define MSG_PS_PITCH_GAD 16012
#define MSG_PS_PITCH_GAD_STR "Pitch:"

#define MSG_PS_TAB_GAD 16013
#define MSG_PS_TAB_GAD_STR "Tab:"

#define MSG_PS_LEFTMARGIN_GAD 16014
#define MSG_PS_LEFTMARGIN_GAD_STR "Left Margin:"

#define MSG_PS_RIGHTMARGIN_GAD 16015
#define MSG_PS_RIGHTMARGIN_GAD_STR "Right Margin:"

#define MSG_PS_TOPMARGIN_GAD 16016
#define MSG_PS_TOPMARGIN_GAD_STR "Top Margin:"

#define MSG_PS_BOTTOMMARGIN_GAD 16017
#define MSG_PS_BOTTOMMARGIN_GAD_STR "Bottom Margin:"

#define MSG_PS_FONTPOINTSIZE_GAD 16018
#define MSG_PS_FONTPOINTSIZE_GAD_STR "Font Point Size:"

#define MSG_PS_LINELEADING_GAD 16019
#define MSG_PS_LINELEADING_GAD_STR "Line Leading:"

#define MSG_PS_LINESPERINCH_GAD 16020
#define MSG_PS_LINESPERINCH_GAD_STR "Lines Per Inch:"

#define MSG_PS_LINESPERPAGE_GAD 16021
#define MSG_PS_LINESPERPAGE_GAD_STR "Lines Per Page:"

#define MSG_PS_ORIENTATION_GAD 16022
#define MSG_PS_ORIENTATION_GAD_STR "Orientation:"

#define MSG_PS_ASPECT_GAD 16023
#define MSG_PS_ASPECT_GAD_STR "Aspect:"

#define MSG_PS_SCALINGTYPE_GAD 16024
#define MSG_PS_SCALINGTYPE_GAD_STR "Scaling Type:"

#define MSG_PS_SCALINGMATH_GAD 16025
#define MSG_PS_SCALINGMATH_GAD_STR "Scaling Math:"

#define MSG_PS_CENTERING_GAD 16026
#define MSG_PS_CENTERING_GAD_STR "Centering:"

#define MSG_PS_SAMPLEPICTURE_HDR 16027
#define MSG_PS_SAMPLEPICTURE_HDR_STR "Picture"

#define MSG_PS_SAMPLESCALING_HDR 16028
#define MSG_PS_SAMPLESCALING_HDR_STR "Sample Scalings"

#define MSG_PS_LEFTEDGE_GAD 16029
#define MSG_PS_LEFTEDGE_GAD_STR "Left Edge:"

#define MSG_PS_TOPEDGE_GAD 16030
#define MSG_PS_TOPEDGE_GAD_STR "Top Edge:"

#define MSG_PS_WIDTH_GAD 16031
#define MSG_PS_WIDTH_GAD_STR "Width:"

#define MSG_PS_HEIGHT_GAD 16032
#define MSG_PS_HEIGHT_GAD_STR "Height:"

#define MSG_PS_IMAGE_GAD 16033
#define MSG_PS_IMAGE_GAD_STR "Image:"

#define MSG_PS_SHADING_GAD 16034
#define MSG_PS_SHADING_GAD_STR "Shading:"

#define MSG_PS_DITHERING_GAD 16035
#define MSG_PS_DITHERING_GAD_STR "Dithering:"

#define MSG_PS_TRANSPARENT_GAD 16036
#define MSG_PS_TRANSPARENT_GAD_STR "Transparent:"

#define MSG_PS_PANEL_1 16037
#define MSG_PS_PANEL_1_STR "Text Options"

#define MSG_PS_PANEL_2 16038
#define MSG_PS_PANEL_2_STR "Text Dimensions"

#define MSG_PS_PANEL_3 16039
#define MSG_PS_PANEL_3_STR "Graphics Options"

#define MSG_PS_PANEL_4 16040
#define MSG_PS_PANEL_4_STR "Graphics Scaling"

#define MSG_PS_MODE_1 16041
#define MSG_PS_MODE_1_STR "PostScript"

#define MSG_PS_MODE_2 16042
#define MSG_PS_MODE_2_STR "Pass Through"

#define MSG_PS_FORMAT_1 16045
#define MSG_PS_FORMAT_1_STR "U.S. Letter"

#define MSG_PS_FORMAT_2 16046
#define MSG_PS_FORMAT_2_STR "U.S. Legal"

#define NSG_PS_FORMAT_3 16047
#define NSG_PS_FORMAT_3_STR "DIN A4"

#define MSG_PS_FORMAT_4 16048
#define MSG_PS_FORMAT_4_STR "Custom"

#define MSG_PS_FONT_1 16049
#define MSG_PS_FONT_1_STR "Courier"

#define MSG_PS_FONT_2 16050
#define MSG_PS_FONT_2_STR "Times-Roman"

#define MSG_PS_FONT_3 16051
#define MSG_PS_FONT_3_STR "Helvetica"

#define MSG_PS_FONT_4 16052
#define MSG_PS_FONT_4_STR "Helv-Narrow"

#define MSG_PS_FONT_5 16053
#define MSG_PS_FONT_5_STR "AvantGarde"

#define MSG_PS_FONT_6 16054
#define MSG_PS_FONT_6_STR "Bookman"

#define MSG_PS_FONT_7 16055
#define MSG_PS_FONT_7_STR "Palatino"

#define MSG_PS_FONT_8 16056
#define MSG_PS_FONT_8_STR "ZapfChancery"

#define MSG_PS_PITCH_1 16057
#define MSG_PS_PITCH_1_STR "Normal"

#define MSG_PS_PITCH_2 16058
#define MSG_PS_PITCH_2_STR "Compressed"

#define MSG_PS_PITCH_3 16059
#define MSG_PS_PITCH_3_STR "Expanded"

#define MSG_PS_ORIENTATION_1 16060
#define MSG_PS_ORIENTATION_1_STR "Portrait"

#define MSG_PS_ORIENTATION_2 16061
#define MSG_PS_ORIENTATION_2_STR "Landscape"

#define MSG_PS_TAB_1 16062
#define MSG_PS_TAB_1_STR "4 Characters"

#define MSG_PS_TAB_2 16063
#define MSG_PS_TAB_2_STR "8 Characters"

#define MSG_PS_TAB_3 16064
#define MSG_PS_TAB_3_STR "1/4 Inch"

#define NSG_PS_TAB_4 16065
#define NSG_PS_TAB_4_STR "1/2 Inch"

#define MSG_PS_TAB_5 16066
#define MSG_PS_TAB_5_STR "1 Inch"

#define MSG_PS_ASPECT_1 16067
#define MSG_PS_ASPECT_1_STR "Normal"

#define MSG_PS_ASPECT_2 16068
#define MSG_PS_ASPECT_2_STR "Sideways"

#define MSG_PS_SCALINGTYPE_1 16069
#define MSG_PS_SCALINGTYPE_1_STR "None"

#define MSG_PS_SCALINGTYPE_2 16070
#define MSG_PS_SCALINGTYPE_2_STR "Aspect; Width"

#define MSG_PS_SCALINGTYPE_3 16071
#define MSG_PS_SCALINGTYPE_3_STR "Aspect; Height"

#define MSG_PS_SCALINGTYPE_4 16072
#define MSG_PS_SCALINGTYPE_4_STR "Aspect; Both"

#define MSG_PS_SCALINGTYPE_5 16073
#define MSG_PS_SCALINGTYPE_5_STR "Fits; Wide"

#define MSG_PS_SCALINGTYPE_6 16074
#define MSG_PS_SCALINGTYPE_6_STR "Fits; Tall"

#define MSG_PS_SCALINGTYPE_7 16075
#define MSG_PS_SCALINGTYPE_7_STR "Fits; Both"

#define MSG_PS_SCALINGMATH_1 16076
#define MSG_PS_SCALINGMATH_1_STR "Integer"

#define MSG_PS_SCALINGMATH_2 16077
#define MSG_PS_SCALINGMATH_2_STR "Fractional"

#define MSG_PS_CENTERING_1 16078
#define MSG_PS_CENTERING_1_STR "None"

#define MSG_PS_CENTERING_2 16079
#define MSG_PS_CENTERING_2_STR "Horizontal"

#define MSG_PS_CENTERING_3 16080
#define MSG_PS_CENTERING_3_STR "Vertical"

#define MSG_PS_CENTERING_4 16081
#define MSG_PS_CENTERING_4_STR "Both Directions"

#define MSG_PS_IMAGE_1 16082
#define MSG_PS_IMAGE_1_STR "Positive"

#define MSG_PS_IMAGE_2 16083
#define MSG_PS_IMAGE_2_STR "Negative"

#define MSG_PS_SHADING_1 16084
#define MSG_PS_SHADING_1_STR "Black & White"

#define MSG_PS_SHADING_2 16085
#define MSG_PS_SHADING_2_STR "Grey Scale"

#define MSG_PS_SHADING_3 16086
#define MSG_PS_SHADING_3_STR "Color"

#define MSG_PS_DITHERING_1 16087
#define MSG_PS_DITHERING_1_STR "Default"

#define MSG_PS_DITHERING_2 16088
#define MSG_PS_DITHERING_2_STR "Dotty"

#define MSG_PS_DITHERING_3 16089
#define MSG_PS_DITHERING_3_STR "Vertical"

#define MSG_PS_DITHERING_4 16090
#define MSG_PS_DITHERING_4_STR "Horizontal"

#define MSG_PS_DITHERING_5 16091
#define MSG_PS_DITHERING_5_STR "Diagonal"

#define MSG_PS_TRANSPARENT_1 16092
#define MSG_PS_TRANSPARENT_1_STR "None"

#define MSG_PS_TRANSPARENT_2 16093
#define MSG_PS_TRANSPARENT_2_STR "White"

#define MSG_PS_TRANSPARENT_3 16094
#define MSG_PS_TRANSPARENT_3_STR "Color 0"

#define MSG_PS_OPTIONS_SYSTEM 16095
#define MSG_PS_OPTIONS_SYSTEM_STR " \000Measuring System"

#define MSG_PS_OPTIONS_CM 16096
#define MSG_PS_OPTIONS_CM_STR " \000Centimeters"

#define MSG_PS_OPTIONS_INCHES 16097
#define MSG_PS_OPTIONS_INCHES_STR " \000Inches"

#define MSG_PS_OPTIONS_POINTS 16098
#define MSG_PS_OPTIONS_POINTS_STR " \000Points"

#endif /* PRINTERPS_PREFS */

#ifdef POINTER_PREFS
#define MSG_PTR_NAME 17000
#define MSG_PTR_NAME_STR "Pointer Preferences"

#define MSG_PTR_NO_DATATYPES_LIBRARY 17001
#define MSG_PTR_NO_DATATYPES_LIBRARY_STR "Requires datatypes.library V39"

#define MSG_PTR_OPEN_CLIPBOARD_STAT 17002
#define MSG_PTR_OPEN_CLIPBOARD_STAT_STR "Could not open clipboard"

#define MSG_PTR_PASTE_GRAPHIC_STAT 17003
#define MSG_PTR_PASTE_GRAPHIC_STAT_STR "Clipboard does not contain graphics"

#define MSG_PTR_NO_GRAPHIC_STAT 17004
#define MSG_PTR_NO_GRAPHIC_STAT_STR "Does not contain graphics"

#define MSG_PTR_REQ_LOAD 17005
#define MSG_PTR_REQ_LOAD_STR "Load Pointer Preferences"

#define MSG_PTR_REQ_SAVE 17006
#define MSG_PTR_REQ_SAVE_STR "Save Pointer Preferences"

#define MSG_PTR_TEST_GAD 17007
#define MSG_PTR_TEST_GAD_STR "Test"

#define MSG_PTR_CLEAR_GAD 17008
#define MSG_PTR_CLEAR_GAD_STR "Clear"

#define MSG_PTR_SET_GAD 17009
#define MSG_PTR_SET_GAD_STR "Set Point"

#define MSG_PTR_RESET_GAD 17010
#define MSG_PTR_RESET_GAD_STR "Reset Colors"

#define MSG_PTR_RED_GAD 17011
#define MSG_PTR_RED_GAD_STR "Red:    \0"

#define MSG_PTR_GREEN_GAD 17012
#define MSG_PTR_GREEN_GAD_STR "Green:    \0"

#define MSG_PTR_BLUE_GAD 17013
#define MSG_PTR_BLUE_GAD_STR "Blue:    \0"

#define MSG_PTR_TYPE_GAD 17014
#define MSG_PTR_TYPE_GAD_STR "Type:"

#define MSG_PTR_NORMAL_GAD 17015
#define MSG_PTR_NORMAL_GAD_STR "Normal"

#define MSG_PTR_BUSY_GAD 17016
#define MSG_PTR_BUSY_GAD_STR "Busy"

#define MSG_PTR_SIZE_GAD 17017
#define MSG_PTR_SIZE_GAD_STR "Size:"

#define MSG_PTR_LORES_GAD 17018
#define MSG_PTR_LORES_GAD_STR "Low Res"

#define MSG_PTR_HIRES_GAD 17019
#define MSG_PTR_HIRES_GAD_STR "High Res"

#define MSG_PTR_SHIRES_GAD 17020
#define MSG_PTR_SHIRES_GAD_STR "Super-High Res"

#define MSG_PTR_EDIT_LOAD_IMAGE 17021
#define MSG_PTR_EDIT_LOAD_IMAGE_STR "J\000Load Image..."

#define MSG_PTR_REQ_LOAD_IMAGE 17022
#define MSG_PTR_REQ_LOAD_IMAGE_STR "Select Image to Open"

#endif /* POINTER_PREFS */

#ifdef WBPATTERN_PREFS
#define MSG_WBP_NAME 18000
#define MSG_WBP_NAME_STR "WBPattern Preferences"

#define MSG_WBP_NO_DATATYPES_LIBRARY 18001
#define MSG_WBP_NO_DATATYPES_LIBRARY_STR "Requires datatypes.library V39"

#define MSG_WBP_OPEN_CLIPBOARD_STAT 18002
#define MSG_WBP_OPEN_CLIPBOARD_STAT_STR "Could not open clipboard"

#define MSG_WBP_PASTE_GRAPHIC_STAT 18003
#define MSG_WBP_PASTE_GRAPHIC_STAT_STR "Clipboard does not contain graphics"

#define MSG_WBP_NO_GRAPHIC_STAT 18004
#define MSG_WBP_NO_GRAPHIC_STAT_STR "Does not contain graphics"

#define MSG_WBP_REQ_LOAD 18005
#define MSG_WBP_REQ_LOAD_STR "Load Backdrop Pattern"

#define MSG_WBP_REQ_SAVE 18006
#define MSG_WBP_REQ_SAVE_STR "Save Backdrop Pattern"

#define MSG_WBP_REQ_WIN_LOAD 18007
#define MSG_WBP_REQ_WIN_LOAD_STR "Load Windows Pattern"

#define MSG_WBP_REQ_WIN_SAVE 18008
#define MSG_WBP_REQ_WIN_SAVE_STR "Save Windows Pattern"

#define MSG_WBP_PATTERN_LABEL 18009
#define MSG_WBP_PATTERN_LABEL_STR "Pattern"

#define MSG_WBP_WORKBENCH_GAD 18010
#define MSG_WBP_WORKBENCH_GAD_STR "Workbench"

#define MSG_WBP_WINDOWS_GAD 18011
#define MSG_WBP_WINDOWS_GAD_STR "Windows"

#define MSG_WBP_TEST_GAD 18012
#define MSG_WBP_TEST_GAD_STR "Test"

#define MSG_WBP_CLEAR_GAD 18013
#define MSG_WBP_CLEAR_GAD_STR "Clear"

#define MSG_WBP_UNDO_GAD 18014
#define MSG_WBP_UNDO_GAD_STR "Undo"

#define MSG_WBP_PRESETS_LABEL 18015
#define MSG_WBP_PRESETS_LABEL_STR "Presets"

#define MSG_WBP_SCREEN_GAD 18016
#define MSG_WBP_SCREEN_GAD_STR "Screen"

#define MSG_WBP_REQ_SCREEN_LOAD 18017
#define MSG_WBP_REQ_SCREEN_LOAD_STR "Load Screen Pattern"

#define MSG_WBP_REQ_SCREEN_SAVE 18018
#define MSG_WBP_REQ_SCREEN_SAVE_STR "Save Screen Pattern"

#define MSG_WBP_SELECT_PICTURE_GAD 18019
#define MSG_WBP_SELECT_PICTURE_GAD_STR "Select Picture..."

#define MSG_WBP_PICTURE_NAME_GAD 18020
#define MSG_WBP_PICTURE_NAME_GAD_STR "Picture Name:"

#define MSG_WBP_PLACEMENT_GAD 18021
#define MSG_WBP_PLACEMENT_GAD_STR "Placement:"

#define MSG_WBP_TYPE_GAD 18022
#define MSG_WBP_TYPE_GAD_STR "Type:"

#define MSG_WBP_TYPE_PICTURE_GAD 18023
#define MSG_WBP_TYPE_PICTURE_GAD_STR "Picture"

#define MSG_WBP_TYPE_PATTERN_GAD 18024
#define MSG_WBP_TYPE_PATTERN_GAD_STR "Pattern"

#define MSG_WBP_SELECT_PICTURE_LABEL 18025
#define MSG_WBP_SELECT_PICTURE_LABEL_STR "Select Picture"

#define MSG_WBP_EDIT_LOAD_IMAGE 18026
#define MSG_WBP_EDIT_LOAD_IMAGE_STR "J\000Load Image..."

#define MSG_WBP_REQ_LOAD_IMAGE 18027
#define MSG_WBP_REQ_LOAD_IMAGE_STR "Select Image to Open"

#endif /* WBPATTERN_PREFS */

#ifdef SOUND_PREFS
#define MSG_SND_NAME 19000
#define MSG_SND_NAME_STR "Sound Preferences"

#define MSG_SND_NO_DATATYPES_LIBRARY 19001
#define MSG_SND_NO_DATATYPES_LIBRARY_STR "Requires datatypes.library V39"

#define MSG_SND_IFF_NOT_8SVX_STAT 19002
#define MSG_SND_IFF_NOT_8SVX_STAT_STR "File is not a valid\nsampled sound"

#define MSG_SND_REQ_LOAD 19003
#define MSG_SND_REQ_LOAD_STR "Load Sound Preferences"

#define MSG_SND_REQ_SAVE 19004
#define MSG_SND_REQ_SAVE_STR "Save Sound Preferences"

#define MSG_SND_REQ_LOAD_SOUND 19005
#define MSG_SND_REQ_LOAD_SOUND_STR "Select IFF Sampled Sound"

#define MSG_SND_FLASH_GAD 19006
#define MSG_SND_FLASH_GAD_STR "Flash Display"

#define MSG_SND_MAKESOUND_GAD 19007
#define MSG_SND_MAKESOUND_GAD_STR "Make Sound"

#define MSG_SND_SOUNDTYPE_GAD 19008
#define MSG_SND_SOUNDTYPE_GAD_STR "Sound Type:"

#define MSG_SND_BEEP 19009
#define MSG_SND_BEEP_STR "Beep"

#define MSG_SND_SAMPLEDSOUND 19010
#define MSG_SND_SAMPLEDSOUND_STR "Sampled Sound"

#define MSG_SND_TEST_GAD 19011
#define MSG_SND_TEST_GAD_STR "Test Sound"

#define MSG_SND_VOLUME_GAD 19012
#define MSG_SND_VOLUME_GAD_STR "Sound Volume:      \0"

#define MSG_SND_PITCH_GAD 19013
#define MSG_SND_PITCH_GAD_STR "Sound Pitch:      \0"

#define MSG_SND_DURATION_GAD 19014
#define MSG_SND_DURATION_GAD_STR "Beep Length:      \0"

#define MSG_SND_SAMPLENAME_GAD 19015
#define MSG_SND_SAMPLENAME_GAD_STR "Sample Name:"

#define MSG_SND_SELECTSAMPLE_GAD 19016
#define MSG_SND_SELECTSAMPLE_GAD_STR "Select Sample..."

#endif /* SOUND_PREFS */

#ifdef PALETTE_PREFS
#define MSG_PAL_NAME 20000
#define MSG_PAL_NAME_STR "Palette Preferences"

#define MSG_PAL_NO_COLORWHEEL 20001
#define MSG_PAL_NO_COLORWHEEL_STR "Requires colorwheel.gadget V39"

#define MSG_PAL_NO_GRADIENT 20002
#define MSG_PAL_NO_GRADIENT_STR "Requires gradientslider.gadget V39"

#define MSG_PAL_REQ_LOAD 20003
#define MSG_PAL_REQ_LOAD_STR "Load Palette Preferences"

#define MSG_PAL_REQ_SAVE 20004
#define MSG_PAL_REQ_SAVE_STR "Save Palette Preferences"

#define MSG_PAL_EDIT_PRESETS 20005
#define MSG_PAL_EDIT_PRESETS_STR " \000Presets"

#define MSG_PAL_RED_GAD 20006
#define MSG_PAL_RED_GAD_STR "       Red:"

#define MSG_PAL_GREEN_GAD 20007
#define MSG_PAL_GREEN_GAD_STR "     Green:"

#define MSG_PAL_BLUE_GAD 20008
#define MSG_PAL_BLUE_GAD_STR "      Blue:"

#define MSG_PAL_HUE_GAD 20009
#define MSG_PAL_HUE_GAD_STR "       Hue:"

#define MSG_PAL_SATURATION_GAD 20010
#define MSG_PAL_SATURATION_GAD_STR "Saturation:"

#define MSG_PAL_BRIGHTNESS_GAD 20011
#define MSG_PAL_BRIGHTNESS_GAD_STR "Brightness:"

#define MSG_PAL_EDIT_PRESET1 20012
#define MSG_PAL_EDIT_PRESET1_STR "1\0Tint"

#define MSG_PAL_EDIT_PRESET2 20013
#define MSG_PAL_EDIT_PRESET2_STR "2\0Pharaoh"

#define MSG_PAL_EDIT_PRESET3 20014
#define MSG_PAL_EDIT_PRESET3_STR "3\0Sunset"

#define MSG_PAL_EDIT_PRESET4 20015
#define MSG_PAL_EDIT_PRESET4_STR "4\0Ocean"

#define MSG_PAL_EDIT_PRESET5 20016
#define MSG_PAL_EDIT_PRESET5_STR "5\0Steel"

#define MSG_PAL_EDIT_PRESET6 20017
#define MSG_PAL_EDIT_PRESET6_STR "6\0Chocolate"

#define MSG_PAL_EDIT_PRESET7 20018
#define MSG_PAL_EDIT_PRESET7_STR "7\0Pewter"

#define MSG_PAL_EDIT_PRESET8 20019
#define MSG_PAL_EDIT_PRESET8_STR "8\0Wine"

#define MSG_PAL_EDIT_PRESET9 20020
#define MSG_PAL_EDIT_PRESET9_STR "9\0A2024"

#define MSG_PAL_OPTIONS_COLORMODEL 20021
#define MSG_PAL_OPTIONS_COLORMODEL_STR " \0Slider Color Model"

#define MSG_PAL_OPTIONS_RGB 20022
#define MSG_PAL_OPTIONS_RGB_STR " \0RGB"

#define MSG_PAL_OPTIONS_HSB 20023
#define MSG_PAL_OPTIONS_HSB_STR " \0HSB"

#define MSG_PAL_SHOW_SAMPLE 20024
#define MSG_PAL_SHOW_SAMPLE_STR "Show Sample..."

#define MSG_PAL_BACKGROUND_PEN 20025
#define MSG_PAL_BACKGROUND_PEN_STR "Background"

#define MSG_PAL_TEXT_PEN 20026
#define MSG_PAL_TEXT_PEN_STR "Text"

#define MSG_PAL_HIGHLIGHT_PEN 20027
#define MSG_PAL_HIGHLIGHT_PEN_STR "Important Text"

#define MSG_PAL_SHINE_PEN 20028
#define MSG_PAL_SHINE_PEN_STR "Bright Edges"

#define MSG_PAL_SHADOW_PEN 20029
#define MSG_PAL_SHADOW_PEN_STR "Dark Edges"

#define MSG_PAL_FILL_PEN 20030
#define MSG_PAL_FILL_PEN_STR "Active Window Titlebars"

#define MSG_PAL_FILLTEXT_PEN 20031
#define MSG_PAL_FILLTEXT_PEN_STR "Active Window Titles"

#define MSG_PAL_BARBLOCK_PEN 20032
#define MSG_PAL_BARBLOCK_PEN_STR "Menu Background"

#define MSG_PAL_BARDETAIL_PEN 20033
#define MSG_PAL_BARDETAIL_PEN_STR "Menu Text"

#define MSG_PAL_SAMPLE_TITLE 20034
#define MSG_PAL_SAMPLE_TITLE_STR "Sample Palette"

#define MSG_PAL_SAMPLE_MENU 20035
#define MSG_PAL_SAMPLE_MENU_STR "Restaurant"

#define MSG_PAL_SAMPLE_ITEM_1 20036
#define MSG_PAL_SAMPLE_ITEM_1_STR "S\000Order Soup..."

#define MSG_PAL_SAMPLE_ITEM_2 20037
#define MSG_PAL_SAMPLE_ITEM_2_STR "B\000Order Bread..."

#define MSG_PAL_SAMPLE_ITEM_3 20038
#define MSG_PAL_SAMPLE_ITEM_3_STR "M\000Start Meal"

#define MSG_PAL_SAMPLE_ITEM_4 20039
#define MSG_PAL_SAMPLE_ITEM_4_STR "T\000Stop Meal"

#define MSG_PAL_SAMPLE_ITEM_5 20040
#define MSG_PAL_SAMPLE_ITEM_5_STR " \000Overeat?"

#define MSG_PAL_SAMPLE_ITEM_6 20041
#define MSG_PAL_SAMPLE_ITEM_6_STR "P\000Pay Bill"

#define MSG_PAL_SAMPLE_ACTIVE 20042
#define MSG_PAL_SAMPLE_ACTIVE_STR "Active"

#define MSG_PAL_SAMPLE_INACTIVE 20043
#define MSG_PAL_SAMPLE_INACTIVE_STR "Inactive"

#define MSG_PAL_SAMPLE_SPECIAL 20044
#define MSG_PAL_SAMPLE_SPECIAL_STR "Special Offer!"

#define MSG_PAL_SAMPLE_BUTTON 20045
#define MSG_PAL_SAMPLE_BUTTON_STR "Eat Food"

#define MSG_PAL_SAMPLE_STRING 20046
#define MSG_PAL_SAMPLE_STRING_STR "Calories:"

#define MSG_PAL_SAMPLE_NUMBER 20047
#define MSG_PAL_SAMPLE_NUMBER_STR "9674"

#define MSG_PAL_SAMPLE_FOOD_1 20048
#define MSG_PAL_SAMPLE_FOOD_1_STR "Bacon"

#define MSG_PAL_SAMPLE_FOOD_2 20049
#define MSG_PAL_SAMPLE_FOOD_2_STR "Lettuce"

#define MSG_PAL_SAMPLE_FOOD_3 20050
#define MSG_PAL_SAMPLE_FOOD_3_STR "Tomatoes"

#define MSG_PAL_SAMPLE_SALT 20051
#define MSG_PAL_SAMPLE_SALT_STR "Salt"

#define MSG_PAL_WHEEL_ABBRV 20052
#define MSG_PAL_WHEEL_ABBRV_STR "GCBMRY"

#define MSG_PAL_4COLOR_MODE 20053
#define MSG_PAL_4COLOR_MODE_STR "4 Color Settings"

#define MSG_PAL_MULTICOLOR_MODE 20054
#define MSG_PAL_MULTICOLOR_MODE_STR "Multicolor Settings"

#endif /* PALETTE_PREFS */


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
    {MSG_PROGRAM_ERROR,(STRPTR)MSG_PROGRAM_ERROR_STR},
    {MSG_NO_ASL_LIBRARY,(STRPTR)MSG_NO_ASL_LIBRARY_STR},
    {MSG_NO_IFFPARSE_LIBRARY,(STRPTR)MSG_NO_IFFPARSE_LIBRARY_STR},
    {MSG_PROJECT_MENU,(STRPTR)MSG_PROJECT_MENU_STR},
    {MSG_PROJECT_OPEN,(STRPTR)MSG_PROJECT_OPEN_STR},
    {MSG_PROJECT_SAVE_AS,(STRPTR)MSG_PROJECT_SAVE_AS_STR},
    {MSG_PROJECT_QUIT,(STRPTR)MSG_PROJECT_QUIT_STR},
    {MSG_EDIT_MENU,(STRPTR)MSG_EDIT_MENU_STR},
    {MSG_EDIT_CUT,(STRPTR)MSG_EDIT_CUT_STR},
    {MSG_EDIT_COPY,(STRPTR)MSG_EDIT_COPY_STR},
    {MSG_EDIT_PASTE,(STRPTR)MSG_EDIT_PASTE_STR},
    {MSG_EDIT_ERASE,(STRPTR)MSG_EDIT_ERASE_STR},
    {MSG_EDIT_UNDO,(STRPTR)MSG_EDIT_UNDO_STR},
    {MSG_EDIT_RESET_TO_DEFAULTS,(STRPTR)MSG_EDIT_RESET_TO_DEFAULTS_STR},
    {MSG_EDIT_LAST_SAVED,(STRPTR)MSG_EDIT_LAST_SAVED_STR},
    {MSG_EDIT_RESTORE,(STRPTR)MSG_EDIT_RESTORE_STR},
    {MSG_OPTIONS_MENU,(STRPTR)MSG_OPTIONS_MENU_STR},
    {MSG_OPTIONS_SAVE_ICONS,(STRPTR)MSG_OPTIONS_SAVE_ICONS_STR},
    {MSG_SAVE_GAD,(STRPTR)MSG_SAVE_GAD_STR},
    {MSG_USE_GAD,(STRPTR)MSG_USE_GAD_STR},
    {MSG_CANCEL_GAD,(STRPTR)MSG_CANCEL_GAD_STR},
    {MSG_OK_GAD,(STRPTR)MSG_OK_GAD_STR},
    {MSG_NORMAL_STAT,(STRPTR)MSG_NORMAL_STAT_STR},
    {MSG_NO_MEMORY_STAT,(STRPTR)MSG_NO_MEMORY_STAT_STR},
    {MSG_IFF_UNKNOWNCHUNK_STAT,(STRPTR)MSG_IFF_UNKNOWNCHUNK_STAT_STR},
    {MSG_IFF_UNKNOWNVERSION_STAT,(STRPTR)MSG_IFF_UNKNOWNVERSION_STAT_STR},
    {MSG_IFFERROR_STAT,(STRPTR)MSG_IFFERROR_STAT_STR},
    {MSG_DOSERROR_STAT,(STRPTR)MSG_DOSERROR_STAT_STR},
#ifdef FONT_PREFS
    {MSG_FNT_NAME,(STRPTR)MSG_FNT_NAME_STR},
    {MSG_FNT_REQ_LOAD,(STRPTR)MSG_FNT_REQ_LOAD_STR},
    {MSG_FNT_REQ_SAVE,(STRPTR)MSG_FNT_REQ_SAVE_STR},
    {MSG_FNT_FONTS_HDR,(STRPTR)MSG_FNT_FONTS_HDR_STR},
    {MSG_FNT_WBFONT,(STRPTR)MSG_FNT_WBFONT_STR},
    {MSG_FNT_SYSFONT,(STRPTR)MSG_FNT_SYSFONT_STR},
    {MSG_FNT_SCREENFONT,(STRPTR)MSG_FNT_SCREENFONT_STR},
    {MSG_FNT_WBFONT_GAD,(STRPTR)MSG_FNT_WBFONT_GAD_STR},
    {MSG_FNT_SYSFONT_GAD,(STRPTR)MSG_FNT_SYSFONT_GAD_STR},
    {MSG_FNT_SCREENFONT_GAD,(STRPTR)MSG_FNT_SCREENFONT_GAD_STR},
    {MSG_FNT_WBFONT_TITLE,(STRPTR)MSG_FNT_WBFONT_TITLE_STR},
    {MSG_FNT_SYSFONT_TITLE,(STRPTR)MSG_FNT_SYSFONT_TITLE_STR},
    {MSG_FNT_SCREENFONT_TITLE,(STRPTR)MSG_FNT_SCREENFONT_TITLE_STR},
    {MSG_FNT_MODE_GAD,(STRPTR)MSG_FNT_MODE_GAD_STR},
    {MSG_FNT_TEXT,(STRPTR)MSG_FNT_TEXT_STR},
    {MSG_FNT_TEXTFIELD,(STRPTR)MSG_FNT_TEXTFIELD_STR},
#endif /* FONT_PREFS */

#ifdef ICONTROL_PREFS
    {MSG_ICTL_NAME,(STRPTR)MSG_ICTL_NAME_STR},
    {MSG_ICTL_REQ_LOAD,(STRPTR)MSG_ICTL_REQ_LOAD_STR},
    {MSG_ICTL_REQ_SAVE,(STRPTR)MSG_ICTL_REQ_SAVE_STR},
    {MSG_ICTL_SHIFT_GAD,(STRPTR)MSG_ICTL_SHIFT_GAD_STR},
    {MSG_ICTL_CTRL_GAD,(STRPTR)MSG_ICTL_CTRL_GAD_STR},
    {MSG_ICTL_ALT_GAD,(STRPTR)MSG_ICTL_ALT_GAD_STR},
    {MSG_ICTL_LAMIGA_GAD,(STRPTR)MSG_ICTL_LAMIGA_GAD_STR},
    {MSG_ICTL_AVOIDFLICKER_GAD,(STRPTR)MSG_ICTL_AVOIDFLICKER_GAD_STR},
    {MSG_ICTL_MENUSNAP_GAD,(STRPTR)MSG_ICTL_MENUSNAP_GAD_STR},
    {MSG_ICTL_GADGETFILTER_GAD,(STRPTR)MSG_ICTL_GADGETFILTER_GAD_STR},
    {MSG_ICTL_MODEPROMOTE_GAD,(STRPTR)MSG_ICTL_MODEPROMOTE_GAD_STR},
    {MSG_ICTL_SCRDRAG_HDR,(STRPTR)MSG_ICTL_SCRDRAG_HDR_STR},
    {MSG_ICTL_MISC_HDR,(STRPTR)MSG_ICTL_MISC_HDR_STR},
#endif /* ICONTROL_PREFS */

#ifdef INPUT_PREFS
    {MSG_INP_NAME,(STRPTR)MSG_INP_NAME_STR},
    {MSG_INP_ERROR_NO_KEYMAP,(STRPTR)MSG_INP_ERROR_NO_KEYMAP_STR},
    {MSG_INP_REQ_LOAD,(STRPTR)MSG_INP_REQ_LOAD_STR},
    {MSG_INP_REQ_SAVE,(STRPTR)MSG_INP_REQ_SAVE_STR},
    {MSG_INP_MOUSE_HDR,(STRPTR)MSG_INP_MOUSE_HDR_STR},
    {MSG_INP_MOUSESPEED_GAD,(STRPTR)MSG_INP_MOUSESPEED_GAD_STR},
    {MSG_INP_ACCELERATION_GAD,(STRPTR)MSG_INP_ACCELERATION_GAD_STR},
    {MSG_INP_DOUBLECLICK_GAD,(STRPTR)MSG_INP_DOUBLECLICK_GAD_STR},
    {MSG_INP_SHOW_GAD,(STRPTR)MSG_INP_SHOW_GAD_STR},
    {MSG_INP_TEST_GAD,(STRPTR)MSG_INP_TEST_GAD_STR},
    {MSG_INP_KEYBOARD_HDR,(STRPTR)MSG_INP_KEYBOARD_HDR_STR},
    {MSG_INP_KEYDELAY_GAD,(STRPTR)MSG_INP_KEYDELAY_GAD_STR},
    {MSG_INP_KEYRATE_GAD,(STRPTR)MSG_INP_KEYRATE_GAD_STR},
    {MSG_INP_KEYTEST_GAD,(STRPTR)MSG_INP_KEYTEST_GAD_STR},
    {MSG_INP_KEYBOARDTYPE_GAD,(STRPTR)MSG_INP_KEYBOARDTYPE_GAD_STR},
    {MSG_INP_CLICKAGAIN,(STRPTR)MSG_INP_CLICKAGAIN_STR},
    {MSG_INP_DCYES,(STRPTR)MSG_INP_DCYES_STR},
    {MSG_INP_DCNO,(STRPTR)MSG_INP_DCNO_STR},
#endif /* INPUT_PREFS */

#ifdef LOCALE_PREFS
    {MSG_LOC_NAME,(STRPTR)MSG_LOC_NAME_STR},
    {MSG_LOC_REQ_LOAD,(STRPTR)MSG_LOC_REQ_LOAD_STR},
    {MSG_LOC_REQ_SAVE,(STRPTR)MSG_LOC_REQ_SAVE_STR},
    {MSG_LOC_AVAILLANG_GAD,(STRPTR)MSG_LOC_AVAILLANG_GAD_STR},
    {MSG_LOC_PREFLANG_GAD,(STRPTR)MSG_LOC_PREFLANG_GAD_STR},
    {MSG_LOC_CLEARLANG_GAD,(STRPTR)MSG_LOC_CLEARLANG_GAD_STR},
    {MSG_LOC_COUNTRY_GAD,(STRPTR)MSG_LOC_COUNTRY_GAD_STR},
    {MSG_LOC_TIMEZONE_BOX,(STRPTR)MSG_LOC_TIMEZONE_BOX_STR},
    {MSG_LOC_HR,(STRPTR)MSG_LOC_HR_STR},
    {MSG_LOC_HR_MIN,(STRPTR)MSG_LOC_HR_MIN_STR},
    {MSG_LOC_GMT,(STRPTR)MSG_LOC_GMT_STR},
#endif /* LOCALE_PREFS */

#ifdef OVERSCAN_PREFS
    {MSG_OSCN_NAME,(STRPTR)MSG_OSCN_NAME_STR},
    {MSG_OSCN_REQ_LOAD,(STRPTR)MSG_OSCN_REQ_LOAD_STR},
    {MSG_OSCN_REQ_SAVE,(STRPTR)MSG_OSCN_REQ_SAVE_STR},
    {MSG_OSCN_MONITORS_GAD,(STRPTR)MSG_OSCN_MONITORS_GAD_STR},
    {MSG_OSCN_EDITTEXT_GAD,(STRPTR)MSG_OSCN_EDITTEXT_GAD_STR},
    {MSG_OSCN_EDITGFX_GAD,(STRPTR)MSG_OSCN_EDITGFX_GAD_STR},
    {MSG_OSCN_MINSIZE,(STRPTR)MSG_OSCN_MINSIZE_STR},
    {MSG_OSCN_TEXTSIZE,(STRPTR)MSG_OSCN_TEXTSIZE_STR},
    {MSG_OSCN_GFXSIZE,(STRPTR)MSG_OSCN_GFXSIZE_STR},
    {MSG_OSCN_MAXSIZE,(STRPTR)MSG_OSCN_MAXSIZE_STR},
    {MSG_OSCN_MONITOR,(STRPTR)MSG_OSCN_MONITOR_STR},
    {MSG_OSCN_TEXTEDIT,(STRPTR)MSG_OSCN_TEXTEDIT_STR},
    {MSG_OSCN_GFXEDIT,(STRPTR)MSG_OSCN_GFXEDIT_STR},
    {MSG_OSCN_EDITSIZE,(STRPTR)MSG_OSCN_EDITSIZE_STR},
    {MSG_OSCN_EDITPOS,(STRPTR)MSG_OSCN_EDITPOS_STR},
    {MSG_OSCN_OVERSCAN_DIM_HDR,(STRPTR)MSG_OSCN_OVERSCAN_DIM_HDR_STR},
#endif /* OVERSCAN_PREFS */

#ifdef PRINTER_PREFS
    {MSG_PRT_NAME,(STRPTR)MSG_PRT_NAME_STR},
    {MSG_PRT_REQ_LOAD,(STRPTR)MSG_PRT_REQ_LOAD_STR},
    {MSG_PRT_REQ_SAVE,(STRPTR)MSG_PRT_REQ_SAVE_STR},
    {MSG_PRT_PRINTERTYPE_GAD,(STRPTR)MSG_PRT_PRINTERTYPE_GAD_STR},
    {MSG_PRT_PRINTERPORT_GAD,(STRPTR)MSG_PRT_PRINTERPORT_GAD_STR},
    {MSG_PRT_PAPERTYPE_GAD,(STRPTR)MSG_PRT_PAPERTYPE_GAD_STR},
    {MSG_PRT_PAPERSIZE_GAD,(STRPTR)MSG_PRT_PAPERSIZE_GAD_STR},
    {MSG_PRT_PRINTPITCH_GAD,(STRPTR)MSG_PRT_PRINTPITCH_GAD_STR},
    {MSG_PRT_PRINTSPACING_GAD,(STRPTR)MSG_PRT_PRINTSPACING_GAD_STR},
    {MSG_PRT_PRINTQUALITY_GAD,(STRPTR)MSG_PRT_PRINTQUALITY_GAD_STR},
    {MSG_PRT_PAPERLENGTH_GAD,(STRPTR)MSG_PRT_PAPERLENGTH_GAD_STR},
    {MSG_PRT_LEFTMARGIN_GAD,(STRPTR)MSG_PRT_LEFTMARGIN_GAD_STR},
    {MSG_PRT_RIGHTMARGIN_GAD,(STRPTR)MSG_PRT_RIGHTMARGIN_GAD_STR},
    {MSG_PRT_PARALLEL,(STRPTR)MSG_PRT_PARALLEL_STR},
    {MSG_PRT_SERIAL,(STRPTR)MSG_PRT_SERIAL_STR},
    {MSG_PRT_SINGLE,(STRPTR)MSG_PRT_SINGLE_STR},
    {MSG_PRT_FANFOLD,(STRPTR)MSG_PRT_FANFOLD_STR},
    {MSG_PRT_USLETTER,(STRPTR)MSG_PRT_USLETTER_STR},
    {MSG_PRT_USLEGAL,(STRPTR)MSG_PRT_USLEGAL_STR},
    {MSG_PRT_NARROWTRACTOR,(STRPTR)MSG_PRT_NARROWTRACTOR_STR},
    {MSG_PRT_WIDETRACTOR,(STRPTR)MSG_PRT_WIDETRACTOR_STR},
    {MSG_PRT_CUSTOM,(STRPTR)MSG_PRT_CUSTOM_STR},
    {MSG_PRT_DINA4,(STRPTR)MSG_PRT_DINA4_STR},
    {MSG_PRT_PICA10,(STRPTR)MSG_PRT_PICA10_STR},
    {MSG_PRT_ELITE12,(STRPTR)MSG_PRT_ELITE12_STR},
    {MSG_PRT_FINE15,(STRPTR)MSG_PRT_FINE15_STR},
    {MSG_PRT_LPI6,(STRPTR)MSG_PRT_LPI6_STR},
    {MSG_PRT_LPI8,(STRPTR)MSG_PRT_LPI8_STR},
    {MSG_PRT_DRAFT,(STRPTR)MSG_PRT_DRAFT_STR},
    {MSG_PRT_LETTER,(STRPTR)MSG_PRT_LETTER_STR},
    {MSG_PRT_DEVICEUNIT_GAD,(STRPTR)MSG_PRT_DEVICEUNIT_GAD_STR},
#endif /* PRINTER_PREFS */

#ifdef PRINTERGFX_PREFS
    {MSG_PGFX_NAME,(STRPTR)MSG_PGFX_NAME_STR},
    {MSG_PGFX_REQ_LOAD,(STRPTR)MSG_PGFX_REQ_LOAD_STR},
    {MSG_PGFX_REQ_SAVE,(STRPTR)MSG_PGFX_REQ_SAVE_STR},
    {MSG_PGFX_RED_GAD,(STRPTR)MSG_PGFX_RED_GAD_STR},
    {MSG_PGFX_GREEN_GAD,(STRPTR)MSG_PGFX_GREEN_GAD_STR},
    {MSG_PGFX_BLUE_GAD,(STRPTR)MSG_PGFX_BLUE_GAD_STR},
    {MSG_PGFX_COLORS_GAD,(STRPTR)MSG_PGFX_COLORS_GAD_STR},
    {MSG_PGFX_SMOOTHING_GAD,(STRPTR)MSG_PGFX_SMOOTHING_GAD_STR},
    {MSG_PGFX_DITHERING_GAD,(STRPTR)MSG_PGFX_DITHERING_GAD_STR},
    {MSG_PGFX_SCALING_GAD,(STRPTR)MSG_PGFX_SCALING_GAD_STR},
    {MSG_PGFX_IMAGE_GAD,(STRPTR)MSG_PGFX_IMAGE_GAD_STR},
    {MSG_PGFX_ASPECT_GAD,(STRPTR)MSG_PGFX_ASPECT_GAD_STR},
    {MSG_PGFX_SHADE_GAD,(STRPTR)MSG_PGFX_SHADE_GAD_STR},
    {MSG_PGFX_THRESHOLD_GAD,(STRPTR)MSG_PGFX_THRESHOLD_GAD_STR},
    {MSG_PGFX_LEFTOFFSET0_GAD,(STRPTR)MSG_PGFX_LEFTOFFSET0_GAD_STR},
    {MSG_PGFX_LEFTOFFSET1_GAD,(STRPTR)MSG_PGFX_LEFTOFFSET1_GAD_STR},
    {MSG_PGFX_CENTERPICTURE_GAD,(STRPTR)MSG_PGFX_CENTERPICTURE_GAD_STR},
    {MSG_PGFX_TYPE_GAD,(STRPTR)MSG_PGFX_TYPE_GAD_STR},
    {MSG_PGFX_WIDTH0_GAD,(STRPTR)MSG_PGFX_WIDTH0_GAD_STR},
    {MSG_PGFX_HEIGHT0_GAD,(STRPTR)MSG_PGFX_HEIGHT0_GAD_STR},
    {MSG_PGFX_WIDTH1_GAD,(STRPTR)MSG_PGFX_WIDTH1_GAD_STR},
    {MSG_PGFX_HEIGHT1_GAD,(STRPTR)MSG_PGFX_HEIGHT1_GAD_STR},
    {MSG_PGFX_WIDTH2_GAD,(STRPTR)MSG_PGFX_WIDTH2_GAD_STR},
    {MSG_PGFX_HEIGHT2_GAD,(STRPTR)MSG_PGFX_HEIGHT2_GAD_STR},
    {MSG_PGFX_WIDTH3_GAD,(STRPTR)MSG_PGFX_WIDTH3_GAD_STR},
    {MSG_PGFX_HEIGHT3_GAD,(STRPTR)MSG_PGFX_HEIGHT3_GAD_STR},
    {MSG_PGFX_WIDTH4_GAD,(STRPTR)MSG_PGFX_WIDTH4_GAD_STR},
    {MSG_PGFX_HEIGHT4_GAD,(STRPTR)MSG_PGFX_HEIGHT4_GAD_STR},
    {MSG_PGFX_DENSITY_GAD,(STRPTR)MSG_PGFX_DENSITY_GAD_STR},
    {MSG_PGFX_COLORCORRECT_HDR,(STRPTR)MSG_PGFX_COLORCORRECT_HDR_STR},
    {MSG_PGFX_LIMITS_HDR,(STRPTR)MSG_PGFX_LIMITS_HDR_STR},
    {MSG_PGFX_ORDERED,(STRPTR)MSG_PGFX_ORDERED_STR},
    {MSG_PGFX_HALFTONE,(STRPTR)MSG_PGFX_HALFTONE_STR},
    {MSG_PGFX_FLOYDSTEINBERG,(STRPTR)MSG_PGFX_FLOYDSTEINBERG_STR},
    {MSG_PGFX_FRACTION,(STRPTR)MSG_PGFX_FRACTION_STR},
    {MSG_PGFX_INTEGER,(STRPTR)MSG_PGFX_INTEGER_STR},
    {MSG_PGFX_POSITIVE,(STRPTR)MSG_PGFX_POSITIVE_STR},
    {MSG_PGFX_NEGATIVE,(STRPTR)MSG_PGFX_NEGATIVE_STR},
    {MSG_PGFX_HORIZONTAL,(STRPTR)MSG_PGFX_HORIZONTAL_STR},
    {MSG_PGFX_VERTICAL,(STRPTR)MSG_PGFX_VERTICAL_STR},
    {MSG_PGFX_BLACK_AND_WHITE,(STRPTR)MSG_PGFX_BLACK_AND_WHITE_STR},
    {MSG_PGFX_GREY_SCALE_1,(STRPTR)MSG_PGFX_GREY_SCALE_1_STR},
    {MSG_PGFX_COLOR,(STRPTR)MSG_PGFX_COLOR_STR},
    {MSG_PGFX_GREY_SCALE_2,(STRPTR)MSG_PGFX_GREY_SCALE_2_STR},
    {MSG_PGFX_IGNORE,(STRPTR)MSG_PGFX_IGNORE_STR},
    {MSG_PGFX_BOUNDED,(STRPTR)MSG_PGFX_BOUNDED_STR},
    {MSG_PGFX_ABSOLUTE,(STRPTR)MSG_PGFX_ABSOLUTE_STR},
    {MSG_PGFX_PIXELS,(STRPTR)MSG_PGFX_PIXELS_STR},
    {MSG_PGFX_MULTIPLY,(STRPTR)MSG_PGFX_MULTIPLY_STR},
    {MSG_PGFX_OPTIONS_METRIC,(STRPTR)MSG_PGFX_OPTIONS_METRIC_STR},
#endif /* PRINTERGFX_PREFS */

#ifdef SCREENMODE_PREFS
    {MSG_SM_NAME,(STRPTR)MSG_SM_NAME_STR},
    {MSG_SM_REQ_LOAD,(STRPTR)MSG_SM_REQ_LOAD_STR},
    {MSG_SM_REQ_SAVE,(STRPTR)MSG_SM_REQ_SAVE_STR},
    {MSG_SM_DISPLAY_MODE_GAD,(STRPTR)MSG_SM_DISPLAY_MODE_GAD_STR},
    {MSG_SM_PROPS_GAD,(STRPTR)MSG_SM_PROPS_GAD_STR},
    {MSG_SM_WIDTH_GAD,(STRPTR)MSG_SM_WIDTH_GAD_STR},
    {MSG_SM_HEIGHT_GAD,(STRPTR)MSG_SM_HEIGHT_GAD_STR},
    {MSG_SM_DEFAULT_GAD,(STRPTR)MSG_SM_DEFAULT_GAD_STR},
    {MSG_SM_COLORS_GAD,(STRPTR)MSG_SM_COLORS_GAD_STR},
    {MSG_SM_AUTOSCROLL_GAD,(STRPTR)MSG_SM_AUTOSCROLL_GAD_STR},
    {MSG_SM_VISIBLESIZE_PROP,(STRPTR)MSG_SM_VISIBLESIZE_PROP_STR},
    {MSG_SM_MAXSIZE_PROP,(STRPTR)MSG_SM_MAXSIZE_PROP_STR},
    {MSG_SM_MINSIZE_PROP,(STRPTR)MSG_SM_MINSIZE_PROP_STR},
    {MSG_SM_MAXCOLORS_PROP,(STRPTR)MSG_SM_MAXCOLORS_PROP_STR},
    {MSG_SM_INTERLACE_PROP,(STRPTR)MSG_SM_INTERLACE_PROP_STR},
    {MSG_SM_ECS_PROP,(STRPTR)MSG_SM_ECS_PROP_STR},
    {MSG_SM_GLOCK_PROP,(STRPTR)MSG_SM_GLOCK_PROP_STR},
    {MSG_SM_NOGLOCK_PROP,(STRPTR)MSG_SM_NOGLOCK_PROP_STR},
    {MSG_SM_DRAG_PROP,(STRPTR)MSG_SM_DRAG_PROP_STR},
    {MSG_SM_NODRAG_PROP,(STRPTR)MSG_SM_NODRAG_PROP_STR},
#endif /* SCREENMODE_PREFS */

#ifdef SERIAL_PREFS
    {MSG_SER_NAME,(STRPTR)MSG_SER_NAME_STR},
    {MSG_SER_REQ_LOAD,(STRPTR)MSG_SER_REQ_LOAD_STR},
    {MSG_SER_REQ_SAVE,(STRPTR)MSG_SER_REQ_SAVE_STR},
    {MSG_SER_BAUDRATE_GAD,(STRPTR)MSG_SER_BAUDRATE_GAD_STR},
    {MSG_SER_INPUTBUF_GAD,(STRPTR)MSG_SER_INPUTBUF_GAD_STR},
    {MSG_SER_HANDSHAKING_GAD,(STRPTR)MSG_SER_HANDSHAKING_GAD_STR},
    {MSG_SER_PARITY_GAD,(STRPTR)MSG_SER_PARITY_GAD_STR},
    {MSG_SER_DATABITS_GAD,(STRPTR)MSG_SER_DATABITS_GAD_STR},
    {MSG_SER_STOPBITS_GAD,(STRPTR)MSG_SER_STOPBITS_GAD_STR},
    {MSG_SER_DEFAULTUNIT_GAD,(STRPTR)MSG_SER_DEFAULTUNIT_GAD_STR},
    {MSG_SER_XON_XOFF,(STRPTR)MSG_SER_XON_XOFF_STR},
    {MSG_SER_RTS_CTS,(STRPTR)MSG_SER_RTS_CTS_STR},
    {MSG_SER_NONE,(STRPTR)MSG_SER_NONE_STR},
    {MSG_SER_PNONE,(STRPTR)MSG_SER_PNONE_STR},
    {MSG_SER_EVEN,(STRPTR)MSG_SER_EVEN_STR},
    {MSG_SER_ODD,(STRPTR)MSG_SER_ODD_STR},
    {MSG_SER_MARK,(STRPTR)MSG_SER_MARK_STR},
    {MSG_SER_SPACE,(STRPTR)MSG_SER_SPACE_STR},
#endif /* SERIAL_PREFS */

#ifdef TIME_PREFS
    {MSG_TIME_NAME,(STRPTR)MSG_TIME_NAME_STR},
    {MSG_TIME_HOURS_GAD,(STRPTR)MSG_TIME_HOURS_GAD_STR},
    {MSG_TIME_MINUTES_GAD,(STRPTR)MSG_TIME_MINUTES_GAD_STR},
#endif /* TIME_PREFS */

#ifdef PRINTERPS_PREFS
    {MSG_PS_NAME,(STRPTR)MSG_PS_NAME_STR},
    {MSG_PS_REQ_LOAD,(STRPTR)MSG_PS_REQ_LOAD_STR},
    {MSG_PS_REQ_SAVE,(STRPTR)MSG_PS_REQ_SAVE_STR},
    {MSG_PS_DRIVERMODE_GAD,(STRPTR)MSG_PS_DRIVERMODE_GAD_STR},
    {MSG_PS_COPIES_GAD,(STRPTR)MSG_PS_COPIES_GAD_STR},
    {MSG_PS_PAPERFORMAT_GAD,(STRPTR)MSG_PS_PAPERFORMAT_GAD_STR},
    {MSG_PS_PAPERWIDTH_GAD,(STRPTR)MSG_PS_PAPERWIDTH_GAD_STR},
    {MSG_PS_PAPERHEIGHT_GAD,(STRPTR)MSG_PS_PAPERHEIGHT_GAD_STR},
    {MSG_PS_HORIZDPI_GAD,(STRPTR)MSG_PS_HORIZDPI_GAD_STR},
    {MSG_PS_VERTDPI_GAD,(STRPTR)MSG_PS_VERTDPI_GAD_STR},
    {MSG_PS_FONT_GAD,(STRPTR)MSG_PS_FONT_GAD_STR},
    {MSG_PS_PITCH_GAD,(STRPTR)MSG_PS_PITCH_GAD_STR},
    {MSG_PS_TAB_GAD,(STRPTR)MSG_PS_TAB_GAD_STR},
    {MSG_PS_LEFTMARGIN_GAD,(STRPTR)MSG_PS_LEFTMARGIN_GAD_STR},
    {MSG_PS_RIGHTMARGIN_GAD,(STRPTR)MSG_PS_RIGHTMARGIN_GAD_STR},
    {MSG_PS_TOPMARGIN_GAD,(STRPTR)MSG_PS_TOPMARGIN_GAD_STR},
    {MSG_PS_BOTTOMMARGIN_GAD,(STRPTR)MSG_PS_BOTTOMMARGIN_GAD_STR},
    {MSG_PS_FONTPOINTSIZE_GAD,(STRPTR)MSG_PS_FONTPOINTSIZE_GAD_STR},
    {MSG_PS_LINELEADING_GAD,(STRPTR)MSG_PS_LINELEADING_GAD_STR},
    {MSG_PS_LINESPERINCH_GAD,(STRPTR)MSG_PS_LINESPERINCH_GAD_STR},
    {MSG_PS_LINESPERPAGE_GAD,(STRPTR)MSG_PS_LINESPERPAGE_GAD_STR},
    {MSG_PS_ORIENTATION_GAD,(STRPTR)MSG_PS_ORIENTATION_GAD_STR},
    {MSG_PS_ASPECT_GAD,(STRPTR)MSG_PS_ASPECT_GAD_STR},
    {MSG_PS_SCALINGTYPE_GAD,(STRPTR)MSG_PS_SCALINGTYPE_GAD_STR},
    {MSG_PS_SCALINGMATH_GAD,(STRPTR)MSG_PS_SCALINGMATH_GAD_STR},
    {MSG_PS_CENTERING_GAD,(STRPTR)MSG_PS_CENTERING_GAD_STR},
    {MSG_PS_SAMPLEPICTURE_HDR,(STRPTR)MSG_PS_SAMPLEPICTURE_HDR_STR},
    {MSG_PS_SAMPLESCALING_HDR,(STRPTR)MSG_PS_SAMPLESCALING_HDR_STR},
    {MSG_PS_LEFTEDGE_GAD,(STRPTR)MSG_PS_LEFTEDGE_GAD_STR},
    {MSG_PS_TOPEDGE_GAD,(STRPTR)MSG_PS_TOPEDGE_GAD_STR},
    {MSG_PS_WIDTH_GAD,(STRPTR)MSG_PS_WIDTH_GAD_STR},
    {MSG_PS_HEIGHT_GAD,(STRPTR)MSG_PS_HEIGHT_GAD_STR},
    {MSG_PS_IMAGE_GAD,(STRPTR)MSG_PS_IMAGE_GAD_STR},
    {MSG_PS_SHADING_GAD,(STRPTR)MSG_PS_SHADING_GAD_STR},
    {MSG_PS_DITHERING_GAD,(STRPTR)MSG_PS_DITHERING_GAD_STR},
    {MSG_PS_TRANSPARENT_GAD,(STRPTR)MSG_PS_TRANSPARENT_GAD_STR},
    {MSG_PS_PANEL_1,(STRPTR)MSG_PS_PANEL_1_STR},
    {MSG_PS_PANEL_2,(STRPTR)MSG_PS_PANEL_2_STR},
    {MSG_PS_PANEL_3,(STRPTR)MSG_PS_PANEL_3_STR},
    {MSG_PS_PANEL_4,(STRPTR)MSG_PS_PANEL_4_STR},
    {MSG_PS_MODE_1,(STRPTR)MSG_PS_MODE_1_STR},
    {MSG_PS_MODE_2,(STRPTR)MSG_PS_MODE_2_STR},
    {MSG_PS_FORMAT_1,(STRPTR)MSG_PS_FORMAT_1_STR},
    {MSG_PS_FORMAT_2,(STRPTR)MSG_PS_FORMAT_2_STR},
    {NSG_PS_FORMAT_3,(STRPTR)NSG_PS_FORMAT_3_STR},
    {MSG_PS_FORMAT_4,(STRPTR)MSG_PS_FORMAT_4_STR},
    {MSG_PS_FONT_1,(STRPTR)MSG_PS_FONT_1_STR},
    {MSG_PS_FONT_2,(STRPTR)MSG_PS_FONT_2_STR},
    {MSG_PS_FONT_3,(STRPTR)MSG_PS_FONT_3_STR},
    {MSG_PS_FONT_4,(STRPTR)MSG_PS_FONT_4_STR},
    {MSG_PS_FONT_5,(STRPTR)MSG_PS_FONT_5_STR},
    {MSG_PS_FONT_6,(STRPTR)MSG_PS_FONT_6_STR},
    {MSG_PS_FONT_7,(STRPTR)MSG_PS_FONT_7_STR},
    {MSG_PS_FONT_8,(STRPTR)MSG_PS_FONT_8_STR},
    {MSG_PS_PITCH_1,(STRPTR)MSG_PS_PITCH_1_STR},
    {MSG_PS_PITCH_2,(STRPTR)MSG_PS_PITCH_2_STR},
    {MSG_PS_PITCH_3,(STRPTR)MSG_PS_PITCH_3_STR},
    {MSG_PS_ORIENTATION_1,(STRPTR)MSG_PS_ORIENTATION_1_STR},
    {MSG_PS_ORIENTATION_2,(STRPTR)MSG_PS_ORIENTATION_2_STR},
    {MSG_PS_TAB_1,(STRPTR)MSG_PS_TAB_1_STR},
    {MSG_PS_TAB_2,(STRPTR)MSG_PS_TAB_2_STR},
    {MSG_PS_TAB_3,(STRPTR)MSG_PS_TAB_3_STR},
    {NSG_PS_TAB_4,(STRPTR)NSG_PS_TAB_4_STR},
    {MSG_PS_TAB_5,(STRPTR)MSG_PS_TAB_5_STR},
    {MSG_PS_ASPECT_1,(STRPTR)MSG_PS_ASPECT_1_STR},
    {MSG_PS_ASPECT_2,(STRPTR)MSG_PS_ASPECT_2_STR},
    {MSG_PS_SCALINGTYPE_1,(STRPTR)MSG_PS_SCALINGTYPE_1_STR},
    {MSG_PS_SCALINGTYPE_2,(STRPTR)MSG_PS_SCALINGTYPE_2_STR},
    {MSG_PS_SCALINGTYPE_3,(STRPTR)MSG_PS_SCALINGTYPE_3_STR},
    {MSG_PS_SCALINGTYPE_4,(STRPTR)MSG_PS_SCALINGTYPE_4_STR},
    {MSG_PS_SCALINGTYPE_5,(STRPTR)MSG_PS_SCALINGTYPE_5_STR},
    {MSG_PS_SCALINGTYPE_6,(STRPTR)MSG_PS_SCALINGTYPE_6_STR},
    {MSG_PS_SCALINGTYPE_7,(STRPTR)MSG_PS_SCALINGTYPE_7_STR},
    {MSG_PS_SCALINGMATH_1,(STRPTR)MSG_PS_SCALINGMATH_1_STR},
    {MSG_PS_SCALINGMATH_2,(STRPTR)MSG_PS_SCALINGMATH_2_STR},
    {MSG_PS_CENTERING_1,(STRPTR)MSG_PS_CENTERING_1_STR},
    {MSG_PS_CENTERING_2,(STRPTR)MSG_PS_CENTERING_2_STR},
    {MSG_PS_CENTERING_3,(STRPTR)MSG_PS_CENTERING_3_STR},
    {MSG_PS_CENTERING_4,(STRPTR)MSG_PS_CENTERING_4_STR},
    {MSG_PS_IMAGE_1,(STRPTR)MSG_PS_IMAGE_1_STR},
    {MSG_PS_IMAGE_2,(STRPTR)MSG_PS_IMAGE_2_STR},
    {MSG_PS_SHADING_1,(STRPTR)MSG_PS_SHADING_1_STR},
    {MSG_PS_SHADING_2,(STRPTR)MSG_PS_SHADING_2_STR},
    {MSG_PS_SHADING_3,(STRPTR)MSG_PS_SHADING_3_STR},
    {MSG_PS_DITHERING_1,(STRPTR)MSG_PS_DITHERING_1_STR},
    {MSG_PS_DITHERING_2,(STRPTR)MSG_PS_DITHERING_2_STR},
    {MSG_PS_DITHERING_3,(STRPTR)MSG_PS_DITHERING_3_STR},
    {MSG_PS_DITHERING_4,(STRPTR)MSG_PS_DITHERING_4_STR},
    {MSG_PS_DITHERING_5,(STRPTR)MSG_PS_DITHERING_5_STR},
    {MSG_PS_TRANSPARENT_1,(STRPTR)MSG_PS_TRANSPARENT_1_STR},
    {MSG_PS_TRANSPARENT_2,(STRPTR)MSG_PS_TRANSPARENT_2_STR},
    {MSG_PS_TRANSPARENT_3,(STRPTR)MSG_PS_TRANSPARENT_3_STR},
    {MSG_PS_OPTIONS_SYSTEM,(STRPTR)MSG_PS_OPTIONS_SYSTEM_STR},
    {MSG_PS_OPTIONS_CM,(STRPTR)MSG_PS_OPTIONS_CM_STR},
    {MSG_PS_OPTIONS_INCHES,(STRPTR)MSG_PS_OPTIONS_INCHES_STR},
    {MSG_PS_OPTIONS_POINTS,(STRPTR)MSG_PS_OPTIONS_POINTS_STR},
#endif /* PRINTERPS_PREFS */

#ifdef POINTER_PREFS
    {MSG_PTR_NAME,(STRPTR)MSG_PTR_NAME_STR},
    {MSG_PTR_NO_DATATYPES_LIBRARY,(STRPTR)MSG_PTR_NO_DATATYPES_LIBRARY_STR},
    {MSG_PTR_OPEN_CLIPBOARD_STAT,(STRPTR)MSG_PTR_OPEN_CLIPBOARD_STAT_STR},
    {MSG_PTR_PASTE_GRAPHIC_STAT,(STRPTR)MSG_PTR_PASTE_GRAPHIC_STAT_STR},
    {MSG_PTR_NO_GRAPHIC_STAT,(STRPTR)MSG_PTR_NO_GRAPHIC_STAT_STR},
    {MSG_PTR_REQ_LOAD,(STRPTR)MSG_PTR_REQ_LOAD_STR},
    {MSG_PTR_REQ_SAVE,(STRPTR)MSG_PTR_REQ_SAVE_STR},
    {MSG_PTR_TEST_GAD,(STRPTR)MSG_PTR_TEST_GAD_STR},
    {MSG_PTR_CLEAR_GAD,(STRPTR)MSG_PTR_CLEAR_GAD_STR},
    {MSG_PTR_SET_GAD,(STRPTR)MSG_PTR_SET_GAD_STR},
    {MSG_PTR_RESET_GAD,(STRPTR)MSG_PTR_RESET_GAD_STR},
    {MSG_PTR_RED_GAD,(STRPTR)MSG_PTR_RED_GAD_STR},
    {MSG_PTR_GREEN_GAD,(STRPTR)MSG_PTR_GREEN_GAD_STR},
    {MSG_PTR_BLUE_GAD,(STRPTR)MSG_PTR_BLUE_GAD_STR},
    {MSG_PTR_TYPE_GAD,(STRPTR)MSG_PTR_TYPE_GAD_STR},
    {MSG_PTR_NORMAL_GAD,(STRPTR)MSG_PTR_NORMAL_GAD_STR},
    {MSG_PTR_BUSY_GAD,(STRPTR)MSG_PTR_BUSY_GAD_STR},
    {MSG_PTR_SIZE_GAD,(STRPTR)MSG_PTR_SIZE_GAD_STR},
    {MSG_PTR_LORES_GAD,(STRPTR)MSG_PTR_LORES_GAD_STR},
    {MSG_PTR_HIRES_GAD,(STRPTR)MSG_PTR_HIRES_GAD_STR},
    {MSG_PTR_SHIRES_GAD,(STRPTR)MSG_PTR_SHIRES_GAD_STR},
    {MSG_PTR_EDIT_LOAD_IMAGE,(STRPTR)MSG_PTR_EDIT_LOAD_IMAGE_STR},
    {MSG_PTR_REQ_LOAD_IMAGE,(STRPTR)MSG_PTR_REQ_LOAD_IMAGE_STR},
#endif /* POINTER_PREFS */

#ifdef WBPATTERN_PREFS
    {MSG_WBP_NAME,(STRPTR)MSG_WBP_NAME_STR},
    {MSG_WBP_NO_DATATYPES_LIBRARY,(STRPTR)MSG_WBP_NO_DATATYPES_LIBRARY_STR},
    {MSG_WBP_OPEN_CLIPBOARD_STAT,(STRPTR)MSG_WBP_OPEN_CLIPBOARD_STAT_STR},
    {MSG_WBP_PASTE_GRAPHIC_STAT,(STRPTR)MSG_WBP_PASTE_GRAPHIC_STAT_STR},
    {MSG_WBP_NO_GRAPHIC_STAT,(STRPTR)MSG_WBP_NO_GRAPHIC_STAT_STR},
    {MSG_WBP_REQ_LOAD,(STRPTR)MSG_WBP_REQ_LOAD_STR},
    {MSG_WBP_REQ_SAVE,(STRPTR)MSG_WBP_REQ_SAVE_STR},
    {MSG_WBP_REQ_WIN_LOAD,(STRPTR)MSG_WBP_REQ_WIN_LOAD_STR},
    {MSG_WBP_REQ_WIN_SAVE,(STRPTR)MSG_WBP_REQ_WIN_SAVE_STR},
    {MSG_WBP_PATTERN_LABEL,(STRPTR)MSG_WBP_PATTERN_LABEL_STR},
    {MSG_WBP_WORKBENCH_GAD,(STRPTR)MSG_WBP_WORKBENCH_GAD_STR},
    {MSG_WBP_WINDOWS_GAD,(STRPTR)MSG_WBP_WINDOWS_GAD_STR},
    {MSG_WBP_TEST_GAD,(STRPTR)MSG_WBP_TEST_GAD_STR},
    {MSG_WBP_CLEAR_GAD,(STRPTR)MSG_WBP_CLEAR_GAD_STR},
    {MSG_WBP_UNDO_GAD,(STRPTR)MSG_WBP_UNDO_GAD_STR},
    {MSG_WBP_PRESETS_LABEL,(STRPTR)MSG_WBP_PRESETS_LABEL_STR},
    {MSG_WBP_SCREEN_GAD,(STRPTR)MSG_WBP_SCREEN_GAD_STR},
    {MSG_WBP_REQ_SCREEN_LOAD,(STRPTR)MSG_WBP_REQ_SCREEN_LOAD_STR},
    {MSG_WBP_REQ_SCREEN_SAVE,(STRPTR)MSG_WBP_REQ_SCREEN_SAVE_STR},
    {MSG_WBP_SELECT_PICTURE_GAD,(STRPTR)MSG_WBP_SELECT_PICTURE_GAD_STR},
    {MSG_WBP_PICTURE_NAME_GAD,(STRPTR)MSG_WBP_PICTURE_NAME_GAD_STR},
    {MSG_WBP_PLACEMENT_GAD,(STRPTR)MSG_WBP_PLACEMENT_GAD_STR},
    {MSG_WBP_TYPE_GAD,(STRPTR)MSG_WBP_TYPE_GAD_STR},
    {MSG_WBP_TYPE_PICTURE_GAD,(STRPTR)MSG_WBP_TYPE_PICTURE_GAD_STR},
    {MSG_WBP_TYPE_PATTERN_GAD,(STRPTR)MSG_WBP_TYPE_PATTERN_GAD_STR},
    {MSG_WBP_SELECT_PICTURE_LABEL,(STRPTR)MSG_WBP_SELECT_PICTURE_LABEL_STR},
    {MSG_WBP_EDIT_LOAD_IMAGE,(STRPTR)MSG_WBP_EDIT_LOAD_IMAGE_STR},
    {MSG_WBP_REQ_LOAD_IMAGE,(STRPTR)MSG_WBP_REQ_LOAD_IMAGE_STR},
#endif /* WBPATTERN_PREFS */

#ifdef SOUND_PREFS
    {MSG_SND_NAME,(STRPTR)MSG_SND_NAME_STR},
    {MSG_SND_NO_DATATYPES_LIBRARY,(STRPTR)MSG_SND_NO_DATATYPES_LIBRARY_STR},
    {MSG_SND_IFF_NOT_8SVX_STAT,(STRPTR)MSG_SND_IFF_NOT_8SVX_STAT_STR},
    {MSG_SND_REQ_LOAD,(STRPTR)MSG_SND_REQ_LOAD_STR},
    {MSG_SND_REQ_SAVE,(STRPTR)MSG_SND_REQ_SAVE_STR},
    {MSG_SND_REQ_LOAD_SOUND,(STRPTR)MSG_SND_REQ_LOAD_SOUND_STR},
    {MSG_SND_FLASH_GAD,(STRPTR)MSG_SND_FLASH_GAD_STR},
    {MSG_SND_MAKESOUND_GAD,(STRPTR)MSG_SND_MAKESOUND_GAD_STR},
    {MSG_SND_SOUNDTYPE_GAD,(STRPTR)MSG_SND_SOUNDTYPE_GAD_STR},
    {MSG_SND_BEEP,(STRPTR)MSG_SND_BEEP_STR},
    {MSG_SND_SAMPLEDSOUND,(STRPTR)MSG_SND_SAMPLEDSOUND_STR},
    {MSG_SND_TEST_GAD,(STRPTR)MSG_SND_TEST_GAD_STR},
    {MSG_SND_VOLUME_GAD,(STRPTR)MSG_SND_VOLUME_GAD_STR},
    {MSG_SND_PITCH_GAD,(STRPTR)MSG_SND_PITCH_GAD_STR},
    {MSG_SND_DURATION_GAD,(STRPTR)MSG_SND_DURATION_GAD_STR},
    {MSG_SND_SAMPLENAME_GAD,(STRPTR)MSG_SND_SAMPLENAME_GAD_STR},
    {MSG_SND_SELECTSAMPLE_GAD,(STRPTR)MSG_SND_SELECTSAMPLE_GAD_STR},
#endif /* SOUND_PREFS */

#ifdef PALETTE_PREFS
    {MSG_PAL_NAME,(STRPTR)MSG_PAL_NAME_STR},
    {MSG_PAL_NO_COLORWHEEL,(STRPTR)MSG_PAL_NO_COLORWHEEL_STR},
    {MSG_PAL_NO_GRADIENT,(STRPTR)MSG_PAL_NO_GRADIENT_STR},
    {MSG_PAL_REQ_LOAD,(STRPTR)MSG_PAL_REQ_LOAD_STR},
    {MSG_PAL_REQ_SAVE,(STRPTR)MSG_PAL_REQ_SAVE_STR},
    {MSG_PAL_EDIT_PRESETS,(STRPTR)MSG_PAL_EDIT_PRESETS_STR},
    {MSG_PAL_RED_GAD,(STRPTR)MSG_PAL_RED_GAD_STR},
    {MSG_PAL_GREEN_GAD,(STRPTR)MSG_PAL_GREEN_GAD_STR},
    {MSG_PAL_BLUE_GAD,(STRPTR)MSG_PAL_BLUE_GAD_STR},
    {MSG_PAL_HUE_GAD,(STRPTR)MSG_PAL_HUE_GAD_STR},
    {MSG_PAL_SATURATION_GAD,(STRPTR)MSG_PAL_SATURATION_GAD_STR},
    {MSG_PAL_BRIGHTNESS_GAD,(STRPTR)MSG_PAL_BRIGHTNESS_GAD_STR},
    {MSG_PAL_EDIT_PRESET1,(STRPTR)MSG_PAL_EDIT_PRESET1_STR},
    {MSG_PAL_EDIT_PRESET2,(STRPTR)MSG_PAL_EDIT_PRESET2_STR},
    {MSG_PAL_EDIT_PRESET3,(STRPTR)MSG_PAL_EDIT_PRESET3_STR},
    {MSG_PAL_EDIT_PRESET4,(STRPTR)MSG_PAL_EDIT_PRESET4_STR},
    {MSG_PAL_EDIT_PRESET5,(STRPTR)MSG_PAL_EDIT_PRESET5_STR},
    {MSG_PAL_EDIT_PRESET6,(STRPTR)MSG_PAL_EDIT_PRESET6_STR},
    {MSG_PAL_EDIT_PRESET7,(STRPTR)MSG_PAL_EDIT_PRESET7_STR},
    {MSG_PAL_EDIT_PRESET8,(STRPTR)MSG_PAL_EDIT_PRESET8_STR},
    {MSG_PAL_EDIT_PRESET9,(STRPTR)MSG_PAL_EDIT_PRESET9_STR},
    {MSG_PAL_OPTIONS_COLORMODEL,(STRPTR)MSG_PAL_OPTIONS_COLORMODEL_STR},
    {MSG_PAL_OPTIONS_RGB,(STRPTR)MSG_PAL_OPTIONS_RGB_STR},
    {MSG_PAL_OPTIONS_HSB,(STRPTR)MSG_PAL_OPTIONS_HSB_STR},
    {MSG_PAL_SHOW_SAMPLE,(STRPTR)MSG_PAL_SHOW_SAMPLE_STR},
    {MSG_PAL_BACKGROUND_PEN,(STRPTR)MSG_PAL_BACKGROUND_PEN_STR},
    {MSG_PAL_TEXT_PEN,(STRPTR)MSG_PAL_TEXT_PEN_STR},
    {MSG_PAL_HIGHLIGHT_PEN,(STRPTR)MSG_PAL_HIGHLIGHT_PEN_STR},
    {MSG_PAL_SHINE_PEN,(STRPTR)MSG_PAL_SHINE_PEN_STR},
    {MSG_PAL_SHADOW_PEN,(STRPTR)MSG_PAL_SHADOW_PEN_STR},
    {MSG_PAL_FILL_PEN,(STRPTR)MSG_PAL_FILL_PEN_STR},
    {MSG_PAL_FILLTEXT_PEN,(STRPTR)MSG_PAL_FILLTEXT_PEN_STR},
    {MSG_PAL_BARBLOCK_PEN,(STRPTR)MSG_PAL_BARBLOCK_PEN_STR},
    {MSG_PAL_BARDETAIL_PEN,(STRPTR)MSG_PAL_BARDETAIL_PEN_STR},
    {MSG_PAL_SAMPLE_TITLE,(STRPTR)MSG_PAL_SAMPLE_TITLE_STR},
    {MSG_PAL_SAMPLE_MENU,(STRPTR)MSG_PAL_SAMPLE_MENU_STR},
    {MSG_PAL_SAMPLE_ITEM_1,(STRPTR)MSG_PAL_SAMPLE_ITEM_1_STR},
    {MSG_PAL_SAMPLE_ITEM_2,(STRPTR)MSG_PAL_SAMPLE_ITEM_2_STR},
    {MSG_PAL_SAMPLE_ITEM_3,(STRPTR)MSG_PAL_SAMPLE_ITEM_3_STR},
    {MSG_PAL_SAMPLE_ITEM_4,(STRPTR)MSG_PAL_SAMPLE_ITEM_4_STR},
    {MSG_PAL_SAMPLE_ITEM_5,(STRPTR)MSG_PAL_SAMPLE_ITEM_5_STR},
    {MSG_PAL_SAMPLE_ITEM_6,(STRPTR)MSG_PAL_SAMPLE_ITEM_6_STR},
    {MSG_PAL_SAMPLE_ACTIVE,(STRPTR)MSG_PAL_SAMPLE_ACTIVE_STR},
    {MSG_PAL_SAMPLE_INACTIVE,(STRPTR)MSG_PAL_SAMPLE_INACTIVE_STR},
    {MSG_PAL_SAMPLE_SPECIAL,(STRPTR)MSG_PAL_SAMPLE_SPECIAL_STR},
    {MSG_PAL_SAMPLE_BUTTON,(STRPTR)MSG_PAL_SAMPLE_BUTTON_STR},
    {MSG_PAL_SAMPLE_STRING,(STRPTR)MSG_PAL_SAMPLE_STRING_STR},
    {MSG_PAL_SAMPLE_NUMBER,(STRPTR)MSG_PAL_SAMPLE_NUMBER_STR},
    {MSG_PAL_SAMPLE_FOOD_1,(STRPTR)MSG_PAL_SAMPLE_FOOD_1_STR},
    {MSG_PAL_SAMPLE_FOOD_2,(STRPTR)MSG_PAL_SAMPLE_FOOD_2_STR},
    {MSG_PAL_SAMPLE_FOOD_3,(STRPTR)MSG_PAL_SAMPLE_FOOD_3_STR},
    {MSG_PAL_SAMPLE_SALT,(STRPTR)MSG_PAL_SAMPLE_SALT_STR},
    {MSG_PAL_WHEEL_ABBRV,(STRPTR)MSG_PAL_WHEEL_ABBRV_STR},
    {MSG_PAL_4COLOR_MODE,(STRPTR)MSG_PAL_4COLOR_MODE_STR},
    {MSG_PAL_MULTICOLOR_MODE,(STRPTR)MSG_PAL_MULTICOLOR_MODE_STR},
#endif /* PALETTE_PREFS */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* LOCALESTR_PREFS_H */
