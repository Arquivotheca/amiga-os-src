/* menus.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

struct EdMenu
{
    UBYTE		 em_Type;        /* NM_XXX from gadtools.h */
    LONG		 em_Label;
    ULONG		 em_Command;
    UWORD		 em_ItemFlags;
};

/*****************************************************************************/

struct EdMenu newmenu[] =
{
    {NM_TITLE, MV_MENU_PROJECT,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_OPEN,		MMC_OPEN,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_SAVE_AS,		MMC_SAVEAS,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_PRINT,		MMC_PRINT,		0},
    {NM_ITEM,  MV_ITEM_ABOUT,		MMC_ABOUT,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_QUIT,		MMC_QUIT,		0},

    {NM_TITLE, MV_MENU_EDIT,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_MARK,		MMC_MARK,		0},
    {NM_ITEM,  MV_ITEM_COPY,		MMC_COPY,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_SELECT_ALL,	MMC_SELECTALL,		0},
    {NM_ITEM,  MV_ITEM_CLEAR_SELECTED,	MMC_CLEARSELECTED,	0},

    {NM_TITLE, MV_MENU_SEARCH,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_FIND,		MMC_FIND,		0},
    {NM_ITEM,  MV_ITEM_FIND_NEXT,	MMC_NEXT,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_GO_TO_LINE,	MMC_GOTO,		0},
    {NM_ITEM,  MSG_NOTHING,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_SET_BOOKMARK,	MMC_SETBOOKMARK,	0},
    {NM_ITEM,  MV_ITEM_GOTO_BOOKMARK,	MMC_GOTOBOOKMARK,	0},

    {NM_TITLE, MV_MENU_WINDOW,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_MINIMIZE,	MMC_MINIMIZE,		0},
    {NM_ITEM,  MV_ITEM_NORMAL,		MMC_NORMAL,		0},
    {NM_ITEM,  MV_ITEM_MAXIMIZE,	MMC_MAXIMIZE,		0},

    {NM_TITLE, MV_MENU_EXTRAS,		MMC_NOP,		0},
    {NM_ITEM,  MV_ITEM_EXECUTE_COMMAND,	MMC_EXECUTE,		0},

    {NM_END,   MSG_NOTHING,		MMC_NOP,		0},
};

/*****************************************************************************/

BOOL LayoutPrefsMenus (struct GlobalData * gd, struct Menu * menus, ULONG tag1,...)
{
    return (LayoutMenusA (menus, gd->gd_VI, (struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Menu *CreateLocaleMenus (struct GlobalData * gd, struct EdMenu * em)
{
    struct NewMenu *nm;
    struct Menu *menus;
    UWORD i;

    i = 0;
    while (em[i++].em_Type != NM_END)
    {
    }

    if (!(nm = AllocVec (sizeof (struct NewMenu) * i, MEMF_CLEAR | MEMF_PUBLIC)))
	return (NULL);

    while (i--)
    {
	nm[i].nm_Type = em[i].em_Type;
	nm[i].nm_Flags = em[i].em_ItemFlags;
	nm[i].nm_UserData = (APTR) em[i].em_Command;

	if (em[i].em_Type == NM_TITLE)
	{
	    nm[i].nm_Label = GetString (gd, em[i].em_Label);
	}
	else if (em[i].em_Command == MMC_NOP)
	{
	    nm[i].nm_Label = NM_BARLABEL;
	}
	else if (em[i].em_Type != NM_END)
	{
	    nm[i].nm_CommKey = GetString (gd, em[i].em_Label);
	    nm[i].nm_Label = &nm[i].nm_CommKey[2];
	    if (nm[i].nm_CommKey[0] == ' ')
	    {
		nm[i].nm_CommKey = NULL;
	    }
	}
    }

    if (menus = CreateMenusA (nm, NULL))
    {
	if (!(LayoutPrefsMenus (gd, menus, GTMN_NewLookMenus, TRUE,
				TAG_DONE)))
	{
	    FreeMenus (menus);
	    menus = NULL;
	}
    }

    FreeVec (nm);

    return (menus);
}

/*****************************************************************************/

void AddStandardMenu (struct GlobalData *gd)
{
    if (gd->gd_Menu = CreateLocaleMenus (gd, newmenu))
	SetMenuStrip (gd->gd_Window, gd->gd_Menu);
}

/*****************************************************************************/

void FreeStandardMenu (struct GlobalData *gd)
{
    FreeMenus (gd->gd_Menu);
}
