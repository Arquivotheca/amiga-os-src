#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#define ERR_NOT_ENOUGH_MEMORY 100
#define ERR_NOT_ENOUGH_MEMORY_STR "Not enough memory"

#define ERR_CANT_OPEN_WINDOW 101
#define ERR_CANT_OPEN_WINDOW_STR "Can't open window"

#define ERR_REQUIRED_ARG_MISSING 102
#define ERR_REQUIRED_ARG_MISSING_STR "Required argument missing"

#define ERR_REQUIRES_FILE_NAME 103
#define ERR_REQUIRES_FILE_NAME_STR "Requires a file name"

#define ERR_CANT_OPEN_FILE 104
#define ERR_CANT_OPEN_FILE_STR "Can't open file"

#define ERR_FILE_ISNT_AN_ILBM 105
#define ERR_FILE_ISNT_AN_ILBM_STR "File isn't an ILBM"

#define TITLE_LOADING 200
#define TITLE_LOADING_STR "Loading..."

#define TITLE_REMAPPING 201
#define TITLE_REMAPPING_STR "Remapping..."

#define TITLE_SCALING 202
#define TITLE_SCALING_STR "Scaling..."

#define TITLE_ZOOMING 203
#define TITLE_ZOOMING_STR "Zooming..."

#define TITLE_CALCULATING 204
#define TITLE_CALCULATING_STR "Calculating..."

#define TITLE_CONVERTING_TRUE_COLOR 205
#define TITLE_CONVERTING_TRUE_COLOR_STR "Converting true color..."

#define STATUS_FULL 220
#define STATUS_FULL_STR "Full"

#define STATUS_SCALE 221
#define STATUS_SCALE_STR "Scale"

#define STATUS_ZOOM 222
#define STATUS_ZOOM_STR "Zoom"

#define MISC_APPMENUITEM 240
#define MISC_APPMENUITEM_STR "WDisplay..."

#define MISC_ABOUT_TITLE 241
#define MISC_ABOUT_TITLE_STR "About WDisplay"

#define MISC_COPYRIGHT 242
#define MISC_COPYRIGHT_STR "Copyright"

#define MISC_WRITTEN_BY 243
#define MISC_WRITTEN_BY_STR "Written by"


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {ERR_NOT_ENOUGH_MEMORY,ERR_NOT_ENOUGH_MEMORY_STR},
    {ERR_CANT_OPEN_WINDOW,ERR_CANT_OPEN_WINDOW_STR},
    {ERR_REQUIRED_ARG_MISSING,ERR_REQUIRED_ARG_MISSING_STR},
    {ERR_REQUIRES_FILE_NAME,ERR_REQUIRES_FILE_NAME_STR},
    {ERR_CANT_OPEN_FILE,ERR_CANT_OPEN_FILE_STR},
    {ERR_FILE_ISNT_AN_ILBM,ERR_FILE_ISNT_AN_ILBM_STR},
    {TITLE_LOADING,TITLE_LOADING_STR},
    {TITLE_REMAPPING,TITLE_REMAPPING_STR},
    {TITLE_SCALING,TITLE_SCALING_STR},
    {TITLE_ZOOMING,TITLE_ZOOMING_STR},
    {TITLE_CALCULATING,TITLE_CALCULATING_STR},
    {TITLE_CONVERTING_TRUE_COLOR,TITLE_CONVERTING_TRUE_COLOR_STR},
    {STATUS_FULL,STATUS_FULL_STR},
    {STATUS_SCALE,STATUS_SCALE_STR},
    {STATUS_ZOOM,STATUS_ZOOM_STR},
    {MISC_APPMENUITEM,MISC_APPMENUITEM_STR},
    {MISC_ABOUT_TITLE,MISC_ABOUT_TITLE_STR},
    {MISC_COPYRIGHT,MISC_COPYRIGHT_STR},
    {MISC_WRITTEN_BY,MISC_WRITTEN_BY_STR},
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
