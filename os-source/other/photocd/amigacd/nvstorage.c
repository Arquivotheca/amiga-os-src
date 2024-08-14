#/*** nvstorage.c ************************************************************
 *
 *  $Id: nvstorage.c,v 1.2 94/03/31 16:09:11 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Non-volatile Storage Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	nvstorage.c,v $
 * Revision 1.2  94/03/31  16:09:11  jjszucs
 * o   Applying mirror manipulation to portrait-orientation images no longer
 *     causes inappropriate cropping.
 * 
 * o   The Normalize glyph is now a 65x65 bitmap, like all other button glyphs.
 *     The glyph was 64x64 and strange lines when the glyph was displayed
 *     as a 65x65 mask was applied.
 * 
 * o   The Zoom In and Zoom Out glyphs had black vertical line in the far
 *     right column. This has been eliminated.
 * 
 * o   The interface panel is now made visible when the thumbnail state
 *     is initially entered at the start of a session.
 * 
 * o   Changed fixed palette range from 16...240 to 0...255 for each
 *     color component (R/G/B). Although the clipped color component range
 *     was theoretically better for image quality, this change significantly
 *     reduces the complexity of the fixed palette code (which is called for
 *     all displayed pixels), increasing image display speed. The observed
 *     impact of the change on image quality is neglible.
 * 
 * o   For NTSC systems, quick scaling was inadvertently being performed
 *     on the X-axis for portrait-orientation images. This increased
 *     the aspect ratio distortion, instead of decreasing the aspect
 *     ratio (as was the intent). This has been corrected, with significant
 *     code savings as a bonus.
 * 
 * o   Center of zoom box now accurately corresponds to zoom cursor for
 *     all cases.
 * 
 * o   Zoom center point is now at the center of the lenses of the magnifying
 *     glass image, not at the center of the entire image (which includes
 *     the handle).
 * 
 * o   Zoom cursor bounds-checking is now correct for all cases.
 * 
 * o   Zooming of portrait-orientation images now works correctly.
 * 
 * o   Yet another attempt was made to implement a smooth scroll in the
 *     thumbnail display. However, due to the depth of the thumbnail screen
 *     (8 bitplanes), this results in a noticable "inchworm" effect. Since this
 *     is undesirable, jump scrolling continues to be used. This is being noted
 *     in this release note (and the associated RCS logs) primarily as a note
 *     to myself (and possibly others) once and for all that this is a
 *     Bad Idea(TM).
 * 
 * o   Yet more changes to the "Easter Egg" text.
 * 
 * 
 * Revision 1.1  94/03/17  16:26:32  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>

#include <libraries/nonvolatile.h>

#include <libraries/photocd.h>

#include <clib/exec_protos.h>
#include <clib/nonvolatile_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nonvolatile_pragmas.h>

/* Local includes */
#include "global.h"
#include "image.h"

/*****************************************************************************

Non-volatile Storage Format for Photo CD Slideshows
===================================================

Application Name
----------------

    Photo CD

Item Name
=========

    <disc serial number>

Data
====

    One or more of the following:

    Offset  Type    Name        Description
    -----   ----    ----        -----------
    +0      BOOL    EOF         TRUE if this is end of file;
                                FALSE if this is a record

    For records:

    +1      UBYTE   Number      Image number
    +2      UBYTE   Slide       Slide number
    +3      UBYTE   Sequence    Sequence number
    +4      UBYTE   Flags       Flags
        .4              IsSlide TRUE if slide; FALSE if not slide
        .3              Mirror  TRUE if mirrored; FALSE if not mirrored
        .2 - .1         Rotation Counter-clockwise rotation
                                00 = 0o; 01 = 90o; 10 = 180o; 11 = 270o
        .0              Magnify TRUE if magnified; FALSE if not magnification
    +5      UBYTE   Magnify X   X-axis magnification origin (/2)
    +6      UBYTE   Magnify Y   Y-axis magnification origin (/2)

 *****************************************************************************/

/****************************************************************************
 *                                                                          *
 *  saveProgram() - save program for disc                                   *
 *                                                                          *
 ****************************************************************************/

BOOL saveProgram(struct appContext *appContext)
{

    UWORD       nRecords;

    struct imageNode    *imageNode;

    struct storageNode  *data;
    ULONG               cbData;
    struct storageNode  *record;

    UBYTE       rotateBits;

    UWORD   error;
    BOOL    result;

    /* Count number of records to save */
    nRecords=0;
    imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
    while (imageNode->in_Node.mln_Succ) {

        if (imageNode->in_Manipulated ||
            (!imageNode->in_IsSlide) ||
            imageNode->in_Slide!=imageNode->in_Number ||
            imageNode->in_Sequence!=imageNode->in_Number) {

            nRecords++;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

    /* Allocate data */
    cbData=(sizeof(struct storageNode)*nRecords)+sizeof(record->sn_EOF);
    data=AllocVec(cbData, NULL);
    if (data) {

        /* Begin with first record */
        record=data;

        /* Loop through image nodes */
        imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
        while (imageNode->in_Node.mln_Succ) {

            /* If image node is manipulated or moved ... */
            if (imageNode->in_Manipulated ||
                (!imageNode->in_IsSlide) ||
                imageNode->in_Slide!=imageNode->in_Number ||
                imageNode->in_Sequence!=imageNode->in_Number) {

                /* Construct record */
                record->sn_EOF=FALSE;
                record->sn_Number=(UBYTE) imageNode->in_Number;
                record->sn_Slide=(UBYTE) imageNode->in_Slide;
                record->sn_Sequence=(UBYTE) imageNode->in_Sequence;
                rotateBits=(imageNode->in_Rotation/90)<<STORAGEB_ROTATE;
                record->sn_Flags=
                    (imageNode->in_IsSlide?STORAGEF_ISSLIDE:NULL)|
                    (imageNode->in_Mirror?STORAGEF_MIRROR:NULL)|
                    (imageNode->in_Magnification==PHOTOCD_RES_BASE?STORAGEF_MAGNIFY:NULL)|
                    rotateBits;
                record->sn_MagnifyX=imageNode->in_MagnifyX/2;
                record->sn_MagnifyY=imageNode->in_MagnifyY/2;

                /* Go to next record */
                record++;

            }

            imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

        }

        /* Construct EOF marker */
        record->sn_EOF=TRUE;

        /* Store data in nonvolatile memory */
        error=StoreNV(
            PROGRAM_NAME,
            appContext->ac_DiscSerNo,
            data,
            (cbData+NONVOLATILE_GRANULARITY-1)/NONVOLATILE_GRANULARITY,
            TRUE
        );
        if (error) {
#ifdef DEBUG
            kprintf("saveProgram(): Error storing save data\n");
#endif /* DEBUG */
        }

        /* Release storage records */
        FreeVec(data);

    } else {

#ifdef DEBUG
        kprintf("saveProgram(): Error allocating save data\n");
#endif /* DEBUG */
        error=Largest(error);

    }

    result=error?TRUE:FALSE;
    return result;

}

/****************************************************************************
 *                                                                          *
 *  loadProgram() - load program for disc                                   *
 *                                                                          *
 ****************************************************************************/

BOOL loadProgram(struct appContext *appContext)
{

    appContext->ac_StorageData=
        GetCopyNV(PROGRAM_NAME, appContext->ac_DiscSerNo, FALSE);
    if (appContext->ac_StorageData) {
        return TRUE;
    }

    return FALSE;

}

/****************************************************************************
 *                                                                          *
 *  unloadProgram() - unload program for disc                               *
 *                                                                          *
 ****************************************************************************/

void unloadProgram(struct appContext *appContext)
{

    /* Free loaded program data */
    if (appContext->ac_StorageData) {

        FreeNVData(appContext->ac_StorageData);
        appContext->ac_StorageData=NULL;

    }

}

/****************************************************************************
 *                                                                          *
 *  programImage() - get program entry for image                            *
 *                                                                          *
 ****************************************************************************/

BOOL programImage(struct appContext *appContext, struct imageNode *imageNode)
{

    struct storageNode *record;

    /* If program loaded ... */
    if (record=appContext->ac_StorageData) {

        /* Loop through records */
        while (!record->sn_EOF) {

            /* If record matches image ... */
            if (imageNode->in_Number==record->sn_Number) {

                /* Load image properties */
                imageNode->in_Slide=record->sn_Slide;
                imageNode->in_Sequence=record->sn_Sequence;
                if (imageNode->in_IsSlide!=(record->sn_Flags&STORAGEF_ISSLIDE)?TRUE:FALSE) {
                    imageNode->in_IsSlide=(record->sn_Flags&STORAGEF_ISSLIDE)?TRUE:FALSE;
                    imageNode->in_Manipulated=TRUE;
                }
                if (imageNode->in_Mirror!=(record->sn_Flags&STORAGEF_MIRROR)?TRUE:FALSE) {
                    imageNode->in_Mirror=(record->sn_Flags&STORAGEF_MIRROR)?TRUE:FALSE;
                    imageNode->in_Manipulated=TRUE;
                }
                if (imageNode->in_Rotation!=
                    ((record->sn_Flags>>STORAGEB_ROTATE)&STORAGEM_ROTATE)*90) {
                    imageNode->in_Rotation=
                        ((record->sn_Flags>>STORAGEB_ROTATE)&STORAGEM_ROTATE)*90;
                    imageNode->in_Manipulated=TRUE;
                }
                imageNode->in_Magnification=
                    record->sn_Flags&STORAGEF_MAGNIFY?PHOTOCD_RES_BASE:PHOTOCD_RES_BASE4;
                if (imageNode->in_Magnification!=PHOTOCD_RES_BASE4) {
                    imageNode->in_Manipulated=TRUE;
                }
                imageNode->in_MagnifyX=record->sn_MagnifyX*2;
                imageNode->in_MagnifyY=record->sn_MagnifyY*2;

                return TRUE;

            }

            record++;

        }

    }

    return FALSE;

}
