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
 * strings.c
 * Written by David N. Junod
 *
 */

#define STRINGARRAY	TRUE

#include "clipview.h"

/*****************************************************************************/

#undef	SysBase
#undef	DOSBase
#undef	IntuitionBase

/*****************************************************************************/

STRPTR null = "";

/*****************************************************************************/

STRPTR GetString (struct GlobalData * gd, LONG id)
{
    STRPTR local;
    UWORD i;

    for (i = 0, local = null; (local == null); i++)
    {
	if (AppStrings[i].as_ID == id)
	    local = AppStrings[i].as_Str;
    }

    if (gd && gd->gd_LocaleBase)
    {
	local = GetCatalogStr (gd->gd_Catalog, id, local);
    }

    return (local);
}

/*****************************************************************************/

VOID PrintF (struct GlobalData * gd, LONG mode, LONG id, STRPTR arg1,...)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Window *win = (gd) ? gd->gd_Window : NULL;
    struct Library *IntuitionBase;
    struct Library *DOSBase;
    UBYTE namebuffer[64];
    struct EasyStruct es;
    STRPTR ermsg = NULL;

    if (id == 0)
	return;

    if (DOSBase = OpenLibrary ("dos.library", 36))
    {
	IntuitionBase = OpenLibrary ("intuition.library", 36);

	if (id < 990)
	{
	    if (Fault (id, NULL, gd->gd_TempBuffer, 510))
	    {
		ermsg = gd->gd_TempBuffer;
	    }
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
	    if ((mode == 2) && gd->gd_Window)
	    {
		sprintf (namebuffer, ermsg, (LONG *) arg1);
		SetWindowTitles (gd->gd_Window, namebuffer, (UBYTE *)~0);
	    }
	    else if ((mode == 0) && gd && gd->gd_Process->pr_CLI)
	    {
		VPrintf (ermsg, (LONG *) & arg1);
		VPrintf ("\n", NULL);
	    }
	    else
	    {
		es.es_StructSize = sizeof (struct EasyStruct);
		es.es_Flags = 0;
		es.es_Title = GetString (gd, TITLE_CLIPVIEW);
		es.es_TextFormat = ermsg;
		es.es_GadgetFormat = GetString (gd, LABEL_CONTINUE);

		EasyRequestArgs (win, &es, NULL, &arg1);
	    }
	}

	CloseLibrary (IntuitionBase);
	CloseLibrary (DOSBase);
    }
}