/*
 *  $Id$
 *
 *  label.image
 *  Method Dispatch Module
 *  Copyright (C) 1994 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *  Unauthorized Duplication or Distribution Prohibited
 *
 */

/*
$Log$
*/

/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************
 *
 *  setAttrsMethod()   -   method for attribute setting
 *
 *****************************************************************************/

static LONG setAttrsMethod(
    struct ClassLib *ClassBase,
    Class *pClass,
    struct Image *pImage,
    struct opSet *pMsg,
    BOOL fInitialize
)
{

    struct objectData *pInstanceData;

    struct TagItem *pTagState, *pTag;
    ULONG tagData;

    BOOL fRefresh;

    /* Fetch instance data */
    pInstanceData=INST_DATA(pClass, pImage);

    /* If initializing ... */
    if (fInitialize) {

        /* Initialize dimensions */
        pImage->Width=LABEL_DEFAULT_WIDTH;
        pImage->Height=LABEL_DEFAULT_HEIGHT;

        /* Initialize label.image instance data */
        pInstanceData->od_Text=NULL;
        pInstanceData->od_TextAttr=NULL;
        pInstanceData->od_TextFont=NULL;
        pInstanceData->od_Underscore=LABEL_DEFAULT_UNDERSCORE;
        pInstanceData->od_Bitmap=NULL;

        /* Need to refresh */
        fRefresh=TRUE;

    /* ... else ... */
    } else {

        /* Call superclass handler */
        fRefresh=DoSuperMethodA(pClass, (Object *) pImage, pMsg);

    }

    /* Process label.image tags */
    pTagState=pMsg->ops_AttrList;
    while (pTag=NextTagItem(&pTagState)) {

        /* Fetch tag data */
        tagData=pTag->ti_Data;

        /* Switch on tag value ... */
        switch (pTag->ti_Tag) {

            case IA_Text:
                pInstanceData->od_Text=(STRPTR) tagData;
                fRefresh=TRUE;
                break;

            /* Text attributes */
            case IA_TextAttr:
                /* Set new text attributes */
                pInstanceData->od_TextAttr=(struct TextAttr *) tagData;
                fRefresh=TRUE;
                break;

            case IA_Underscore:
                pInstanceData->od_Underscore=tagData;
                fRefresh=TRUE;
                break;

            case IA_BitMap:
                pInstanceData->od_Bitmap=(struct BitMap *) tagData;
                fRefresh=TRUE;
                break;

            case SYSIA_DrawInfo:
                pInstanceData->od_DrawInfo=(struct DrawInfo *) tagData;
                break;

            case IA_BGPen:
                pInstanceData->od_BackgroundPen=tagData;
                fRefresh=TRUE;
                break;

            case IA_FGPen:
                pInstanceData->od_TextPen=tagData;
                fRefresh=TRUE;
                break;

            case IA_Width:
                pImage->Width=(WORD) tagData;
                fRefresh=TRUE;
                break;

            case IA_Height:
                pImage->Height=(WORD) tagData;
                fRefresh=TRUE;
                break;

        }

    }

    /* If default background or text pen selected, fetch
       from DrawInfo structure */
    if (pInstanceData->od_BackgroundPen==-1) {

        pInstanceData->od_BackgroundPen=
            (LONG) pInstanceData->od_DrawInfo->dri_Pens[BACKGROUNDPEN];

    }
    if (pInstanceData->od_TextPen==-1) {

        pInstanceData->od_TextPen=
            (LONG) pInstanceData->od_DrawInfo->dri_Pens[TEXTPEN];

    }

    /* Return refresh flag */
    return fRefresh;

}

/*****************************************************************************
 *
 *  drawMethod()   -   method for drawing
 *
 *****************************************************************************/

static LONG drawMethod (
    struct ClassLib *ClassBase,
    Class *pClass,
    struct Image *pImage,
    struct impDraw *pMsg)
{

    struct objectData *pInstanceData;
    struct DrawInfo *pDrawInfo;

    struct RastPort *rastPort;

    struct textFragment *pTextFragments;

    /* Fetch instance data */
    pInstanceData=INST_DATA(pClass, pImage);

    /* Fetch RastPort */
    rastPort=pMsg->imp_RPort;

    /* Fetch DrawInfo */
    pDrawInfo=pMsg->imp_DrInfo?
        pMsg->imp_DrInfo:
        pInstanceData->od_DrawInfo;

    /* If this is a text label ... */
    if (pInstanceData->od_Text) {

        /* Layout text */
        pTextFragments=layoutText(
            ClassBase,
            pInstanceData,
            &pImage->Width,
            &pImage->Height,
            pInstanceData->od_DrawInfo
        );
        if (pTextFragments) {

            /* Render text */
            renderText(
                ClassBase,
                pInstanceData,
                rastPort,
                pTextFragments,
                pMsg->imp_Offset.X,
                pMsg->imp_Offset.Y,
                pDrawInfo
            );

            /* Release text fragments */
            unlayoutText(
                ClassBase,
                pTextFragments
            );

        }

    /* ... else if this is a bitmap label ... */
    } else if (pInstanceData->od_Bitmap) {

        /* Render bitmap */
        renderBitmap(
            ClassBase,
            rastPort,
            pInstanceData->od_Bitmap,
            pMsg->imp_Offset.X, pMsg->imp_Offset.Y,
            pImage->Width, pImage->Height
        );

    }

    /* Success */
    return 0L;

}

/*****************************************************************************
 *
 *  domainMethod()   -   method for domain query
 *
 *****************************************************************************/

static LONG domainMethod(
    struct ClassLib *ClassBase,
    Class *pClass,
    struct gpDomain *pMsg
)
{

    struct Image *pImage;
    struct objectData *pInstanceData;

    struct textFragment *pTextFragment;

    SHORT width, height;

#ifdef DEBUG
    kprintf("domainMethod(): Entry\n");
#endif /* DEBUG */

    /* Construct temporary image object */
    pImage=
        NewObjectA(pClass, NULL, pMsg->gpd_Attrs);
#ifdef DEBUG
    kprintf(" pImage=%08lx\n", pImage);
#endif /* DEBUG */
    if (pImage) {

        /* Fetch instance data of object */
        pInstanceData=INST_DATA(pClass, pImage);

        /* For text label ... */
        if (pInstanceData->od_Text) {

            /* Layout text */
#ifdef DEBUG
            kprintf("  layoutText\n");
#endif /* DEBUG */
            pTextFragment=layoutText(
                ClassBase,
                pInstanceData,
                &width, &height,
                pInstanceData->od_DrawInfo
            );
            if (pTextFragment) {

#ifdef DEBUG
                kprintf("    layoutText() ok\n");
#endif /* DEBUG */

                /* Set width and height in message */
                pMsg->gpd_Width=width;
                pMsg->gpd_Height=height;

            } else {

#ifdef DEBUG
                kprintf("    layoutText() failed\n");
#endif /* DEBUG */

            }

        }

        /* Destroy temporary image object */
#ifdef DEBUG
        kprintf (" dispose image\n");
#endif /* DEBUG */
        DisposeObject(pImage);

    } else {

        return TRUE;

    }

    /* Success */
#ifdef DEBUG
    kprintf("   Success\n");
#endif /* DEBUG */
    return FALSE;

}

/*****************************************************************************
 *
 *  newMethod()   -   method for instantiation
 *
 *****************************************************************************/

static LONG newMethod(
    struct ClassLib *ClassBase,
    Class *pClass,
    struct Image *pImage,
    struct opSet *pMsg
)
{

    struct Image *pNewImage;

    /* Create new image object */
    pNewImage=(struct Image *) DoSuperMethodA(
        pClass,
        (Object *) pImage,
        pMsg
    );
    if (pImage) {

        /* Initialize attributes */
        setAttrsMethod(
            ClassBase,
            pClass,
            pNewImage,
            pMsg,
            TRUE
        );

    }

    /* Return instantiated image object */
    return ((LONG) pNewImage);

}

/*****************************************************************************
 *
 *  ClassDispatcher()   -   method dispatcher for class
 *
 *****************************************************************************/

LONG ASM ClassDispatcher (
    REG (a0) Class *pClass,
    REG (a1) ULONG *pMsg,
    REG (a2) struct Image *pImage
)
{

    struct ClassLib *ClassBase;

    /* Fetch class library base */
    ClassBase=(struct ClassLib *) pClass->cl_UserData;

    /* Switch on message ... */
    switch (*pMsg)
    {

        /* New */
        case OM_NEW:
            return newMethod(
                ClassBase,
                pClass,
                pImage,
                (struct opSet *) pMsg
            );

        /* Set */
        case OM_SET:
        /* Update */
        case OM_UPDATE:
            return setAttrsMethod(
                ClassBase,
                pClass,
                pImage,
                (struct opSet *) pMsg,
                FALSE
            );

        /* Draw */
        case IM_DRAW:
            return drawMethod(
                ClassBase,
                pClass,
                pImage,
                (struct impDraw *) pMsg
            );

        /* Domain query */
        case IM_DOMAIN:
            return domainMethod(
                ClassBase,
                pClass,
                (struct gpDomain *) pMsg
            );

        /* Default */
        default:
            /* Call superclass method */
            return DoSuperMethodA(
                pClass,
                (Object *) pImage,
                (Msg *) pMsg
            );

    }

}
