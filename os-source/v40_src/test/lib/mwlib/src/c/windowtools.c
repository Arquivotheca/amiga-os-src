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
/*****************************************************************************
    WindowTools.c

    Mitchell/Ware           Version 2.01            3/1/88

    Tools for handling the windows and screens - UPDATED FOR VERSION 1.4

    struct Screen *CreateScreen(width, height, depth, modes)

        SetScrLeftTop(left, top)
        SetScrPens(detail, block)
        SetScrType(type)
        SetScrFont(font)
        SetScrTitle(title)
        SetScrGadgets(gadgets)

        struct TagItem *AllocScreenTag(entries)
		struct TagItem *SetScreenTag(tag, data) - returns the tag address


    struct Window *CreateWindow(scr, key, width, height, title, maxvectors)

        SetWindowGadgets(gadlist);
        SetWinLeftTop (left, top)
        SetWinIDCMP (flags)
        SetWinFlags (flags)
        SetWinWH (minwidth, minheight, maxwidth, maxheight)

        struct TagItem *AllocWindowTag(entries)
		struct TagItem *SetWindowTag(tag, data) - returns the tag address

    void SetWindowPointer(window, btp)
    void ClearWindowPointer(window)

	NOTE:
		The SetScreenTag() and SetWindowTag() calls automatically constructs
		an array that will be automatically terminated with TAG_DONE.

		The AllocScreenTag() & AllocWindowTag() functions will return a
		pointer to the beginning of the array. An extra entry is
		automatically added to allow for the termination entry (TAG_DONE).

	WARNING:
		Their is no checking if you use more entries than you've allocated.
		If yo exceed that, undefined results you will get, followed by
		a swift visit from the GURU (or the GREEN BOX!)
*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/tagitem.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "Tools.h"
#include "protos.h"

#define PUB     (MEMF_PUBLIC | MEMF_CLEAR)
#define CHIP    (MEMF_CHIP | MEMF_CLEAR)
#define XW      640	/* this will change */
#define YW      400	/* this will change */

static struct ExtNewScreen ns, defns =    /* Default Screen       */
{
        0, 0, 320, 200, 5,      /* LeftEdge, TopEdge, Width, Height, Depth */
        0, 1,                   /* DetailPen, BlockPen  */
        NULL,                   /* ViewModes */
        CUSTOMSCREEN,           /* Type      */
        NULL,                   /* Font */
        "WindowTools v.2.00 (c) 1988-89 Mitchell/Ware Systems",   /* Title */
        NULL, NULL              /* Gadgets, CustomBitMap        */
};
static struct TagItem *nstag; /* next available tag */

static struct ExtNewWindow nw, defnw =    /* Default Window       */
{
        0, 0,           /* LeftEdge, TopEdge */
        160, 100,       /* Width, Height */
        -1, -1,           /* DetailPen, BlockPen */
        NULL,           /* IDCMP flags */
        WINDOWSIZING | WINDOWDEPTH | WINDOWCLOSE | WINDOWDRAG,   /* flags */
        NULL, NULL,     /* first gadget, checkmark */
        "Window",       /* Title */
        NULL, NULL,     /* Screen, Bitmap */
        20, 10,         /* MinWidth, MinHeight */
        0, 0,           /* MaxWidth, MaxHeight */
};
static struct TagItem *nwtag;	/* next available tag */

static BOOL cold = TRUE;
void _InitWT()
{
    if (cold)
    {
        cold = FALSE;
        nw = defnw;
        ns = defns;
    }
}

#
/*****************************************************************************

    Screen Control

*****************************************************************************/

struct Screen *CreateScreen(width, height, depth, modes)
SHORT width, height, depth;
USHORT modes;
{
    struct Screen *scr;

    _InitWT();

    ns.Width = width;
    ns.Height = height;
    ns.Depth = depth;
    ns.ViewModes = modes;
    if (scr = OpenScreen(&ns))
        ShowTitle(scr, FALSE);  /* default to no title bar */

    ns = defns;
    return scr;
}

void SetScrLeftTop(left, top)
int left, top;
{
    _InitWT();

    ns.LeftEdge = left;
    ns.TopEdge = top;
}

void SetScrPens(detail, block)
int detail, block;
{
    _InitWT();

    ns.DetailPen = detail;
    ns.BlockPen = block;
}

void SetScrType(type)
USHORT type;
{
    _InitWT();

    ns.Type = type;
}

void SetScrFont(font)
struct TextAttr *font;
{
    _InitWT();

    ns.Font = font;
}

void SetScrTitle(title)
UBYTE *title;
{
    _InitWT();

    ns.DefaultTitle = title;
}

void SetScrGadgets(gadgets)
struct Gadget *gadgets;
{
    _InitWT();

    ns.Gadgets = gadgets;
}

/* 1.4 related screen tag calls
*/
struct TagItem *AllocScreenTag(entries)
ULONG entries;
{
	if (!(ns.Extension = calloc(++entries, sizeof(struct TagItem))))
		fuckup("AST:Out Of Memory");

	return nstag = ns.Extension;
}

struct TagItem *SetScreenTag(tag, data)
ULONG tag;
ULONG data;
{
	nstag->ti_Tag = tag;
	nstag->ti_Data = data;
	(++nstag)->ti_Tag = TAG_DONE;

	return nstag - 1;
}

#
/*****************************************************************************

    Window Control

*****************************************************************************/

struct Window *CreateWindow(scr, key, width, height, title, maxvectors)
struct Screen *scr;     /* Null if a workbench window */
struct Remember **key;
SHORT width, height;
UBYTE *title;
USHORT maxvectors;    /* allocate AreaInfo structures if non-z. */
{
    struct Window *w;

    _InitWT();

    nw.Type = (scr) ? CUSTOMSCREEN : WBENCHSCREEN;
    nw.Screen = scr;
    nw.Width = width;
    nw.Height = height;
    nw.Title = title;
    w = OpenWindow(&nw);
    nw.FirstGadget = NULL;  /* avoid problems */

    if (w && maxvectors && key)   /* init for vectors */
        if (!AddVectorBuffs(key, w->RPort, maxvectors))
        {
            CloseWindow(w);
            return NULL;
        }

    nw = defnw;
    return w;
}

void SetWindowGadgets(g)
struct Gadget *g;
{
    _InitWT();

    nw.FirstGadget = g;
}

void SetWinLeftTop (left, top)
int left, top;
{
    _InitWT();

    nw.LeftEdge = left;
    nw.TopEdge = top;
}

void SetWinIDCMP (iflags)
ULONG iflags;
{
    _InitWT();

    nw.IDCMPFlags = iflags;
}

void SetWinFlags (flags)
ULONG flags;
{
    _InitWT();

    nw.Flags = flags;
}

void SetWinWH (minwidth, minheight, maxwidth, maxheight)
int minwidth, minheight, maxwidth, maxheight;
{
    _InitWT();

    nw.MinWidth = minwidth;
    nw.MinHeight = minheight;
    nw.MaxWidth = maxwidth;
    nw.MaxHeight = maxheight;
}

/* 1.4 related window tag calls
*/
struct TagItem *AllocWindowTag(entries)
ULONG entries;
{
	if (!(nw.Extension = calloc(++entries, sizeof(struct TagItem))))
		fuckup("AST:Out Of Memory");

	return nwtag = nw.Extension;
}

struct TagItem *SetWindowTag(tag, data)
ULONG tag;
ULONG data;
{
	nwtag->ti_Tag = tag;
	nwtag->ti_Data = data;
	(++nwtag)->ti_Tag = TAG_DONE;

	return nwtag - 1;
}
#
/*****************************************************************************

    Miscellaneous functions

*****************************************************************************/

void SetWindowPointer(win, btp)
struct Window *win;
struct btp *btp;
{
    SetPointer(win, btp->pointer, btp->height, btp->width, btp->xoff, btp->yoff);
}

void ClearWindowPointer(win)
struct Window *win;
{
    ClearPointer(win);
}
