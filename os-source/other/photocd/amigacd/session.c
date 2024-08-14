/*** session.c ***************************************************************
 *
 *  $Id: session.c,v 1.6 94/03/17 16:26:57 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Session Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   session.c,v $
 * Revision 1.6  94/03/17  16:26:57  jjszucs
 * Program load/save implemented
 *
 * Revision 1.5  94/03/16  18:21:14  jjszucs
 * Zoom manipulation implemented.
 *
 * Fixed palette image display implemented with
 * DISPLAY_FIXED_PALETTE conditional compilation
 * option.
 *
 * Pan X/Y offsets eliminated from image node
 * structure, no longer needed as pan is not
 * necessary.
 *
 * All image manipulation features correctly mark
 * images as manipulated. Normalize button is
 * now correctly enabled/disabled depending on
 * state of image.
 *
 * Control list cleared during all state transitions.
 * Prevents potential crashes due to inappropriate
 * controls remaining available.
 *
 * Further changes to "Easter Egg" screen per davidj
 * and eric.
 *
 * Revision 1.4  94/03/09  17:08:19  jjszucs
 * Slideshow playback code implemented.
 *
 * Revision 1.3  94/03/08  13:59:26  jjszucs
 * End of session clean-up is more robust, eliminating problems
 * with using multiple discs in a single run of the application.
 *
 * Revision 1.2  94/02/18  15:57:23  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.1  94/01/06  11:58:07  jjszucs
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
#include <exec/memory.h>
#include <exec/lists.h>

#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/photocd_protos.h>
#include <clib/debug_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/photocd_pragmas.h>

/* ANSI includes */
#include <string.h>

/* Local includes */
#include "global.h"
#include "interface.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  beginSession() - begin session with Photo CD disc                       *
 *                                                                          *
 ****************************************************************************/

BOOL beginSession(struct appContext *appContext)
{

    struct TagItem *discInfo, *imageInfo;

    BOOL status;

    UWORD nImage;

    struct imageNode *imageNode;

    UBYTE *string;

    /*
     *  Initialization
     */

    /* If session is already open ... */
    if (appContext->ac_PhotoCDHandle) {

        /* No action; return success */
        return TRUE;

    }

    /*
     *  Open Photo CD handle
     */

    /* Open Photo CD handle */
    if (appContext->ac_PhotoCDHandle=OpenPhotoCD(TAG_DONE)) {

        /*
         *  Load image list
         */

        /* Busy user interface */
        interfaceBusyStart(appContext, GLYPH_BUSY_THUMBNAIL);

        /* Initialize image list */
        NewList((struct List *) &appContext->ac_ImageList);

        /* Clear slide counter */
        appContext->ac_NSlides=0;

        /* Obtain disc information */
        if (discInfo=ObtainPhotoCDInfo(appContext->ac_PhotoCDHandle,
            PCD_Disc, TRUE,
            TAG_DONE)) {

            /* Get number of images */
            appContext->ac_NImages=GetTagData(PCDDisc_nImages,0,discInfo);

            /* Fetch disc serial number */
            string=(UBYTE *) GetTagData(
                PCDDisc_SerNo,
                (ULONG) "",
                discInfo);
            appContext->ac_DiscSerNo=
                AllocVec(
                    strlen(string)+1,
                    NULL);
            if (appContext->ac_DiscSerNo) {

                strcpy(appContext->ac_DiscSerNo, string);

                /* Load program for disc (if any) */
                loadProgram(appContext);

                /* Loop through images */
                for (nImage=1;nImage<=appContext->ac_NImages;nImage++) {

                    /* Create image node */
                    imageNode=AllocVec(sizeof(*imageNode),MEMF_CLEAR);
                    if (imageNode) {

                        /* Obtain image information */
                        if (imageInfo=ObtainPhotoCDInfo(appContext->ac_PhotoCDHandle,
                            PCD_Image, nImage,
                            TAG_DONE)) {

                            /* Initialize image node to defaults */
                            imageNode->in_Number=nImage;
                            imageNode->in_Slide=nImage;
                            imageNode->in_Sequence=nImage;
                            imageNode->in_IsSlide=TRUE;
                            imageNode->in_Mirror=FALSE;
                            imageNode->in_Rotation=
                                GetTagData(PCDImg_Rotation,0,imageInfo);
                            imageNode->in_Magnification=PHOTOCD_RES_BASE4;
                            imageNode->in_MagnifyX=0;
                            imageNode->in_MagnifyY=0;
                            imageNode->in_Manipulated=FALSE;
                            imageNode->in_Thumbnail=NULL;

                            /* Load program for image */
                            programImage(appContext, imageNode);

                            /* Add image node to image list */
                            AddTail(
                                (struct List *) &appContext->ac_ImageList,
                                (struct Node *) imageNode);

                            /* Increment initial slide counter */
                            if (imageNode->in_IsSlide) {
                                appContext->ac_NSlides++;
                            }

                            /* Release image information */
                            ReleasePhotoCDInfo(imageInfo);

                            /* Success */
                            status=TRUE;

                        } else {

                            status=FALSE;

                        }

                    } else {

                        status=FALSE;

                    }

                    /* Update progress indicator */
                    interfaceBusyUpdate(appContext, appContext->ac_NImages, nImage);

                }

            } else {

#ifdef DEBUG
                kprintf("beginSession(): Can't allocate disc serial number\n");
#endif /* DEBUG */
                status=FALSE;

            }

            /* Release disc information */
            ReleasePhotoCDInfo(discInfo);

            /* Set image selection to first image */
            appContext->ac_Selection=
                IsListEmpty((struct List *) &appContext->ac_ImageList)?
                    NULL:
                    (struct imageNode *) appContext->ac_ImageList.mlh_Head;

            /* Set top of display to global meta-image */
            appContext->ac_Top=NULL;

            /* Clear slideshow play */
            appContext->ac_SlideshowPlay=FALSE;

            /* Unbusy interface */
            interfaceBusyEnd(appContext);

        } else {

            status=FALSE;

        }

    } else {

        status=FALSE;

    }

    /*
     *  Exit
     */

    /* Return */
    return status;

}

/****************************************************************************
 *                                                                          *
 *  endSession() - end session with Photo CD disc                           *
 *                                                                          *
 ****************************************************************************/

void endSession(struct appContext *appContext)
{

    struct imageNode *imageNode, *nextImageNode;

    /* If Photo CD handle open ... */
    if (appContext->ac_PhotoCDHandle) {

        /*
         *  Unload image list
         */

        /* Loop through image list */
        imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
        while (nextImageNode=(struct imageNode *) imageNode->in_Node.mln_Succ) {

            /* Unload thumbnail */
            unloadThumbnail(appContext,imageNode);

            /* Release image node */
            FreeVec(imageNode);

            /* Go to next image node */
            imageNode=nextImageNode;

        }

        /*
         *  Release disc information
         */

        /* Release serial number */
        if (appContext->ac_DiscSerNo) {

            FreeVec(appContext->ac_DiscSerNo);
            appContext->ac_DiscSerNo=NULL;

        }

        /*
         *  Close Photo CD handle
         */

        /* Close Photo CD handle */
        ClosePhotoCD(appContext->ac_PhotoCDHandle);
        appContext->ac_PhotoCDHandle=NULL;

    }

}

/****************************************************************************
 *                                                                          *
 *  imageBySequence() - find image by sequence number                       *
 *                                                                          *
 ****************************************************************************/

struct imageNode *imageBySequence(struct appContext *appContext, UWORD seqNumber)
{

    struct imageNode *imageNode;

    /* Special case search for image number 0 for speed */
    if (!seqNumber) {
        return NULL;
    }

    /* Loop through image list */
    for (imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
         imageNode->in_Node.mln_Succ;
         imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ) {

        /* If this image node matches ... */
        if (imageNode->in_Sequence==seqNumber) {

            /* Return */
            return imageNode;

        }

    }

    /* Not found */
    return NULL;

}
