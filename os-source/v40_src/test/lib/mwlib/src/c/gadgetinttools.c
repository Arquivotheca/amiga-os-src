/****** GadgetIntTools ********************************************************
    GadgetIntTools.c

    Mitchell/Ware Systems           Version 1.01            12-Sep-90

    Tools for Creating Gadgets for InterfaceTools. Reies on GadgetTools.

    Make a Border list
        MakeIntGEBorders(struct Remember **key, IntGEntry ige[])

    Make a IntGE Gadget list
        MakeIntGEList(struct Remember **key, IntGEActive list[],
                        struct CustomGadgetClass *class_list)

    Refresh a IntGE Gadget list - currently only does the string gadgets
        RefreshIntGEList(IntGEActive list[], struct Window *win)

    NOTE: A NULL GADGET is one with type NULL (in the IntGEList). Such a gadget
          is used for boxing large regions not directly related to the
          normal action of a gadget. MakeRGList is set up to ignore such
          'gadgets'.

*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <math.h>
#include <string.h>
#include "tools.h"

#define PUB     (MEMF_PUBLIC | MEMF_CLEAR)
#define CHIP    (MEMF_CHIP | MEMF_CLEAR)

#define BE      2       /* box expansion default for MakeIntGEBorders */

extern long GadgetIDCounter;

#
/*****************************************************************************

    Make InterfaceTools Gadget List

*****************************************************************************/

struct Gadget *MakeIntGEList (struct Remember **key, struct IntGEActive list[], struct CustomGadgetClass *cgc)
{
    struct Gadget *g, *fg = NULL, *MakeCRBGadget(), *MakePropGadget(), *MakeStrGadget();
    struct StringInfo *sinfo;
    struct PropInfo *pinfo;
    struct IntGEActive *l, *ll;
    UBYTE *buf, *undo;
    struct CustomGadgetClass *pcgc;

    GadgetIDCounter = 0; /* so that gadgets can index the list */
    for (l = list; l->entry; ++l)
    {
        if (!l->g)
        {
            if (!l->entry->type || (l->entry->type == BOOLGADGET))
            {
                l->g = MakeCRBGadget(key, l->entry->left, l->entry->top, l->entry->width,
                        l->entry->height, l->entry->flags, l->entry->activation);
                l->g->GadgetID = l->entry->userid;
                l->g->UserData = l;
                l->g->NextGadget = NULL;

                if (l->entry->norm)
                {
                    l->g->GadgetRender = l->entry->norm;
                    l->g->Flags |= GADGIMAGE;
                }

                if (l->entry->sel)
                {
                    l->g->SelectRender = l->entry->sel;
                    l->g->Flags |= GADGHIMAGE;
                }

                l->g->Activation |= GADGIMMEDIATE;
            }
            else if (l->entry->type == PROPGADGET)
            {
                l->g = MakePropGadget(key, l->entry->norm,
                                l->entry->left, l->entry->top, l->entry->width, l->entry->height,
                                l->entry->flags,
                                l->entry->general | ((l->entry->norm) ? NULL : AUTOKNOB),
                                l->entry->activation);

                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->VertPot = l->vertpot;
                pinfo->HorizBody = l->horizbody;
                pinfo->VertBody = l->vertbody;
            }
            else if (l->entry->type == STRGADGET)
            {
                /* For the StringGadget, l->entry->general is the size of buffer
                    in bytes. The space will be allocated both for the
                    main buffer and the undo buffer. You can directly
                    get at these buffers through the gadget.

                    ALSO, if l->entry->fix is non-zero and the activation
                    is LONGINT, then fix is used for the placement
                    of a decimal point. The LONGINT activation is cleared,
                    and the strings are initialized with the apporiate
                    numerical digits & points. This works in conjunction
                    with RequesterTools to translate the string into a
                    number to be placed in l->value. */

                buf = AllocRemember(key, l->entry->general, PUB);
                undo = AllocRemember(key, l->entry->general, PUB);
                if (!buf || !undo)
                    return NULL;    /* Out of Memory */
                l->g = MakeStrGadget(key, l->entry->left, l->entry->top, l->entry->width, l->entry->height,
                            l->entry->general, buf, undo, l->entry->flags, l->entry->activation, NULL);

                if (l->entry->activation & LONGINT)
                {
                    if (l->entry->fix)
                    {
                        l->g->Activation &= ~LONGINT;
                        sfix_dec(buf, l->entry->fix, l->value);
                    }
                    else
                    {
                        sprintf(buf, "%d", l->value);
                        sinfo = l->g->SpecialInfo;
                        sinfo->LongInt = l->value;
                    }
                }
                else if (l->string)
                    memcpy(buf, l->string, l->entry->general);
            }
            else if (l->entry->type == PROP_HORIZ)
            {
                l->g = MakeIndHorizProp(key, l->entry->norm, l->entry->left, l->entry->top, l->entry->width, l->entry->flags, l->entry->activation);
                GadgetIDCounter -= 2;
                l->g->NextGadget->GadgetID
                    = l->g->NextGadget->NextGadget->GadgetID
                        = l->g->GadgetID;

                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->HorizBody = l->horizbody;
            }
            else if (l->entry->type == PROP_VERT)
            {
                l->g = MakeIndVertProp(key, l->entry->norm, l->entry->left, l->entry->top, l->entry->height, l->entry->flags, l->entry->activation);
                GadgetIDCounter -= 2;
                l->g->NextGadget->GadgetID
                    = l->g->NextGadget->NextGadget->GadgetID
                        = l->g->GadgetID;

                pinfo = l->g->SpecialInfo;
                pinfo->VertPot = l->vertpot;
                pinfo->VertBody = l->vertbody;
            }
            else if (l->entry->type < 0)   /* Custom Gadget */
            {
                for (pcgc = cgc; pcgc; pcgc = pcgc->next)
                    if (l->entry->type == pcgc->class)
                    {
                        l->class = pcgc;
                        if (pcgc->create && !(*pcgc->create)(key, l, pcgc))
                            l->g = NULL;    /* if error, force a return this way */

                        break;
                    }
            }
            /* else ignore- gadtype is NULL */

            if (l->entry->type && l->entry->type != EVENT_ENTRY && !l->g)
                return NULL;    /* Out of Memory */

            if (l->entry->type && l > list)    /* if not a null gad && not the first */
            {
                for (ll = l - 1; ll > list; --ll)  /* skip null gadgets */
                    if (ll->g)
                        break;

                if (ll->g)  /* if not the first null gadget */
                {
                    g = ll->g;

                    /* Skip over composite gadgets (if any)
                    */
                    while (g->NextGadget)
                        g = g->NextGadget;

                    /* Now set this gadget to point to the current one.
                    */
                    g->NextGadget = l->g;
                }
                else
                    fatal("MakeIntGEList: First Gadget in IntGEList is NULL!");
            }
        }

        if (l->g)
        {
            if (l->selected)
                l->g->Flags |= SELECTED;
            else
                l->g->Flags &= ~SELECTED;

            if (!fg) fg = l->g;
        }
    }
    return fg;  /* first gadget or NULL */
}
#
/*****************************************************************************

    Refresh Requester Gadget List

    Update the STRING & PROPORTIONAL Gadgets!

    NOTE: CLears l->string pointer !!!!!

*****************************************************************************/

void RefreshIntGEList (struct IntGEActive list[], struct Window *win)
{
    struct StringInfo *sinfo;
    struct PropInfo *pinfo;
    struct IntGEActive *l;

    if (win) RemoveGList(win, list->g, -1);
    for (l = list; l->entry; ++l)
    {
        if (l->g)
        {
            if (!l->entry->type || (l->entry->type == BOOLGADGET))
            {
            }
            else if (l->entry->type == PROPGADGET)
            {/*
                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->VertPot = l->vertpot;
                pinfo->HorizBody = l->horizbody;
                pinfo->VertBody = l->vertbody;  */
            }
            else if (l->entry->type == STRGADGET)
            {
                sinfo = l->g->SpecialInfo;

                if (l->entry->activation & LONGINT)
                {
                    if (l->entry->fix)
                    {
                        l->g->Activation &= ~LONGINT;
                        sfix_dec(sinfo->Buffer, l->entry->fix, l->value);
                    }
                    else
                    {
                        sprintf(sinfo->Buffer, "%d", l->value);
                        sinfo->LongInt = l->value;
                    }
                }
                else if (l->string)
                    memcpy(sinfo->Buffer, l->string, l->entry->general), l->string = NULL;
            }
            else if (l->entry->type == PROP_HORIZ)
            {
            }
            else if (l->entry->type == PROP_VERT)
            {
            }
        }
        /*
        if (l->selected)
            l->g->Flags |= SELECTED;
        else
            l->g->Flags &= ~SELECTED;   */
    }

    if (win)
    {
        AddGList(win, list->g, 0, -1, NULL);
        for (l = list; l->entry->width; ++l)
            if (l->g && l->entry->type == STRGADGET)
                RefreshGList(l->g, win, NULL, 1);
    }
}
#
/******************************************************************************

    MakeIntGEBorders()

******************************************************************************/

#define CFS c.cf.seperation

MakeIntGEBorders(struct Remember **key, struct IntGEActive *iga) /* return a linked list of borders */
{
    struct Border *blist, *b;
    struct IntGEActive *a;
    short *xy, i;
    GadControl c;
    short draw; /* draw mode */
    double x, y, w, h, pi12; /* work variables */

    for (a = iga, blist = NULL; a->entry; ++a)
        if (a->entry->control & (CF_BOX | CF_BOX2 | CF_SHADOW | CF_SHADOW2
                                | CF_RAISE | CF_RAISE2 | CF_LOWER | CF_LOWER2
                                | CF_CUSTB | CF_CUSTB2 | CF_RADIO))
        {
            c.c = a->entry->control;
            if (!CFS)
                CFS = BE;

            draw = (c.cf.xor) ? COMPLEMENT : JAM1;

            if (c.cf.box)
            {
                b = _MakeABorder(key, 5, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = a->entry->width;       *xy++ = a->entry->height;
                *xy++ = -1;             *xy++ = a->entry->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = a->entry->width;       *xy++ = -1;
                *xy++ = a->entry->width;       *xy++ = a->entry->height;
            }

            if (c.cf.shadow)
            {
                b = _MakeABorder(key, 7, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = a->entry->width + 1;   *xy++ = a->entry->height + 1;
                *xy++ = 1;              *xy++ = a->entry->height + 1;
                *xy++ = 1;              *xy++ = a->entry->height + 2;
                *xy++ = a->entry->width + 2;   *xy++ = a->entry->height + 2;

                *xy++ = a->entry->width + 2;   *xy++ = 1;
                *xy++ = a->entry->width + 1;   *xy++ = 1;
                *xy++ = a->entry->width + 1;   *xy++ = a->entry->height + 1;
            }

            if (c.cf.box2)
            {
                b = _MakeABorder(key, 5, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = a->entry->width+CFS-1;  *xy++ = a->entry->height+CFS-1;
                *xy++ = -CFS;            *xy++ = a->entry->height+CFS-1;
                *xy++ = -CFS;            *xy++ = -CFS;
                *xy++ = a->entry->width+CFS-1;  *xy++ = -CFS;
                *xy++ = a->entry->width+CFS-1;  *xy++ = a->entry->height+CFS-1;
            }

            if (c.cf.shadow2)
            {
                b = _MakeABorder(key, 7, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = a->entry->width+CFS;    *xy++ = a->entry->height+CFS;
                *xy++ = 2-CFS;           *xy++ = a->entry->height+CFS;
                *xy++ = 2-CFS;           *xy++ = a->entry->height+CFS+1;
                *xy++ = a->entry->width+CFS+1;  *xy++ = a->entry->height+CFS+1;

                *xy++ = a->entry->width+CFS+1;  *xy++ = 2-CFS;
                *xy++ = a->entry->width+CFS;    *xy++ = 2-CFS;
                *xy++ = a->entry->width+CFS;    *xy++ = a->entry->height+CFS;
            }

            if (c.cf.raise)
            {
                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -1;             *xy++ = a->entry->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = a->entry->width;       *xy++ = -1;

                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 0;              *xy++ = a->entry->height;
                *xy++ = a->entry->width;       *xy++ = a->entry->height;
                *xy++ = a->entry->width;       *xy++ = 0;
            }

            if (c.cf.lower)
            {
                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -1;             *xy++ = a->entry->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = a->entry->width;       *xy++ = -1;

                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 0;              *xy++ = a->entry->height;
                *xy++ = a->entry->width;       *xy++ = a->entry->height;
                *xy++ = a->entry->width;       *xy++ = 0;
            }

            if (c.cf.raise2)
            {
                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -CFS;               *xy++ = a->entry->height - 1 +CFS;
                *xy++ = -CFS;               *xy++ = -CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = -CFS;

                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 1 -CFS;             *xy++ = a->entry->height - 1 +CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = a->entry->height - 1 +CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = 1 -CFS;
            }

            if (c.cf.lower2)
            {
                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -CFS;               *xy++ = a->entry->height - 1 +CFS;
                *xy++ = -CFS;               *xy++ = -CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = -CFS;

                b = _MakeABorder(key, 3, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 1 -CFS;             *xy++ = a->entry->height - 1 +CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = a->entry->height - 1 +CFS;
                *xy++ = a->entry->width - 1 +CFS;  *xy++ = 1 -CFS;
            }

            if (c.cf.custb)
            {
                b = _MakeABorder(key, 0, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                b->XY = a->entry->custbXY;
                b->Count = a->entry->custb;
            }

            if (c.cf.custb2)
            {
                b = _MakeABorder(key, 0, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                b->XY = a->entry->custb2XY;
                b->Count = a->entry->custb2;
            }

            if (c.cf.radio)
            {
                /* caculate constant & central point
                */
                pi12 = PI / 12.0;
                w = a->entry->width / 2.0;
                h = a->entry->height / 2.0;
                x = a->entry->left + w;
                y = a->entry->top + h;

                /* Create the dark semi-circle first so it gets drawn last
                */
                b = _MakeABorder(key, 13, a->entry->left, a->entry->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                for (i = -3; i <= 9; ++i)
                {
                    *xy++ = x + w * sin(i *  pi12); *xy++ = y + h * cos(i * pi12);
                }

                /* And now for the bright
                */
                b = _MakeABorder(key, 13, a->entry->left, a->entry->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                for (i = 9; i <= 21; ++i)
                {
                    *xy++ = x + w * sin(i *  pi12); *xy++ = y + h * cos(i * pi12);
                }
            }
        }

    return blist;
}
