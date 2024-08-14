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

#define ERR_NO_FREE_STORE 0
#define ERR_REQUIRES_3_0 1
#define ERR_COULDNT_OPEN_NONVOLATILE 2
#define ERR_COULDNT_OPEN_LOWLEVEL 3
#define ERR_COULDNT_OPEN_ASL 4
#define ERR_COULDNT_OPEN_TABS 5
#define ERR_COULDNT_OPEN_BUTTON 6
#define ERR_COULDNT_OPEN_LED 7
#define ERR_COULDNT_LOCK_DIRECTORY 8
#define ERR_REQUIRES_A_DIRECTORY_NAME 9
#define ERR_COULDNT_CREATE_NONVOLATILE 10
#define ERR_COULDNT_CREATE_POINTER 11
#define TAB_STARTUP 12
#define TAB_STORAGE 13
#define TAB_LANGUAGE 14
#define ID_AMIGA 15
#define ID_AMIGACD 16
#define ID_LOCK 17
#define ID_DELETE 18
#define ID_LOCKED 19
#define ID_UNLOCKED 20
#define ID_PREPARE 21
#define WIN_TITLE 22
#define WIN_PREPARE 23

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define ERR_NO_FREE_STORE_STR "Not enough memory"
#define ERR_REQUIRES_3_0_STR "Requires AmigaDOS Version 3.0"
#define ERR_COULDNT_OPEN_NONVOLATILE_STR "Couldn't open nonvolatile.library"
#define ERR_COULDNT_OPEN_LOWLEVEL_STR "Couldn't open lowlevel.library"
#define ERR_COULDNT_OPEN_ASL_STR "Couldn't open asl.library"
#define ERR_COULDNT_OPEN_TABS_STR "Couldn't open tabs.gadget"
#define ERR_COULDNT_OPEN_BUTTON_STR "Couldn't open button.gadget"
#define ERR_COULDNT_OPEN_LED_STR "Couldn't open led.image"
#define ERR_COULDNT_LOCK_DIRECTORY_STR "Couldn't lock directory %s"
#define ERR_REQUIRES_A_DIRECTORY_NAME_STR "Requires a directory name"
#define ERR_COULDNT_CREATE_NONVOLATILE_STR "Couldn't create nonvolatile directory"
#define ERR_COULDNT_CREATE_POINTER_STR "Couldn't create nonvolatile pointer file"
#define TAB_STARTUP_STR "Startup"
#define TAB_STORAGE_STR "Storage"
#define TAB_LANGUAGE_STR "Language"
#define ID_AMIGA_STR "Amiga"
#define ID_AMIGACD_STR "Amiga CD"
#define ID_LOCK_STR "Lock"
#define ID_DELETE_STR "Delete"
#define ID_LOCKED_STR "Locked"
#define ID_UNLOCKED_STR "Unlocked"
#define ID_PREPARE_STR "Prepare..."
#define WIN_TITLE_STR "CD Preferences"
#define WIN_PREPARE_STR "Select Volume to Prepare NV On"

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
    {ERR_NO_FREE_STORE,(STRPTR)ERR_NO_FREE_STORE_STR},
    {ERR_REQUIRES_3_0,(STRPTR)ERR_REQUIRES_3_0_STR},
    {ERR_COULDNT_OPEN_NONVOLATILE,(STRPTR)ERR_COULDNT_OPEN_NONVOLATILE_STR},
    {ERR_COULDNT_OPEN_LOWLEVEL,(STRPTR)ERR_COULDNT_OPEN_LOWLEVEL_STR},
    {ERR_COULDNT_OPEN_ASL,(STRPTR)ERR_COULDNT_OPEN_ASL_STR},
    {ERR_COULDNT_OPEN_TABS,(STRPTR)ERR_COULDNT_OPEN_TABS_STR},
    {ERR_COULDNT_OPEN_BUTTON,(STRPTR)ERR_COULDNT_OPEN_BUTTON_STR},
    {ERR_COULDNT_OPEN_LED,(STRPTR)ERR_COULDNT_OPEN_LED_STR},
    {ERR_COULDNT_LOCK_DIRECTORY,(STRPTR)ERR_COULDNT_LOCK_DIRECTORY_STR},
    {ERR_REQUIRES_A_DIRECTORY_NAME,(STRPTR)ERR_REQUIRES_A_DIRECTORY_NAME_STR},
    {ERR_COULDNT_CREATE_NONVOLATILE,(STRPTR)ERR_COULDNT_CREATE_NONVOLATILE_STR},
    {ERR_COULDNT_CREATE_POINTER,(STRPTR)ERR_COULDNT_CREATE_POINTER_STR},
    {TAB_STARTUP,(STRPTR)TAB_STARTUP_STR},
    {TAB_STORAGE,(STRPTR)TAB_STORAGE_STR},
    {TAB_LANGUAGE,(STRPTR)TAB_LANGUAGE_STR},
    {ID_AMIGA,(STRPTR)ID_AMIGA_STR},
    {ID_AMIGACD,(STRPTR)ID_AMIGACD_STR},
    {ID_LOCK,(STRPTR)ID_LOCK_STR},
    {ID_DELETE,(STRPTR)ID_DELETE_STR},
    {ID_LOCKED,(STRPTR)ID_LOCKED_STR},
    {ID_UNLOCKED,(STRPTR)ID_UNLOCKED_STR},
    {ID_PREPARE,(STRPTR)ID_PREPARE_STR},
    {WIN_TITLE,(STRPTR)WIN_TITLE_STR},
    {WIN_PREPARE,(STRPTR)WIN_PREPARE_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x00\x00\x12"
    ERR_NO_FREE_STORE_STR "\x00"
    "\x00\x00\x00\x01\x00\x1E"
    ERR_REQUIRES_3_0_STR "\x00"
    "\x00\x00\x00\x02\x00\x22"
    ERR_COULDNT_OPEN_NONVOLATILE_STR "\x00"
    "\x00\x00\x00\x03\x00\x20"
    ERR_COULDNT_OPEN_LOWLEVEL_STR "\x00\x00"
    "\x00\x00\x00\x04\x00\x1A"
    ERR_COULDNT_OPEN_ASL_STR "\x00"
    "\x00\x00\x00\x05\x00\x1A"
    ERR_COULDNT_OPEN_TABS_STR "\x00"
    "\x00\x00\x00\x06\x00\x1C"
    ERR_COULDNT_OPEN_BUTTON_STR "\x00"
    "\x00\x00\x00\x07\x00\x18"
    ERR_COULDNT_OPEN_LED_STR "\x00"
    "\x00\x00\x00\x08\x00\x1C"
    ERR_COULDNT_LOCK_DIRECTORY_STR "\x00\x00"
    "\x00\x00\x00\x09\x00\x1A"
    ERR_REQUIRES_A_DIRECTORY_NAME_STR "\x00"
    "\x00\x00\x00\x0A\x00\x26"
    ERR_COULDNT_CREATE_NONVOLATILE_STR "\x00"
    "\x00\x00\x00\x0B\x00\x2A"
    ERR_COULDNT_CREATE_POINTER_STR "\x00\x00"
    "\x00\x00\x00\x0C\x00\x08"
    TAB_STARTUP_STR "\x00"
    "\x00\x00\x00\x0D\x00\x08"
    TAB_STORAGE_STR "\x00"
    "\x00\x00\x00\x0E\x00\x0A"
    TAB_LANGUAGE_STR "\x00\x00"
    "\x00\x00\x00\x0F\x00\x06"
    ID_AMIGA_STR "\x00"
    "\x00\x00\x00\x10\x00\x0A"
    ID_AMIGACD_STR "\x00\x00"
    "\x00\x00\x00\x11\x00\x06"
    ID_LOCK_STR "\x00\x00"
    "\x00\x00\x00\x12\x00\x08"
    ID_DELETE_STR "\x00\x00"
    "\x00\x00\x00\x13\x00\x08"
    ID_LOCKED_STR "\x00\x00"
    "\x00\x00\x00\x14\x00\x0A"
    ID_UNLOCKED_STR "\x00\x00"
    "\x00\x00\x00\x15\x00\x0C"
    ID_PREPARE_STR "\x00\x00"
    "\x00\x00\x00\x16\x00\x10"
    WIN_TITLE_STR "\x00\x00"
    "\x00\x00\x00\x17\x00\x20"
    WIN_PREPARE_STR "\x00\x00"
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
