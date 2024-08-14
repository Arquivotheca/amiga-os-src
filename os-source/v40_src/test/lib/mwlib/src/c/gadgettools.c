/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.

    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.
*/
/****** mwlib.library/GadgetTools *********************************************
    GadgetTools.c

    Mitchell/Ware Systems           Version 4.16            06-Dec-88

    Tools for Creating Gadgets.

    Set the ID Counter:
    SetGadgetIDCounter (number)
        number - first number to start ID with. Will be incremented...

    Make BOOLEAN Gadget with imagery:
        MakeBGadget (key, image, alternate, left, top, flags, activation)
            key - a Remember pointer
            image - the Rendering image
            alternate - the alternate SELECT image, or NULL if none
            left - LeftEdge
            top - TopEdge
            flags - Gadget Flags - GADGIMAGE set always, GADGHCOMP
                    default if no GADGHIGHBITS specified.
            activation - Gadget Activation - defaults to RELVERIFY

    Make CUSTOM REQUEST BOOLEAN GADGET:
        MakeCRBGadget (key, left, top, width, height, flags, activation, type)

    Make a PROPORTIONAL Gadget
        MakePropGadget (key, knob_image, left, top, width, height,
                                               flags, prop_flags, activation)

    Make a STRING Gadget
        MakeStrGadget (key, left, top, width, height, maxchars, buf, undobuf,
                                                        flags, activation, type)

    Make a Border list
        MakeRTGBorders(key, rtglist)

    Make a Requester Gadget list
        MakeRTGList(key, rtglist, class_list)
            struct Remember **key;
            RTGList rtglist[];
            struct CustomGadgetClass *class_list;

    Refresh a Requester Gadget list - currently only does the string gadgets
        RefreshRTGList(rtglist, win, req)

    Count all the gadgets in a list of gadgets
        GadgetCount(gadlist)
            struct *Gadget *gadlist;

    NOTE: A NULL GADGET is one with type NULL (in the RTGList). Such a gadget
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
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "tools.h"
#include "protos.h"

#define PUB     (MEMF_PUBLIC | MEMF_CLEAR)
#define CHIP    (MEMF_CHIP | MEMF_CLEAR)

#define BE      2       /* box expansion default for MakeRTGBorders */

long GadgetIDCounter = 0;

void SetGadgetIDCounter(n)
int n;
{
    GadgetIDCounter = n;
}

/* BOOLEAN GADGET
*/

struct Gadget *MakeBGadget(k, i, a,  LeftEdge, TopEdge, flags, activation)
struct Remember **k;
struct Image *i, *a;            /* image and possibly an alternate */
int LeftEdge, TopEdge;
USHORT flags, activation;
{
    struct Gadget *g = NULL;

    if (!(g = AllocRemember(k, sizeof(*g), PUB)))
        return NULL;

    g->NextGadget = NULL; /* the user must AddGadget */
    g->LeftEdge = LeftEdge;
    g->TopEdge = TopEdge;
    g->Width = i->Width;
    g->Height = i->Height;
    g->Flags = flags | GADGIMAGE;
    if (!(flags & GADGHIGHBITS))      /* if no highbits set, default */
    {
        g->Flags |= (a) ? GADGHIMAGE : GADGHCOMP;
    }

    g->Activation = (activation) ? activation : RELVERIFY;
    g->GadgetType = BOOLGADGET;
    g->GadgetRender = i;
    g->SelectRender = a;
    g->GadgetText = NULL;
    g->GadgetID = GadgetIDCounter++;
    g->UserData = NULL; /* user must set this */
    return g;
}

struct Gadget *MakeCRBGadget (k, left, top, width, height, flags, activation)
struct Remember **k;
int left, top;
int width, height;
ULONG flags, activation;
{
    struct Gadget *g =0;

    if (!(g = AllocRemember(k, sizeof(*g), PUB)))
        return NULL;

    g->NextGadget = NULL;
    g->LeftEdge = left;
    g->TopEdge = top;
    g->Width = width;
    g->Height = height;
    g->Flags = flags;
    g->Activation = activation;
    g->GadgetType = BOOLGADGET | REQGADGET;
    g->GadgetRender = NULL;
    g->SelectRender = NULL;
    g->GadgetText = NULL;
    g->MutualExclude = NULL;
    g->GadgetID = GadgetIDCounter++;
    g->UserData = NULL;
    return g;
}

/*****************************************************************************

    Make Requester Gadget List

*****************************************************************************/

struct Gadget *MakeRTGList (key, list, cgc)
struct Remember **key;
struct RTGList *list;
struct CustomGadgetClass *cgc; /* list of custom gadget classes */
{
    struct Gadget *g, *MakeCRBGadget(), *MakePropGadget(), *MakeStrGadget();
    struct StringInfo *sinfo;
    struct PropInfo *pinfo;
    struct RTGList *l, *ll;
    UBYTE *buf, *undo;
    struct CustomGadgetClass *pcgc;

    GadgetIDCounter = 0; /* so that gadgets can index the list */
    for (l = list; l->width; ++l)
    {
        if (!l->g)
        {
            if (!l->type || (l->type == BOOLGADGET))
            {
                l->g = MakeCRBGadget(key, l->left, l->top, l->width,
                        l->height, l->flags, l->activation);
                l->g->GadgetID = l->userid;
                l->g->UserData = l;
                l->g->NextGadget = NULL;
            /*  l->g->MutualExclude = l->general; not until 1.4! Simulated
                                                  for RequesterTools.   */
                l->g->GadgetType |= REQGADGET;

                if (l->norm)
                {
                    l->g->GadgetRender = l->norm;
                    l->g->Flags |= GADGIMAGE;
                }

                if (l->sel)
                {
                    l->g->SelectRender = l->sel;
                    l->g->Flags |= GADGHIMAGE;
                }

                l->g->Activation |= GADGIMMEDIATE;
            }
            else if (l->type == PROPGADGET)
            {
                if (!l->norm)
                    l->general |= AUTOKNOB;
                l->g = MakePropGadget(key, l->norm,
                                l->left, l->top, l->width, l->height,
                                l->flags, l->general, l->activation);

                l->g->GadgetType |= REQGADGET;

                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->VertPot = l->vertpot;
                pinfo->HorizBody = l->horizbody;
                pinfo->VertBody = l->vertbody;
            }
            else if (l->type == STRGADGET)
            {
                /* For the StringGadget, l->general is the size of buffer
                    in bytes. The space will be allocated both for the
                    main buffer and the undo buffer. You can directly
                    get at these buffers through the gadget.

                    ALSO, if l->fix is non-zero and the activation
                    is LONGINT, then fix is used for the placement
                    of a decimal point. The LONGINT activation is cleared,
                    and the strings are initialized with the apporiate
                    numerical digits & points. This works in conjunction
                    with RequesterTools to translate the string into a
                    number to be placed in l->value. */

                buf = AllocRemember(key, l->general, PUB);
                undo = AllocRemember(key, l->general, PUB);
                if (!buf || !undo)
                    return NULL;    /* Out of Memory */
                l->g = MakeStrGadget(key, l->left, l->top, l->width, l->height,
                            l->general, buf, undo, l->flags, l->activation, REQGADGET);

                l->g->GadgetType |= REQGADGET;

                if (l->activation & LONGINT)
                {
                    if (l->fix)
                    {
                        l->g->Activation &= ~LONGINT;
                        sfix_dec(buf, l->fix, l->value);
                    }
                    else
                    {
                        sprintf(buf, "%d", l->value);
                        sinfo = l->g->SpecialInfo;
                        sinfo->LongInt = l->value;
                    }
                }
                else if (l->string)
                    memcpy(buf, l->string, l->general);
            }
            else if (l->type == PROP_HORIZ)
            {
                l->g = MakeIndHorizProp(key, l->norm, l->left, l->top, l->width, l->flags, l->activation);
                GadgetIDCounter -= 2;
                l->g->NextGadget->GadgetID
                    = l->g->NextGadget->NextGadget->GadgetID
                        = l->g->GadgetID;

                l->g->GadgetType |= REQGADGET;
                l->g->NextGadget->GadgetType |= REQGADGET;
                l->g->NextGadget->NextGadget->GadgetType |= REQGADGET;

                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->HorizBody = l->horizbody;
            }
            else if (l->type == PROP_VERT)
            {
                l->g = MakeIndVertProp(key, l->norm, l->left, l->top, l->height, l->flags, l->activation);
                GadgetIDCounter -= 2;
                l->g->NextGadget->GadgetID
                    = l->g->NextGadget->NextGadget->GadgetID
                        = l->g->GadgetID;

                l->g->GadgetType |= REQGADGET;
                l->g->NextGadget->GadgetType |= REQGADGET;
                l->g->NextGadget->NextGadget->GadgetType |= REQGADGET;

                pinfo = l->g->SpecialInfo;
                pinfo->VertPot = l->vertpot;
                pinfo->VertBody = l->vertbody;
            }
            else if (l->type < 0)   /* Custom Gadget */
            {
                for (pcgc = cgc; pcgc; pcgc = pcgc->next)
                    if (l->type == pcgc->class)
                    {
                        l->class = pcgc;
                        if (pcgc->create && !(*pcgc->create)(key, l, pcgc))
                            l->g = NULL;    /* if error, force a return */
                        else
                            l->g->GadgetType |= REQGADGET;
                        break;
                    }
            }
            /* else ignore- gadtype is NULL */

            if (l->type && !l->g)
                return NULL;    /* Out of Memory */

            if (l->type && l > list)    /* if not a null gad && not the first */
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
                    fuckup("MakeRTGList: First Gadget in RTGList is NULL!");
            }
        }

        if (l->selected)
            l->g->Flags |= SELECTED;
        else
            l->g->Flags &= ~SELECTED;
    }
    return list->g;
}

/*****************************************************************************

    Refresh Requester Gadget List

    Update the STRING & PROPORTIONAL Gadgets!

    NOTE: CLears l->string pointer !!!!!

*****************************************************************************/

void RefreshRTGList (list, win, req)
struct RTGList *list;
struct Window *win;
struct Requester *req;
{
    struct StringInfo *sinfo;
    struct PropInfo *pinfo;
    struct RTGList *l;

    if (win) RemoveGList(win, list->g, -1);
    for (l = list; l->width; ++l)
    {
        if (l->g)
        {
            if (!l->type || (l->type == BOOLGADGET))
            {
            }
            else if (l->type == PROPGADGET)
            {/*
                pinfo = l->g->SpecialInfo;
                pinfo->HorizPot = l->horizpot;
                pinfo->VertPot = l->vertpot;
                pinfo->HorizBody = l->horizbody;
                pinfo->VertBody = l->vertbody;  */
            }
            else if (l->type == STRGADGET)
            {
                sinfo = l->g->SpecialInfo;

                if (l->activation & LONGINT)
                {
                    if (l->fix)
                    {
                        l->g->Activation &= ~LONGINT;
                        sfix_dec(sinfo->Buffer, l->fix, l->value);
                    }
                    else
                    {
                        sprintf(sinfo->Buffer, "%d", l->value);
                        sinfo->LongInt = l->value;
                    }
                }
                else if (l->string)
                    memcpy(sinfo->Buffer, l->string, l->general), l->string = NULL;
            }
            else if (l->type == PROP_HORIZ)
            {
            }
            else if (l->type == PROP_VERT)
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
        AddGList(win, list->g, 0, -1, req);
        for (l = list; l->width; ++l)
            if (l->g && l->type == STRGADGET)
                RefreshGList(l->g, win, req, 1);
    }
}

void RefreshRGList (list, win, req) /* for the OLD calls (name spelled incorrect) */
struct RTGList *list;
struct Window *win;
struct Requester *req;
{
    RefreshRTGList (list, win, req);
}
#
/*****************************************************************************

    PROP GADGET

*****************************************************************************/

struct Gadget *MakePropGadget (key, knob_image,
                                left, top, width, height,
                                    flags, prop_flags, activation)
struct Remember **key;
struct Image *knob_image; /* if NULL, automatically implies AUTOKNOB */
int left, top, width, height;
ULONG flags, prop_flags, activation;
{
    struct PropInfo *prop;
    struct Gadget *g;

    g = AllocRemember(key, sizeof(*g), PUB);
    prop = AllocRemember(key, sizeof(*prop), PUB);
    if (!g || !prop)
        return NULL;

    g->NextGadget = NULL;
    g->LeftEdge = left;
    g->TopEdge = top;
    g->Width = width;
    g->Height = height;
    g->Flags = flags | GADGHNONE | GADGIMAGE;
    g->Activation = activation | GADGIMMEDIATE;
    g->GadgetType = PROPGADGET;
    g->GadgetText = NULL;
    g->MutualExclude = NULL;
    g->SpecialInfo = prop;
    g->GadgetID = GadgetIDCounter++;
    g->UserData = NULL;


    if (!knob_image)  /* if AUTOKNOB */
    {
        knob_image = AllocRemember(key, sizeof(*knob_image), PUB);
        if (!knob_image)
            return NULL;
        prop_flags |= AUTOKNOB;
    }
    g->GadgetRender = knob_image;

    prop->Flags = prop_flags;
    prop->HorizPot = MAXPOT / 2;
    prop->VertPot = MAXPOT / 2;
    prop->HorizBody = MAXBODY;
    prop->VertBody = MAXBODY;

    return g;
}
#
/*****************************************************************************

    STRING GADGET

*****************************************************************************/

struct Gadget *
MakeStrGadget (key, LeftEdge, TopEdge, Width, Height,
                            MaxChars, buf, undo, Flags, Activation, Type)
struct Remember **key;
int LeftEdge, TopEdge, Width, Height;
int MaxChars;
UBYTE *buf, *undo;
ULONG Flags, Activation, Type;
{
    struct StringInfo *s = NULL;
    struct Gadget *g = NULL;

    s = AllocRemember(key, sizeof(*s), PUB);
    g = AllocRemember(key, sizeof(*g), PUB);

    if (!s || !g)
    {
        return NULL;
    }

    s->Buffer = buf;
    s->UndoBuffer = undo;
    s->MaxChars = MaxChars;
    s->BufferPos = 0;
    s->DispPos = 0;

    g->NextGadget = NULL;
    g->LeftEdge = LeftEdge;
    g->TopEdge = TopEdge;
    g->Width = Width;
    g->Height = Height;
    g->Flags = Flags | GADGHCOMP;
    g->Activation = Activation;
    g->GadgetType = Type | STRGADGET;
    g->GadgetRender = NULL;
    g->SelectRender = NULL;
    g->GadgetText = NULL;
    g->MutualExclude = NULL;
    g->SpecialInfo = s;
    g->GadgetID = GadgetIDCounter++;
    g->UserData = NULL;

    return g;
}
#
/*****************************************************************************

    BORDER LIST

*****************************************************************************/

struct Border *_MakeABorder(key, vectors, left, top, front, back, draw)
struct Remember **key;
short vectors;
short left, top;
short front, back;
short draw;
{
    struct Border *b;

    if (b = AllocRemember(key, sizeof(*b), PUB))
    {
        b->LeftEdge = left;
        b->TopEdge = top;
        b->FrontPen = front;
        b->BackPen = back;
        b->DrawMode = draw;
        b->Count = vectors;
        if (!(b->XY = AllocRemember(key, sizeof(short) * 2 * vectors, PUB)))
            b = NULL;
    }
    return b;
}

#define CFS c.cf.seperation

MakeRTGBorders(key, rlist) /* return a linked list of borders */
struct Remember **key;
struct RTGList *rlist;
{
    struct Border *blist, *b;
    struct RTGList *r;
    short *xy, i;
    GadControl c;
    short draw; /* draw mode */
    double x, y, w, h, pi12; /* work variables */

    for (r = rlist, blist = NULL; r->width; ++r)
        if (r->control & (CF_BOX | CF_BOX2 | CF_SHADOW | CF_SHADOW2
                                | CF_RAISE | CF_RAISE2 | CF_LOWER | CF_LOWER2
                                | CF_CUSTB | CF_CUSTB2 | CF_RADIO))
        {
            c.c = r->control;
            if (!CFS)
                CFS = BE;

            draw = (c.cf.xor) ? COMPLEMENT : JAM1;

            if (c.cf.box)
            {
                b = _MakeABorder(key, 5, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = r->width;       *xy++ = r->height;
                *xy++ = -1;             *xy++ = r->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = r->width;       *xy++ = -1;
                *xy++ = r->width;       *xy++ = r->height;
            }

            if (c.cf.shadow)
            {
                b = _MakeABorder(key, 7, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = r->width + 1;   *xy++ = r->height + 1;
                *xy++ = 1;              *xy++ = r->height + 1;
                *xy++ = 1;              *xy++ = r->height + 2;
                *xy++ = r->width + 2;   *xy++ = r->height + 2;

                *xy++ = r->width + 2;   *xy++ = 1;
                *xy++ = r->width + 1;   *xy++ = 1;
                *xy++ = r->width + 1;   *xy++ = r->height + 1;
            }

            if (c.cf.box2)
            {
                b = _MakeABorder(key, 5, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = r->width+CFS-1;  *xy++ = r->height+CFS-1;
                *xy++ = -CFS;            *xy++ = r->height+CFS-1;
                *xy++ = -CFS;            *xy++ = -CFS;
                *xy++ = r->width+CFS-1;  *xy++ = -CFS;
                *xy++ = r->width+CFS-1;  *xy++ = r->height+CFS-1;
            }

            if (c.cf.shadow2)
            {
                b = _MakeABorder(key, 7, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = r->width+CFS;    *xy++ = r->height+CFS;
                *xy++ = 2-CFS;           *xy++ = r->height+CFS;
                *xy++ = 2-CFS;           *xy++ = r->height+CFS+1;
                *xy++ = r->width+CFS+1;  *xy++ = r->height+CFS+1;

                *xy++ = r->width+CFS+1;  *xy++ = 2-CFS;
                *xy++ = r->width+CFS;    *xy++ = 2-CFS;
                *xy++ = r->width+CFS;    *xy++ = r->height+CFS;
            }

            if (c.cf.raise)
            {
                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -1;             *xy++ = r->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = r->width;       *xy++ = -1;

                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 0;              *xy++ = r->height;
                *xy++ = r->width;       *xy++ = r->height;
                *xy++ = r->width;       *xy++ = 0;
            }

            if (c.cf.lower)
            {
                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -1;             *xy++ = r->height;
                *xy++ = -1;             *xy++ = -1;
                *xy++ = r->width;       *xy++ = -1;

                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 0;              *xy++ = r->height;
                *xy++ = r->width;       *xy++ = r->height;
                *xy++ = r->width;       *xy++ = 0;
            }

            if (c.cf.raise2)
            {
                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -CFS;               *xy++ = r->height - 1 +CFS;
                *xy++ = -CFS;               *xy++ = -CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = -CFS;

                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 1 -CFS;             *xy++ = r->height - 1 +CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = r->height - 1 +CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = 1 -CFS;
            }

            if (c.cf.lower2)
            {
                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = -CFS;               *xy++ = r->height - 1 +CFS;
                *xy++ = -CFS;               *xy++ = -CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = -CFS;

                b = _MakeABorder(key, 3, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                *xy++ = 1 -CFS;             *xy++ = r->height - 1 +CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = r->height - 1 +CFS;
                *xy++ = r->width - 1 +CFS;  *xy++ = 1 -CFS;
            }

            if (c.cf.custb)
            {
                b = _MakeABorder(key, 0, r->left, r->top, c.cf.color_b, 0, draw);
                b->NextBorder = blist; blist = b;
                b->XY = r->custbXY;
                b->Count = r->custb;
            }

            if (c.cf.custb2)
            {
                b = _MakeABorder(key, 0, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                b->XY = r->custb2XY;
                b->Count = r->custb2;
            }

            if (c.cf.radio)
            {
                /* caculate constant & central point
                */
                pi12 = PI / 12.0;
                w = r->width / 2.0;
                h = r->height / 2.0;
                x = r->left + w;
                y = r->top + h;

                /* Create the dark semi-circle first so it gets drawn last
                */
                b = _MakeABorder(key, 13, r->left, r->top, c.cf.color_d, 0, draw);
                b->NextBorder = blist; blist = b;
                xy = b->XY;

                for (i = -3; i <= 9; ++i)
                {
                    *xy++ = x + w * sin(i *  pi12); *xy++ = y + h * cos(i * pi12);
                }

                /* And now for the bright
                */
                b = _MakeABorder(key, 13, r->left, r->top, c.cf.color_b, 0, draw);
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
#
/*****************************************************************************

    Gadget Counter

*****************************************************************************/

int GadgetCount(g)
struct Gadget *g;
{
    register i;

    for (i = 0; g; g = g->NextGadget, ++i)
        ;

    return i;
}
