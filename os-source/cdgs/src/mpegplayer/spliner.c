
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <dos/dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "mpegplayerbase.h"
#include "tasks.h"
#include "utils.h"
#include "splines.h"
#include "spliner.h"


/*****************************************************************************/


#define ANIMINTERVAL (3600)
#define HALF(a)      ((*(a)+(a)[1])>>1)
#define FIX(x)       (((LONG)(x)) << 7)
#define FIXH(x)      (((LONG)((*(x)+(x)[1])>>1)) << 7)

static const STRPTR nextlegal[] = {"01458", "236", "01458", "236", "01458", "23", "01458", "", "0145"};
static const WORD   advval[] = { 3, 2, 3, 2, 1, 0, 1, 0, 1 };


/*****************************************************************************/


static VOID MakeFunc(struct MPEGPlayerLib *MPEGPlayerBase)
{
WORD  i;
WORD  goallen;
WORD  sofar;
char *p;
char *nextpossib;

    MPEGPlayerBase->mp_Spliner_closed = Random(4,&MPEGPlayerBase->mp_Spliner_seed);
    switch (MPEGPlayerBase->mp_Spliner_closed)
    {
        case 3: MPEGPlayerBase->mp_Spliner_closed = 2;

        case 2: goallen = 3 + Random(4,&MPEGPlayerBase->mp_Spliner_seed);
                break;

        case 1: goallen = 4 + Random(7,&MPEGPlayerBase->mp_Spliner_seed);
                break;

        case 0: goallen = 2 + Random(8,&MPEGPlayerBase->mp_Spliner_seed);
                break;
    }

    while (1)
    {
        if (MPEGPlayerBase->mp_Spliner_closed == 0)
            nextpossib = "0145";
        else
            nextpossib = "0123456";

        sofar = 0;
        p = MPEGPlayerBase->mp_Spliner_realfunc;
        while (sofar < goallen)
        {
            i          = nextpossib[Random((WORD)strlen(nextpossib),&MPEGPlayerBase->mp_Spliner_seed)]   - '0';
            *p++       = i;
            nextpossib = nextlegal[i];
            sofar     += advval[i];
        }

        if (sofar == goallen)
        {
            if (MPEGPlayerBase->mp_Spliner_closed == 0)
            {
                if (nextpossib[0] == '0')
                    break;
            }
            else
            {
                if (*nextpossib == '0' || MPEGPlayerBase->mp_Spliner_realfunc[0] < 4  ||  *(p-1) < 4)
                {
                    if ((*nextpossib == '0') ?
                             ((MPEGPlayerBase->mp_Spliner_realfunc[0]  & 2) != 0) : ((MPEGPlayerBase->mp_Spliner_realfunc[0]  & 2) == 0))
                    {
                        if (MPEGPlayerBase->mp_Spliner_realfunc[0] != 5)
                        {
                            MPEGPlayerBase->mp_Spliner_realfunc[0] ^=  2;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    *p = 100;
    MPEGPlayerBase->mp_Spliner_maxPoints = goallen;
    switch (MPEGPlayerBase->mp_Spliner_closed)
    {
        case 2: for (i=0; i<p-MPEGPlayerBase->mp_Spliner_realfunc; i++)
                    p[i] = MPEGPlayerBase->mp_Spliner_realfunc[i];
                p[p-MPEGPlayerBase->mp_Spliner_realfunc] = 100;
                break;

        case 1: break;

        case 0: MPEGPlayerBase->mp_Spliner_maxPoints++;
                break;
    }
}


/*****************************************************************************/


static VOID Draw_S_F(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    MPEGPlayerBase->mp_Spliner_Context.OldX = HALF(xptr);
    MPEGPlayerBase->mp_Spliner_Context.OldY = HALF(yptr);

    DrawSpline(&MPEGPlayerBase->mp_Spliner_Context,
               FIX(MPEGPlayerBase->mp_Spliner_Context.OldX),
               FIX(MPEGPlayerBase->mp_Spliner_Context.OldY),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIXH(xptr+2), FIXH(yptr+2));
}


/*****************************************************************************/


static VOID Draw_SF(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    MPEGPlayerBase->mp_Spliner_Context.OldX = HALF(xptr);
    MPEGPlayerBase->mp_Spliner_Context.OldY = HALF(yptr);

    DrawSpline(&MPEGPlayerBase->mp_Spliner_Context,
               FIX(MPEGPlayerBase->mp_Spliner_Context.OldX),
               FIX(MPEGPlayerBase->mp_Spliner_Context.OldY),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIX(xptr[3]), FIX(yptr[3]));
}


/*****************************************************************************/


static VOID DrawS_F(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    MPEGPlayerBase->mp_Spliner_Context.OldX = *xptr;
    MPEGPlayerBase->mp_Spliner_Context.OldY = *yptr;

    DrawSpline(&MPEGPlayerBase->mp_Spliner_Context,
               FIX(*xptr),   FIX(*yptr),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIXH(xptr+2), FIXH(yptr+2));
}


/*****************************************************************************/


static VOID DrawSF(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    MPEGPlayerBase->mp_Spliner_Context.OldX = *xptr;
    MPEGPlayerBase->mp_Spliner_Context.OldY = *yptr;

    DrawSpline(&MPEGPlayerBase->mp_Spliner_Context,
               FIX(*xptr),   FIX(*yptr),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIX(xptr[3]), FIX(yptr[3]));
}


/*****************************************************************************/


static VOID Draw_LF(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    Move(MPEGPlayerBase->mp_Spliner_Context.RastPort,HALF(xptr),HALF(yptr));
    xptr++;
    yptr++;
    Draw(MPEGPlayerBase->mp_Spliner_Context.RastPort,*xptr,*yptr);
}


/*****************************************************************************/


static VOID DrawL_F(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    Move(MPEGPlayerBase->mp_Spliner_Context.RastPort, *xptr, *yptr);
    Draw(MPEGPlayerBase->mp_Spliner_Context.RastPort, HALF(xptr),HALF(yptr));
}


/*****************************************************************************/


static VOID DrawLF(struct MPEGPlayerLib *MPEGPlayerBase, WORD *xptr, WORD *yptr)
{
    Move(MPEGPlayerBase->mp_Spliner_Context.RastPort,*xptr,*yptr);
    xptr++;
    yptr++;
    Draw(MPEGPlayerBase->mp_Spliner_Context.RastPort,*xptr,*yptr);
}


/*****************************************************************************/


static VOID DrawnLF(struct MPEGPlayerLib *MPEGPlayerBase, WORD *dummya, WORD *dummyb)
{
}


/*****************************************************************************/


static VOID (*funcs[])(struct MPEGPlayerLib *MPEGPlayerBase, WORD *, WORD *) =
{
    DrawSF,
    DrawS_F,
    Draw_SF,
    Draw_S_F,
    DrawLF,
    DrawL_F,
    Draw_LF,
    NULL,
    DrawnLF
};


/*****************************************************************************/


static VOID DrawFunc(struct MPEGPlayerLib *MPEGPlayerBase, struct Box *bptr)
{
LONG i;
WORD *x, *y;
char *p;

    switch (MPEGPlayerBase->mp_Spliner_closed)
    {
        case 2: for (i=0, x=&(bptr->x[0]),  y=&(bptr->y[0]); i<MPEGPlayerBase->mp_Spliner_maxPoints;   i++, x++, y++)
                {
                    x[MPEGPlayerBase->mp_Spliner_maxPoints] = MPEGPlayerBase->mp_Spliner_screenWidth - 1 - *x;
                    y[MPEGPlayerBase->mp_Spliner_maxPoints] = MPEGPlayerBase->mp_Spliner_screenHeight -  1 - *y;
                }
setup:
                x[MPEGPlayerBase->mp_Spliner_maxPoints] = bptr->x[0];
                y[MPEGPlayerBase->mp_Spliner_maxPoints] = bptr->y[0];
                x++;
                y++;
                x[MPEGPlayerBase->mp_Spliner_maxPoints] = bptr->x[1];
                y[MPEGPlayerBase->mp_Spliner_maxPoints] = bptr->y[1];
                break;

        case 1: x = &(bptr->x[0]);
                y = &(bptr->y[0]);
                goto setup;
    }

    p = MPEGPlayerBase->mp_Spliner_realfunc;
    x = &(bptr->x[0]);
    y = &(bptr->y[0]);
    while (*p < 10)
    {
        (funcs[*p])(MPEGPlayerBase, x,  y);
        i = advval[*p];
        x += i;
        y += i;
        p++;
    }
}


/*****************************************************************************/


static VOID Adv(struct MPEGPlayerLib *MPEGPlayerBase, WORD *o, WORD *d, WORD *n, WORD w)
{
    *n = *o + *d;
    if (*n < 0)
    {
        *n = 0;
        *d = Random(6,&MPEGPlayerBase->mp_Spliner_seed) + 1;
    }
    else if (*n >= w)
    {
        *n = w - 1;
        *d = - Random(6,&MPEGPlayerBase->mp_Spliner_seed) - 1;
    }
}


/*****************************************************************************/


static VOID AdvanceLines(struct MPEGPlayerLib *MPEGPlayerBase)
{
WORD i;

    for (i=0; i < MPEGPlayerBase->mp_Spliner_maxPoints; i++)
    {
        Adv(MPEGPlayerBase, MPEGPlayerBase->mp_Spliner_ox+i, MPEGPlayerBase->mp_Spliner_dx+i, MPEGPlayerBase->mp_Spliner_nx+i, MPEGPlayerBase->mp_Spliner_screenWidth);
        Adv(MPEGPlayerBase, MPEGPlayerBase->mp_Spliner_oy+i, MPEGPlayerBase->mp_Spliner_dy+i, MPEGPlayerBase->mp_Spliner_ny+i, MPEGPlayerBase->mp_Spliner_screenHeight);
    }
}


/*****************************************************************************/


static VOID DrawNew(struct MPEGPlayerLib *MPEGPlayerBase)
{
WORD       i;
struct Box *bptr;

    while (MPEGPlayerBase->mp_Spliner_numlines >= MPEGPlayerBase->mp_Spliner_maxlines)
    {
        SetAPen(MPEGPlayerBase->mp_Spliner_Context.RastPort,0);
        bptr = MPEGPlayerBase->mp_Spliner_eptr;
        DrawFunc(MPEGPlayerBase,bptr);

        SetAPen(MPEGPlayerBase->mp_Spliner_Context.RastPort,1);
        MPEGPlayerBase->mp_Spliner_numlines--;
        bptr++;

        if (bptr == MPEGPlayerBase->mp_Spliner_store + MAXLINES)
            bptr = MPEGPlayerBase->mp_Spliner_store;
        MPEGPlayerBase->mp_Spliner_eptr = bptr;
    }

    bptr = MPEGPlayerBase->mp_Spliner_ptr;
    for (i=0; i<MPEGPlayerBase->mp_Spliner_maxPoints; i++)
    {
        bptr->x[i] = MPEGPlayerBase->mp_Spliner_ox[i] = MPEGPlayerBase->mp_Spliner_nx[i];
        bptr->y[i] = MPEGPlayerBase->mp_Spliner_oy[i] = MPEGPlayerBase->mp_Spliner_ny[i];
    }

    DrawFunc(MPEGPlayerBase,bptr);
    MPEGPlayerBase->mp_Spliner_numlines++;
    bptr++;

    if (bptr == MPEGPlayerBase->mp_Spliner_store + MAXLINES)
    {
        bptr = MPEGPlayerBase->mp_Spliner_store;
        if  (MPEGPlayerBase->mp_Spliner_mdelta == 1)
        {
            MPEGPlayerBase->mp_Spliner_maxlines++;
            if (MPEGPlayerBase->mp_Spliner_maxlines >= MAXLINES - 1)
                MPEGPlayerBase->mp_Spliner_mdelta = -1;
        }
        else
        {
            MPEGPlayerBase->mp_Spliner_maxlines--;
            if (MPEGPlayerBase->mp_Spliner_maxlines <= 2)
                MPEGPlayerBase->mp_Spliner_mdelta = 1;
        }
    }

    MPEGPlayerBase->mp_Spliner_ptr = bptr;
}


/*****************************************************************************/


static VOID StartLines(struct MPEGPlayerLib *MPEGPlayerBase)
{
WORD i;

    MPEGPlayerBase->mp_Spliner_ptr      = MPEGPlayerBase->mp_Spliner_store;
    MPEGPlayerBase->mp_Spliner_eptr     = MPEGPlayerBase->mp_Spliner_store;
    MPEGPlayerBase->mp_Spliner_numlines = 0;

    if (MPEGPlayerBase->mp_Spliner_dx[0] == 0)
    {
        for (i = 0; i < MAXPOINTS; i++)
        {
            MPEGPlayerBase->mp_Spliner_ox[i] = Random(MPEGPlayerBase->mp_Spliner_screenWidth,&MPEGPlayerBase->mp_Spliner_seed);
            MPEGPlayerBase->mp_Spliner_oy[i] = Random(MPEGPlayerBase->mp_Spliner_screenHeight,&MPEGPlayerBase->mp_Spliner_seed);
            MPEGPlayerBase->mp_Spliner_dx[i] = 2 + Random(3,&MPEGPlayerBase->mp_Spliner_seed);
            MPEGPlayerBase->mp_Spliner_dy[i] = 2 + Random(3,&MPEGPlayerBase->mp_Spliner_seed);
        }
    }

    MPEGPlayerBase->mp_Spliner_nr = 53;
    MPEGPlayerBase->mp_Spliner_ng = 33;
    MPEGPlayerBase->mp_Spliner_nb = 35;
    MPEGPlayerBase->mp_Spliner_dr = -3;
    MPEGPlayerBase->mp_Spliner_dg = 5;
    MPEGPlayerBase->mp_Spliner_db = 7;

    SetRGB32(&MPEGPlayerBase->mp_PlayScreen->ViewPort,9,
             MPEGPlayerBase->mp_Spliner_nr << 24,
             MPEGPlayerBase->mp_Spliner_ng << 24,
             MPEGPlayerBase->mp_Spliner_nb << 24);

    for (i=0; i < MPEGPlayerBase->mp_Spliner_maxlines; i++)
    {
        AdvanceLines(MPEGPlayerBase);
        DrawNew(MPEGPlayerBase);
    }
}


/*****************************************************************************/


static VOID Colors(struct MPEGPlayerLib *MPEGPlayerBase)
{
WORD or, og, ob;

    or = MPEGPlayerBase->mp_Spliner_nr;
    og = MPEGPlayerBase->mp_Spliner_ng;
    ob = MPEGPlayerBase->mp_Spliner_nb;
    Adv(MPEGPlayerBase, &or, &MPEGPlayerBase->mp_Spliner_dr, &MPEGPlayerBase->mp_Spliner_nr, 128);
    Adv(MPEGPlayerBase, &og, &MPEGPlayerBase->mp_Spliner_dg, &MPEGPlayerBase->mp_Spliner_ng, 128);
    Adv(MPEGPlayerBase, &ob, &MPEGPlayerBase->mp_Spliner_db, &MPEGPlayerBase->mp_Spliner_nb, 128);

    SetRGB32(&MPEGPlayerBase->mp_PlayScreen->ViewPort,9,
             MPEGPlayerBase->mp_Spliner_nr << 24,
             MPEGPlayerBase->mp_Spliner_ng << 24,
             MPEGPlayerBase->mp_Spliner_nb << 24);
}


/*****************************************************************************/


static void CreateDPF(struct MPEGPlayerLib *MPEGPlayerBase)
{
    MPEGPlayerBase->mp_DPFBitMap = AllocBitMap(MPEGPlayerBase->mp_PlayScreen->Width,
    					       MPEGPlayerBase->mp_PlayScreen->Height,
    					       1,BMF_CLEAR|BMF_INTERLEAVED,NULL);

    InitRastPort(&MPEGPlayerBase->mp_DPFRastPort);
    MPEGPlayerBase->mp_DPFRastPort.BitMap = MPEGPlayerBase->mp_DPFBitMap;
    MPEGPlayerBase->mp_DPFRasInfo.BitMap  = MPEGPlayerBase->mp_DPFBitMap;

    Forbid();
    MPEGPlayerBase->mp_PlayScreen->ViewPort.RasInfo->Next  = &MPEGPlayerBase->mp_DPFRasInfo;
    MPEGPlayerBase->mp_PlayScreen->ViewPort.Modes         |= DUALPF;
    Permit();

    MakeScreen(MPEGPlayerBase->mp_PlayScreen);

    if (ViewAddress() == GfxBase->ActiView)
        RethinkDisplay();
}


/*****************************************************************************/


static void DeleteDPF(struct MPEGPlayerLib *MPEGPlayerBase)
{
    Forbid();
    MPEGPlayerBase->mp_PlayScreen->ViewPort.RasInfo->Next = NULL;
    MPEGPlayerBase->mp_PlayScreen->ViewPort.Modes         &= ~DUALPF;
    Permit();

    MakeScreen(MPEGPlayerBase->mp_PlayScreen);

    if (ViewAddress() == GfxBase->ActiView)
        RethinkDisplay();

    WaitBlit();
    FreeBitMap(MPEGPlayerBase->mp_DPFBitMap);
    MPEGPlayerBase->mp_DPFBitMap = NULL;
}


/*****************************************************************************/


#undef SysBase

static void SplinerTask(void)
{
struct MPEGPlayerLib *MPEGPlayerBase;
struct ExecBase      *SysBase;
UWORD                 animTimeOut;

    SysBase        = (*((struct ExecBase **) 4));
    MPEGPlayerBase = SysBase->ThisTask->tc_UserData;

    CreateDPF(MPEGPlayerBase);

    MPEGPlayerBase->mp_Spliner_mdelta           = -1;
    MPEGPlayerBase->mp_Spliner_maxlines         = MAXLINES/2;
    MPEGPlayerBase->mp_Spliner_screenWidth      = MPEGPlayerBase->mp_PlayScreen->Width;
    MPEGPlayerBase->mp_Spliner_screenHeight     = MPEGPlayerBase->mp_PlayScreen->Height;
    MPEGPlayerBase->mp_Spliner_Context.CGfxBase = GfxBase;
    MPEGPlayerBase->mp_Spliner_Context.RastPort = &MPEGPlayerBase->mp_DPFRastPort;
    MPEGPlayerBase->mp_Spliner_Context.OldX     = 0;
    MPEGPlayerBase->mp_Spliner_Context.OldY     = 0;

    SetAPen(MPEGPlayerBase->mp_Spliner_Context.RastPort,1);

    animTimeOut = 0;
    while ((SetSignal(0,0) & SIGBREAKF_CTRL_C) == 0)
    {
        if (!animTimeOut)
        {
            animTimeOut = ANIMINTERVAL;
            MakeFunc(MPEGPlayerBase);
            SetRast(MPEGPlayerBase->mp_Spliner_Context.RastPort,0);
            StartLines(MPEGPlayerBase);
        }

        AdvanceLines(MPEGPlayerBase);
        DrawNew(MPEGPlayerBase);
        Colors(MPEGPlayerBase);

        animTimeOut--;
    }

    SetRast(MPEGPlayerBase->mp_Spliner_Context.RastPort,0);
    DeleteDPF(MPEGPlayerBase);

    Forbid();
    MPEGPlayerBase->mp_SplinerTask = NULL;
}

#define SysBase MPEGPlayerBase->mp_SysLib


/*****************************************************************************/


void StartSpliner(struct MPEGPlayerLib *MPEGPlayerBase)
{
    if (!MPEGPlayerBase->mp_SplinerTask)
    {
        MPEGPlayerBase->mp_SplinerTask = MakeTask(MPEGPlayerBase, "Spliner", -1,
                                                  SplinerTask, 4000);
    }
}


/*****************************************************************************/


void StopSpliner(struct MPEGPlayerLib *MPEGPlayerBase)
{
    if (MPEGPlayerBase->mp_SplinerTask)
    {
        Signal(MPEGPlayerBase->mp_SplinerTask,SIGBREAKF_CTRL_C);

        while (MPEGPlayerBase->mp_SplinerTask)
            WaitTOF();
    }
}
