/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * texttable.h
 *
 */

#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/****************************************************************************/


#define ERROR_REQUIRES_V39 5000
#define ERROR_REQUIRES_V39_STR "Requires V39"

#define ERROR_CLIPBOARD_EMPTY 5001
#define ERROR_CLIPBOARD_EMPTY_STR "Clipboard Empty"

#define ERROR_COULDNT_SAVE_FILE 5002
#define ERROR_COULDNT_SAVE_FILE_STR "Couldn't save file %s"

#define TITLE_CLIPVIEW 5003
#define TITLE_CLIPVIEW_STR "ClipView"

#define TITLE_PRINTING 5004
#define TITLE_PRINTING_STR "Printing..."

#define TITLE_SELECT_FILE_TO_SAVE_TO 5005
#define TITLE_SELECT_FILE_TO_SAVE_TO_STR "Select File to Save to"

#define LABEL_ABORT 5006
#define LABEL_ABORT_STR "Abort"

#define LABEL_CONTINUE 5007
#define LABEL_CONTINUE_STR "Continue"

#define LABEL_SAVE 5008
#define LABEL_SAVE_STR "Save"

#define MENU_PROJECT 5009
#define MENU_PROJECT_STR "Project"

#define ITEM_SAVE_AS 5010
#define ITEM_SAVE_AS_STR "A\0Save As..."

#define ITEM_PRINT 5011
#define ITEM_PRINT_STR "P\0Print..."

#define ITEM_ABOUT 5012
#define ITEM_ABOUT_STR "?\0About..."

#define ITEM_QUIT 5013
#define ITEM_QUIT_STR "Q\0Quit"

#define MENU_EDIT 5014
#define MENU_EDIT_STR "Edit"

#define ITEM_MARK 5015
#define ITEM_MARK_STR "B\0Mark"

#define ITEM_COPY 5016
#define ITEM_COPY_STR "C\0Copy"

#define LPDERR_NOERR 5100
#define LPDERR_NOERR_STR "Operation Successful"

#define LPDERR_CANCEL 5101
#define LPDERR_CANCEL_STR "User Canceled Print Request"

#define LPDERR_NOTGRAPHICS 5102
#define LPDERR_NOTGRAPHICS_STR "Printer Cannot Ouput Graphics"

#define LPDERR_OBS1 5103
#define LPDERR_OBS1_STR "obsolete"

#define LPDERR_BADDIMENSION 5104
#define LPDERR_BADDIMENSION_STR "Print Dimensions are Illegal"

#define LPDERR_OBS2 5105
#define LPDERR_OBS2_STR "obsolete"

#define LPDERR_INTERNALMEMORY 5106
#define LPDERR_INTERNALMEMORY_STR "Not Enough Memory"

#define LPDERR_BUFFERMEMORY 5107
#define LPDERR_BUFFERMEMORY_STR "Not Enough Memory"


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {ERROR_REQUIRES_V39,(STRPTR)ERROR_REQUIRES_V39_STR},
    {ERROR_CLIPBOARD_EMPTY,(STRPTR)ERROR_CLIPBOARD_EMPTY_STR},
    {ERROR_COULDNT_SAVE_FILE,(STRPTR)ERROR_COULDNT_SAVE_FILE_STR},
    {TITLE_CLIPVIEW,(STRPTR)TITLE_CLIPVIEW_STR},
    {TITLE_PRINTING,(STRPTR)TITLE_PRINTING_STR},
    {TITLE_SELECT_FILE_TO_SAVE_TO,(STRPTR)TITLE_SELECT_FILE_TO_SAVE_TO_STR},
    {LABEL_ABORT,(STRPTR)LABEL_ABORT_STR},
    {LABEL_CONTINUE,(STRPTR)LABEL_CONTINUE_STR},
    {LABEL_SAVE,(STRPTR)LABEL_SAVE_STR},
    {MENU_PROJECT,(STRPTR)MENU_PROJECT_STR},
    {ITEM_SAVE_AS,(STRPTR)ITEM_SAVE_AS_STR},
    {ITEM_PRINT,(STRPTR)ITEM_PRINT_STR},
    {ITEM_ABOUT,(STRPTR)ITEM_ABOUT_STR},
    {ITEM_QUIT,(STRPTR)ITEM_QUIT_STR},
    {MENU_EDIT,(STRPTR)MENU_EDIT_STR},
    {ITEM_MARK,(STRPTR)ITEM_MARK_STR},
    {ITEM_COPY,(STRPTR)ITEM_COPY_STR},
    {LPDERR_NOERR,(STRPTR)LPDERR_NOERR_STR},
    {LPDERR_CANCEL,(STRPTR)LPDERR_CANCEL_STR},
    {LPDERR_NOTGRAPHICS,(STRPTR)LPDERR_NOTGRAPHICS_STR},
    {LPDERR_OBS1,(STRPTR)LPDERR_OBS1_STR},
    {LPDERR_BADDIMENSION,(STRPTR)LPDERR_BADDIMENSION_STR},
    {LPDERR_OBS2,(STRPTR)LPDERR_OBS2_STR},
    {LPDERR_INTERNALMEMORY,(STRPTR)LPDERR_INTERNALMEMORY_STR},
    {LPDERR_BUFFERMEMORY,(STRPTR)LPDERR_BUFFERMEMORY_STR},
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
