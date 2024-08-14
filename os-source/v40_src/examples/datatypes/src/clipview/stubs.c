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
 * stubs.c
 * Written by David N. Junod
 *
 */

#include "clipview.h"

/*****************************************************************************/

ULONG easyrequest (struct GlobalData * gd, struct EasyStruct * es, ULONG data,...)
{
    return ((ULONG) EasyRequestArgs (gd->gd_Window, es, NULL, (ULONG *) & data));
}

/*****************************************************************************/

BOOL aslrequesttags (struct GlobalData *gd, struct FileRequester *fr, Tag tag1, ...)
{
    return (AslRequest (fr, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG setgadgetattrs (struct GlobalData *gd, struct Gadget *g, struct Window *w, Tag tag1, ...)
{
    return (SetGadgetAttrsA (g, w, NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG setdtattrs (struct GlobalData *gd, Object *o, struct Window *w, Tag tag1, ...)
{
    return (SetDTAttrsA (o, w, NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG setattrs (struct GlobalData *gd, Object *o, Tag tag1, ...)
{
    return (SetAttrsA (o, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG getdtattrs (struct GlobalData *gd, Object *o, Tag tag1, ...)
{
    return (GetDTAttrsA (o, (struct TagItem *) &tag1));
}

/*****************************************************************************/

APTR newobject (struct GlobalData *gd, Class *cl, STRPTR name, Tag tag1, ...)
{
    return (NewObjectA (cl, name, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct Screen *openscreentags (struct GlobalData *gd, Tag tag1, ...)
{
    return (OpenScreenTagList (NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

struct Window *openwindowtags (struct GlobalData *gd, Tag tag1, ...)
{
    return (OpenWindowTagList (NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

APTR newdtobject (struct GlobalData * gd, STRPTR name, Tag tag1,...)
{
    return (NewDTObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Menu *createmenus (struct GlobalData *gd, struct NewMenu *nm, Tag tag1, ...)
{
    return (CreateMenusA (nm, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct Menu *layoutmenus (struct GlobalData *gd, struct Menu *menu, Tag tag1, ...)
{
    return ((struct Menu *)LayoutMenusA (menu, gd->gd_VI, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct Menu *layoutmenuitems (struct GlobalData *gd, struct MenuItem *mi, Tag tag1, ...)
{
    return ((struct Menu *)LayoutMenuItemsA (mi, gd->gd_VI, (struct TagItem *)&tag1));
}

/*****************************************************************************/

VOID setwindowpointer (struct GlobalData *gd, struct Window *win, Tag tag1, ...)
{
    return (SetWindowPointerA (win, (struct TagItem *)&tag1));
}
