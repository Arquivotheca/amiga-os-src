/* cstubs.c
 *
 */

#include "wbpattern.h"

struct Screen *OpenPrefsScreen (EdDataPtr ed, ULONG tag1,...)
{
    return (OpenScreenTagList (NULL, (struct TagItem *) & tag1));
}

VOID DrawBB (EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags,...)
{
    DrawBevelBoxA (ed->ed_Window->RPort, x, y, w, h, (struct TagItem *) & tags);
}

VOID setgadgetattrs (EdDataPtr ed, struct Gadget * gad, ULONG tags,...)
{
    SetGadgetAttrsA (gad, ed->ed_Window, NULL, (struct TagItem *) & tags);
}

APTR NewPrefsObject (EdDataPtr ed, struct IClass * cl, UBYTE * name, ULONG data,...)
{
    return (NewObjectA (cl, name, (struct TagItem *) & data));
}
