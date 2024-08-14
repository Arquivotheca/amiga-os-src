/* stubs.c
 *
 */

#include "graffiti.h"

/*****************************************************************************/

VOID setwindowpointer (struct GlobalData *gd, struct Window *win, Tag tag1, ...)
{
    if (win)
	SetWindowPointerA (win, (struct TagItem *)&tag1);
}

/*****************************************************************************/

struct Gadget *creategadget (struct GlobalData * gd, ULONG kind, struct Gadget * prev, struct NewGadget * ng, ULONG data,...)
{
    return (CreateGadgetA (kind, prev, ng, (struct TagItem *)&data));
}

/*****************************************************************************/

struct Menu *createmenus (struct GlobalData *gd, struct NewMenu *nm, ULONG data, ...)
{
    return (CreateMenusA (nm, (struct TagItem *)&data));
}

/*****************************************************************************/

struct Menu *layoutmenus (struct GlobalData *gd, ULONG data, ...)
{
    return (LayoutMenusA (gd->gd_Menu, gd->gd_VI, (struct TagItem *)&data));
}

/*****************************************************************************/

struct Transaction *alloctransaction (struct GlobalData * gd, Tag tag1,...)
{

    return (AllocTransactionA ((struct TagItem *) & tag1));
}

/*****************************************************************************/

BOOL nipcinquiry (struct GlobalData * gd, struct Hook * h, ULONG time, ULONG num, Tag tag1,...)
{

    return (NIPCInquiryA (h, time, num, (struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Entity *createentity (struct GlobalData * gd, Tag tag1,...)
{

    return (CreateEntityA ((struct TagItem *) & tag1));
}

/*****************************************************************************/

int stcgnm (char *dest, char *src)
{
    int i, j;

    j = strlen (src);
    for (i = 0; i < j; i++)
    {
	if (src[i] == '@')
	{
	    strcpy (dest, &src[i+1]);
	    return (j-i);
	}
    }
    strcpy (dest, src);
    return (j);
}

/*****************************************************************************/

int stcgfp (char *dest, char *src)
{
    int i, j;

    j = strlen (src);
    for (i = 0; i < j; i++)
    {
	if (src[i] == ':')
	{
	    dest[i] = 0;
	    return (i);
	}
	dest[i] = src[i];
    }
    dest[0] = 0;
    return (0);
}

/*****************************************************************************/

int stcgfn (char *dest, char *src)
{
    int i, j;

    j = strlen (src);
    for (i = 0; i < j; i++)
    {
	if (src[i] == ':')
	{
	    strcpy (dest, &src[i+1]);
	    return (j-i);
	}
    }
    strcpy (dest, src);
    return (j);
}

/*****************************************************************************/

void lprintf (struct GlobalData * gd, STRPTR fmt, void *arg1,...)
{
    VPrintf (fmt, (LONG *) & arg1);
}

