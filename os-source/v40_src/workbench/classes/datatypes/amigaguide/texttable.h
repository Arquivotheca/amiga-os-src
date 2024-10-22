#ifndef TEXTTABLE_H
#define TEXTTABLE_H


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

#define ERR_NOT_ENOUGH_MEMORY 100
#define ERR_CANT_OPEN_DATABASE 101
#define ERR_CANT_FIND_NODE 102
#define ERR_CANT_OPEN_NODE 103
#define ERR_CANT_OPEN_WINDOW 104
#define ERR_INVALID_COMMAND 105
#define ERR_CANT_COMPLETE 106
#define ERR_PORT_CLOSED 107
#define ERR_CANT_CREATE_PORT 108
#define ERR_CANT_SEARCH 109
#define ERR_CANT_SET_BOOKMARK 110
#define ERR_CANT_LOAD_BOOKMARK 111
#define ERR_CANT_OPEN_OUTPUT 112
#define ERR_KEYWORD_NOT_FOUND 113
#define ERR_COULDNT_LOCATE 114
#define ERR_COULDNT_SAVE_FILE 115
#define ERR_COULDNT_OPEN_FILE 116
#define ERR_COULDNT_LOCATE_DATATYPE 117
#define ERR_COULDNT_SAVE_PREFS 118
#define TITLE_LOADING 200
#define TITLE_BUILDING_SEARCH_DOCUMENT 201
#define TITLE_TEXT_SEARCH 202
#define TITLE_AMIGAGUIDE 203
#define TITLE_PRINTING 204
#define TITLE_SELECT_FILE_TO_OPEN 205
#define TITLE_SELECT_FILE_TO_SAVE_TO 206
#define LABEL_CONTENTS 220
#define LABEL_INDEX 221
#define LABEL_HELP 222
#define LABEL_RETRACE 223
#define LABEL_BROWSE_PREV 224
#define LABEL_BROWSE_NEXT 225
#define LABEL_ABORT 226
#define LABEL_CONTINUE 227
#define LABEL_OPEN 228
#define LABEL_SAVE 229
#define FRTITLE_FIND_DOCUMENT 300
#define FRTITLE_FIND_TEXT 301
#define FRLABEL_FIND 310
#define FRLABEL_NEXT 311
#define FRLABEL_CANCEL 312
#define FRLABEL_IGNORE_CASE 313
#define FRLABEL_ONLY_WHOLE_WORDS 314
#define FRLABEL_SCAN_DATABASE 315
#define FRNODE_FOUND_NO_OCCURRENCES 330
#define FRNODE_FOUND_ONE_OCCURRENCE 331
#define FRNODE_FOUND_OCCURRENCES 332
#define MENU_PROJECT 400
#define ITEM_OPEN 401
#define ITEM_SAVE_AS 402
#define ITEM_PRINT 403
#define ITEM_ABOUT 404
#define ITEM_QUIT 405
#define MENU_EDIT 425
#define ITEM_MARK 426
#define ITEM_COPY 427
#define ITEM_SELECT_ALL 428
#define ITEM_CLEAR_SELECTED 429
#define MENU_SEARCH 450
#define ITEM_FIND 451
#define ITEM_FIND_NEXT 452
#define ITEM_GO_TO_LINE 453
#define ITEM_SET_BOOKMARK 454
#define ITEM_GOTO_BOOKMARK 455
#define MENU_WINDOW 460
#define ITEM_MINIMIZE 461
#define ITEM_NORMAL 462
#define ITEM_MAXIMIZE 463
#define MENU_EXTRAS 470
#define ITEM_EXECUTE_COMMAND 471
#define MENU_PREFS 480
#define ITEM_SAVE_AS_DEFAULTS 481
#define LPDERR_NOERR 5100
#define LPDERR_CANCEL 5101
#define LPDERR_NOTGRAPHICS 5102
#define LPDERR_OBS1 5103
#define LPDERR_BADDIMENSION 5104
#define LPDERR_OBS2 5105
#define LPDERR_INTERNALMEMORY 5106
#define LPDERR_BUFFERMEMORY 5107

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define ERR_NOT_ENOUGH_MEMORY_STR "Not enough memory"
#define ERR_CANT_OPEN_DATABASE_STR "Can't open database"
#define ERR_CANT_FIND_NODE_STR "Can't find node"
#define ERR_CANT_OPEN_NODE_STR "Can't open node"
#define ERR_CANT_OPEN_WINDOW_STR "Can't open window"
#define ERR_INVALID_COMMAND_STR "Invalid command"
#define ERR_CANT_COMPLETE_STR "Can't complete"
#define ERR_PORT_CLOSED_STR "Port closed"
#define ERR_CANT_CREATE_PORT_STR "Can't create port"
#define ERR_CANT_SEARCH_STR "Can't search in search document"
#define ERR_CANT_SET_BOOKMARK_STR "Can't set bookmark"
#define ERR_CANT_LOAD_BOOKMARK_STR "Can't load bookmark"
#define ERR_CANT_OPEN_OUTPUT_STR "Can't open output"
#define ERR_KEYWORD_NOT_FOUND_STR "Keyword not found in cross-reference table"
#define ERR_COULDNT_LOCATE_STR "Couldn't locate %s"
#define ERR_COULDNT_SAVE_FILE_STR "Couldn't save %s"
#define ERR_COULDNT_OPEN_FILE_STR "Couldn't open %s"
#define ERR_COULDNT_LOCATE_DATATYPE_STR "Unknown data type for %s"
#define ERR_COULDNT_SAVE_PREFS_STR "Couldn't save preferences"
#define TITLE_LOADING_STR "Loading..."
#define TITLE_BUILDING_SEARCH_DOCUMENT_STR "Building search document..."
#define TITLE_TEXT_SEARCH_STR "Text Search: %s"
#define TITLE_AMIGAGUIDE_STR "AmigaGuide"
#define TITLE_PRINTING_STR "Printing..."
#define TITLE_SELECT_FILE_TO_OPEN_STR "Select File to Open"
#define TITLE_SELECT_FILE_TO_SAVE_TO_STR "Select File to Save to"
#define LABEL_CONTENTS_STR "Contents"
#define LABEL_INDEX_STR "Index"
#define LABEL_HELP_STR "Help"
#define LABEL_RETRACE_STR "Retrace"
#define LABEL_BROWSE_PREV_STR "Browse <"
#define LABEL_BROWSE_NEXT_STR "Browse >"
#define LABEL_ABORT_STR "Abort"
#define LABEL_CONTINUE_STR "Continue"
#define LABEL_OPEN_STR "Open"
#define LABEL_SAVE_STR "Save"
#define FRTITLE_FIND_DOCUMENT_STR "Find Document"
#define FRTITLE_FIND_TEXT_STR "Find Text"
#define FRLABEL_FIND_STR "_Find"
#define FRLABEL_NEXT_STR "_Next"
#define FRLABEL_CANCEL_STR "_Cancel"
#define FRLABEL_IGNORE_CASE_STR "I_gnore Case"
#define FRLABEL_ONLY_WHOLE_WORDS_STR "_Only Whole Words"
#define FRLABEL_SCAN_DATABASE_STR "_Scan Database"
#define FRNODE_FOUND_NO_OCCURRENCES_STR "\nFound no occurrences of:\n\"%s\"\n"
#define FRNODE_FOUND_ONE_OCCURRENCE_STR "\nFound 1 occurrence of:\n\"%s\"\n"
#define FRNODE_FOUND_OCCURRENCES_STR "\nFound %ld occurrences of:\n\"%s\"\n"
#define MENU_PROJECT_STR "Project"
#define ITEM_OPEN_STR "O\0Open..."
#define ITEM_SAVE_AS_STR "A\0Save As..."
#define ITEM_PRINT_STR "P\0Print"
#define ITEM_ABOUT_STR "?\0About..."
#define ITEM_QUIT_STR "Q\0Quit"
#define MENU_EDIT_STR "Edit"
#define ITEM_MARK_STR "B\0Mark"
#define ITEM_COPY_STR "C\0Copy"
#define ITEM_SELECT_ALL_STR "[\0Select All"
#define ITEM_CLEAR_SELECTED_STR "]\0Clear Selected"
#define MENU_SEARCH_STR "Search"
#define ITEM_FIND_STR "F\0Find..."
#define ITEM_FIND_NEXT_STR "N\0Find Next"
#define ITEM_GO_TO_LINE_STR "G\0Go To Line #..."
#define ITEM_SET_BOOKMARK_STR " \0Set Bookmark"
#define ITEM_GOTO_BOOKMARK_STR " \0Go To Bookmark"
#define MENU_WINDOW_STR "Window"
#define ITEM_MINIMIZE_STR " \0Minimize"
#define ITEM_NORMAL_STR " \0Normal"
#define ITEM_MAXIMIZE_STR " \0Maximize"
#define MENU_EXTRAS_STR "Extras"
#define ITEM_EXECUTE_COMMAND_STR "E\0Execute Command..."
#define MENU_PREFS_STR "Prefs"
#define ITEM_SAVE_AS_DEFAULTS_STR "9\0Save As Defaults"
#define LPDERR_NOERR_STR "Operation Successful"
#define LPDERR_CANCEL_STR "User Canceled Print Request"
#define LPDERR_NOTGRAPHICS_STR "Printer Cannot Ouput Graphics"
#define LPDERR_OBS1_STR "obsolete"
#define LPDERR_BADDIMENSION_STR "Print Dimensions are Illegal"
#define LPDERR_OBS2_STR "obsolete"
#define LPDERR_INTERNALMEMORY_STR "Not Enough Memory"
#define LPDERR_BUFFERMEMORY_STR "Not Enough Memory"

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
    {ERR_NOT_ENOUGH_MEMORY,(STRPTR)ERR_NOT_ENOUGH_MEMORY_STR},
    {ERR_CANT_OPEN_DATABASE,(STRPTR)ERR_CANT_OPEN_DATABASE_STR},
    {ERR_CANT_FIND_NODE,(STRPTR)ERR_CANT_FIND_NODE_STR},
    {ERR_CANT_OPEN_NODE,(STRPTR)ERR_CANT_OPEN_NODE_STR},
    {ERR_CANT_OPEN_WINDOW,(STRPTR)ERR_CANT_OPEN_WINDOW_STR},
    {ERR_INVALID_COMMAND,(STRPTR)ERR_INVALID_COMMAND_STR},
    {ERR_CANT_COMPLETE,(STRPTR)ERR_CANT_COMPLETE_STR},
    {ERR_PORT_CLOSED,(STRPTR)ERR_PORT_CLOSED_STR},
    {ERR_CANT_CREATE_PORT,(STRPTR)ERR_CANT_CREATE_PORT_STR},
    {ERR_CANT_SEARCH,(STRPTR)ERR_CANT_SEARCH_STR},
    {ERR_CANT_SET_BOOKMARK,(STRPTR)ERR_CANT_SET_BOOKMARK_STR},
    {ERR_CANT_LOAD_BOOKMARK,(STRPTR)ERR_CANT_LOAD_BOOKMARK_STR},
    {ERR_CANT_OPEN_OUTPUT,(STRPTR)ERR_CANT_OPEN_OUTPUT_STR},
    {ERR_KEYWORD_NOT_FOUND,(STRPTR)ERR_KEYWORD_NOT_FOUND_STR},
    {ERR_COULDNT_LOCATE,(STRPTR)ERR_COULDNT_LOCATE_STR},
    {ERR_COULDNT_SAVE_FILE,(STRPTR)ERR_COULDNT_SAVE_FILE_STR},
    {ERR_COULDNT_OPEN_FILE,(STRPTR)ERR_COULDNT_OPEN_FILE_STR},
    {ERR_COULDNT_LOCATE_DATATYPE,(STRPTR)ERR_COULDNT_LOCATE_DATATYPE_STR},
    {ERR_COULDNT_SAVE_PREFS,(STRPTR)ERR_COULDNT_SAVE_PREFS_STR},
    {TITLE_LOADING,(STRPTR)TITLE_LOADING_STR},
    {TITLE_BUILDING_SEARCH_DOCUMENT,(STRPTR)TITLE_BUILDING_SEARCH_DOCUMENT_STR},
    {TITLE_TEXT_SEARCH,(STRPTR)TITLE_TEXT_SEARCH_STR},
    {TITLE_AMIGAGUIDE,(STRPTR)TITLE_AMIGAGUIDE_STR},
    {TITLE_PRINTING,(STRPTR)TITLE_PRINTING_STR},
    {TITLE_SELECT_FILE_TO_OPEN,(STRPTR)TITLE_SELECT_FILE_TO_OPEN_STR},
    {TITLE_SELECT_FILE_TO_SAVE_TO,(STRPTR)TITLE_SELECT_FILE_TO_SAVE_TO_STR},
    {LABEL_CONTENTS,(STRPTR)LABEL_CONTENTS_STR},
    {LABEL_INDEX,(STRPTR)LABEL_INDEX_STR},
    {LABEL_HELP,(STRPTR)LABEL_HELP_STR},
    {LABEL_RETRACE,(STRPTR)LABEL_RETRACE_STR},
    {LABEL_BROWSE_PREV,(STRPTR)LABEL_BROWSE_PREV_STR},
    {LABEL_BROWSE_NEXT,(STRPTR)LABEL_BROWSE_NEXT_STR},
    {LABEL_ABORT,(STRPTR)LABEL_ABORT_STR},
    {LABEL_CONTINUE,(STRPTR)LABEL_CONTINUE_STR},
    {LABEL_OPEN,(STRPTR)LABEL_OPEN_STR},
    {LABEL_SAVE,(STRPTR)LABEL_SAVE_STR},
    {FRTITLE_FIND_DOCUMENT,(STRPTR)FRTITLE_FIND_DOCUMENT_STR},
    {FRTITLE_FIND_TEXT,(STRPTR)FRTITLE_FIND_TEXT_STR},
    {FRLABEL_FIND,(STRPTR)FRLABEL_FIND_STR},
    {FRLABEL_NEXT,(STRPTR)FRLABEL_NEXT_STR},
    {FRLABEL_CANCEL,(STRPTR)FRLABEL_CANCEL_STR},
    {FRLABEL_IGNORE_CASE,(STRPTR)FRLABEL_IGNORE_CASE_STR},
    {FRLABEL_ONLY_WHOLE_WORDS,(STRPTR)FRLABEL_ONLY_WHOLE_WORDS_STR},
    {FRLABEL_SCAN_DATABASE,(STRPTR)FRLABEL_SCAN_DATABASE_STR},
    {FRNODE_FOUND_NO_OCCURRENCES,(STRPTR)FRNODE_FOUND_NO_OCCURRENCES_STR},
    {FRNODE_FOUND_ONE_OCCURRENCE,(STRPTR)FRNODE_FOUND_ONE_OCCURRENCE_STR},
    {FRNODE_FOUND_OCCURRENCES,(STRPTR)FRNODE_FOUND_OCCURRENCES_STR},
    {MENU_PROJECT,(STRPTR)MENU_PROJECT_STR},
    {ITEM_OPEN,(STRPTR)ITEM_OPEN_STR},
    {ITEM_SAVE_AS,(STRPTR)ITEM_SAVE_AS_STR},
    {ITEM_PRINT,(STRPTR)ITEM_PRINT_STR},
    {ITEM_ABOUT,(STRPTR)ITEM_ABOUT_STR},
    {ITEM_QUIT,(STRPTR)ITEM_QUIT_STR},
    {MENU_EDIT,(STRPTR)MENU_EDIT_STR},
    {ITEM_MARK,(STRPTR)ITEM_MARK_STR},
    {ITEM_COPY,(STRPTR)ITEM_COPY_STR},
    {ITEM_SELECT_ALL,(STRPTR)ITEM_SELECT_ALL_STR},
    {ITEM_CLEAR_SELECTED,(STRPTR)ITEM_CLEAR_SELECTED_STR},
    {MENU_SEARCH,(STRPTR)MENU_SEARCH_STR},
    {ITEM_FIND,(STRPTR)ITEM_FIND_STR},
    {ITEM_FIND_NEXT,(STRPTR)ITEM_FIND_NEXT_STR},
    {ITEM_GO_TO_LINE,(STRPTR)ITEM_GO_TO_LINE_STR},
    {ITEM_SET_BOOKMARK,(STRPTR)ITEM_SET_BOOKMARK_STR},
    {ITEM_GOTO_BOOKMARK,(STRPTR)ITEM_GOTO_BOOKMARK_STR},
    {MENU_WINDOW,(STRPTR)MENU_WINDOW_STR},
    {ITEM_MINIMIZE,(STRPTR)ITEM_MINIMIZE_STR},
    {ITEM_NORMAL,(STRPTR)ITEM_NORMAL_STR},
    {ITEM_MAXIMIZE,(STRPTR)ITEM_MAXIMIZE_STR},
    {MENU_EXTRAS,(STRPTR)MENU_EXTRAS_STR},
    {ITEM_EXECUTE_COMMAND,(STRPTR)ITEM_EXECUTE_COMMAND_STR},
    {MENU_PREFS,(STRPTR)MENU_PREFS_STR},
    {ITEM_SAVE_AS_DEFAULTS,(STRPTR)ITEM_SAVE_AS_DEFAULTS_STR},
    {LPDERR_NOERR,(STRPTR)LPDERR_NOERR_STR},
    {LPDERR_CANCEL,(STRPTR)LPDERR_CANCEL_STR},
    {LPDERR_NOTGRAPHICS,(STRPTR)LPDERR_NOTGRAPHICS_STR},
    {LPDERR_OBS1,(STRPTR)LPDERR_OBS1_STR},
    {LPDERR_BADDIMENSION,(STRPTR)LPDERR_BADDIMENSION_STR},
    {LPDERR_OBS2,(STRPTR)LPDERR_OBS2_STR},
    {LPDERR_INTERNALMEMORY,(STRPTR)LPDERR_INTERNALMEMORY_STR},
    {LPDERR_BUFFERMEMORY,(STRPTR)LPDERR_BUFFERMEMORY_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x64\x00\x12"
    ERR_NOT_ENOUGH_MEMORY_STR "\x00"
    "\x00\x00\x00\x65\x00\x14"
    ERR_CANT_OPEN_DATABASE_STR "\x00"
    "\x00\x00\x00\x66\x00\x10"
    ERR_CANT_FIND_NODE_STR "\x00"
    "\x00\x00\x00\x67\x00\x10"
    ERR_CANT_OPEN_NODE_STR "\x00"
    "\x00\x00\x00\x68\x00\x12"
    ERR_CANT_OPEN_WINDOW_STR "\x00"
    "\x00\x00\x00\x69\x00\x10"
    ERR_INVALID_COMMAND_STR "\x00"
    "\x00\x00\x00\x6A\x00\x10"
    ERR_CANT_COMPLETE_STR "\x00\x00"
    "\x00\x00\x00\x6B\x00\x0C"
    ERR_PORT_CLOSED_STR "\x00"
    "\x00\x00\x00\x6C\x00\x12"
    ERR_CANT_CREATE_PORT_STR "\x00"
    "\x00\x00\x00\x6D\x00\x20"
    ERR_CANT_SEARCH_STR "\x00"
    "\x00\x00\x00\x6E\x00\x14"
    ERR_CANT_SET_BOOKMARK_STR "\x00\x00"
    "\x00\x00\x00\x6F\x00\x14"
    ERR_CANT_LOAD_BOOKMARK_STR "\x00"
    "\x00\x00\x00\x70\x00\x12"
    ERR_CANT_OPEN_OUTPUT_STR "\x00"
    "\x00\x00\x00\x71\x00\x2C"
    ERR_KEYWORD_NOT_FOUND_STR "\x00\x00"
    "\x00\x00\x00\x72\x00\x14"
    ERR_COULDNT_LOCATE_STR "\x00\x00"
    "\x00\x00\x00\x73\x00\x12"
    ERR_COULDNT_SAVE_FILE_STR "\x00\x00"
    "\x00\x00\x00\x74\x00\x12"
    ERR_COULDNT_OPEN_FILE_STR "\x00\x00"
    "\x00\x00\x00\x75\x00\x1A"
    ERR_COULDNT_LOCATE_DATATYPE_STR "\x00\x00"
    "\x00\x00\x00\x76\x00\x1A"
    ERR_COULDNT_SAVE_PREFS_STR "\x00"
    "\x00\x00\x00\xC8\x00\x0C"
    TITLE_LOADING_STR "\x00\x00"
    "\x00\x00\x00\xC9\x00\x1C"
    TITLE_BUILDING_SEARCH_DOCUMENT_STR "\x00"
    "\x00\x00\x00\xCA\x00\x10"
    TITLE_TEXT_SEARCH_STR "\x00"
    "\x00\x00\x00\xCB\x00\x0C"
    TITLE_AMIGAGUIDE_STR "\x00\x00"
    "\x00\x00\x00\xCC\x00\x0C"
    TITLE_PRINTING_STR "\x00"
    "\x00\x00\x00\xCD\x00\x14"
    TITLE_SELECT_FILE_TO_OPEN_STR "\x00"
    "\x00\x00\x00\xCE\x00\x18"
    TITLE_SELECT_FILE_TO_SAVE_TO_STR "\x00\x00"
    "\x00\x00\x00\xDC\x00\x0A"
    LABEL_CONTENTS_STR "\x00\x00"
    "\x00\x00\x00\xDD\x00\x06"
    LABEL_INDEX_STR "\x00"
    "\x00\x00\x00\xDE\x00\x06"
    LABEL_HELP_STR "\x00\x00"
    "\x00\x00\x00\xDF\x00\x08"
    LABEL_RETRACE_STR "\x00"
    "\x00\x00\x00\xE0\x00\x0A"
    LABEL_BROWSE_PREV_STR "\x00\x00"
    "\x00\x00\x00\xE1\x00\x0A"
    LABEL_BROWSE_NEXT_STR "\x00\x00"
    "\x00\x00\x00\xE2\x00\x06"
    LABEL_ABORT_STR "\x00"
    "\x00\x00\x00\xE3\x00\x0A"
    LABEL_CONTINUE_STR "\x00\x00"
    "\x00\x00\x00\xE4\x00\x06"
    LABEL_OPEN_STR "\x00\x00"
    "\x00\x00\x00\xE5\x00\x06"
    LABEL_SAVE_STR "\x00\x00"
    "\x00\x00\x01\x2C\x00\x0E"
    FRTITLE_FIND_DOCUMENT_STR "\x00"
    "\x00\x00\x01\x2D\x00\x0A"
    FRTITLE_FIND_TEXT_STR "\x00"
    "\x00\x00\x01\x36\x00\x06"
    FRLABEL_FIND_STR "\x00"
    "\x00\x00\x01\x37\x00\x06"
    FRLABEL_NEXT_STR "\x00"
    "\x00\x00\x01\x38\x00\x08"
    FRLABEL_CANCEL_STR "\x00"
    "\x00\x00\x01\x39\x00\x0E"
    FRLABEL_IGNORE_CASE_STR "\x00\x00"
    "\x00\x00\x01\x3A\x00\x12"
    FRLABEL_ONLY_WHOLE_WORDS_STR "\x00"
    "\x00\x00\x01\x3B\x00\x10"
    FRLABEL_SCAN_DATABASE_STR "\x00\x00"
    "\x00\x00\x01\x4A\x00\x20"
    FRNODE_FOUND_NO_OCCURRENCES_STR "\x00"
    "\x00\x00\x01\x4B\x00\x1E"
    FRNODE_FOUND_ONE_OCCURRENCE_STR "\x00"
    "\x00\x00\x01\x4C\x00\x22"
    FRNODE_FOUND_OCCURRENCES_STR "\x00\x00"
    "\x00\x00\x01\x90\x00\x08"
    MENU_PROJECT_STR "\x00"
    "\x00\x00\x01\x91\x00\x0A"
    ITEM_OPEN_STR "\x00"
    "\x00\x00\x01\x92\x00\x0E"
    ITEM_SAVE_AS_STR "\x00\x00"
    "\x00\x00\x01\x93\x00\x08"
    ITEM_PRINT_STR "\x00"
    "\x00\x00\x01\x94\x00\x0C"
    ITEM_ABOUT_STR "\x00\x00"
    "\x00\x00\x01\x95\x00\x08"
    ITEM_QUIT_STR "\x00\x00"
    "\x00\x00\x01\xA9\x00\x06"
    MENU_EDIT_STR "\x00\x00"
    "\x00\x00\x01\xAA\x00\x08"
    ITEM_MARK_STR "\x00\x00"
    "\x00\x00\x01\xAB\x00\x08"
    ITEM_COPY_STR "\x00\x00"
    "\x00\x00\x01\xAC\x00\x0E"
    ITEM_SELECT_ALL_STR "\x00\x00"
    "\x00\x00\x01\xAD\x00\x12"
    ITEM_CLEAR_SELECTED_STR "\x00\x00"
    "\x00\x00\x01\xC2\x00\x08"
    MENU_SEARCH_STR "\x00\x00"
    "\x00\x00\x01\xC3\x00\x0A"
    ITEM_FIND_STR "\x00"
    "\x00\x00\x01\xC4\x00\x0C"
    ITEM_FIND_NEXT_STR "\x00"
    "\x00\x00\x01\xC5\x00\x12"
    ITEM_GO_TO_LINE_STR "\x00"
    "\x00\x00\x01\xC6\x00\x10"
    ITEM_SET_BOOKMARK_STR "\x00\x00"
    "\x00\x00\x01\xC7\x00\x12"
    ITEM_GOTO_BOOKMARK_STR "\x00\x00"
    "\x00\x00\x01\xCC\x00\x08"
    MENU_WINDOW_STR "\x00\x00"
    "\x00\x00\x01\xCD\x00\x0C"
    ITEM_MINIMIZE_STR "\x00\x00"
    "\x00\x00\x01\xCE\x00\x0A"
    ITEM_NORMAL_STR "\x00\x00"
    "\x00\x00\x01\xCF\x00\x0C"
    ITEM_MAXIMIZE_STR "\x00\x00"
    "\x00\x00\x01\xD6\x00\x08"
    MENU_EXTRAS_STR "\x00\x00"
    "\x00\x00\x01\xD7\x00\x16"
    ITEM_EXECUTE_COMMAND_STR "\x00\x00"
    "\x00\x00\x01\xE0\x00\x06"
    MENU_PREFS_STR "\x00"
    "\x00\x00\x01\xE1\x00\x14"
    ITEM_SAVE_AS_DEFAULTS_STR "\x00\x00"
    "\x00\x00\x13\xEC\x00\x16"
    LPDERR_NOERR_STR "\x00\x00"
    "\x00\x00\x13\xED\x00\x1C"
    LPDERR_CANCEL_STR "\x00"
    "\x00\x00\x13\xEE\x00\x1E"
    LPDERR_NOTGRAPHICS_STR "\x00"
    "\x00\x00\x13\xEF\x00\x0A"
    LPDERR_OBS1_STR "\x00\x00"
    "\x00\x00\x13\xF0\x00\x1E"
    LPDERR_BADDIMENSION_STR "\x00\x00"
    "\x00\x00\x13\xF1\x00\x0A"
    LPDERR_OBS2_STR "\x00\x00"
    "\x00\x00\x13\xF2\x00\x12"
    LPDERR_INTERNALMEMORY_STR "\x00"
    "\x00\x00\x13\xF3\x00\x12"
    LPDERR_BUFFERMEMORY_STR "\x00"
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


#endif /* TEXTTABLE_H */
