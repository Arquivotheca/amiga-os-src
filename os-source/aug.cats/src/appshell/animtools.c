/* animtools.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 */

#if 1
#include "appshell_internal.h"
#else
#include <internal/gfxextend.h>
#include <graphics/gfxbase.h>
#include <exec/memory.h>
#include <internal/gfxextend_proto.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <pragmas/exec.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/layers.h>

extern struct Library *SysBase, *IntuitionBase, *GfxBase, *LayersBase;
#endif

struct GelsInfo *setupGelSys (struct RastPort * rPort, BYTE reserved)
{
    struct GelsInfo *gInfo;
    struct VSprite *vsHead;
    struct VSprite *vsTail;

    if (gInfo = (struct GelsInfo *) AllocVec ((LONG) sizeof (struct GelsInfo), MEMF_CLEAR))
    {
	if (gInfo->nextLine = (WORD *) AllocVec ((LONG) sizeof (WORD) * 8, MEMF_CLEAR))
	{
	    if (gInfo->lastColor = (WORD **)
		AllocVec ((LONG) sizeof (LONG) * 8, MEMF_CLEAR))
	    {
		if (gInfo->collHandler = (struct collTable *)
		    AllocVec ((LONG) sizeof (struct collTable), MEMF_CLEAR))
		{
		    if (vsHead = (struct VSprite *)
			AllocVec ((LONG) sizeof (struct VSprite), MEMF_CLEAR))
		    {
			if (vsTail = (struct VSprite *)
			    AllocVec ((LONG) sizeof (struct VSprite), MEMF_CLEAR))
			{
			    gInfo->sprRsrvd = reserved;
			    gInfo->leftmost = 1;
			    gInfo->rightmost =
			      (rPort->BitMap->BytesPerRow << 3) - 1;
			    gInfo->topmost = 1;
			    gInfo->bottommost = rPort->BitMap->Rows - 1;

			    rPort->GelsInfo = gInfo;

			    InitGels (vsHead, vsTail, gInfo);

			    return (gInfo);
			}
			FreeVec (vsHead);
		    }
		    FreeVec (gInfo->collHandler);
		}
		FreeVec (gInfo->lastColor);
	    }
	    FreeVec (gInfo->nextLine);
	}
	FreeVec (gInfo);
    }
    return (NULL);
}

VOID cleanupGelSys (struct GelsInfo * gInfo, struct RastPort * rPort)
{

    rPort->GelsInfo = NULL;

    FreeVec (gInfo->collHandler);
    FreeVec (gInfo->lastColor);
    FreeVec (gInfo->nextLine);
    FreeVec (gInfo->gelHead);
    FreeVec (gInfo->gelTail);
    FreeVec (gInfo);
}

struct VSprite *makeVSprite (NEWVSPRITE * nVSprite)
{
    struct VSprite *vsprite;
    LONG line_size;
    LONG plane_size;

    line_size = (LONG) sizeof (WORD) * nVSprite->nvs_WordWidth;
    plane_size = line_size * nVSprite->nvs_LineHeight;

    if (vsprite = (struct VSprite *)
	AllocVec ((LONG) sizeof (struct VSprite), MEMF_CLEAR))
    {
	if (vsprite->BorderLine = (WORD *)
	    AllocVec (line_size, MEMF_CHIP))
	{
	    if (vsprite->CollMask = (WORD *)
		AllocVec (plane_size, MEMF_CHIP))
	    {
		vsprite->Y = nVSprite->nvs_Y;
		vsprite->X = nVSprite->nvs_X;
		vsprite->Flags = nVSprite->nvs_Flags;
		vsprite->Width = nVSprite->nvs_WordWidth;
		vsprite->Depth = nVSprite->nvs_ImageDepth;
		vsprite->Height = nVSprite->nvs_LineHeight;
		vsprite->MeMask = nVSprite->nvs_MeMask;
		vsprite->HitMask = nVSprite->nvs_HitMask;
		vsprite->ImageData = nVSprite->nvs_Image;
		vsprite->SprColors = nVSprite->nvs_ColorSet;
		vsprite->PlanePick = 0x00;
		vsprite->PlaneOnOff = 0x00;

		InitMasks (vsprite);
		return (vsprite);
	    }
	    FreeVec (vsprite->BorderLine);
	}
	FreeVec (vsprite);
    }
    return (NULL);
}

struct Bob *makeBob (NEWBOB * nBob)
{
    struct Bob *bob;
    struct VSprite *vsprite;
    NEWVSPRITE nVSprite;
    LONG rassize;

    rassize = (LONG) sizeof (UWORD) *
      nBob->nb_WordWidth * nBob->nb_LineHeight * nBob->nb_RasDepth;

    if (bob = (struct Bob *)
	AllocVec ((LONG) sizeof (struct Bob), MEMF_CLEAR))
    {
	if (bob->SaveBuffer = (WORD *) AllocVec (rassize, MEMF_CHIP))
	{
	    nVSprite.nvs_WordWidth = nBob->nb_WordWidth;
	    nVSprite.nvs_LineHeight = nBob->nb_LineHeight;
	    nVSprite.nvs_ImageDepth = nBob->nb_ImageDepth;
	    nVSprite.nvs_Image = nBob->nb_Image;
	    nVSprite.nvs_X = nBob->nb_X;
	    nVSprite.nvs_Y = nBob->nb_Y;
	    nVSprite.nvs_ColorSet = NULL;
	    nVSprite.nvs_Flags = nBob->nb_BFlags;
	    nVSprite.nvs_MeMask = nBob->nb_MeMask;
	    nVSprite.nvs_HitMask = nBob->nb_HitMask;

	    if (vsprite = makeVSprite (&nVSprite))
	    {
		vsprite->PlanePick = nBob->nb_PlanePick;
		vsprite->PlaneOnOff = nBob->nb_PlaneOnOff;

		vsprite->VSBob = bob;
		bob->BobVSprite = vsprite;
		bob->ImageShadow = vsprite->CollMask;
		bob->Flags = 0;
		bob->Before = NULL;
		bob->After = NULL;
		bob->BobComp = NULL;

		if (nBob->nb_DBuf)
		{
		    if (NULL != (bob->DBuffer = (struct DBufPacket *) AllocVec (
			      (LONG) sizeof (struct DBufPacket), MEMF_CLEAR)))
		    {
			if (NULL != (bob->DBuffer->BufBuffer =
				     (WORD *) AllocVec (rassize, MEMF_CHIP)))
			{
			    return (bob);
			}
			FreeVec (bob->DBuffer);
		    }
		}
		else
		{
		    bob->DBuffer = NULL;
		    return (bob);
		}

		freeVSprite (vsprite);
	    }
	    FreeVec (bob->SaveBuffer);
	}
	FreeVec (bob);
    }
    return (NULL);
}

struct Bob *makeImageBob (NEWIMAGEBOB * nIBob)
{
    struct Image *im = nIBob->nib_Image;
    NEWBOB nBob;

    nBob.nb_Image = im->ImageData;
    nBob.nb_WordWidth = (im->Width + 15) / 16;
    nBob.nb_LineHeight = im->Height;
    nBob.nb_ImageDepth = im->Depth;
    nBob.nb_PlanePick = im->PlanePick;
    nBob.nb_PlaneOnOff = im->PlaneOnOff;
    nBob.nb_BFlags = nIBob->nib_BFlags;
    nBob.nb_DBuf = nIBob->nib_DBuf;
    nBob.nb_RasDepth = nIBob->nib_RasDepth;
    nBob.nb_X = nIBob->nib_X;
    nBob.nb_Y = nIBob->nib_Y;
    nBob.nb_HitMask = nIBob->nib_HitMask;
    nBob.nb_MeMask = nIBob->nib_MeMask;
    return (makeBob (&nBob));
}

VOID freeVSprite (struct VSprite * vsprite)
{
    LONG line_size;
    LONG plane_size;

    line_size = (LONG) sizeof (WORD) * vsprite->Width;
    plane_size = line_size * vsprite->Height;

    FreeVec (vsprite->BorderLine);
    FreeVec (vsprite->CollMask);

    FreeVec (vsprite);
}

VOID freeBob (struct Bob * bob, LONG rasdepth)
{
    LONG rassize;

    rassize = (LONG) sizeof (UWORD) *
      bob->BobVSprite->Width * bob->BobVSprite->Height * rasdepth;

    if (bob->DBuffer)
    {
	FreeVec (bob->DBuffer->BufBuffer);
	FreeVec (bob->DBuffer);
    }

    FreeVec (bob->SaveBuffer);
    freeVSprite (bob->BobVSprite);
    FreeVec (bob);
}
