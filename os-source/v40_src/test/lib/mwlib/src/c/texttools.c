/****** MWLib/TextTools *******************************************************
    TextTools.c - Create an IntuiText chain

    Mitchell/Ware Systems       Version 2.00        5/22/88

    ________________________________________________________________________

    IntuiText *MakeIText(key, text, left, top, itchain, ta)
        key         - the Remember structure
        text        - null-terminated text
        left, top   - where it will appear
        itchain     - ptr to next IntuiText structure
        ta          - pointer to a TextAttr or NULL


    void SetITFrontBack(front, back)
            sets the default front pen and back pen

    void SetITDrawMode(mode)
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <Tools.h>

static int FrontPen = 1, BackPen = 0;
static USHORT DrawMode = JAM1;

void SetITFrontBack(front, back)
int front, back;
{
    FrontPen = front;
    BackPen = back;
}

void SetITDrawMode(mode)
{
    DrawMode = mode;
}

struct IntuiText *MakeIText(key, text, left, top, itchain, ta)
struct Remember **key;
UBYTE *text;
int left, top;
struct IntuiText *itchain;
struct TextAttr *ta;
{
    struct IntuiText *it = NULL;

    if (!(it = (struct IntuiText *) AllocRemember(key, sizeof(*it), MEMF_CLEAR | MEMF_PUBLIC)))
        return NULL;

    it->FrontPen = FrontPen;
    it->BackPen = BackPen;
    it->LeftEdge = left;
    it->TopEdge = top;
    it->IText = text;
    it->NextText = itchain;
    it->ITextFont = ta;
    return it;
}

struct IntuiText *MakeITList(key, list) /* Make a chain of IntuiText structs */
struct Remember **key;
struct ITList *list;
{
    struct ITList *l;

    for (l = list; l->string; ++l)
    {
        SetITFrontBack(l->pen, BackPen);
        l->itext = MakeIText(key, l->string, l->left, l->top, NULL, (l->ta.ta_Name) ? &l->ta : NULL);
        if (l->itext)
        {
            if (l > list)
                (l - 1)->itext->NextText = l->itext;
        }
        else /* out of memory */
            return NULL;
    }

    return list->itext;
}
