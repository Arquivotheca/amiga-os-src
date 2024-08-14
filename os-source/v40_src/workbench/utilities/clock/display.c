
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <string.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "clock.h"
#include "circle.h"
#include "date.h"


/*****************************************************************************/


extern struct Library *GfxBase;
extern struct Library *SysBase;
extern struct Library *LayersBase;
extern struct Library *IntuitionBase;

extern UBYTE  doSeconds;
extern UBYTE  doDates;
extern STRPTR dateFormat;
extern STRPTR timeFormat;

extern struct Screen   *sp;
extern struct Window   *wp;
extern struct RastPort *rp;
extern struct DrawInfo *di;


/*****************************************************************************/


WORD  radius_x, radius_y;	/* x & y radii of circle */
WORD  center_x, center_y;	/* coordinates of circle's center */
BOOL  secondsHand;
ULONG oldDays;
UWORD oldWidth  = 0;
UWORD oldHeight = 0;
char  dateStr[50];
char  timeStr[50];

ULONG facePen;
ULONG faceBorderPen;
ULONG handsPen;
ULONG secondsPen;


/*****************************************************************************/


#define NUMVECTORS 20

struct RastPort clonerp;
UBYTE           av[NUMVECTORS*5];
struct AreaInfo ai;
struct TmpRas   tr;
APTR            raster;


/*****************************************************************************/


struct Hand
{
    WORD handsize;		/* handsize relative to clock radius */
    WORD fat;			/* width of hand */
    WORD cent_x, cent_y;	/* coord of hand end point */
    WORD left_x, left_y;	/* coord of hand left-side point */
    WORD rght_x, rght_y;	/* coord of hand right-side point */
    WORD beta;			/* angle at which hand is at */
    WORD old_beta;		/* last angle calculated */
};


/**c***************************************************************************/


/* analog clock hand array members */
#define	HOURS		0
#define MINUTES		1
#define SECONDS		2


/* relative sizes of analog clock hands */
#define RAD		16
#define BIG		13
#define LITTLE		9
#define SECOND		13


struct Hand hand[] =
{
    {LITTLE, 6, 0, 0, 0, 0, 0, 0, 0, 0},
    {BIG,    3, 0, 0, 0, 0, 0, 0, 0, 0},
    {SECOND, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


/*****************************************************************************/


BOOL PreRender(VOID)
{
    if (raster = AllocRaster(wp->Width+15,wp->Height))
    {
        InitTmpRas(&tr,raster,RASSIZE(wp->Width+15,wp->Height));
        clonerp.TmpRas = &tr;
        return(TRUE);
    }

    oldWidth = 0;   /* This will cause a complete redraw the next time through */

    return(FALSE);
}


/*****************************************************************************/


VOID PostRender(VOID)
{
    WaitBlit();
    FreeRaster(raster,wp->Width+15,wp->Height);
    clonerp.TmpRas = NULL;
}


/*****************************************************************************/


/* plots the coordinates for a hand in the AreaInfo of the rastport */
VOID PlotHand(WORD i)
{
    AreaMove(&clonerp, center_x, center_y);
    AreaDraw(&clonerp, hand[i].left_x, hand[i].left_y);
    AreaDraw(&clonerp, hand[i].cent_x, hand[i].cent_y);
    AreaDraw(&clonerp, hand[i].rght_x, hand[i].rght_y);
    AreaDraw(&clonerp, center_x, center_y);
}


/*****************************************************************************/


/* draws all the clock hands */
VOID DrawHands(BOOL plotSeconds)
{
WORD  i;

    /* Fill in the "hand" array with the current info */
    for (i = 0; i < 3; i++)
    {
	hand[i].cent_x = cosine( hand[i].handsize * radius_x / RAD, (hand[i].beta + 270)) + center_x;
	hand[i].cent_y = sine( hand[i].handsize * radius_y / RAD, (hand[i].beta + 270)) + center_y;

	hand[i].left_x = cosine(( hand[i].handsize - 2) * radius_x / RAD, (hand[i].beta + 270 - hand[i].fat)) + center_x;
	hand[i].left_y = sine(( hand[i].handsize - 2) * radius_y / RAD, (hand[i].beta + 270 - hand[i].fat)) + center_y;

	hand[i].rght_x = cosine(( hand[i].handsize - 2) * radius_x / RAD, (hand[i].beta + 270 + hand[i].fat)) + center_x;
	hand[i].rght_y = sine(( hand[i].handsize - 2) * radius_y / RAD, (hand[i].beta + 270 + hand[i].fat)) + center_y;
    }

    SetAPen(&clonerp,handsPen);
    PlotHand(HOURS);
    PlotHand(MINUTES);
    AreaEnd(&clonerp);

    if (plotSeconds)
    {
        SetAPen(&clonerp,secondsPen);
        PlotHand(SECONDS);
        AreaEnd(&clonerp);
    }

    secondsHand = plotSeconds;
}


/*****************************************************************************/


/* draws the entire clock imagery */
VOID DrawClock(BOOL plotSeconds)
{
WORD i, rx, ry;
WORD rx1, rx2, ry1, ry2;

    /* Draw clock's face */
    rx = (radius_x = (wp->Width - (WORD)wp->BorderRight - (WORD)wp->BorderLeft - 4) / 2);
    ry = (radius_y = (wp->Height - (WORD)wp->BorderTop - (WORD)wp->BorderBottom - 12) / 2);

    if (!doDates)
        ry = (radius_y += 4);

    center_x = radius_x + 2 + wp->BorderLeft;
    center_y = radius_y + 2 + wp->BorderTop;

    if (PreRender())
    {
        if (sp->BitMap.Depth > 1)
        {
            SetDrMd(&clonerp, JAM1);

            SetAPen(&clonerp, facePen);
            AreaEllipse(&clonerp,center_x,center_y,rx,ry);
            AreaEnd(&clonerp);

            SetAPen(&clonerp, faceBorderPen);
            DrawEllipse(&clonerp,center_x,center_y,rx,ry);
            DrawEllipse(&clonerp,center_x,center_y,rx-1,ry);
        }
        else
        {
            SetAPen(&clonerp, faceBorderPen);
        }

        /* draw hour and tick marks */
        rx = ( rx - rx / 18 );
        ry = ( ry - ry / 18 );

        i = rx / 15;
        rx1 = rx - i;
        rx2 = rx1 - i;

        i = ry / 15;
        ry1 = ry - i;
        ry2 = ry1 - i;

        for (i = 0; i < 360; i += 6)
        {
            if (((i / 5) * 5) == i)         /* draw a diamond hour mark */
            {
                AreaMove(&clonerp, cosine(rx,i)+center_x, sine(ry,i)+center_y );
                AreaDraw(&clonerp, cosine(rx1,i-3)+center_x, sine(ry1,i-3)+center_y);
                AreaDraw(&clonerp, cosine(rx2,i)+center_x, sine(ry2,i)+center_y);
                AreaDraw(&clonerp, cosine(rx1,i+3)+center_x, sine(ry1,i+3)+center_y);
                AreaDraw(&clonerp, cosine(rx,i)+center_x, sine(ry,i)+center_y);
                AreaEnd(&clonerp);
            }

            else if ((radius_y > 30) && (radius_x > 60))     /* draw a tick mark */
            {
                Move(&clonerp,cosine(rx, i) + center_x, sine(ry, i) + center_y);
                Draw(&clonerp,cosine(rx1, i) + center_x, sine(ry1, i) + center_y);
            }
        }

        DrawHands(plotSeconds);

        PostRender();
    }
}


/*****************************************************************************/


/* Calculate the angle of each clock hand based on the number of seconds.
 * Fills in the "hands" array with the results
 */
VOID CalcBetas(ULONG seconds)
{
ULONG secs;
ULONG mins;
ULONG hrs;

    hrs  = (seconds / (60*60)) % 24;
    mins = (seconds / 60) % 60;
    secs = seconds % 60;

    hand[SECONDS].beta = secs * 6;
    hand[MINUTES].beta = (WORD)(mins * 6 + secs / 10);
    hand[HOURS].beta   = (WORD)(hrs * 30 + mins / 2);
}


/*****************************************************************************/


/* draws the current date in the bottom of the analog clock */
VOID ShowDate(ULONG seconds)   /* seconds since 01-Jan-78 */
{
struct Region    *region;
struct Rectangle  rect;
UWORD             len;

    GetDate(seconds,dateFormat,dateStr);
    if (region = NewRegion())
    {
        rect.MinX = wp->BorderLeft;
        rect.MaxX = wp->Width - wp->BorderRight - 1;
        rect.MinY = wp->Height - wp->BorderBottom - 9;
        rect.MaxY = wp->Height - wp->BorderBottom - 1;

        if (OrRectRegion(region,&rect))
        {
            region = InstallClipRegion(wp->WLayer,region);

            SetRast(&clonerp,di->dri_Pens[BACKGROUNDPEN]);

            if (doDates)
            {
                len = strlen(dateStr);
                SetAPen(&clonerp,di->dri_Pens[TEXTPEN]);
                Move(&clonerp,(wp->Width - TextLength(&clonerp,dateStr,len))/2,
                               wp->Height-wp->BorderBottom-clonerp.TxHeight+clonerp.TxBaseline);
                Text(&clonerp,dateStr,len);
            }
            region = InstallClipRegion(wp->WLayer,region);
        }
        DisposeRegion(region);
    }
}


/*****************************************************************************/


VOID DrawAnalog(ULONG seconds, BOOL initial)
{
BOOL  draw;
ULONG days;

    if (initial)
    {
        clonerp          = *rp;
        clonerp.AreaInfo = &ai;
        InitArea(&ai,av,NUMVECTORS);
    }

    LockLayerInfo(&wp->WScreen->LayerInfo);

    if ((wp->Width != oldWidth) || (wp->Height != oldHeight) || initial
    || (doDates && !oldDays) || (!doDates && oldDays))
    {
        oldWidth    = wp->Width;
        oldHeight   = wp->Height;
        BeginRefresh(wp);		/* clear any damage regions */
        EndRefresh(wp,TRUE);

        if (sp->BitMap.Depth == 1)
        {
            facePen       = di->dri_Pens[BACKGROUNDPEN];
            faceBorderPen = di->dri_Pens[TEXTPEN];
            handsPen      = faceBorderPen;
            secondsPen    = faceBorderPen;
        }
        else
        {
            facePen       = di->dri_Pens[SHINEPEN];
            faceBorderPen = di->dri_Pens[SHADOWPEN];
            handsPen      = di->dri_Pens[SHADOWPEN];
            secondsPen    = di->dri_Pens[FILLPEN];

            if (handsPen == facePen)
            {
                handsPen      = di->dri_Pens[BACKGROUNDPEN];
                faceBorderPen = di->dri_Pens[BACKGROUNDPEN];
            }

            if (secondsPen == facePen)
                secondsPen = di->dri_Pens[BACKGROUNDPEN];
        }

        SetAPen(&clonerp,di->dri_Pens[BACKGROUNDPEN]);    /* zap current contents */
        RectFill(&clonerp,wp->BorderLeft,
                          wp->BorderTop,
                          wp->Width-wp->BorderRight-1,
                          wp->Height-wp->BorderBottom-1);

        CalcBetas(seconds);
        DrawClock(doSeconds);		/* draw the entire clock display */
        oldDays = 0;			/* causes date to be redrawn */
    }
    else if (LAYERREFRESH & wp->WLayer->Flags)
    {
        BeginRefresh(wp);
        DrawClock(secondsHand);		/* draw the entire clock display through the damage region */
        EndRefresh(wp,TRUE);
        oldDays = 0; 			/* causes date to be redrawn */
    }
    else
    {
        CalcBetas(seconds);

        draw = FALSE;
        if (hand[HOURS].beta != hand[HOURS].old_beta)
        {
            PlotHand(HOURS);
            draw = TRUE;
        }

        if (hand[MINUTES].beta != hand[MINUTES].old_beta)
        {
            PlotHand(MINUTES);
            draw = TRUE;
        }

        if ((!doSeconds && secondsHand) || (doSeconds && (hand[SECONDS].beta != hand[SECONDS].old_beta)))
        {
            PlotHand(SECONDS);
            draw = TRUE;
        }

        if (draw)
        {
            SetAPen(&clonerp, facePen);

            if (PreRender())
            {
                AreaEnd(&clonerp);                /* erase all the hands */
                DrawHands(doSeconds);
                PostRender();
            }
        }
    }

    if (doDates)
    {
        days = seconds / (60*60*24);
        if (days != oldDays)
        {
            oldDays = days;
            ShowDate(seconds);
        }
    }

    UnlockLayerInfo(&wp->WScreen->LayerInfo);

    hand[HOURS].old_beta   = hand[HOURS].beta;
    hand[MINUTES].old_beta = hand[MINUTES].beta;
    hand[SECONDS].old_beta = hand[SECONDS].beta;
}


/*****************************************************************************/


VOID UpdateDigital(ULONG seconds)
{
ULONG days;

    if (doDates && (seconds % 4 < 2))
    {
        days = seconds / (60*60*24);
        if (days != oldDays)
        {
            GetDate(seconds,dateFormat,dateStr);
            oldDays = days;
        }
        SetWindowTitles(wp,dateStr,(STRPTR)-1);
    }
    else
    {
        GetTime(seconds,timeFormat,timeStr);
        SetWindowTitles(wp,timeStr,(STRPTR)-1);
    }
}
