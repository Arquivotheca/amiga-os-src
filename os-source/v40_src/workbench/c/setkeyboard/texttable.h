

#include <exec/types.h>


/* Text strings used by ciapatcher */
enum AppStringsID
{
    MSG_NORMAL,
    MSG_ERROR_CANT_OPEN_RESOURCE,
    MSG_ERROR_CANT_OPEN_CONSOLE_DEV,
    MSG_ERROR_CANT_SET_CONSOLE_MAP,
    MSG_ERROR_KEYMAP_NOT_FOUND,
    MSG_ERROR_INVALID_KEYMAP
};


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};



STRPTR GetString(struct LocaleInfo *li,enum AppStringsID id);
