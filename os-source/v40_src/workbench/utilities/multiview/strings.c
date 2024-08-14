/* strings.c
 *
 */

#define STRINGARRAY	TRUE

#include "multiview.h"

/*****************************************************************************/

#undef	SysBase

/*****************************************************************************/

STRPTR GetString (struct GlobalData * gd, LONG id)
{
    STRPTR local = NULL;
    UWORD i;

    i = 0;
    while (!local)
    {
	if (AppStrings[i].as_ID == id)
	    local = AppStrings[i].as_Str;
	i++;
    }

    if (gd && gd->gd_LocaleBase)
	local = GetCatalogStr (gd->gd_Catalog, id, local);

    return (local);
}

/*****************************************************************************/

#undef	DOSBase
#undef	IntuitionBase

/*****************************************************************************/

VOID PrintF (struct GlobalData * gd, LONG mode, LONG id, STRPTR arg1,...)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Window *win = (gd) ? gd->gd_Window : NULL;
    struct Library *IntuitionBase;
    struct Library *DOSBase;
    struct EasyStruct es;
    STRPTR ermsg = NULL;
    UBYTE buffer[128];

    if (id == 0)
	return;

    if (DOSBase = OpenLibrary ("dos.library", 36))
    {
	IntuitionBase = OpenLibrary ("intuition.library", 36);

	if (id < 990)
	{
	    if (Fault (id, NULL, buffer, sizeof (buffer)))
		ermsg = buffer;
	}
	else if ((id >= 2000) && (id <= 2999))
	{
	    ermsg = GetDTString (id);
	}
	else if (((id >= 990) && (id <= 999)) || ((id >= 7000) && (id <= 7999)))
	{
	    ermsg = GetString (gd, id);
	}

	if (ermsg)
	{
	    if ((mode == 2) && win)
	    {
		sprintf (gd->gd_Title, ermsg, (LONG *) arg1);
		SetWindowTitles (gd->gd_Window, gd->gd_Title, "MultiView");
	    }
	    else if ((mode == 0) && ((gd == NULL) || (gd && gd->gd_Process->pr_CLI)))
	    {
		VPrintf (ermsg, (LONG *) & arg1);
		VPrintf ("\n", NULL);
	    }
	    else if (win)
	    {
		es.es_StructSize = sizeof (struct EasyStruct);
		es.es_Flags = 0;
		es.es_Title = GetString (gd, MV_TITLE_MULTIVIEW);
		es.es_TextFormat = ermsg;
		es.es_GadgetFormat = GetString (gd, MV_LABEL_CONTINUE);

		EasyRequestArgs (win, &es, NULL, &arg1);
	    }
	}

	CloseLibrary (IntuitionBase);
	CloseLibrary (DOSBase);
    }
}
