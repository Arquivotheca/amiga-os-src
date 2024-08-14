/* :ts=4
*
*   screen.c -- for library
*
*   William A. Ware                         B116
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY                        *
*   Copyright (C) 1991, Silent Software, Incorporated.                      *
*   All Rights Reserved.                                                    *
****************************************************************************/

#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/io.h>

#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/rastport.h>

#include <cdtv/debox.h>
#include <cdtv/cdtv.h>
#include <cdtv/cdtvprefs.h>

#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <libraries/dos.h>

#include "/playerprefsbase.h"
#include "viewmgr.h"
#include "/screensaver/screensaver.h"
#include "keyclick.h"

#include "/playerprefs_private_pragmas.h"
#include "/playerprefs_protos.h"

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/debox_protos.h>
struct BitMap *DeBox_AllocBitMap( UWORD, UWORD, UWORD );
VOID DeBox_FreeBitMap (struct BitMap *);

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/debox_pragmas.h>
#pragma libcall DeBoxBase DeBox_AllocBitMap 66 21003
#pragma libcall DeBoxBase DeBox_FreeBitMap 6C 801


extern char __far TimerDeviceName[];


/*
 * Spoof the #pragmas
 */
#define SysBase     (PlayerPrefsBase->ppb_ExecBase)
#define DeBoxBase   (PlayerPrefsBase->ppb_DeBoxBase)
#define GfxBase     (PlayerPrefsBase->ppb_GfxBase)



/*------------------------------------------------------------------*/


/****** playerprefs.library/CreateMask **********************************
*
*   NAME
*   CreateMask -- Create a mask from a specific color in the bitmap.
*
*   SYNOPSIS
*   mask = CreateMask (bm, color, mymask);
*    d0        a0  d0:8     a1
*
*   UBYTE *CreateMask (struct BitMap *, UBYTE, UBYTE *);
*
*   FUNCTION
*   This routine creates a single bitplane "mask" out of the supplied
*   BitMap and 'color' argument, and returns a pointer to it.
*
*   For all pixels in the bitmap having a value equal to 'color', a zero
*   bit will be written in the corresponding position in the mask plane.
*   All other bits will be set to one.  Thus, the 'color' argument
*   specifies which color is "transparent."  This is useful for creating
*   masks for use with DPaint brushes.
*
*   If 'mymask' is non-NULL, CreateMask() will deposit the mask into the
*   memory pointed to by 'mymask.'  The memory buffer must reside in
*   CHIP RAM, and must be at least as large as one bitplane from the
*   supplied BitMap.
*
*   If 'mymask' is NULL, CreateMask() will attempt to allocate a single
*   bitplane in CHIP RAM with size and geometry identical to the planes
*   in the supplied BitMap.
*
*   INPUTS
*   bm  - pointer to a bitmap with all bitplanes in CHIP RAM
*   color   - pixel value to be used in making the mask
*   mymask  - pointer to a mask plane in CHIP RAM, or NULL
*
*   RESULT
*   mask    - pointer to resulting mask plane or NULL if an allocation
*         failed
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/

UBYTE * __asm LIBCreateMask(    register __a0   struct BitMap           *bm,
                                register __d0   UBYTE                   color,
                                register __a1   UBYTE               *mask,
                                register __a6   struct PlayerPrefsBase
                                                    *PlayerPrefsBase )
{
    struct BitMap   bm1;
    struct BitMap   bm2;
    register WORD   i;
    LONG            xsize;

    CopyMem((char *)bm,(char *)&bm1,(long)sizeof(struct BitMap *));
    CopyMem((char *)bm,(char *)&bm2,(long)sizeof(struct BitMap *));

    bm1.Depth = 1;
    bm2.Depth = 1;
    xsize = bm1.BytesPerRow << 3;

    if (!mask)
        mask = AllocMem((long)(bm1.BytesPerRow * bm1.Rows),MEMF_CHIP);
    if (!mask) return(NULL);

    bm1.Planes[0] = mask;

    BltBitMap(&bm2,0L,0L,&bm1,0L,0L,xsize,(long)bm1.Rows,0xffL,0xffL,NULL);
    for(i=0;i<bm->Depth;i++)
    {
        bm2.Planes[0] = bm->Planes[i];
        BltBitMap(&bm2,0L,0L,&bm1,0L,0L,xsize,(long)bm1.Rows,
                  (color & (1<<i) ? 0x80L:0x20L),0xffL,NULL);
    }
        /* INVERT */
    BltBitMap(&bm2,0L,0L,&bm1,0L,0L,xsize,(long)bm1.Rows,
                0x50L,0xffL, NULL );
    return(mask);
}


/*------------------------------------------------------------------*/
/****** playerprefs.library/CenterCDTVView **********************************
*
*   NAME
*   CenterCDTVView -- Centers a view based on the CDTVPrefs
*
*   SYNOPSIS
*   CenterCDTVView (cdtvprefs, view, width, height)
*              a0       a1   d0:16  d1:16
*
*   VOID CenterCDTVView (struct CDTVPrefs *, struct View *, WORD, WORD);
*
*   FUNCTION
*   Centers a View based on the settings in the CDTVPrefs structure.
*   The View's Dx- and DyOffset values are brought in line with the
*   CDTVPrefs structure.  The first ViewPort in the View is then
*   centered within the View based on GfxBase values and the 'width'
*   and 'height' arguments.
*
*   If 'cdtvprefs' is NULL, the preferences will be read from the
*   bookmark.
*
*   The View->ViewPort->RasInfo->RyOffset will be modified if the
*   resulting ViewPort DyOffset would be too high on the screen for
*   the system to handle.
*
*   Modification of the copper lists is not atomic; you should not call
*   this routine on a View that is currently being displayed.
*
*   INPUTS
*   cdtvprefs - pointer to a CDTVPrefs structure or a NULL
*   view      - pointer to a View with only one ViewPort
*   width     - width of the ViewPort to be modified
*   height    - height of the ViewPort to be modified
*
*   EXAMPLE
*
*   NOTES
*   This routine should be used only on Views having but one ViewPort.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/

VOID __asm LIBCenterCDTVView(   register __a0 struct CDTVPrefs *cdtv,
                                register __a1 struct View *v,
                                register __d0 WORD width,
                                register __d1 WORD height,
                                register __a6 struct PlayerPrefsBase
                                                        *PlayerPrefsBase )

{
    register struct ViewPort *vp;
    struct CDTVPrefs cp;
    register int t,min;

    if (!cdtv)
    {
        FillCDTVPrefs(&cp);
        cdtv = &cp;
    }
    vp = v->ViewPort;
    v->DxOffset = cdtv->DisplayX;
    v->DyOffset = cdtv->DisplayY;

    t = GfxBase->NormalDisplayColumns;
    if (!(vp->Modes & HIRES)) t>>=1;
    t = (t-width)>>1;

    vp->DWidth = width;
    vp->DxOffset = t;


    t = GfxBase->NormalDisplayRows;
    min = -v->DyOffset+13;      /* 11 pixels down */
    if (vp->Modes & LACE)
    {
        t += t;
        min += min;
    }
    t = (t-height)>>1;
    if (t < min)
    {
        vp->RasInfo->RyOffset = min-t;
        height -= (min-t);
        t = min;
    }
    else vp->RasInfo->RyOffset = 0;
    vp->DHeight = height;
    vp->DyOffset = t;

    if(GfxBase->LibNode.lib_Version >= 36) {
        if(vp->DyOffset <0) {
            v->DyOffset += vp->DyOffset;
            vp->DyOffset=0;
        }
    }

    MakeVPort( v, vp );
    MrgCop( v );
}


/****** playerprefs.library/GrabBm ****************************************
*
*   NAME
*   GrabBm -- Decompress a DeBox-compressed image in a useful way.
*
*   SYNOPSIS
*   success = GrabBm (header, data, bmiptr, bmptr, maskptr);
*     d0            a0     a1     a2     a3      a4
*
*   LONG GrabBm (struct CompHeader *, UBYTE *, struct BMInfo **,
*            struct BitMap **, UBYTE **);
*
*   FUNCTION
*   This function is a supplement to the debox.library.  It decompresses
*   the image described by 'header' and 'data,' and places the results
*   in pointers referenced by 'bmiptr,' 'bmptr,' and 'maskptr.'
*
*   The pointer referenced by 'bmiptr' will be filled with a pointer to
*   the BMInfo structure.  The pointer referenced by 'bmptr' will be
*   filled with a pointer to the decompressed bitmap.  The pointer
*   referenced by 'maskptr' will be filled with a pointer to the image's
*   mask plane, if present.  Note that GrabBm() allocates all these
*   structures for you; all you need supply are pointers to your
*   pointers.
*
*   If 'bmiptr,' 'bmptr,' or 'maskptr' are NULL, the corresponding
*   entity is not created.  (If all three are NULL, this function
*   becomes a no-op.)
*
*   If any internal operation fails, zero is returned, and 'bmiptr,'
*   'bmptr,' and 'maskptr' are filled with NULL.
*
*   INPUTS
*   header  - pointer to CompHeader structure, or NULL if data and
*         header are together
*   data    - pointer to compressed image
*   bmiptr  - pointer to client's BMInfo pointer
*   bmptr   - pointer to client's BitMap pointer
*   maskptr - pointer to client's mask pointer
*
*   RESULT
*   success - non-zero if successful; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   FreeGrabBm(), debox.library/DecompBitMap()
*
*****************************************************************************
*/
__asm LIBGrabBm(    register __a0   struct Header           *h,
                    register __a1   UBYTE                   *data,
                    register __a2   struct BMInfo           **bmiptr,
                    register __a3   struct BitMap           **bmptr,
                    register __a4   UBYTE                   **mask,
                    register __a6   struct PlayerPrefsBase  *PlayerPrefsBase )
{
    register UBYTE ms = 0;
    register struct BMInfo *bmi;
    register struct BitMap *bm;

    if (!(bmi = DecompBMInfo( NULL,NULL,data )))    goto xit;

    /* If only the bmiptr (or less) is passed then we are finished
     */
    if (!bmptr && !mask) goto shortcut;

    if (bmi->bmi_Flags & BMIF_HAS_MASK)             ms = 1;
    if (!(bm = DeBox_AllocBitMap(bmi->bmi_Depth+ms,bmi->bmi_Width,bmi->bmi_Height)))
        goto xit;
    bm->Depth -= ms;
    if (DecompBitMap(NULL,data,bm,bm->Planes[bmi->bmi_Depth]) < 0)
        goto xit;

    if (mask)
    {
        if (!ms)
        {
            if (bmi->bmi_Flags & BMIF_TRANSPARENT_COLOR)
                if (!(*mask = CreateMask(   bm,bmi->bmi_TransparentColor,NULL )))
                    goto xit;
        }
        else
            *mask = bm->Planes[ bm->Depth ];
    }
    else
    {
        if (bm->Planes[bm->Depth])
            FreeMem( bm->Planes[bm->Depth], (long)(bm->BytesPerRow * bm->Rows));
    }
    bm->Planes[bm->Depth] = 0L;


shortcut:
    if (bmiptr) *bmiptr = bmi;
    else        FreeBMInfo(bmi);

    if (bmptr)  *bmptr = bm;
    else        DeBox_FreeBitMap( bm );

    return(1);
xit:
    if (bm)     {bm->Depth += ms; DeBox_FreeBitMap(bm);}
    if (bmi)    FreeBMInfo(bmi);
    if (bmiptr) *bmiptr = NULL;
    if (bmptr)  *bmptr  = NULL;
    if (mask)   *mask   = NULL;
    return(0);
}


/*-----------------------------------------------------------------------*/


/****** playerprefs.library/FreeGrabBm **********************************
*
*   NAME
*   FreeGrabBm -- Frees resources allocated by a call to GrabBm().
*
*   SYNOPSIS
*   FreeGrabBm (bmiptr, bmptr, maskptr);
*             a0     a1       a2
*
*   VOID FreeGrabBm (struct BMInfo **, struct BitMap **, UBYTE **);
*
*   FUNCTION
*   This routine frees resources allocated with GrabBm().
*
*   'bmiptr' is a pointer to your BMInfo pointer.  'bmptr' is a pointer
*   to your BitMap pointer.  'maskptr' is a pointer to your mask
*   pointer.  NULL may be supplied for any of these parameters; no
*   attempt will be made to free the associated object.
*
*   On exit, the pointers referenced by 'bmiptr,' 'bmptr,' and 'maskptr'
*   will be filled with NULL.
*
*   INPUTS
*   bmiptr  - pointer to client's BMInfo pointer
*   bmptr   - pointer to client's BitMap pointer
*   maskptr - pointer to client's mask pointer
*
*   EXAMPLE
*   struct BMInfo *bmi;
*   struct BitMap *bm;
*
*   GrabBm (&bmi, &bm, NULL);
*   ...
*   FreeBm (&bmi, &bm, NULL);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   GrabBm()
*
*****************************************************************************
*/


VOID __asm LIBFreeGrabBm(   register __a0 struct BMInfo **bmi,
                            register __a1 struct BitMap **bm,
                            register __a2 UBYTE **mask,
                            register __a6 struct PlayerPrefsBase
                                                *PlayerPrefsBase)
{
    if (mask)
    {
        if (*mask)      {(*bm)->Planes[(*bm)->Depth++] = *mask;}
        *mask = 0;
    }
    if (bm)
    {
        if (*bm)        DeBox_FreeBitMap(*bm);
        *bm = 0;
    }
    if (bmi)
    {
        if (*bmi)       FreeBMInfo(*bmi);
        *bmi = 0;
    }
}

/*-----------------------------------------------------------------------*/


/****** playerprefs.library/FillBackDrop **********************************
*
*   NAME
*   FillBackDrop -- Fill a bitmap with a backdrop pattern.
*
*   SYNOPSIS
*   success = FillBackDrop (bm, header, strip);
*     D0            A0    A1     A2
*
*   LONG            success;
*   struct BitMap       *bm;
*   struct CompHeader   *header;
*   UBYTE           *strip;
*
*   FUNCTION
*   This routine fills a bitmap with a compressed image by stamping it
*   repeatedly horizontally across the bitmap.
*
*   'strip' points to the compressed image, which ideally should be a
*   tall, thin image at least as high as the destination bitmap.  The
*   image is centered in Y and stamped repeatedly across the destination 
*   bitmap starting at X=0 and incrementing in X by the strip's width.
*
*   This is useful for creating a gradated dithered background without
*   having to store the whole thing.
*
*   INPUTS
*   bm  - pointer to destination BitMap
*   header  - pointer to CompHeader structure
*   strip   - pointer to compressed strip image
*
*   RESULT
*   success - non-zero if the operation succeeded; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/


int __asm LIBFillBackDrop(  register __a0 struct BitMap *bm,
                            register __a1 struct Header *header,
                            register __a2 UBYTE *strip,
                            register __a6 struct PlayerPrefsBase
                                                    *PlayerPrefsBase)
{
    struct BitMap *sbm = NULL;
    struct BMInfo *sbmi = NULL;
    register WORD i,mw,w,h,syoff,byoff;

    if (!GrabBm( header,strip,&sbmi,&sbm,NULL)) return(0);
    mw = bm->BytesPerRow << 3;
    syoff = byoff = 0;

    if (sbmi->bmi_Height > (h = bm->Rows))
    {
        /* Strip larger than the bitmap - clip strip. */
        syoff = (sbmi->bmi_Height - h)>>1;
    }
    else
    {
        if (sbmi->bmi_Height < h)
        {
            h = sbmi->bmi_Height;
            byoff = (h - sbmi->bmi_Height)>>1;
        }
    }
    for(i=0;i<mw;i+=sbmi->bmi_Width)
    {
        w = sbmi->bmi_Width;
        if (w+i >= mw) w = mw-i;
        BltBitMap( sbm, 0,syoff,bm, i,byoff, w,h, 0xc0,0xff,NULL);
    }

    /* We might want to do a top and bottom fill for strips that are smaller
     * than the width & height of the Screen.
     */

    FreeGrabBm( &sbmi,&sbm,NULL );
    return(1);
}

/*-----------------------------------------------------------------------*/



/****** playerprefs.library/FillForeground **********************************
*
*   NAME
*   FillForeground -- Render a foreground image into a bitmap.
*
*   SYNOPSIS
*   success = FillForeground (bm, header, data, xoff, yoff, bmiptr);
*     D0              A0    A1     A2    D0    D1     A3
*
*   LONG            success;
*   struct BitMap       *bm;
*   struct CompHeader   *header;
*   UBYTE           *data;
*   WORD            xoff, yoff;
*   struct BMInfo       **bmiptr;
*
*   FUNCTION
*   This routine renders an image described by 'header' and 'data' and
*   renders it into the bitmap pointed to by 'bm.'
*
*   'data' points to the compressed image, which is decompressed and
*   rendered into the bitmap.  The upper-left corner of the image is
*   placed at coordinates xoff,yoff in the bitmap.  If the image has
*   a mask associated with it, the image is rendered through the mask
*   "cookie-cut style."
*
*   If 'bmiptr' is non-NULL, then a pointer to the BMInfo of the
*   compressed image will be stored in the pointer referenced by
*   'bmiptr.'
*
*   INPUTS
*   bm  - pointer to BitMap to receive image
*   header  - pointer to a CompHeader structure
*   data    - pointer to compressed image
*   xoff    - X-offset of image in destination BitMap
*   yoff    - Y-offset of image in destination BitMap
*   bmiptr  - pointer to BMInfo pointer, or NULL
*
*   RESULT
*   success - non-zero if operation succeeded; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   GrabBm()
*
*****************************************************************************
*/


int __asm LIBFillForeground(register __a0 struct BitMap *mbm,
                            register __a1 struct Header *header,
                            register __a2 UBYTE *data,
                            register __d0 WORD xoff,
                            register __d1 WORD yoff,
                            register __a3 struct BMInfo **bmiptr,
                            register __a6 struct PlayerPrefsBase
                                                    *PlayerPrefsBase)
{
    struct BitMap       *bm = NULL;
    struct BMInfo       *bmi = NULL;
    UBYTE               *mask = NULL;
    struct RastPort     rp;
    int                 ans = 0;

    if (!GrabBm( header,data,&bmi,&bm,&mask)) goto xit;

    InitRastPort( &rp );
    rp.BitMap = mbm;

    if (mask)
    {
        BltMaskBitMapRastPort(  bm, 0,0, &rp, xoff, yoff,
                                bmi->bmi_Width,bmi->bmi_Height,
                                (ABC|ABNC|ANBC),mask);
    }
    else
    {
        BltBitMapRastPort(  bm,0,0,&rp, xoff, yoff,
                            bmi->bmi_Width,bmi->bmi_Height,0xc0 );
    }

    if (bmiptr)
    {
        *bmiptr = bmi;
        bmi = NULL;     /* and don't deallocated bmi */
    }
    ans = 1;
xit:
    FreeGrabBm( &bmi,&bm,&mask );
    return(ans);
}


/*-----------------------------------------------------------------------*/



/****** playerprefs.library/BuildBg **********************************
*
*   NAME
*   BuildBg -- Build bitmap from ground up.
*
*   SYNOPSIS
*   success = BuildBg (bbg, bmiptr, bmptr);
*
*   LONG            success;
*   struct BuildBg_Info *bbg;
*   struct BMInfo       **bmiptr;
*   struct BitMap       **bmptr;
*
*   FUNCTION
*   This routine allocates and renders a bitmap according to the
*   information contained in the BuildBg_Info structure pointed to by
*   'bbg.'
*
*   The background is rendered according to the rules specified in
*   FillBackDrop().  The foreground is rendered according to the rules
*   specified in FillForeground().
*
*   The pointer referenced by 'bmiptr' will be filled with a pointer to
*   the BMInfo of the resulting image.  The pointer referenced by
*   'bmptr' will be filled with a pointer to the created bitmap.  Both
*   'bmiptr' and 'bmptr' may be passed as NULL; the associated object
*   will not be created.  (Passing both as NULL turns this routine into
*   a very expensive no-op.)
*
*   If any internal operation fails, zero is returned and nothing is
*   allocated.
*
*   INPUTS
*   bbg - pointer to BuildBg_Info structure:
*       BackHeader  - pointer to CompHeader structure for background
*       BackData    - pointer to compressed 'strip' for background
*       ForHeader   - pointer to CompHeader structure for foreground
*       ForData - pointer to compressed image for foreground
*       width   - width in pixels of bitmap to be created
*       height  - height in pixels of bitmap to be created
*       depth   - number of planes of bitmap to be created
*       ForXOff - X-offset of foreground image in bitmap
*       ForYOff - Y-offset of foreground image in bitmap
*       flags   - reserved
*   bmiptr  - pointer to BMInfo pointer
*   bmptr   - pointer to BitMap pointer
*
*   RESULT
*   success - non-zero if all went well; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   FillBackDrop(), FillForeground()
*
*****************************************************************************
*/


int __asm LIBBuildBg(   register __a0 struct BuildBg_Info   *bbg,
                        register __a1 struct BMInfo         **bmiptr,
                        register __a2 struct BitMap         **bmptr,
                        register __a6 struct PlayerPrefsBase
                                                    *PlayerPrefsBase)
{
    struct BitMap       *mbm;
    struct BMInfo       *bmi = NULL;
    int                 ans = 0;

    if (!(mbm = DeBox_AllocBitMap( bbg->Depth, bbg->Width, bbg->Height )))
        return(0);

    if ( bbg->BackData )
    {
	if (!(FillBackDrop( mbm, bbg->BackHeader, bbg->BackData ))) goto xit;
    }
    if (!(FillForeground( mbm, bbg->ForHeader, bbg->ForData,
                          bbg->ForXOff,bbg->ForYOff, &bmi )))  goto xit;

    bmi->bmi_Width = bbg->Width;    /* Mod bmi to match the mbm */
    bmi->bmi_Height = bbg->Height;
    bmi->bmi_Depth = bbg->Depth;
    if (bmptr)  {*bmptr  = mbm; mbm = NULL;}    /* don't deallocate mbm */
    if (bmiptr) {*bmiptr = bmi; bmi = NULL;}    /* and don't deallocated bmi */

    ans = 1;
xit:
    if (bmi) FreeBMInfo ( bmi );
    if (mbm) DeBox_FreeBitMap ( mbm );
    return(ans);
}




/*-----------------------------------------------------------------------*/

/****i* playerprefs.library/WriteTinyStr ***********************************
*
*   NAME
*   WriteTinyStr -- Write a limited string into a BitMap with a very
*           small font.
*
*   SYNOPSIS
*   WriteTinyStr (bm, x, y, color, str);
*             A0  D0 D1  D2    A1
*
*   struct BitMap   *bm;
*   WORD        x, y;
*   UBYTE       color;
*   UBYTE       *str;
*
*   FUNCTION
*   Writes a really dinky string into the BitMap pointed to by 'bm' at
*   the coordinates specified by 'x' and 'y' using a pen value of
*   'color.'
*
*   The available character set is limited to the numerals '0' - '9',
*   the letters 'A' - 'Z', 'a' - 'z' (which get mapped to their
*   uppercase counterparts), and the symbol '.'.  All other characters
*   are mapped to a blank space.
*
*   INPUTS
*   bm  - pointer to receive text
*   x   - leftmost edge of rendered text
*   y   - topmost edge of rendered text
*   color   - pen color used to render text
*   str - pointer to string to be rendered, NULL-terminated
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/
#define NUMBERDATA_W        112
#define NUMBERDATA_H        5

#define NUMDATA_SIZE  (RASSIZE(NUMBERDATA_W,NUMBERDATA_H))

UWORD __far NumberData[] = {
    0xEBFB,0xFFFC,0x7DF7,0xFEF3,0x65AB,0x2CFD,0xB6DE,
    0xA89B,0x21B4,0x5B2C,0x92A3,0x67F6,0xDB15,0xB6D2,
    0xABBF,0xF9FC,0x7D2E,0xD3A3,0xA5F7,0x5D95,0xB554,
    0xAA12,0x69A4,0x5B2C,0x96AB,0x65F6,0x6A55,0xBEA8,
    0xEBF3,0xF9E5,0x5DF7,0x9EFF,0x7DAA,0x3B97,0x56AE
};

__asm LIBWriteTinyStr(  register __a0 struct BitMap *bm,
                        register __d0 WORD x,
                        register __d1 WORD y,
                        register __d2 UBYTE color,
                        register __a1 UBYTE *str,
                        register __a6 struct PlayerPrefsBase *PlayerPrefsBase)
{
    struct RastPort         rp;
    struct BitMap           *sbm;
    UBYTE                   *onpl,*offpl;
    int                     i;
    int                     val;
    register UBYTE          no;


    if (!(sbm= DeBox_AllocBitMap( 2, NUMBERDATA_W,NUMBERDATA_H ))) return(0);

    onpl = sbm->Planes[0];
    offpl = sbm->Planes[1];
    sbm->Depth = bm->Depth;
    for(i=0;i<bm->Depth;i++,color >>= 1)
        sbm->Planes[i] = (color & 1 ? onpl:offpl);
    CopyMem( NumberData,onpl,NUMDATA_SIZE);
    MemSet( offpl, 0, NUMDATA_SIZE );

    InitRastPort(&rp);
    rp.BitMap = bm;

    while(*str)
    {
        val = -1;
        no = *str++;
        if ((no >= '0') && (no <= '9')) val = no - 48;
        if ((no >= 'A') && (no <= 'Z')) val = no - ('A' - 11);
        if ((no >= 'a') && (no <= 'z')) val = no - ('a' - 11);
        if (no == '.')                  val = 10;
        if (val == -1) { x+= 4; continue; }
        BltMaskBitMapRastPort( sbm,val*3,0,&rp,x,y,3,5,(ABC|ABNC|ANBC),onpl );
        x += 4;
    }
    WaitBlit();
    sbm->Depth = 2;
    sbm->Planes[0] = onpl;
    sbm->Planes[1] = offpl;
    DeBox_FreeBitMap( sbm );
    return(1);
}



/*-------------------------------------------------------------------------*/


/****** playerprefs.library/SafeWaitIO *************************************
*
*   NAME
*   SafeWaitIO -- Safer I/O completion mechanism.
*
*   SYNOPSIS
*   error = SafeWaitIO (ior);
*    D0          A0
*
*   LONG            error;
*   struct IORequest    *ior;
*
*   FUNCTION
*   This routine is identical to WaitIO(), except that it prevents
*   additional calls to WaitIO() on IORequests that have already
*   completed.  To elaborate, the sequence:
*
*       SendIO (ior);
*       WaitIO (ior);
*       WaitIO (ior);   *  WaitIO() on already-completed IOR.  *
*
*   is unsafe, since WaitIO() unconditionally calls Remove() on the
*   IORequest.  SafeWaitIO() detects and avoids this condition.
*
*   SafeWaitIO() and WaitIO() CANNOT be intermixed if you wish to reap
*   the benefits of SafeWaitIO().
*
*   INPUTS
*   ior - pointer to an IORequest structure
*
*   RESULT
*   error   - copy of the IORequest.io_Error field
*
*   EXAMPLE
*   SendIO (ior);
*   SafeWaitIO (ior);
*   SafeWaitIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   SafeDoIO()
*
*****************************************************************************
*/
LONG __asm LIBSafeWaitIO(   register __a0 struct IORequest *ioreq,
                register __a6 struct PlayerPrefsBase 
                *PlayerPrefsBase )
{
    register LONG   retval;

    Forbid();
    if (ioreq->io_Message.mn_Node.ln_Succ) {
        /*
         * The I/O packet has not been checked back in.  WaitIO() on
         * it and flag it as well and truly completed.
         */
        retval = WaitIO(ioreq);
        ioreq->io_Message.mn_Node.ln_Succ = NULL;
    } 
    else {
        /*
         * The I/O packet is already in; use existing error code.
         */
        retval = ioreq->io_Error;
    }
    Permit();
    return (retval);
}



/****** playerprefs.library/FreeIORequest **********************************
*
*   NAME
*   FreeIORequest -- Free an I/O session started with AllocIORequest().
*
*   SYNOPSIS
*   FreeIORequest (ior);
*           A0
*
*   struct IORequest    *ior;
*
*   FUNCTION
*   This routine frees all resources procured with AllocIORequest().
*
*   Any outstanding IO is aborted and allowed to complete.  The device
*   associated with the IORequest is closed, the associated replyport is
*   deleted, and the IORequest itself is then freed.
*
*   INPUTS
*   ior - pointer to IORequest structure
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   AllocIORequest()
*
*****************************************************************************
*/
VOID __asm LIBFreeIORequest(    register __a0 void *ioreq,
                                register __a6 struct PlayerPrefsBase 
                                                        *PlayerPrefsBase )  
{
    register struct IORequest *r;

    if (!(r = (struct IORequest *)ioreq)) return;
    if (r->io_Device)
    {
        if (r->io_Message.mn_Node.ln_Succ) AbortIO(r);
        SafeWaitIO(r);
        CloseDevice( r );
    }
    if (r->io_Message.mn_ReplyPort)
        DeleteMsgPort( r->io_Message.mn_ReplyPort );
    DeleteExtIO( r );
}

/****** playerprefs.library/AllocIORequest *********************************
*
*   NAME
*   AllocIORequest -- Start a session with an Exec device.
*
*   SYNOPSIS
*   ior = AllocIORequest (devname, unit, flags, size);
*   D0          A0      D0     D1    D2
*
*   struct IORequest    *ior;
*   char            *devname;
*   ULONG           unit, flags, size;
*
*   FUNCTION
*   This routine does all the mish-mash necessary to start a session
*   with an Exec-style I/O device.
*
*   A nameless replyport is created, an IORequest structure is created,
*   the named device is opened with the specified unit number and
*   flags, and a pointer to the IORequest is returned.  If any of these
*   operations fail, NULL is returned and nothing is allocated.
*
*   INPUTS
*   devname - pointer to name of device
*   unit    - device unit number, as used by OpenDevice()
*   flags   - device flags, as used by OpenDevice()
*   size    - size of desired IORequest structure, in bytes
*
*   RESULT
*   ior - pointer to allocated IORequest structure, or NULL
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   FreeIORequest()
*
*****************************************************************************
*/
VOID * __asm LIBAllocIORequest( register __a0 char *devname, 
                                register __d0 ULONG unitNumber, 
                                register __d1 ULONG flags, 
                                register __d2 ULONG size,
                                register __a6 struct PlayerPrefsBase 
                                                        *PlayerPrefsBase )
{
    register struct IORequest *r;
    register struct MsgPort *m;
    
    if (!(m = (struct MsgPort *)CreateMsgPort())) return(NULL);
    if (!(r = (struct IORequest *)CreateExtIO( m,size)))
    {
        DeleteMsgPort(m);
        return(NULL);
    }
    if (OpenDevice( devname, unitNumber, r, flags )) 
    {
        r->io_Device = NULL;
        FreeIORequest( r );
        return(NULL);
    }
    return( (VOID *)r );
}



/****** playerprefs.library/SafeDoIO ***************************************
*
*   NAME
*   SafeDoIO -- Safer I/O operation mechanism.
*
*   SYNOPSIS
*   error = SafeDoIO (ior);
*    D0        A0
*
*   LONG            error;
*   struct IORequest    *ior;
*
*   FUNCTION
*   Identical in operation to DoIO().  Exists for the same reason as
*   SafeWaitIO().  In particular:
*
*       DoIO (ior);
*       WaitIO (ior);
*
*   is unsafe.
*   
*   SafeDoIO() and DoIO() cannot be intermixed.  See the SafeWaitIO()
*   docs for more details.
*
*   INPUTS
*   ior - pointer to an IORequest structure
*
*   RESULT
*   error   - copy of IORequest.io_Error field
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   SafeWaitIO()
*
*****************************************************************************
*/
LONG __asm LIBSafeDoIO (  register __a0 struct IORequest *ioreq,
                          register __a6 struct PlayerPrefsBase 
                                                        *PlayerPrefsBase )
{
    register LONG   retval;

    retval = DoIO (ioreq);
    ioreq->io_Message.mn_Node.ln_Succ = NULL;   /*  It's back.  */
    return (retval);
}





/****i* playerprefs.library/SetVersionStr **********************************
*
*   NAME
*   SetVersionStr -- Set the version string in PlayerPrefsBase.
*
*   SYNOPSIS
*   SetVersionStr (str);
*           A0
*
*   char    *str;
*
*   FUNCTION
*   The string pointed to by 'str' is copied into PlayerPrefsBase's
*   private copy of the version string.  The maximum permitted length
*   is seven characters, and will be truncated if longer.
*
*   INPUTS
*   str - pointer to string
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/
VOID __asm LIBSetVersionStr( register __a0 char *str,
                        register __a6 struct PlayerPrefsBase *PlayerPrefsBase)
{
    char *s;
    int c = 0;


    ObtainSemaphore( &PlayerPrefsBase->ppb_LibLock );
    s = PlayerPrefsBase->ppb_CDTVVersionStr;
    for(;*str && (c < 7);c++)
        *s++ = *str++;
    for(;c < 8;c++)
        *s++ = 0;
    ReleaseSemaphore( &PlayerPrefsBase->ppb_LibLock );
}


/*=========================================================================
 * WARNING!!! Critical section code - Works with a critical section.
 * modifiy and program with lots of thought
 *=========================================================================
 */

/*=========================================================================
 * launchPPTask()
 *
 * Allows a task to be started with and a success code to be given weather
 * it was started correctly.  The Task that has been started  =M U S T= call
 * sigPPTask() before it exits.
 *========================================================================
 */
 
int
launchPPTask(char *taskname,LONG pri ,APTR prog, ULONG stacksize,
             struct PlayerPrefsBase *PlayerPrefsBase)
{
    int ans = 0;
    
    ObtainSemaphore( &PlayerPrefsBase->ppb_LibLock );
    
    PlayerPrefsBase->ppb_TitleSigNo = AllocSignal(-1);
    PlayerPrefsBase->ppb_TitleSigTask = FindTask(NULL);
    if (!CreateTask( taskname,pri,prog,stacksize )) goto xit;

    if (PlayerPrefsBase->ppb_TitleSigNo != -1)
    {
        Wait( 1 << PlayerPrefsBase->ppb_TitleSigNo);
        FreeSignal( PlayerPrefsBase->ppb_TitleSigNo );
    }
    while(PlayerPrefsBase->ppb_TitleSigTask) WaitTOF();
    Forbid();
    ans = PlayerPrefsBase->ppb_TitleAns;
    Permit();
xit:
    ReleaseSemaphore( &PlayerPrefsBase->ppb_LibLock );
    return(ans);
}

/*=======================================================================
 * sigPPTask()
 *
 * Signals the creating task that it can return to normal running.
 *
 * It sould always be called by a task created by launch pp.  
 * a return value of zero sould be used if the task is going to xit.
 *======================================================================
 */
VOID sigPPTask(struct PlayerPrefsBase *PlayerPrefsBase, int val) {

    Forbid();

    if (PlayerPrefsBase->ppb_TitleSigTask && (PlayerPrefsBase->ppb_TitleSigNo != -1)) {

        Signal(PlayerPrefsBase->ppb_TitleSigTask, 1<<PlayerPrefsBase->ppb_TitleSigNo);
        }

    PlayerPrefsBase->ppb_TitleSigTask = NULL;
    PlayerPrefsBase->ppb_TitleAns = val;
    Permit();
    }

/*-------------------------------------------------------------------------*/

/****i* playerprefs.library/CDTVTitle **************************************
*
*   NAME
*   CDTVTitle -- Display and manipulate the CDTV "strap" screen.
*
*   SYNOPSIS
*   success = CDTVTitle (state);
*     D0              D0
*
*   LONG    success;
*   LONG    state;
*
*   FUNCTION
*   Creates and displays the rotating CDTV logo displayed at boot time.
*   The parameter 'state' determines what it should do.  'state' can
*   have the following values:
*
*   TITLESTATUS_NORMAL:
*       Brings up CDTV strap screen with normal colors.  If the
*       screen is already up, the colors will be faded to the normal
*       colors.
*
*   TITLESTATUS_ERROR:
*       Brings up CDTV strap screen with error colors (red).  If the
*       screen is already up, the colors will be faded to the error
*       colors.
*
*   TITLESTATUS_LEAVE:
*       Fades out CDTV strap screen and frees all resources.
*
*   INPUTS
*   state   - state in which to display the screen
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*/

__asm CDTVTitleFunc(    register __d0 WORD state,
                        register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    return(0);
    }







/****** playerprefs.library/FadeCMap ***************************************
*
*   NAME
*   FadeCMap -- Calculate an intermediate color map.
*
*   SYNOPSIS
*   FadeCMap (map0, map15, destmap, ncolors, level);
*          A0     A1     A2       D0      D1
*
*   UWORD   *map0, *map15, *destmap;
*   WORD    ncolors, level;
*
*   FUNCTION
*   This routine calculates an intermediate color map between the color
*   maps specified by 'map0' and 'map15' using a "fade level" specified
*   by 'level.'  The resulting color map is placed in the buffer pointed
*   to by 'destmap.'
*
*   The valid range of values for 'level' is 0 - 15.  A value of zero
*   yields the colors in 'map0;' a value of 15 yields the colors in
*   'map15.'  Values outside this range are clipped.
*
*   'ncolors' is the number of colors in the color maps.  Values greater
*   than 32 are truncated (for no readily apparent reason).
*
*   INPUTS
*   map0    - pointer to fade level 0 color map
*   map15   - pointer to fade level 15 color map
*   destmap - pointer to buffer to receive new color map
*   ncolors - number of colors in color map
*   level   - fade level; range 0 - 15
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*   BetweenColor()
*
*****************************************************************************
*/
VOID __asm LIBFadeCMap( register __a0 UWORD *cmap0,
                        register __a1 UWORD *cmap15,
                        register __a2 UWORD *ncmap,
                        register __d0 WORD  ncr,
                        register __d1 WORD  lv,
                        register __a6 struct PlayerPrefsBase *PlayerPrefsBase)
{
    if (ncr > 32) ncr = 32;
    if (lv >= 15)
    {
        CopyMem( cmap15, ncmap, ncr << 1 );
        return;
    }
    if (lv <  0)
    {
        CopyMem( cmap0, ncmap, ncr << 1 );
        return;
    }
    while (ncr--)
    {
        *ncmap++ = BetweenColor( *cmap0++,*cmap15++,lv );
    }
    return;
}


