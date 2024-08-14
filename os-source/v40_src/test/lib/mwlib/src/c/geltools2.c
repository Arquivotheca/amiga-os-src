#ifdef DONE
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
/****** MWLib.library/GelTools2__background__ *********************************

    NAME
        GelTools2 -- Routines to allocate/control the Gels System

    VERSION
        1.00    05 Mar 1992 - Adapted from the old GelsTools Code

    AUTHOR
        Copyright © 1992 Mitchell/Ware Systems, Inc. All Rights Reserved.

    SYNOPSIS
        struct GelTools *MakeGels (struct GelsInfo *GelsInfo, RastPort)
               sets up the defaults of the gel system.


    PURPOSE
        Provides a consistent interface to the Gels animation system.

******************************************************************************/

/*      GelTools - Adapted Version of the one presented in the ROM KERNAL
*
*       Mitchell/Ware           Version 1.02            12/7/87
*
*       This version makes use of the AllocRemember routine, so
*       INTUITION must be installed. Calls with Image refer to
*       the Image struct as set up by ILBM2C utility, or the ILBMTools system.
*
*       MakeVSprite (key, Image, colorset, flags)
*               allocates space for a normal VSprite.
*
*       MakeBob (key, Image, flags)
*               makes a Bob from given info present in Image. Links
*               Bobs toghether. Last bob is Top bob.
*
*       DoubleBob (key, bob)
*               double-buffer this bob..
*
*       NOTE:   None of these routines add the bob or the vsprite
*               to the Gel List. That must be done seperately.
*
*/

// Includes
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gels.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/sprite.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>

#define PUB     (MEMF_PUBLIC | MEMF_CLEAR)
#define CHIP    (MEMF_CHIP | MEMF_CLEAR)

// Structs
struct GelsTools {
    struct VSprite *SpriteHead, *SpriteTail;
    struct GelsInfo *ginfo;
    };

struct Bob *prevBob = NULL;     /* last BOB Made */
void border_dummy()
{
    return;
}

struct GelsTools *CreateGels (struct GelsInfo *g, struct RastPort *r)     // returns a Gels structure if succedes.
{
    long non = 0;
    struct GelsTools *gt;

    if (gt = AllocVec(sizeof(*gt), MEMF_CLEAR | MEMF_PUBLIC))
    {
        g->sprRsrvd = -1; /* no reserved sprites */
        prevBob = NULL;  /* clear the Prev. bob reference. New list. */

        /* Allocate Memory */

        non |= !(gt->SpriteHead = AllocRemember(k, sizeof(struct VSprite), PUB));
        non |= !(gt->SpriteTail = AllocRemember(k, sizeof(struct VSprite), PUB));
        non |= !(g->nextLine = AllocRemember(k, sizeof(WORD) * 8, PUB));
        non |= !(g->lastColor = AllocRemember(k, sizeof(LONG) * 8, PUB));
        non |= !(g->collHandler = AllocRemember(k, sizeof(struct collTable), PUB));
        if (non)
            return -1;      /* memory allocation failed somewhere */

        g->leftmost = 0;
        g->rightmost = r->BitMap->BytesPerRow * 8 - 1;
        g->topmost = 0;
        g->bottommost = r->BitMap->Rows - 1;
        r->GelsInfo = g;        /* link the two structures */

        InitGels(SpriteHead, SpriteTail, g);
        SetCollision(0, border_dummy, g);
        WaitTOF();
    }

    return gt;
}

struct VSprite *CreateVSprite(k, i, colorset, flags) /* does NOT do an addvsprite */
struct Remember **k;
struct Image *i;
WORD *colorset;
SHORT flags;
{
        int non = 0;
        struct VSprite *v;

        non |= !(v = AllocRemember(k, sizeof(*v), PUB));
        if (non)
                return NULL;

        v->Flags = flags;
        v->X = i->LeftEdge;
        v->Y = i->TopEdge;
        v->Height = i->Height;
        v->Width = (i->Width - 1)/16 + 1; /* Word Width */
        v->Depth = i->Depth;
        v->MeMask = 1;
        v->HitMask = 1;
        v->ImageData = i->ImageData;
        non |= !(v->BorderLine = AllocRemember(k, sizeof(WORD) * v->Width, PUB));
        non |= !(v->CollMask = AllocRemember(k, sizeof(WORD) * v->Width * i->Height, CHIP));
        if (non)
                return NULL;

        v->SprColors = colorset;
        v->PlanePick = 0x00;    /* not used for VSprite. Bob will handle it. */
        v->PlaneOnOff = 0x00;
        InitMasks(v);
        return v;
}

struct Bob *CreateBob(k, i, flags)  /* returns NULL if fails. Calls MakeVSprite. */
struct Remember **k;
struct Image *i;
SHORT flags;
{
        struct Bob *b;
        struct VSprite *v;
        SHORT wordwidth;
        int non = 0;

        wordwidth = (i->Width + 15) / 16;
        if (!(v = MakeVSprite(k, i, NULL, flags)))
                return NULL;

        v->PlanePick = i->PlanePick;
        v->PlaneOnOff = i->PlaneOnOff;
        if (!(b = AllocRemember(k, sizeof(*b), PUB)))
                return NULL;
        v->VSBob = b;
        b->Flags = 0;
        b->SaveBuffer = AllocRemember(k, sizeof(SHORT) * wordwidth * i->Height * i->Depth, CHIP);
        non |= !b->SaveBuffer;

        b->ImageShadow = v->CollMask;

        /* InterBob Priority is such that earliest is lowest. Last is top */
        b->Before = NULL;
        b->After = prevBob;
        if (prevBob)    /* link previous bob to this one */
                prevBob->Before = b;

        b->BobVSprite = v;

        b->BobComp = NULL;      /* no animation */
        b->DBuffer = NULL;      /* no Dbuff */
        return prevBob = b;       /* caller must AddBob himself */
}

struct Bob *MakeADoubleBob(k, bob)   /* double-buffer this bob */
struct Remember **k;
struct Bob *bob;
{
        struct DBufPacket *db;

        if (!(db = AllocRemember(k, sizeof(*db), CHIP)))
                return NULL;

        bob->DBuffer = db;
        db->BufBuffer = AllocRemember(k, sizeof(WORD)
                * bob->BobVSprite->Width
                * bob->BobVSprite->Height
                * bob->BobVSprite->Depth, CHIP);

        return bob;
}
#endif
